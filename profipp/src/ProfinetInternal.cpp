#include "ProfinetInternal.h"
#include "ProfinetProperties.h"
#include "dapModule.h"
#include "networktools.h"

#include "pnet_api.h"

#include <memory>
#include <functional>
#include <chrono>
#include <thread>
//TODO: Only use when POSIX
// to set priority of threads to RT.
#include <pthread.h>
#include <filesystem>
#include <sstream>
#include <cstdarg>
#include <cstdio>


inline constexpr static uint32_t arepNull{UINT32_MAX};

namespace profinet
{
ProfinetInternal::ProfinetInternal() : 
   device(), alarmAllowed{true}, arep{arepNull}, initialized{false}, arepForReady(arepNull)
{

}

ProfinetInternal::~ProfinetInternal()
{

}
static inline std::string strPrintf (const char* format, ...)
{
    std::va_list args;
    std::string retval;

    va_start (args, format);
    retval.resize (vsnprintf (0, 0, format, args));
    vsnprintf (&retval[0], retval.size () + 1, format, args);
    va_end (args);

    return retval;
}
void ProfinetInternal::Log(LogLevel logLevel, const char* format, ...) noexcept
{
   if(!logFun)
      return;
   va_list args;
   std::string message;

   va_start (args, format);
   message.resize (vsnprintf (0, 0, format, args));
   vsnprintf (&message[0], message.size () + 1, format, args);
   va_end (args);
   logFun(logLevel, std::move(message));
}

bool ProfinetInternal::Initialize(const Profinet& configuration_, LoggerType logger)
{
   logFun = logger;
   configuration = configuration_;

   auto& deviceConfiguration{configuration.GetDevice()};
   auto& deviceProperties{deviceConfiguration.properties};
   device.Initialize(deviceConfiguration);

   auto& properties = configuration.GetProperties();
   pnetCfg = InitializePnetConfig();

   // Determine available network interfaces, and determine if configured interfaces are valid and what their IPs etc is.
   auto availableNetworkInterfaces = tools::GetNetworkInterfaces();

   mainNetworkInterface=properties.mainNetworkInterface;
   networkInterfaces = properties.networkInterfaces;
   tools::NetworkInterface mainInterfaceInfos;
   if (mainNetworkInterface.size()<=0)
   {
      Log(logError, "Configured main network interface name for device is empty.");
      return false;
   }
   else
   {
      auto search = availableNetworkInterfaces.find(mainNetworkInterface);
      if(search == availableNetworkInterfaces.end())
      {
         Log(logError,
            "Configured main network interface name for device, %s, is not available on local machine.",
            mainNetworkInterface.c_str());
         return false;
      }
      mainInterfaceInfos = search->second;
   }
   if (networkInterfaces.size() > PNET_MAX_PHYSICAL_PORTS)
   {
      Log(logError,
         "Device configured with %u interface names, but "
         "PNET_MAX_PHYSICAL_PORTS is only %u.",
         networkInterfaces.size(),
         PNET_MAX_PHYSICAL_PORTS);
      return false;
   }
   else if(networkInterfaces.size() > 0)
   {
      for(auto& name : networkInterfaces)
      {
         if (name.size()==0)
         {
            Log(logError, "One of the configured network interface names for the device is empty.");
            return false;
         }
         else if( availableNetworkInterfaces.find(name)== availableNetworkInterfaces.end())
         {
            Log(logError,
               "One of the configured network interface names of the device, %s, is not available on local machine.",
               name.c_str());
            return false;
         }
      }
      if(std::find(begin(networkInterfaces), end(networkInterfaces), mainNetworkInterface)==std::end(networkInterfaces))
      {
         Log(logError,
               "If list of configured network interface names for the device is non-empty, it must contain the main network interface name.");
            return false;
      }
   }
   else
   {
      networkInterfaces.push_back(mainNetworkInterface);
   }

   // Create network interface configuration (part of profinet config)
   pnet_if_cfg_t networkInterfaceConfig{};
   networkInterfaceConfig.main_netif_name = mainNetworkInterface.c_str();
   int i=0;
   for (auto& name : networkInterfaces)
   {
      networkInterfaceConfig.physical_ports[i].netif_name = name.c_str();
      networkInterfaceConfig.physical_ports[i].default_mau_type = deviceProperties.defaultMautype;
      i++;
   }
   // Read IP, netmask, gateway from operating system
   auto copyIP = [](uint32_t source, pnet_cfg_ip_addr_t& dest)
   {
      dest.a = ((source >> 24) & 0xFF);
      dest.b = ((source >> 16) & 0xFF);
      dest.c = ((source >> 8) & 0xFF);
      dest.d = (source & 0xFF);
   };
   copyIP(mainInterfaceInfos.ipRaw, networkInterfaceConfig.ip_cfg.ip_addr);
   copyIP(mainInterfaceInfos.gatewayRaw, networkInterfaceConfig.ip_cfg.ip_gateway);
   copyIP(mainInterfaceInfos.maskRaw, networkInterfaceConfig.ip_cfg.ip_mask);

   // Continue initializing the pnet config
   pnetCfg.if_cfg = std::move(networkInterfaceConfig);
   pnetCfg.num_physical_ports = networkInterfaces.size();

   /* Operating system specific settings */
   pnetCfg.pnal_cfg.snmp_thread.prio = properties.snmpThreadPriority;
   pnetCfg.pnal_cfg.snmp_thread.stack_size = properties.snmpThreadStacksize;
   pnetCfg.pnal_cfg.eth_recv_thread.prio = properties.ethThreadPriority;
   pnetCfg.pnal_cfg.eth_recv_thread.stack_size = properties.ethThreadStacksize;
   pnetCfg.pnal_cfg.bg_worker_thread.prio = properties.bgWorkerThreadPriority;
   pnetCfg.pnal_cfg.bg_worker_thread.stack_size = properties.bgWorkerThreadStacksize;

   std::filesystem::path filepath;
   if(properties.pathStorageDirectory.empty())
      filepath = std::filesystem::current_path();
   else
      filepath = properties.pathStorageDirectory;
   if(!std::filesystem::is_directory(filepath))
   {
      Log(logError, "The given persistent file storage directory does not exist or is not a directory: %s", properties.pathStorageDirectory.c_str() );
      return false;
   }
   std::string filepathStr{filepath};
   //TODO replace properties.pathStorageDirectory by filepathStr.c_str()
   strcpy (pnetCfg.file_directory, properties.pathStorageDirectory.c_str());
   Log(logInfo, "Persistent file storage directory set to: %s\n", pnetCfg.file_directory);

   /* Initialise stack */
   alarmAllowed = true;
   arep = arepNull;

   profinetStack = pnet_init(&pnetCfg);

   if (!profinetStack)
   {
      int errsv = errno; 
      Log(logError,"Failed to initialize P-Net: %s. Do you have enough Ethernet interface permission?", strerror(errsv));
      return false;
   }
   
   initialized = true;
   return true;

}

/**
 * Plug all DAP (sub)modules
 * Use existing callback functions to plug the (sub-)modules
 * @param app              InOut:   Application handle
 * @param number_of_ports  In:      Number of active ports
 */
bool ProfinetInternal::PlugDap(pnet_t* pnet, uint16_t number_of_ports)
{
   const pnet_data_cfg_t emptyDataCfg = {
      .data_dir = PNET_DIR_NO_IO,
      .insize = 0,
      .outsize = 0,
   };

   Log(logDebug, "Plugging DAP module.");
   auto api = configuration.GetDevice().properties.api;


   CallbackExpModuleInd(pnet, api, dap::slot, dap::moduleId);
   CallbackExpSubmoduleInd(pnet, api, dap::slot, dap::subslotIdentity, dap::moduleId, dap::submoduleIdIdentity, &emptyDataCfg);
   CallbackExpSubmoduleInd(pnet, api, dap::slot, dap::subslotInterface1, dap::moduleId, dap::submoduleIdInterface1, &emptyDataCfg);
   CallbackExpSubmoduleInd(pnet, api, dap::slot, dap::subslotInterface1Port1, dap::moduleId,dap:: submoduleIdInterface1Port1, &emptyDataCfg);
   if (number_of_ports >= 2)
      CallbackExpSubmoduleInd(pnet, api, dap::slot, dap::subslotInterface1Port2, dap::moduleId, dap::submoduleIdInterface1Port2, &emptyDataCfg);
   if (number_of_ports >= 3)
      CallbackExpSubmoduleInd(pnet, api, dap::slot, dap::subslotInterface1Port3, dap::moduleId, dap::submoduleIdInterface1Port3, &emptyDataCfg);
   if (number_of_ports >= 4)
      CallbackExpSubmoduleInd(pnet, api, dap::slot, dap::subslotInterface1Port4, dap::moduleId, dap::submoduleIdInterface1Port4, &emptyDataCfg);

   Log(logDebug, "Done plugging DAP module.");
   return true;
}

void ProfinetInternal::SetLed(bool on)
{
   // TODO: Implement Callback
}

/**
 * @brief Prints IO provider or consumer status. Status is only printed if it is not the same as last time,
 * i.e. if iocs_current != iocs_new.
 * 
 * @param slot 
 * @param subslot 
 * @param ioxs_str Custom string, identifying if producer or consumer status is printed.
 * @param iocs_current Last IOPS/IOCS
 * @param iocs_new New IOPS/IOCS
 */
static std::string PrintIoxsChange(
   uint32_t slot,
   uint32_t subslot,
   const char * ioxs_str,
   uint8_t ioxs_new)
{
   if (ioxs_new == PNET_IOXS_BAD)
   {
      return strPrintf(
         "PLC reports %s BAD for slot %u subslot %u \n",
         ioxs_str,
         slot,
         subslot);
   }
   else if (ioxs_new == PNET_IOXS_GOOD)
   {
      return strPrintf(
         "PLC reports %s GOOD for slot %u subslot %u \n",
         ioxs_str,
         slot,
         subslot);
   }
   else
   {
      return strPrintf(
         "PLC reports %s %u for input slot %u subslot %u.\n"
         "  Is the PLC in STOP mode?\n",
         ioxs_str,
         ioxs_new,
         slot,
         subslot);
   }
}

void ProfinetInternal::HandleCyclicData ()
{
   static std::vector<uint8_t> buffer{};
   auto api{configuration.GetDevice().properties.api};
   for (auto itModules = device.begin(); itModules != device.end(); itModules++)
   {
      uint32_t slot{itModules->first};
      ModuleInstance& module{itModules->second};
      for (auto itSubmodules = module.begin(); itSubmodules != module.end(); itSubmodules++)
      {
         uint32_t subslot{itSubmodules->first};
         SubmoduleInstance& submodule{itSubmodules->second};
         std::size_t inputLength = submodule.GetInputLengthInBytes();
         std::size_t outputLength = submodule.GetOutputLengthInBytes();
         
         if (inputLength > 0)
         {
            if(buffer.capacity() < inputLength)
               buffer.reserve(inputLength);
            /* Get data from the PLC */
            bool indata_updated;
            uint8_t indata_iops;
            uint16_t inputLengthTmp = static_cast<uint16_t>(inputLength);
            int ret = pnet_output_get_data_and_iops (
               profinetStack,
               api,
               slot,
               subslot,
               &indata_updated,
               buffer.data(),
               &inputLengthTmp,
               &indata_iops);

            if(ret != 0)
            {
               Log(logDebug,
                  "Error getting input data for slot %u subslot %u. Setting inputs to defaults...",
                  slot,
                  subslot);
               submodule.SetLastInputIops(PNET_IOXS_BAD);
               submodule.SetDefaultInput();
            }
            else
            {
               if (submodule.GetLastInputIops() != indata_iops)
               {
                  Log(logDebug, PrintIoxsChange (
                     slot,
                     subslot,
                     "Provider Status (IOPS)",
                     indata_iops).c_str());
                  submodule.SetLastInputIops(indata_iops);
               }
               if (inputLength != inputLengthTmp)
               {
                  Log(logError, "Wrong input data length for slot %u subslot %u: received %u, expected %u. Setting inputs to defaults...",slot, subslot, inputLengthTmp, inputLength);
                  submodule.SetDefaultInput();
               }
               else if (indata_iops == PNET_IOXS_GOOD)
               {
                  if(!submodule.SetInput(buffer.data(), inputLength))
                  {
                     Log(logError, "Error setting received input of submodule in slot %u, subslot %u. Setting inputs to defaults...",slot, subslot);
                     submodule.SetDefaultInput();
                  }
               }
               else
               {
                  submodule.SetDefaultInput();
               }
            }
         }

         if (outputLength>0)
         {
            if(buffer.capacity() < outputLength)
               buffer.reserve(outputLength);
            std::size_t writtenOutputLength{outputLength};
            
            
            /* Send input data to the PLC */
            int ret;
            if(submodule.GetOutput(buffer.data(), &writtenOutputLength))
            {
               ret = pnet_input_set_data_and_iops (
                  profinetStack,
                  api,
                  slot,
                  subslot,
                  buffer.data(),
                  static_cast<uint16_t>(writtenOutputLength),
                  PNET_IOXS_GOOD);
            }
            else
            {
               Log(logError, "Failed to get output for submodule in slot %u subslot %u. Sending producer state BAD to controller. Is there something wrong with the application logic?",
                  slot,
                  subslot);
               ret = pnet_input_set_data_and_iops (
                  profinetStack,
                  api,
                  slot,
                  subslot,
                  NULL,
                  static_cast<uint16_t>(0),
                  PNET_IOXS_BAD);
            }
            // TODO: Do something if ret = -1?

            uint8_t outdata_iocs;
            ret = pnet_input_get_iocs (
               profinetStack,
               api,
               slot,
               subslot,
               &outdata_iocs);
            if(ret == 0)
            {
               if (submodule.GetLastOutputIocs() != outdata_iocs)
               {
                  Log(logDebug, PrintIoxsChange(
                     slot,
                     subslot,
                     "Consumer Status (IOCS)",
                     outdata_iocs).c_str());
                  submodule.SetLastOutputIocs(outdata_iocs);
               }
            }
            else
            {
               if(submodule.GetLastOutputIocs() != PNET_IOXS_BAD)
               {
                  Log(logWarning, "Could not get consumer status of controller for output for slot %u subslot %u. Assuming IOCS BAD.",
                     slot,
                     subslot);
                  submodule.SetLastOutputIocs(PNET_IOXS_BAD);
               }   
            }
         }
      }
   }
}
inline bool ProfinetInternal::IsConnectedToController()
{
   return arep != arepNull;
}
/**
 * Send application ready to the PLC
 * @param net              InOut: p-net stack instance
 * @param arep             In:    Arep
 */
inline bool ProfinetInternal::SendApplicationReady(uint32_t arep)
{
   int ret = -1;

   Log(logDebug,
      "Application will signal that it is ready for data, for AREP %u.",
      arep);

   ret = pnet_application_ready (profinetStack, arep);
   if (ret != 0)
   {
      Log(logError,
         "Error returned when application telling that it is ready for "
         "data. Have you set IOCS or IOPS for all subslots?");
         return false;
   }
   else
      return true;

   /* When the PLC sends a confirmation to this message, the
      pnet_ccontrol_cnf() callback will be triggered.  */
}

/**
 * Send alarm ACK to the PLC
 *
 * @param net              InOut: p-net stack instance
 * @param arep             In:    Arep
 * @param p_alarm_arg      In:    Alarm argument (slot, subslot etc)
 */
inline bool ProfinetInternal::HandleSendAlarmAck()
{
   pnet_pnio_status_t pnio_status = {0, 0, 0, 0};
   int ret;

   ret = pnet_alarm_send_ack (profinetStack, arep, &lastAlarmArguments, &pnio_status);
   if (ret != 0)
   {
      Log(logError, "Error when sending alarm ACK. Error: %d.", ret);
      return false;
   }
   else
   {
      Log(logDebug, "Alarm ACK sent.");
      return true;
   }
}

void ProfinetInternal::loop()
{
   arep = arepNull;

   SetLed(false);
   PlugDap(profinetStack, networkInterfaces.size());
   Log(logInfo, "Waiting for PLC connect request...");

   // Main event loop
   while(true)
   {
      synchronizationEvents.ReceiveEvents();
      if(synchronizationEvents.ProcessReadyForData())
      {
         SendApplicationReady(arepForReady);
      }
      else if(synchronizationEvents.ProcessAlarm())
      {
         HandleSendAlarmAck();
      }
      else if(synchronizationEvents.ProcessCycle())
      {
         if(IsConnectedToController())
         {
            HandleCyclicData();
         }

         // Run p-net stack 
         pnet_handle_periodic(profinetStack);
      }
      else if (synchronizationEvents.ProcessAbort())
      {
         arep = arepNull;
         alarmAllowed = true;
         Log(logInfo, "Connection closed.");
         Log(logInfo, "Waiting for PLC connect request...");
      }
   }
}

bool ProfinetInternal::Start()
{
   if (!initialized)
   {
      Log(logError, "Profinet has to be initialized before Start() can be called.");
      return false;
   }
   Log(logInfo, "Starting profinet interface...");

  // Create timer, which regularly schedules cyclic data processing
  // TODO: Ever stop this timer?
  std::thread timer([this]()
   {
      auto lastTime{std::chrono::steady_clock::now()};
      const auto period{std::chrono::microseconds{configuration.GetProperties().cycleTimeUs}};

      while(true)
      {
         this->synchronizationEvents.SignalCycle();
         lastTime += period;
         std::this_thread::sleep_until(lastTime);
      }
   });
   sched_param timerScheduleParameters;
   timerScheduleParameters.sched_priority = configuration.GetProperties().cycleTimerPriority;
   if(pthread_setschedparam(timer.native_handle(), SCHED_FIFO, &timerScheduleParameters))
   {
      Log(logWarning, "Could not set scheduling policy and priority for timer thread which schedules cyclic processing of profinet data.");
   }
   timer.detach();

  // Create thread which is responsible for cyclic data processing, alarms etc.
  // TODO: Ever stop this thread?
   std::thread mainThread(std::bind(&ProfinetInternal::loop, this));
   sched_param mainThreadScheduleParameters;
   mainThreadScheduleParameters.sched_priority = configuration.GetProperties().cycleTimerPriority;
   if(pthread_setschedparam(mainThread.native_handle(), SCHED_FIFO, &mainThreadScheduleParameters))
   {
      Log(logWarning, "Could not set scheduling policy and priority for main thread which processes profinet cyclic data, alarms etc.");
   }
   mainThread.detach();

   return true;
}


template<auto F, typename Result, typename... Args>
   Result wrapFunction(pnet_t* pnet, void* state, Args...args)
{
	ProfinetInternal* connection = static_cast<ProfinetInternal*>(state);
	return std::invoke(F, *connection, pnet, args...);
}

pnet_cfg_t ProfinetInternal::InitializePnetConfig()
{
   pnet_cfg_t pnet_cfg{};
   memset (&pnet_cfg, 0, sizeof (pnet_cfg_t));

   auto highByte = [](const uint16_t value)-> uint8_t{return static_cast<uint8_t>((value >> 8) & 0xFF);};
   auto lowByte = [](const uint16_t value)-> uint8_t{return static_cast<uint8_t>(value & 0xFF);};

   pnet_cfg.tick_us = configuration.GetProperties().cycleTimeUs;

   auto& props{configuration.GetDevice().properties};
   /* Identification & Maintenance */
   pnet_cfg.im_0_data.im_vendor_id_hi = highByte(props.vendorID);
   pnet_cfg.im_0_data.im_vendor_id_lo = lowByte (props.vendorID);

   pnet_cfg.im_0_data.im_hardware_revision = props.imHardwareRevision;
   pnet_cfg.im_0_data.im_sw_revision_prefix = props.swRevPrefix;
   pnet_cfg.im_0_data.im_sw_revision_functional_enhancement = props.swRevMajor;
   pnet_cfg.im_0_data.im_sw_revision_bug_fix = props.swRevMinor;
   pnet_cfg.im_0_data.im_sw_revision_internal_change = props.swRevPatch;
   pnet_cfg.im_0_data.im_revision_counter = props.imRevisionCounter;
   pnet_cfg.im_0_data.im_profile_id = props.profileID;
   pnet_cfg.im_0_data.im_profile_specific_type = props.profileSpecType;
   pnet_cfg.im_0_data.im_version_major = 1; /** Always 1 */
   pnet_cfg.im_0_data.im_version_minor = 1; /** Always 1 */
   pnet_cfg.im_0_data.im_supported = props.supportedIMs;

   snprintf (
      pnet_cfg.im_0_data.im_order_id,
      sizeof (pnet_cfg.im_0_data.im_order_id),
      "%s",
      props.orderID.c_str());
   snprintf (
      pnet_cfg.im_0_data.im_serial_number,
      sizeof (pnet_cfg.im_0_data.im_serial_number),
      "%s",
      props.serialNumber.c_str());
   snprintf (
      pnet_cfg.im_1_data.im_tag_function,
      sizeof (pnet_cfg.im_1_data.im_tag_function),
      "%s",
      props.tagFunction.c_str());
   snprintf (
      pnet_cfg.im_1_data.im_tag_location,
      sizeof (pnet_cfg.im_1_data.im_tag_location),
      "%s",
      props.tagLocation.c_str());
   snprintf (
      pnet_cfg.im_2_data.im_date,
      sizeof (pnet_cfg.im_2_data.im_date),
      "%s",
      props.imDate.c_str());
   snprintf (
      pnet_cfg.im_3_data.im_descriptor,
      sizeof (pnet_cfg.im_3_data.im_descriptor),
      "%s",
      props.descriptor.c_str());
   snprintf (
      pnet_cfg.im_4_data.im_signature,
      sizeof (pnet_cfg.im_4_data.im_signature),
      "%s",
      props.signature.c_str());

   /* Device configuration */
   pnet_cfg.device_id.vendor_id_hi = highByte (props.vendorID);
   pnet_cfg.device_id.vendor_id_lo = lowByte (props.vendorID);
   pnet_cfg.device_id.device_id_hi = highByte (props.deviceID);
   pnet_cfg.device_id.device_id_lo = lowByte (props.deviceID);
   pnet_cfg.oem_device_id.vendor_id_hi = highByte (props.oemVendorID);
   pnet_cfg.oem_device_id.vendor_id_lo = lowByte (props.oemVendorID);
   pnet_cfg.oem_device_id.device_id_hi = highByte (props.oemDeviceID);
   pnet_cfg.oem_device_id.device_id_lo = lowByte (props.oemVendorID);

   snprintf (
      pnet_cfg.product_name,
      sizeof (pnet_cfg.product_name),
      "%s",
      props.productName.c_str());

   pnet_cfg.send_hello = true;

   /* Timing */
   pnet_cfg.min_device_interval = props.minDeviceInterval;

   /* Will be overwritten as part of network configuration. */
   pnet_cfg.num_physical_ports = 1;

   snprintf (
      pnet_cfg.station_name,
      sizeof (pnet_cfg.station_name),
      "%s",
      props.stationName.c_str());

   /* Diagnosis mechanism */
   /* We prefer using "Extended channel diagnosis" instead of
    * "Qualified channel diagnosis" format on the wire,
    * as this is better supported by Wireshark.
    */
   pnet_cfg.use_qualified_diagnosis = false;

   // Initialize callbacks
   pnet_cfg.state_cb = wrapFunction<&ProfinetInternal::CallbackStateInd>;
   pnet_cfg.connect_cb = wrapFunction<&ProfinetInternal::CallbackConnectInd>;
   pnet_cfg.release_cb = wrapFunction<&ProfinetInternal::CallbackReleaseInd>;
   pnet_cfg.dcontrol_cb = wrapFunction<&ProfinetInternal::CallbackDControlInd>;
   pnet_cfg.ccontrol_cb = wrapFunction<&ProfinetInternal::CallbackCControlCnf>;
   pnet_cfg.read_cb = wrapFunction<&ProfinetInternal::CallbackReadInd>;
   pnet_cfg.write_cb = wrapFunction<&ProfinetInternal::CallbackWriteInd>;
   pnet_cfg.exp_module_cb = wrapFunction<&ProfinetInternal::CallbackExpModuleInd>;
   pnet_cfg.exp_submodule_cb = wrapFunction<&ProfinetInternal::CallbackExpSubmoduleInd>;
   pnet_cfg.new_data_status_cb = wrapFunction<&ProfinetInternal::CallbackNewDataStatusInd>;
   pnet_cfg.alarm_ind_cb = wrapFunction<&ProfinetInternal::CallbackAlarmInd>;
   pnet_cfg.alarm_cnf_cb = wrapFunction<&ProfinetInternal::CallbackAlarmCnf>;
   pnet_cfg.alarm_ack_cnf_cb = wrapFunction<&ProfinetInternal::CallbackAlarmAckCnf>;
   pnet_cfg.reset_cb = wrapFunction<&ProfinetInternal::CallbackResetInd>;
   pnet_cfg.signal_led_cb = wrapFunction<&ProfinetInternal::CallbackSignalLedInd>;

   pnet_cfg.cb_arg = (void *)this;

   return pnet_cfg;
}

/*********************************** Callbacks ********************************/

int ProfinetInternal::CallbackConnectInd (
   pnet_t * net,
   uint32_t arep,
   pnet_result_t * p_result)
{
   Log(logInfo, "PLC connected to device (AREP: %u). Establishing communication...", arep);
   /*
    * TODO:
    *  Handle the request on an application level.
    *  This is a very simple application which does not need to handle anything.
    *  All the needed information is in the AR data structure.
    */

   return 0;
}

int ProfinetInternal::CallbackReleaseInd (
   pnet_t * net,
   uint32_t arep,
   pnet_result_t * p_result)
{
   Log(logInfo, "PLC disconnected from device (AREP: %u).", arep);

   device.SetDefaultInputsAll();

   // TODO: Should device, modules etc be removed?

   return 0;
}


const char * dcontrol_cmd_to_string (
   pnet_control_command_t control_command)
{
   const char * s = NULL;

   switch (control_command)
   {
   case PNET_CONTROL_COMMAND_PRM_BEGIN:
      s = "PRM_BEGIN";
      break;
   case PNET_CONTROL_COMMAND_PRM_END:
      s = "PRM_END";
      break;
   case PNET_CONTROL_COMMAND_APP_RDY:
      s = "APP_RDY";
      break;
   case PNET_CONTROL_COMMAND_RELEASE:
      s = "RELEASE";
      break;
   default:
      s = "<error>";
      break;
   }

   return s;
}

int ProfinetInternal::CallbackDControlInd (
   pnet_t * net,
   uint32_t arep,
   pnet_control_command_t control_command,
   pnet_result_t * p_result)
{
   Log(logDebug,
      "The PLC is done with parameter writing (AREP: %u  Command: %s).",
      arep,
      dcontrol_cmd_to_string (control_command));
   return 0;
}

int ProfinetInternal::CallbackCControlCnf (
   pnet_t * net,
   uint32_t arep,
   pnet_result_t * p_result)
{
   Log(logDebug,
      "The PLC has received Application Ready message (AREP: %u  Status codes: %d %d %d %d).",
      arep,
      p_result->pnio_status.error_code,
      p_result->pnio_status.error_decode,
      p_result->pnio_status.error_code_1,
      p_result->pnio_status.error_code_2);

   return 0;
}

int ProfinetInternal::CallbackWriteInd (
   pnet_t * net,
   uint32_t arep,
   uint32_t api,
   uint16_t slot,
   uint16_t subslot,
   uint16_t idx,
   uint16_t sequence_number,
   uint16_t write_length,
   const uint8_t * p_write_data,
   pnet_result_t * p_result)
{
   Log(logDebug,
      "PLC writes value of parameter %u in slot %2u, subslot %2u (AREP: %u, API: %u, Sequence: %2u, Length: %u).",
      (unsigned)idx,
      slot,
      subslot,
      arep,
      api,
      sequence_number,
      write_length);

   ModuleInstance* moduleInstance = device.GetModule(slot);
   if(!moduleInstance)
   {
      Log(logWarning,
         "PLC could not write value of parameter %u in slot %2u, subslot %2u: no module plugged in slot %2u  (AREP: %u, API: %u).",
         (unsigned)idx,
         slot,
         subslot,
         slot,
         arep,
         api);
      p_result->pnio_status.error_code = PNET_ERROR_CODE_WRITE;
      p_result->pnio_status.error_decode = PNET_ERROR_DECODE_PNIORW;
      p_result->pnio_status.error_code_1 = PNET_ERROR_CODE_1_APP_WRITE_ERROR;
      p_result->pnio_status.error_code_2 = 0; // User specific

      return -1;
   }           
   
   SubmoduleInstance* submoduleInstance = moduleInstance->GetSubmodule(subslot);  
   if(!submoduleInstance)
   {
      Log(logWarning,
         "PLC could not write value of parameter %u in slot %2u, subslot %2u: no submodule plugged in subslot %2u of module (AREP: %u, API: %u).",
         (unsigned)idx,
         slot,
         subslot,
         subslot,
         arep,
         api);
      p_result->pnio_status.error_code = PNET_ERROR_CODE_WRITE;
      p_result->pnio_status.error_decode = PNET_ERROR_DECODE_PNIORW;
      p_result->pnio_status.error_code_1 = PNET_ERROR_CODE_1_APP_WRITE_ERROR;
      p_result->pnio_status.error_code_2 = 0; // User specific

      return -1;
   }           
   ParameterInstance* parameterInstance = submoduleInstance->GetParameter(idx);
   if(!submoduleInstance)
   {
      Log(logWarning,
         "PLC could not write value of parameter %u in slot %2u, subslot %2u: submodule does not have parameter with index %u  (AREP: %u, API: %u).",
         (unsigned)idx,
         slot,
         subslot,
         (unsigned)idx,
         arep,
         api);
      p_result->pnio_status.error_code = PNET_ERROR_CODE_WRITE;
      p_result->pnio_status.error_decode = PNET_ERROR_DECODE_PNIORW;
      p_result->pnio_status.error_code_1 = PNET_ERROR_CODE_1_APP_WRITE_ERROR;
      p_result->pnio_status.error_code_2 = 0; // User specific

      return -1;
   }         
   bool success = parameterInstance->Set(p_write_data, static_cast<std::size_t>(write_length));
   if (!success)
   {
      Log(logWarning,
         "PLC could not write value of parameter %u in slot %2u, subslot %2u: Write process itself failed. Maybe the data sent from the PLC is invalid, or the parameter is misconfigured (AREP: %u, API: %u).",
         (unsigned)idx,
         slot,
         subslot,
         arep,
         api);
      p_result->pnio_status.error_code = PNET_ERROR_CODE_WRITE;
      p_result->pnio_status.error_decode = PNET_ERROR_DECODE_PNIORW;
      p_result->pnio_status.error_code_1 = PNET_ERROR_CODE_1_APP_WRITE_ERROR;
      p_result->pnio_status.error_code_2 = 0; // User specific 
      return -1;
   }
   return 0;
}

int ProfinetInternal::CallbackReadInd (
   pnet_t * net,
   uint32_t arep,
   uint32_t api,
   uint16_t slot,
   uint16_t subslot,
   uint16_t idx,
   uint16_t sequence_number,
   uint8_t ** pp_read_data,
   uint16_t * p_read_length,
   pnet_result_t * p_result)
{
   Log(logDebug,
      "PLC reads value of parameter %u in slot %2u, subslot %2u (AREP: %u, API: %u, Sequence: %2u, Max Length: %u).",
      (unsigned)idx,
      slot,
      subslot,
      arep,
      api,
      sequence_number,
      (unsigned)*p_read_length);

   ModuleInstance* moduleInstance = device.GetModule(slot);
   if(!moduleInstance)
   {
      Log(logWarning,
         "PLC could not read value of parameter %u in slot %2u, subslot %2u: no module plugged in slot %2u  (AREP: %u, API: %u).",
         (unsigned)idx,
         slot,
         subslot,
         slot,
         arep,
         api);
      p_result->pnio_status.error_code = PNET_ERROR_CODE_READ;
      p_result->pnio_status.error_decode = PNET_ERROR_DECODE_PNIORW;
      p_result->pnio_status.error_code_1 = PNET_ERROR_CODE_1_APP_READ_ERROR;
      p_result->pnio_status.error_code_2 = 0; // User specific

      return -1;
   }           
   
   SubmoduleInstance* submoduleInstance = moduleInstance->GetSubmodule(subslot);  
   if(!submoduleInstance)
   {
        Log(logWarning,
         "PLC could not read value of parameter %u in slot %2u, subslot %2u: no submodule plugged in subslot %2u of module (AREP: %u, API: %u).",
         (unsigned)idx,
         slot,
         subslot,
         subslot,
         arep,
         api);
      p_result->pnio_status.error_code = PNET_ERROR_CODE_READ;
      p_result->pnio_status.error_decode = PNET_ERROR_DECODE_PNIORW;
      p_result->pnio_status.error_code_1 = PNET_ERROR_CODE_1_APP_READ_ERROR;
      p_result->pnio_status.error_code_2 = 0; // User specific

      return -1;
   }           
   ParameterInstance* parameterInstance = submoduleInstance->GetParameter(idx);
   if(!submoduleInstance)
   {
        Log(logWarning,
         "PLC could not read value of parameter %u in slot %2u, subslot %2u: submodule does not have parameter with index %u  (AREP: %u, API: %u).",
         (unsigned)idx,
         slot,
         subslot,
         (unsigned)idx,
         arep,
         api);
      p_result->pnio_status.error_code = PNET_ERROR_CODE_READ;
      p_result->pnio_status.error_decode = PNET_ERROR_DECODE_PNIORW;
      p_result->pnio_status.error_code_1 = PNET_ERROR_CODE_1_APP_READ_ERROR;
      p_result->pnio_status.error_code_2 = 0; // User specific

      return -1;
   }         
   size_t length = static_cast<std::size_t>(*p_read_length);
   bool success = parameterInstance->Get(pp_read_data, &length);
   if (!success)
   {
      Log(logWarning,
         "PLC could read value of parameter %u in slot %2u, subslot %2u: Read process itself failed. Is there a failure in the application logic, or is the parameter misconfigured (AREP: %u, API: %u).",
         (unsigned)idx,
         slot,
         subslot,
         arep,
         api);
     p_result->pnio_status.error_code = PNET_ERROR_CODE_READ;
      p_result->pnio_status.error_decode = PNET_ERROR_DECODE_PNIORW;
      p_result->pnio_status.error_code_1 = PNET_ERROR_CODE_1_APP_READ_ERROR;
      p_result->pnio_status.error_code_2 = 0; // User specific 
      return -1;
   }
   *p_read_length = static_cast<uint16_t>(length);
   return 0;
}


inline std::string ioxsToString (const pnet_ioxs_values_t& ioxs)
{
   switch (ioxs)
   {
   case PNET_IOXS_BAD:
      return "BAD";
   case PNET_IOXS_GOOD:
      return "GOOD";
   }

   return "unknown IOPS/IOCS";
}

/**
 * Set initial input data, provider and consumer status for a subslot.
 *
 * @param app              In:    Application handle
 */
bool ProfinetInternal::SetInitialDataAndIoxs()
{
   auto api{configuration.GetDevice().properties.api};
   std::vector<uint8_t> buffer{};
   for (auto itModules = device.begin(); itModules != device.end(); itModules++)
   {
      uint32_t slot{itModules->first};
      ModuleInstance& module{itModules->second};
      for (auto itSubmodules = module.begin(); itSubmodules != module.end(); itSubmodules++)
      {
         uint32_t subslot{itSubmodules->first};
         SubmoduleInstance& submodule{itSubmodules->second};
         std::size_t inputLength = submodule.GetInputLengthInBytes();
         std::size_t outputLength = submodule.GetOutputLengthInBytes();
         
         if (inputLength > 0)
         {
            if(buffer.capacity() < inputLength)
               buffer.reserve(inputLength);

            
            int ret = pnet_output_set_iocs (
               profinetStack,
               api,
               slot,
               subslot,
               PNET_IOXS_GOOD);

            if (ret == 0)
            {
               Log(logDebug,
                  "Successfully set initial input consumer state for slot %2u subslot %5u to %9s.",
                  slot,
                  subslot,
                  ioxsToString (PNET_IOXS_GOOD).c_str());
            }
            else
            {
               Log(logWarning,
                  "Failed to set initial input consumer state for slot %2u subslot %5u to %9s.",
                  slot,
                  subslot,
                  ioxsToString (PNET_IOXS_GOOD).c_str());
            }
         }
         else
         {
            Log(logDebug,
                  "No need to set initial input consumer state for slot %2u subslot %5u: submodule has no inputs.",
                  slot,
                  subslot);
         }
         if (outputLength>0 || slot == dap::slot)
         {
            uint16_t outputLengthTmp = static_cast<uint16_t>(outputLength);
            if(buffer.capacity() < outputLength)
               buffer.reserve(outputLength);
            uint8_t outdata_iops = PNET_IOXS_BAD;
            bool send{true};
            std::size_t writtenLength{0};
            if (slot == dap::slot)
               outdata_iops = PNET_IOXS_GOOD;
            else
            {
               writtenLength = outputLength;
               if(submodule.GetOutput(buffer.data(), &writtenLength))
                  outdata_iops = PNET_IOXS_GOOD;
               else
               {
                  outdata_iops = PNET_IOXS_BAD;
                  Log(logWarning,
                     "Error getting output from submodule in slot %2u subslot %2u. Possible error in application logic. Setting producer state to BAD.",
                     slot,
                     subslot);
               }
            }
            
            int ret = pnet_input_set_data_and_iops (
               profinetStack,
               api,
               slot,
               subslot,
               buffer.data(),
               writtenLength,
               outdata_iops);

            /*
               * If a submodule is still plugged but not used in current AR,
               * setting the data and IOPS will fail.
               * This is not a problem.
               * Log message below will only be printed for active submodules.
               */
            if (ret == 0)
            {
               Log(logDebug,
                  "Sucessfully set initial output data and producer state for slot %2u subslot %2u to %s.",
                  slot,
                  subslot,
                  ioxsToString(static_cast<pnet_ioxs_values_t>(outdata_iops)).c_str());
            }
            else
            {
               Log(logWarning,
                  "Could not set initial output data and producer state for slot %2u subslot %2u: error occured.",
                  slot,
                  subslot);
            }
         }
         else
         {
            Log(logDebug,
                  "No need to set initial output data and producer state for slot %2u subslot %5u: submodule has no outputs.",
                  slot,
                  subslot);
         }
      }
   }
   return true;
}

inline std::pair<std::string, std::string> ErrorCodesToStrings (
   uint16_t err_cls,
   uint16_t err_code)
{
   std::pair<std::string, std::string> result{};

   switch (err_cls)
   {
   case PNET_ERROR_CODE_1_RTA_ERR_CLS_PROTOCOL:
      result.first = "Real-Time Acyclic Protocol";
      switch (err_code)
      {
      case PNET_ERROR_CODE_2_ABORT_AR_CONSUMER_DHT_EXPIRED:
         result.second = "Device missed cyclic data "
                         "deadline, device terminated AR";
         break;
      case PNET_ERROR_CODE_2_ABORT_AR_CMI_TIMEOUT:
         result.second = "Communication initialization "
                         "timeout, device terminated AR";
         break;
      case PNET_ERROR_CODE_2_ABORT_AR_RELEASE_IND_RECEIVED:
         result.second = "AR release indication received";
         break;
      case PNET_ERROR_CODE_2_ABORT_DCP_STATION_NAME_CHANGED:
         result.second = "DCP station name changed, "
                         "device terminated AR";
         break;
      case PNET_ERROR_CODE_2_ABORT_DCP_RESET_TO_FACTORY:
         result.second = "DCP reset to factory or factory "
                         "reset, device terminated AR";
         break;
      default:
         result.second = "unknown error code";
      }
      break;

   case PNET_ERROR_CODE_1_CTLDINA:
      result.first = "CTLDINA = Name and IP assignment from controller";
      switch (err_code)
      {
      case PNET_ERROR_CODE_2_CTLDINA_ARP_MULTIPLE_IP_ADDRESSES:
         result.second = "Multiple users of same IP address";
         break;
      default:
         result.second = "unknown error code";
      }
      break;
   default:
      result.first = "unknown error class";
      result.second = "unknown error code";
   }
   return result;
}

int ProfinetInternal::CallbackStateInd (
   pnet_t * net,
   uint32_t arep,
   pnet_event_values_t event)
{
   if (event == PNET_EVENT_ABORT)
   {
      uint16_t errorClass{0};  // Error code 1
      uint16_t errorCode{0}; // Error code 2 
      if (pnet_get_ar_error_codes (net, arep, &errorClass, &errorCode) == 0)
      {
         auto errorDescription{ErrorCodesToStrings(errorClass, errorCode)};
         Log(logInfo,
            "PLC aborted connection. Error class: %s (0x%02x),  Error code: %s (0x%02x).",
            errorDescription.first.c_str(),
            (unsigned)errorClass,
            errorDescription.second.c_str(),
            (unsigned)errorCode);
      }
      else
      {
         Log(logInfo,
            "PLC aborted connection. No error status available.");
      }
      // Reset all inputs of all submodules. 
      device.SetDefaultInputsAll();

      // Only abort AR with correct session key
      synchronizationEvents.SignalAbort();
   }
   else if (event == PNET_EVENT_PRMEND)
   {
      if (IsConnectedToController())
      {
         Log(logWarning, "AREP out of sync. Trying to resynchronize connection.");
      }
      this->arep = arep;
      SetInitialDataAndIoxs();

      pnet_set_provider_state (net, true);

      // Send application ready at next tick. Do not call pnet_application_ready() here as it will affect  the internal stack states
      arepForReady = arep;
      synchronizationEvents.SignalReadyForData();
   }
   else if (event == PNET_EVENT_DATA)
   {
      Log(logInfo, "Cyclic data transmission started.");
   }
   return 0;
}

int ProfinetInternal::CallbackResetInd (
   pnet_t * net,
   bool should_reset_application,
   uint16_t reset_mode)
{
   Log(logInfo,
      "PLC reset indication. Application reset mandatory: %u  Reset mode: %d.",
      should_reset_application,
      reset_mode);

   return 0;
}

int ProfinetInternal::CallbackSignalLedInd (
   pnet_t * net, 
   bool led_state)
{
   Log(logDebug, "Profinet signal LED indication. New state: %s.", led_state? "On" : "Off");
   SetLed(led_state);
   return 0;
}

int ProfinetInternal::CallbackExpModuleInd (
   pnet_t * net,
   uint32_t api,
   uint16_t slot,
   uint32_t moduleId)
{
   Log(logDebug, "Pulling old module from slot %2u (API: %u)...", slot, api);
   int result = pnet_pull_module (net, api, slot);
   if (result == 0)
   {
        device.RemoveFromSlot(slot);
   }

   Log(logDebug,
      "Plugging module with ID 0x%x into slot %2u (API: %u)...",
     (unsigned)moduleId,
      slot,
      api);
   result = pnet_plug_module (net, api, slot, moduleId);
   if(result != 0)
   {
      Log(logError,
         "Failed to plug module with ID 0x%x into slot %2u (API: %u, Ret: %u).",
         (unsigned)moduleId,
         slot,
         api,
         result);
      return result;
   }

   auto& deviceConfig{configuration.GetDevice()};
   auto moduleConfig{deviceConfig.modules[moduleId]};
   auto moduleInstance{device.CreateInSlot(slot)};
   if(!moduleInstance)
   {
      Log(logError,
         "Failed to instanciate module with ID 0x%x to plug into slot %2u (API: %u).",
         (unsigned)moduleId,
         slot,
         api);
      return -1;
   }
   if (moduleConfig)
   {
      if(!moduleInstance->Initialize(moduleConfig->module, slot))
      {
          Log(logWarning,
            "Failed to initialize module with ID 0x%x to plug into slot %2u not found (API: %u). Initializing module to \"unknown\" instead...",
            (unsigned)moduleId,
            slot,
            api);

         moduleInstance->InitializeUnknown(slot);
      }
   }
   else
   {
      /*
      * Needed to pass Behavior scenario 2
      */
      Log(logWarning,
         "Definition/configuration of module with ID 0x%x to plug into slot %2u not found (API: %u). Plugging \"unknown\" module instead...",
         (unsigned)moduleId,
         slot,
         api);

      moduleInstance->InitializeUnknown(slot);
   }
   return 0;
}
inline std::string SubmodDirToString (pnet_submodule_dir_t direction)
{
   switch (direction)
   {
   case PNET_DIR_NO_IO:
      return "NO_IO";
   case PNET_DIR_INPUT:
      return "INPUT";
   case PNET_DIR_OUTPUT:
      return "OUTPUT";
   case PNET_DIR_IO:
      return "INPUT_OUTPUT";
   }

   return "unknown IO direction";
}
int ProfinetInternal::CallbackExpSubmoduleInd (
   pnet_t * net,
   uint32_t api,
   uint16_t slot,
   uint16_t subslot,
   uint32_t moduleId,
   uint32_t submoduleId,
   const pnet_data_cfg_t * p_exp_data)
{
   int ret = -1;
   int result = 0;
   pnet_data_cfg_t data_cfg;

   auto& deviceConfig{configuration.GetDevice()};
   const Submodule* submoduleConfig{nullptr};
   auto moduleWithPlugInfo{deviceConfig.modules[moduleId]};
   if(!moduleWithPlugInfo)
   {
      //
      // Needed for behavior scenario 2 to pass.
      // Iops will be set to bad for this subslot
      //
      Log(logWarning,
         "Trying to plug submodule into slot %2u subslot %u, but couldn't find corresponing definition/configuration of module ID 0x%x not found (API: %u). Using empty module configuration instead...",
         slot,
         subslot,
         (unsigned)moduleId,
         api);
   }
   else
   {
      auto& [plugInfo, moduleConfig] = *moduleWithPlugInfo;
      submoduleConfig = moduleConfig.submodules[submoduleId];
      if(!submoduleConfig)
      {
            //
            // Needed for behavior scenario 2 to pass.
            // Iops will be set to bad for this subslot
            //
            Log(logWarning,
               "Trying to plug submodule into slot %2u subslot %u, but couldn't find corresponing definition/configuration of submodule with ID 0x%x, belonging to module with ID 0x%x (API: %u). Using empty submodule configuration instead...",
               slot,
               subslot,
               (unsigned)submoduleId,
               (unsigned)moduleId,
               api);
      }
   }

   if (submoduleConfig)
   {
      size_t lengthInput = submoduleConfig->inputs.GetLengthInBytes();
      size_t lengthOutput = submoduleConfig->outputs.GetLengthInBytes();
       if(lengthInput > 0 && lengthOutput > 0)
         data_cfg.data_dir = PNET_DIR_IO;
      else if(lengthInput > 0)
         data_cfg.data_dir = PNET_DIR_OUTPUT; // Perspective change between device (here) and controller (profinet)
      else if(lengthOutput > 0)
         data_cfg.data_dir = PNET_DIR_INPUT; // Perspective change between device (here) and controller (profinet)
      else
         data_cfg.data_dir = PNET_DIR_NO_IO;
      data_cfg.insize = lengthOutput;  // Perspective change between device (here) and controller (profinet)
      data_cfg.outsize = lengthInput;  // Perspective change between device (here) and controller (profinet)
      
   }
   else
   {
      data_cfg.data_dir = p_exp_data->data_dir;
      data_cfg.insize = p_exp_data->insize;
      data_cfg.outsize = p_exp_data->outsize;
   }

   ModuleInstance* moduleInstance = device.GetModule(slot);
   if(!moduleInstance)
   {
      Log(logWarning,
         "Trying to plug submodule into slot %2u subslot %u, but no module plugged into slot %2u, yet (API: %u). Creating module now and initializing it with \"empty\" configuration...",
         slot,
         subslot,
         slot,
         api);
        
        //
        // Needed for behavior scenario 2 to pass.
        // Iops will be set to bad for this subslot
        //
      moduleInstance = device.CreateInSlot(slot);
      if(!moduleInstance)
      {
         Log(logError,
            "Trying to plug submodule into slot %2u subslot %u, but no module plugged into slot %2u, yet (API: %u). Then, also creation of a corresponding module failed.",
            slot,
            subslot,
            slot,
            api);
         return -1;
      }
      moduleInstance->InitializeUnknown(slot);
   }


   Log(logDebug,
      "Pulling old submodule from slot %2u, subslot %u (API: %u)...",
      slot,
      subslot,
      api);

   result = pnet_pull_submodule (net, api, slot, subslot);
   if (result == 0)
   {
        moduleInstance->RemoveFromSubslot(subslot);
   }

   Log(logDebug,
      "Plugging submodule with ID 0x%x into subslot %u of module with ID 0x%x in slot %2u (API: %u)...",
      (unsigned)submoduleId, subslot,
      (unsigned)moduleId, slot,
      api);

   if (
      data_cfg.data_dir != p_exp_data->data_dir ||
      data_cfg.insize != p_exp_data->insize ||
      data_cfg.outsize != p_exp_data->outsize)
   {
      Log(logWarning,
         "Inconsistent IO-sizes encountered while plugging submodule with ID 0x%x into subslot %u of module with ID 0x%x in slot %2u (API: %u): "
         "PLC expected input of %u bytes and output of %u bytes. Submodule configuration defines input of %u bytes and output of %u bytes.",
         (unsigned)submoduleId, subslot,
         (unsigned)moduleId, slot,
         api,
         p_exp_data->outsize, p_exp_data->insize,
         data_cfg.outsize, data_cfg.insize);
   }
   ret = pnet_plug_submodule (
      net,
      api,
      slot,
      subslot,
      moduleId,
      submoduleId,
      data_cfg.data_dir,
      data_cfg.insize,
      data_cfg.outsize);

   if (ret == 0)
   {
        SubmoduleInstance* submoduleInstance = moduleInstance->CreateInSubslot(subslot);
        if(submoduleConfig)
            submoduleInstance->Initialize(*submoduleConfig, subslot);
         else
         {
            submoduleInstance->InitializeUnknown(subslot, data_cfg.insize, data_cfg.outsize);
         }
   }
   else
   {
      Log(logError,
         "Failed to plug submodule with ID 0x%x into subslot %u of module with ID 0x%x in slot %2u (API: %u): "
         "Ret:%u.",
         (unsigned)submoduleId, subslot,
         (unsigned)moduleId, slot,
         api,
         ret);
      return ret;
   }
   return ret;
}

int ProfinetInternal::CallbackNewDataStatusInd (
   pnet_t * net,
   uint32_t arep,
   uint32_t crep,
   uint8_t changes,
   uint8_t data_status)
{
   bool isRunning = data_status & (1U << PNET_DATA_STATUS_BIT_PROVIDER_STATE);
   bool isValid = data_status & (1U << PNET_DATA_STATUS_BIT_DATA_VALID);

   Log(logInfo,
      "PLC signaled change of status (AREP: %u, data status new: 0x%02x "
      "changed bits: 0x%02x): %s, %s, %s, %s, %s. %s",
      arep,
      data_status,
      changes,
      isRunning ? "run" : "stop",
      isValid ? "data valid" : "data invalid",
      (data_status & (1U << PNET_DATA_STATUS_BIT_STATE)) ? "primary" : "backup",
      (data_status & (1U << PNET_DATA_STATUS_BIT_STATION_PROBLEM_INDICATOR))
         ? "normal operation"
         : "problem",
      (data_status & (1U << PNET_DATA_STATUS_BIT_IGNORE))
         ? "ignore data status"
         : "evaluate data status",
      (!isRunning || !isValid) ? "Setting device inputs to default values." : "");

   if (isRunning == false || isValid == false)
   {
      device.SetDefaultInputsAll();
   }
   return 0;
}

int ProfinetInternal::CallbackAlarmInd (
   pnet_t * net,
   uint32_t arep,
   const pnet_alarm_argument_t * p_alarm_argument,
   uint16_t data_len,
   uint16_t data_usi,
   const uint8_t * p_data)
{
   Log(logDebug,
      "Alarm indication. AREP: %u API: %d Slot: %d Subslot: %d "
      "Type: %d Seq: %d Length: %d USI: %d.",
      arep,
      p_alarm_argument->api_id,
      p_alarm_argument->slot_nbr,
      p_alarm_argument->subslot_nbr,
      p_alarm_argument->alarm_type,
      p_alarm_argument->sequence_number,
      data_len,
      data_usi);

   lastAlarmArguments = *p_alarm_argument;
   synchronizationEvents.SignalAlarm();
   return 0;
}

int ProfinetInternal::CallbackAlarmCnf (
   pnet_t * net,
   uint32_t arep,
   const pnet_pnio_status_t * p_pnio_status)
{
   Log(logDebug,
      "PLC alarm confirmation. AREP: %u  Status code %u, "
      "%u, %u, %u.",
      arep,
      p_pnio_status->error_code,
      p_pnio_status->error_decode,
      p_pnio_status->error_code_1,
      p_pnio_status->error_code_2);

   alarmAllowed = true;
   return 0;
}

int ProfinetInternal::CallbackAlarmAckCnf (
   pnet_t * net, 
   uint32_t arep, 
   int res)
{
   Log(logDebug,
      "PLC alarm ACK confirmation. AREP: %u  Result: "
      "%d.",
      arep,
      res);

   return 0;
}
}