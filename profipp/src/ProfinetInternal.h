#ifndef PROFINETINTERNAL_H
#define PROFINETINTERNAL_H

#pragma once

#include "Profinet.h"
#include "DeviceInstance.h"
#include "pnet_api.h"
#include "logging.h"

#include <map>
#include <mutex>
#include <condition_variable>

namespace profinet
{
class ProfinetInternal final : public ProfinetControl 
{
public:
    ProfinetInternal();
    ~ProfinetInternal();
    
    ProfinetInternal (const ProfinetInternal&) = delete;
    ProfinetInternal& operator= (const ProfinetInternal&) = delete;

    bool Initialize(const Profinet& configuration, LoggerType logger = logging::CreateConsoleLogger());
    virtual bool Start() override;
    bool IsConnectedToController();
private:
    DeviceInstance device;
    Profinet configuration;
    LoggerType logFun;
    void Log(LogLevel logLevel, const char* format, ...) noexcept;    
    bool alarmAllowed;
    bool initialized;
    pnet_t* profinetStack;
    pnet_cfg_t pnetCfg;
    pnet_alarm_argument_t lastAlarmArguments{};

    uint32_t arep;
    uint32_t arepForReady;

private:
    // Helper functions
    bool SendApplicationReady(uint32_t arep);
    pnet_cfg InitializePnetConfig();
    bool PlugDap(pnet_t* net, uint16_t number_of_ports);
    bool HandleSendAlarmAck ();
    bool SetInitialDataAndIoxs();
    void HandleCyclicData();
    void SetLed(bool on);

private:
    std::string mainNetworkInterface{};
    std::vector<std::string> networkInterfaces{};
    
    class
    {
    private:
        std::mutex mutex{};
        std::condition_variable condition{};
        // variable used by many threads. Should be saveguarded by mutex. After changing, condition must be notified.
        unsigned int signaledEvents{0};
        // variable only used by worker thread. Does not have to be saveguarded.
        unsigned int receivedEvents{0};
        // bitmasks for the different signals.
        const unsigned int eventCycle{1};
        const unsigned int eventReadyForData{2};
        const unsigned int eventAlarm{4};
        const unsigned int eventAbort{8};
    public:
        /**
         * Should only be called by worker thread.
         * Worker thread waits until at least one signal is signaled, and then receives all signals.
         */
        inline void ReceiveEvents()
        {
            std::unique_lock lock{mutex};
            if(!signaledEvents)
                condition.wait(lock);
            receivedEvents |= signaledEvents;
            signaledEvents = 0;
        }
        /**
         * Signals to the worker thread that it should process cyclic data.
         */
        inline void SignalCycle()
        {
            {
                std::lock_guard lock{mutex};
                signaledEvents |= eventCycle;
            }
            condition.notify_one();
        }
        /**
         * Signals to the worker thread that it should send the ready for data signal to the PLC.
         */
        inline void SignalReadyForData()
        {
            {
                std::lock_guard lock{mutex};
                signaledEvents |= eventReadyForData;
            }
            condition.notify_one();
        }
        /**
         * Signals to the worker thread that it should process an alarm.
         */
        inline void SignalAlarm()
        {
            {
                std::lock_guard lock{mutex};
                signaledEvents |= eventAlarm;
            }
            condition.notify_one();
        }
        /**
         * Signals to the worker thread that it should abort.
         */
        inline void SignalAbort()
        {
            {
                std::lock_guard lock{mutex};
                signaledEvents |= eventAbort;
            }
            condition.notify_one();
        }
        /**
         * Should only be called by worker thread.
         * Checks if it received the signal for cyclic data processing.
         * Also, resets this signal.
         */
        inline bool ProcessCycle()
        {
            bool temp = (receivedEvents & eventCycle);
            receivedEvents &= ~eventCycle;
            return temp;
        }
        /**
         * Should only be called by worker thread.
         * Checks if it received the signal for ready for data.
         * Also, resets this signal.
         */
        inline bool ProcessReadyForData()
        {
            bool temp = (receivedEvents & eventReadyForData);
            receivedEvents &= ~eventReadyForData;
            return temp;
        }
        /**
         * Should only be called by worker thread.
         * Checks if it received the signal for alarm.
         * Also, resets this signal.
         */
        inline bool ProcessAlarm()
        {
            bool temp = (receivedEvents & eventAlarm);
            receivedEvents &= ~eventAlarm;
            return temp;
        }
        /**
         * Should only be called by worker thread.
         * Checks if it received the signal to abort.
         * Also, resets this signal.
         */
        inline bool ProcessAbort()
        {
            bool temp = (receivedEvents & eventAbort);
            receivedEvents &= ~eventAbort;
            return temp;
        }
    } synchronizationEvents;

    
public:
    // Do not call directly
    void loop();
/*
    * Application call-back functions
    *
    * The application should define call-back functions which are called by
    * the stack when specific events occurs within the stack.
    *
    * Note that most of these functions are mandatory in the sense that they must
    * exist and return 0 (zero) for a functioning stack. Some functions are
    * required to perform specific functionality.
    *
    * The call-back functions should return 0 (zero) for a successful call and
    * -1 for an unsuccessful call.
    */

    /**
     * Indication to the application that a Connect request was received from the
     * controller.
     *
     * This application call-back function is called by the Profinet stack on every
     * Connect request from the Profinet controller.
     *
     * The connection will be opened if this function returns 0 (zero) and the stack
     * is otherwise able to establish a connection.
     *
     * If this function returns something other than 0 (zero) then the Connect
     * request is refused by the device. In case of error the application should
     * provide error information in \a p_result.
     *
     * It is optional to implement this callback (assumes success if not
     * implemented).
     *
     * @param net              InOut: The p-net stack instance
     * @param arep             In:    The AREP.
     * @param p_result         Out:   Detailed error information if return != 0.
     * @return  0  on success.
     *          -1 if an error occurred.
     */
    int CallbackConnectInd (
        pnet_t* net,
        uint32_t arep,
        pnet_result_t * p_result);

    /**
     * Indication to the application that a Release request was received from the
     * controller.
     *
     * This application call-back function is called by the Profinet stack on every
     * Release request from the Profinet controller.
     *
     * The connection will be closed regardless of the return value from this
     * function. In case of error the application should provide error information
     * in \a p_result.
     *
     * It is optional to implement this callback (assumes success if not
     * implemented).
     *
     * @param net              InOut: The p-net stack instance
     * @param arep             In:    The AREP.
     * @param p_result         Out:   Detailed error information if return != 0.
     * @return  0  on success.
     *          -1 if an error occurred.
     */
    int CallbackReleaseInd (
        pnet_t* net,
        uint32_t arep,
        pnet_result_t * p_result);

    /**
     * Indication to the application that a DControl request was received from the
     * controller. Typically this means that the controller is done writing
     * parameters.
     *
     * This application call-back function is called by the Profinet stack on every
     * DControl request from the Profinet controller.
     *
     * The application is not required to take any action but the function must
     * return 0 (zero) for proper function of the stack. If this function returns
     * something other than 0 (zero) then the DControl request is refused by the
     * device. In case of error the application should provide error information in
     * \a p_result.
     *
     * It is optional to implement this callback (assumes success if not
     * implemented).
     *
     * @param net              InOut: The p-net stack instance
     * @param arep             In:    The AREP.
     * @param control_command  In:    The DControl command code.
     * @param p_result         Out:   Detailed error information if return != 0.
     * @return  0  on success.
     *          -1 if an error occurred.
     */
    int CallbackDControlInd (
        pnet_t* net,
        uint32_t arep,
        pnet_control_command_t control_command,
        pnet_result_t * p_result);

    /**
     * Indication to the application that a CControl confirmation was received from
     * the controller. Typically this means that the controller has received our
     * "Application ready" message.
     *
     * This application call-back function is called by the Profinet stack on every
     * CControl confirmation from the Profinet controller.
     *
     * The application is not required to take any action.
     * The return value from this call-back function is ignored by the Profinet
     * stack. In case of error the application should provide error information in
     * \a p_result.
     *
     * It is optional to implement this callback (assumes success?).
     *
     * @param net              InOut: The p-net stack instance
     * @param arep             In:    The AREP.
     * @param p_result         Out:   Detailed error information.
     * @return 0 on success. Other values are ignored.
     */
    int CallbackCControlCnf (
        pnet_t* net,
        uint32_t arep,
        pnet_result_t * p_result);

    /**
     * Indication to the application that a state transition has occurred within the
     * Profinet stack.
     *
     * This application call-back function is called by the Profinet stack on
     * specific state transitions within the Profinet stack.
     *
     * At the very least the application must react to the PNET_EVENT_PRMEND state
     * transition. After this event the application must call \a
     * pnet_application_ready(), when it has finished its setup and it is ready to
     * exchange data.
     *
     * The return value from this call-back function is ignored by the Profinet
     * stack.
     *
     * It is optional to implement this callback (but then it would be difficult
     * to know when to call the \a pnet_application_ready() function).
     *
     * @param net              InOut: The p-net stack instance
     * @param arep             In:    The AREP.
     * @param state            In:    The state transition event. See
     *                                pnet_event_values_t.
     * @return 0 on success. Other values are ignored.
     */
    int CallbackStateInd (
        pnet_t* net,
        uint32_t arep,
        pnet_event_values_t state);

    /**
     * Indication to the application that an IODRead request was received from the
     * controller.
     *
     * This application call-back function is called by the Profinet stack on every
     * IODRead request from the Profinet controller which specify an
     * application-specific value of \a idx (0x0000 - 0x7fff). All other values of
     * \a idx are handled internally by the Profinet stack.
     *
     * The application must verify the value of \a idx, and that \a p_read_length is
     * large enough. Further, the application must provide a
     * pointer to the binary value in \a pp_read_data and the size, in bytes, of the
     * binary value in \a p_read_length.
     *
     * The Profinet stack does not perform any endianness conversion on the binary
     * value.
     *
     * In case of error the application should provide error information in \a
     * p_result.
     *
     * @param net              InOut: The p-net stack instance
     * @param arep             In:    The AREP.
     * @param api              In:    The AP identifier.
     * @param slot             In:    The slot number.
     * @param subslot          In:    The sub-slot number.
     * @param idx              In:    The data record index.
     * @param sequence_number  In:    The sequence number.
     * @param pp_read_data     Out:   A pointer to the binary value.
     * @param p_read_length    InOut: The maximum (in) and actual (out) length in
     *                                bytes of the binary value.
     * @param p_result         Out:   Detailed error information if returning != 0
     * @return  0  on success.
     *          -1 if an error occurred.
     */
    int CallbackReadInd (
        pnet_t* net,
        uint32_t arep,
        uint32_t api,
        uint16_t slot,
        uint16_t subslot,
        uint16_t idx,
        uint16_t sequence_number,
        uint8_t ** pp_read_data,
        uint16_t * p_read_length,
        pnet_result_t * p_result);

    /**
     * Indication to the application that an IODWrite request was received from the
     * controller.
     *
     * This application call-back function is called by the Profinet stack on every
     * IODWrite request from the Profinet controller which specify an
     * application-specific value of \a idx (0x0000 - 0x7fff). All other values of
     * \a idx are handled internally by the Profinet stack.
     *
     * The application must verify the values of \a idx and \a write_length and save
     * (copy) the binary value in \a p_write_data. A future IODRead must return the
     * latest written value.
     *
     * The Profinet stack does not perform any endianness conversion on the binary
     * value.
     *
     * In case of error the application should provide error information in \a
     * p_result.
     *
     * @param net              InOut: The p-net stack instance
     * @param arep             In:    The AREP.
     * @param api              In:    The API identifier.
     * @param slot             In:    The slot number.
     * @param subslot          In:    The sub-slot number.
     * @param idx              In:    The data record index.
     * @param sequence_number  In:    The sequence number.
     * @param write_length     In:    The length in bytes of the binary value.
     * @param p_write_data     In:    A pointer to the binary value.
     * @param p_result         Out:   Detailed error information if returning != 0
     * @return  0  on success.
     *          -1 if an error occurred.
     */
    int CallbackWriteInd (
        pnet_t* net,
        uint32_t arep,
        uint32_t api,
        uint16_t slot,
        uint16_t subslot,
        uint16_t idx,
        uint16_t sequence_number,
        uint16_t write_length,
        const uint8_t * p_write_data,
        pnet_result_t * p_result);

    /**
     * Indication to the application that a module is requested by the controller in
     * a specific slot.
     *
     * This application call-back function is called by the Profinet stack to
     * indicate that the controller has requested the presence of a specific module,
     * ident number \a module_ident, in the slot number \a slot.
     *
     * The application must react to this by configuring itself accordingly (if
     * possible) and call function pnet_plug_module() to configure the stack for
     * this module.
     *
     * If the wrong module ident number is plugged then the stack will accept this,
     * but signal to the controller that a substitute module is fitted.
     *
     * This function should return 0 (zero) if a valid module was plugged. Or return
     * -1 if the application cannot handle this request.
     *
     * @param net              InOut: The p-net stack instance
     * @param api              In:    The AP identifier.
     * @param slot             In:    The slot number.
     * @param module_ident     In:    The module ident number.
     * @return  0  on success.
     *          -1 if an error occurred.
     */
    int CallbackExpModuleInd (
        pnet_t* net,
        uint32_t api,
        uint16_t slot,
        uint32_t module_ident);

    /**
     * Indication to the application that a sub-module is requested by the
     * controller in a specific sub-slot.
     *
     * This application call-back function is called by the Profinet stack to
     * indicate that the controller has requested the presence of a specific
     * sub-module with ident number \a submodule_ident, in the sub-slot number
     * \a subslot, with module ident number \a module_ident in slot \a slot.
     *
     * If a module has not been plugged in the slot \a slot then an automatic plug
     * request is issued internally by the stack.
     *
     * The application must react to this by configuring itself accordingly (if
     * possible) and call function \a pnet_plug_submodule() to configure the stack
     * with the correct input/output data sizes.
     *
     * If the wrong sub-module ident number is plugged then the stack will accept
     * this, but signal to the controller that a substitute sub-module is fitted.
     *
     * This function should return 0 (zero) if a valid sub-module was plugged,
     * or return -1 if the application cannot handle this request.
     *
     * @param net              InOut: The p-net stack instance
     * @param api              In:    The AP identifier.
     * @param slot             In:    The slot number.
     * @param subslot          In:    The sub-slot number.
     * @param module_ident     In:    The module ident number.
     * @param submodule_ident  In:    The sub-module ident number.
     * @param p_exp_data       In:    The expected data configuration (sizes and
     *                                direction)
     * @return  0  on success.
     *          -1 if an error occurred.
     */
    int CallbackExpSubmoduleInd (
        pnet_t* net,
        uint32_t api,
        uint16_t slot,
        uint16_t subslot,
        uint32_t module_ident,
        uint32_t submodule_ident,
        const pnet_data_cfg_t * p_exp_data);

    /**
     * Indication to the application that the data status received from the
     * controller has changed.
     *
     * This application call-back function is called by the Profinet stack to
     * indicate that the received data status has changed.
     *
     * The application is not required by the Profinet stack to take any action. It
     * may use this information as it wishes. The return value from this call-back
     * function is ignored by the Profinet stack.
     *
     * @param net              InOut: The p-net stack instance
     * @param arep             In:    The AREP.
     * @param crep             In:    The CREP.
     * @param changes          In:    The changed bits in the received data status.
     *                                See pnet_data_status_bits_t
     * @param data_status      In:    Current received data status (after changes).
     *                                See pnet_data_status_bits_t
     * @return 0 on success. Other values are ignored.
     */
    int CallbackNewDataStatusInd (
        pnet_t* net,
        uint32_t arep,
        uint32_t crep,
        uint8_t changes,
        uint8_t data_status);

    /**
     * The IO-controller has sent an alarm to the device.
     *
     * This functionality is used for alarms triggered by the IO-controller.
     *
     * When receiving this indication, the application shall
     * respond with \a pnet_alarm_send_ack(), which
     * may be called in the context of this callback.
     *
     * @param net              InOut: The p-net stack instance
     * @param arep             In:    The AREP.
     * @param p_alarm_argument In:    The alarm argument (with slot, subslot,
     *                                alarm_type etc)
     * @param data_len         In:    Data length
     * @param data_usi         In:    Alarm USI
     * @param p_data           In:    Alarm data
     * @return  0  on success.
     *          Other values are ignored.
     */
    int CallbackAlarmInd (
        pnet_t* net,
        uint32_t arep,
        const pnet_alarm_argument_t * p_alarm_argument,
        uint16_t data_len,
        uint16_t data_usi,
        const uint8_t * p_data);

    /**
     * The controller acknowledges the alarm sent previously.
     * It is now possible to send another alarm.
     *
     * This functionality is used for alarms triggered by the IO-device.
     *
     * It is optional to implement this callback.
     * The return value from this call-back function is ignored by the Profinet
     * stack.
     *
     * @param net              InOut: The p-net stack instance
     * @param arep             In:    The AREP.
     * @param p_pnio_status    In:    Detailed ACK information.
     * @return 0 on success. Other values are ignored.
     */
    int CallbackAlarmCnf (
        pnet_t* net,
        uint32_t arep,
        const pnet_pnio_status_t * p_pnio_status);

    /**
     * The controller acknowledges the alarm ACK sent previously.
     *
     * This functionality is used for alarms triggered by the IO-controller.
     *
     * It is optional to implement this callback.
     * The return value from this call-back function is ignored by the Profinet
     * stack.
     *
     * @param net              InOut: The p-net stack instance
     * @param arep             In:    The AREP.
     * @param res              In:    0 if ACK was received by the remote side.
     *                                  This is cnf(+).
     *                                -1 if ACK was not received by the remote
     *                                  side. This is cnf(-).
     * @return 0 on success. Other values are ignored.
     */
    int CallbackAlarmAckCnf (
        pnet_t* net, 
        uint32_t arep, 
        int res);

    /**
     * Indication to the application that a reset request was received from the
     * IO-controller.
     *
     * The IO-controller can ask for communication parameters or application
     * data to be reset, or to do a factory reset.
     *
     * This application call-back function is called by the Profinet stack on every
     * reset request (via the DCP "Set" command) from the Profinet controller.
     *
     * The application should reset the application data if
     * \a should_reset_application is true. For other cases this callback is
     * triggered for diagnostic reasons.
     *
     * The return value from this call-back function is ignored by the Profinet
     * stack.
     *
     * It is optional to implement this callback (if you do not have any application
     * data that could be reset).
     *
     * Reset modes:
     *  * 0:  (Power-on reset, not from IO-controller. Will not trigger this.)
     *  * 1:  Reset application data
     *  * 2:  Reset communication parameters (done by the stack)
     *  * 99: Reset all (factory reset).
     *
     * The reset modes 1-9 (out of which 1 and 2 are supported here) are defined
     * by the Profinet standard. Value 99 is used here to indicate that the
     * IO-controller has requested a factory reset via another mechanism.
     *
     * In order to remain responsive to DCP communication and Ethernet switching,
     * the device should not do a hard or soft reset for reset mode 1 or 2. It is
     * allowed for the factory reset case (but not mandatory).
     *
     * No \a arep information is available, as this callback typically is triggered
     * when there is no active connection.
     *
     * @param net                      InOut: The p-net stack instance
     * @param should_reset_application In:    True if the user should reset the
     *                                        application data.
     * @param reset_mode               In:    Detailed reset information.
     * @return 0 on success. Other values are ignored.
     */
    int CallbackResetInd (
        pnet_t* net,
        bool should_reset_application,
        uint16_t reset_mode);

    /**
     * Indication to the application that the Profinet signal LED should change
     * state.
     *
     * Use this callback to implement control of the LED.
     *
     * It is optional to implement this callback (but a compliant Profinet device
     * must have a signal LED)
     *
     * No \a arep information is available, as this callback typically is triggered
     * when there is no active connection.
     *
     * @param net              InOut: The p-net stack instance
     * @param led_state        In:    True if the signal LED should be on.
     * @return  0  on success.
     *          -1 if an error occurred. Will trigger a log message.
     */
    int CallbackSignalLedInd (
        pnet_t* net, 
        bool led_state);


};
}
#endif