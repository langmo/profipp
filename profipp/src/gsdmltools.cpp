#include "gsdmltools.h"
#include "pugixml.hpp"
#include "dapModule.h"

#include <string>
#include <stdexcept>
#include <cstdarg>
#include <cstdio>
#include <chrono>  // now()
#include <sstream> // stringstream
#include <fstream> // ofstream
#include <iomanip> // put_time()
#include <filesystem>

namespace profinet::gsdml
{

inline std::string str_printf (const char* format, ...)
{
    using namespace std;

    std::va_list args;
    std::string retval;

    va_start (args, format);
    retval.resize (vsnprintf (0, 0, format, args));
    vsnprintf (&retval[0], retval.size () + 1, format, args);
    va_end (args);

    return retval;
}
std::string GenerateGsdmlFileName(const Profinet& profinet)
{
    auto now{std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())};
    std::stringstream dateString{};
    dateString << std::put_time(std::localtime(&now), "%Y%m%d");
    auto& properties{profinet.GetDevice().properties};
    return str_printf("GSDML-V2.4-%s-%s-%s.xml", properties.vendorName.c_str(), properties.deviceName.c_str(), dateString.str().c_str());
}
bool CreateGsdml(const Profinet& profinet, const std::string& pathToFolder)
{
    std::filesystem::path folderPath{pathToFolder};
    if(!std::filesystem::is_directory(folderPath))
        return false;
    std::ofstream fileStream(folderPath / GenerateGsdmlFileName(profinet));
    return CreateGsdml(profinet, fileStream);
}
inline bool CreateDOM(const Profinet& profinet, pugi::xml_document& doc)
{
    auto& props{profinet.GetDevice().properties};
    std::map<std::string, std::string> texts{};
    auto addText = [&texts](const std::string& identifier, const std::string& text) -> const char*
    {
        std::string completeID = "ID_TEXT_" + identifier;
        return texts.insert_or_assign(std::move(completeID), text).first->first.c_str();

    };
    auto& device{profinet.GetDevice()};
    auto dapModuleWithPlugInfo{device.modules[dap::moduleId]};
    if(!dapModuleWithPlugInfo)
        return false;
    auto& [dapPlugInfo, dapModule] {*dapModuleWithPlugInfo};
    auto getModuleID = [](uint16_t moduleId) -> std::string {return str_printf("ID_MODULE%u",moduleId);};
    auto textIDDeviceInfo = addText("DEVICE_INFO", props.deviceInfoText);
    auto textIDDeviceName = addText("DEVICE_NAME", props.deviceName);

    // XML definition
    auto decl{doc.prepend_child(pugi::node_declaration)};
    decl.append_attribute("version") = "1.0";
    decl.append_attribute("encoding") = "utf-8";
    // Profile header. Never changes, always the same
    auto profile{doc.append_child("ISO15745Profile")};
    profile.append_attribute("xmlns").set_value("http://www.profibus.com/GSDML/2003/11/DeviceProfile");
    profile.append_attribute("xmlns:xsi").set_value("http://www.w3.org/2001/XMLSchema-instance");
    profile.append_attribute("xsi:schemaLocation").set_value("http://www.profibus.com/GSDML/2003/11/DeviceProfile ..\\xsd\\GSDML-DeviceProfile-V2.4.xsd");
    auto profileHeader{profile.append_child("ProfileHeader")};
    profileHeader.append_child("ProfileIdentification").text()="PROFINET Device Profile";
    profileHeader.append_child("ProfileRevision").text()="1.00";
    profileHeader.append_child("ProfileName").text()="Device Profile for PROFINET Devices";
    profileHeader.append_child("ProfileSource").text()="PROFIBUS Nutzerorganisation e. V. (PNO)";
    profileHeader.append_child("ProfileClassID").text()="Device";
    auto profileHeaderReference{profileHeader.append_child("ISO15745Reference")};
    profileHeaderReference.append_child("ISO15745Part").text()=4;
    profileHeaderReference.append_child("ISO15745Edition").text()=1;
    profileHeaderReference.append_child("ProfileTechnology").text()="GSDML";

    // Profile body
    auto body{profile.append_child("ProfileBody")};
    auto identity{body.append_child("DeviceIdentity")};
    identity.append_attribute("VendorID").set_value(str_printf("%#.4x", props.vendorID).c_str() );
    identity.append_attribute("DeviceID").set_value(str_printf("%#.4x", props.deviceID).c_str());
    identity.append_child("InfoText").append_attribute("TextId").set_value(textIDDeviceInfo);
    identity.append_child("VendorName").append_attribute("Value").set_value(props.vendorName.c_str());
    auto family = body.append_child("DeviceFunction").append_child("Family");
    family.append_attribute("MainFamily").set_value("I/O");
    family.append_attribute("ProductFamily").set_value(props.deviceProductFamily.c_str());

    /////////////////////////////////////////
    // application process (where the meat goes in)
    ////////////////////////////////////////
    auto applicationProcess{body.append_child("ApplicationProcess")};
    //DeviceAccessPointList
    auto deviceAccessPointList{applicationProcess.append_child("DeviceAccessPointList")};
    auto accessItem{deviceAccessPointList.append_child("DeviceAccessPointItem")};
    accessItem.append_attribute("ID").set_value("IDD_1");
    accessItem.append_attribute("PNIO_Version").set_value("V2.4");
    accessItem.append_attribute("PhysicalSlots").set_value(str_printf("0..%u", props.numSlots).c_str());
    accessItem.append_attribute("ModuleIdentNumber").set_value(str_printf("%#.8x", dapModule.GetId()).c_str());
    accessItem.append_attribute("MinDeviceInterval").set_value(str_printf("%u", props.minDeviceInterval).c_str());
    accessItem.append_attribute("DNS_CompatibleName").set_value(props.stationName.c_str());
    accessItem.append_attribute("FixedInSlots").set_value(str_printf("%u", dapPlugInfo.fixedSlot).c_str());
    accessItem.append_attribute("ObjectUUID_LocalIndex").set_value("1");
    accessItem.append_attribute("DeviceAccessSupported").set_value("false");
    accessItem.append_attribute("MultipleWriteSupported").set_value("true");
    accessItem.append_attribute("CheckDeviceID_Allowed").set_value("true");
    accessItem.append_attribute("NameOfStationNotTransferable").set_value("false");
    accessItem.append_attribute("LLDP_NoD_Supported").set_value("true");
    accessItem.append_attribute("ResetToFactoryModes").set_value("1..2");
    // Module Info
    auto accessModuleInfo{accessItem.append_child("ModuleInfo")};
    accessModuleInfo.append_child("Name").append_attribute("TextId").set_value(textIDDeviceName);
    accessModuleInfo.append_child("InfoText").append_attribute("TextId").set_value(textIDDeviceInfo);
    accessModuleInfo.append_child("VendorName").append_attribute("Value").set_value(props.vendorName.c_str());
    accessModuleInfo.append_child("OrderNumber").append_attribute("Value").set_value(props.orderID.c_str());
    accessModuleInfo.append_child("HardwareRelease").append_attribute("Value").set_value(str_printf("%c%u.%u", props.hwRevPrefix, props.hwRevMajor, props.hwRevMinor).c_str());
    accessModuleInfo.append_child("SoftwareRelease").append_attribute("Value").set_value(str_printf("%c%u.%u.%u", props.swRevPrefix, props.swRevMajor, props.swRevMinor, props.swRevPatch).c_str());
    // Certification Info
    auto certificationInfo{accessItem.append_child("CertificationInfo")};
    certificationInfo.append_attribute("NetloadClass").set_value("I");
    certificationInfo.append_attribute("ApplicationClass").set_value("");
    certificationInfo.append_attribute("ConformanceClass").set_value("B");
    // IOConfigData
    auto ioConfigData{accessItem.append_child("IOConfigData")};
    ioConfigData.append_attribute("MaxOutputLength").set_value("244");
    ioConfigData.append_attribute("MaxInputLength").set_value("244");
    // Usable Modules
    auto useableModules{accessItem.append_child("UseableModules")};
    for(auto& moduleWithPlugInfo : device.modules)
    {
        if(moduleWithPlugInfo.module.GetId() == dap::moduleId)
            continue;
        auto moduleItemRef{useableModules.append_child("ModuleItemRef")};
        if(moduleWithPlugInfo.plugInfo.IsFixedSlot())
            moduleItemRef.append_attribute("FixedInSlots").set_value(str_printf("%u", moduleWithPlugInfo.plugInfo.fixedSlot).c_str());
        else
        {
            if(moduleWithPlugInfo.plugInfo.allowedSlots.empty())
            {
                moduleItemRef.append_attribute("AllowedInSlots").set_value(str_printf("1..%u", props.numSlots).c_str());
            }
            else
            {
                std::string allowedSlots{""};
                bool first = true;
                for(auto slot : moduleWithPlugInfo.plugInfo.allowedSlots)
                {
                    if(first)
                    {
                        first = false;
                        allowedSlots+=str_printf("%u", slot);
                    }
                    else
                        allowedSlots+=str_printf(" %u", slot);
                }
                moduleItemRef.append_attribute("AllowedInSlots").set_value(allowedSlots.c_str());
            }
        }
        moduleItemRef.append_attribute("ModuleItemTarget").set_value(getModuleID(moduleWithPlugInfo.module.GetId()).c_str());
    }

    // VirtualSubmoduleList
    // TODO: Currently not synchronized to actual dap module settings
    auto virtualSubmoduleList{accessItem.append_child("VirtualSubmoduleList")};
    auto virtualSubmoduleItem{virtualSubmoduleList.append_child("VirtualSubmoduleItem")};
    virtualSubmoduleItem.append_attribute("ID").set_value("IDS_1");
    virtualSubmoduleItem.append_attribute("MayIssueProcessAlarm").set_value("false");
    virtualSubmoduleItem.append_attribute("Writeable_IM_Records").set_value("1 2 3");
    virtualSubmoduleItem.append_attribute("SubmoduleIdentNumber").set_value(str_printf("%#.8x", dap::submoduleIdIdentity).c_str());
    virtualSubmoduleItem.append_child("IOData");
    auto moduleInfo{virtualSubmoduleItem.append_child("ModuleInfo")};
    moduleInfo.append_child("Name").append_attribute("TextId").set_value(textIDDeviceName);
    moduleInfo.append_child("InfoText").append_attribute("TextId").set_value(textIDDeviceInfo);

    // SystemDefinedSubmoduleList
    // TODO: Currently not synchronized to actual dap module settings
    auto systemDefinedSubmoduleList{accessItem.append_child("SystemDefinedSubmoduleList")};
    auto interfaceSubmoduleItem{systemDefinedSubmoduleList.append_child("InterfaceSubmoduleItem")};
    interfaceSubmoduleItem.append_attribute("TextId").set_value(addText("NETIF1_NAME", "X1"));
    interfaceSubmoduleItem.append_attribute("ID").set_value("IDS_I");
    interfaceSubmoduleItem.append_attribute("SubmoduleIdentNumber").set_value(str_printf("%#.8x", dap::submoduleIdInterface1).c_str());
    interfaceSubmoduleItem.append_attribute("DCP_BoundarySupported").set_value("true");
    interfaceSubmoduleItem.append_attribute("PTP_BoundarySupported").set_value("true");
    interfaceSubmoduleItem.append_attribute("NetworkComponentDiagnosisSupported").set_value("false");
    interfaceSubmoduleItem.append_attribute("SupportedProtocols").set_value("SNMP;LLDP");
    interfaceSubmoduleItem.append_attribute("SupportedRT_Classes").set_value("RT_CLASS_1");
    interfaceSubmoduleItem.append_attribute("SubslotNumber").set_value(str_printf("%u", dap::subslotInterface1).c_str());
    auto applicationRelations{interfaceSubmoduleItem.append_child("ApplicationRelations")};
    applicationRelations.append_attribute("StartupMode").set_value("Advanced");
    auto timingProperties{applicationRelations.append_child("TimingProperties")};
    timingProperties.append_attribute("ReductionRatio").set_value("1 2 4 8 16 32 64 128 256 512");
    timingProperties.append_attribute("SendClock").set_value("32");
    //..
    auto portSubmoduleItem{systemDefinedSubmoduleList.append_child("PortSubmoduleItem")};
    portSubmoduleItem.append_attribute("TextId").set_value(addText("NETIF1_PORT1", "X1 P1"));
    portSubmoduleItem.append_attribute("ID").set_value("IDS_P1");
    portSubmoduleItem.append_attribute("SubmoduleIdentNumber").set_value(str_printf("%#.8x", dap::submoduleIdInterface1Port1).c_str());
    portSubmoduleItem.append_attribute("SubslotNumber").set_value(str_printf("%u", dap::subslotInterface1Port1).c_str());
    portSubmoduleItem.append_attribute("MaxPortTxDelay").set_value("160");
    portSubmoduleItem.append_attribute("MaxPortRxDelay").set_value("350");
    auto mauTypeList{portSubmoduleItem.append_child("MAUTypeList")};
    mauTypeList.append_child("MAUTypeItem").append_attribute("Value").set_value("30");
    mauTypeList.append_child("MAUTypeItem").append_attribute("Value").set_value("16");
    mauTypeList.append_child("MAUTypeItem").append_attribute("Value").set_value("5");

    // Module List
    auto moduleList{applicationProcess.append_child("ModuleList")};
    for(auto& moduleWithPlugInfo : device.modules)
    {
        auto& [plugInfo, module]{moduleWithPlugInfo};
        if(module.GetId() == dap::moduleId)
            continue;
        auto moduleID = getModuleID(module.GetId());

        auto moduleItem{moduleList.append_child("ModuleItem")};
        moduleItem.append_attribute("ModuleIdentNumber").set_value(str_printf("%#.8x", module.GetId()).c_str());
        moduleItem.append_attribute("ID").set_value(moduleID.c_str());
        // TODO: PhysicalSubslots attribute
        // module Info
        auto moduleInfo{moduleItem.append_child("ModuleInfo")};
        auto& moduleProps = module.properties;
        moduleInfo.append_child("Name").append_attribute("TextId").set_value(addText(str_printf("MODULE%u_NAME", module.GetId()), moduleProps.name));
        moduleInfo.append_child("InfoText").append_attribute("TextId").set_value(addText(str_printf("MODULE%u_INFOTEXT", module.GetId()), moduleProps.infoText.c_str()));
        moduleInfo.append_child("HardwareRelease").append_attribute("Value").set_value(moduleProps.hardwareRelease.c_str());
        moduleInfo.append_child("SoftwareRelease").append_attribute("Value").set_value(moduleProps.softwareRelease.c_str());

        // TODO: UseableSubmodules list

        // Virtual submodules
        auto virtualSubmoduleList{moduleItem.append_child("VirtualSubmoduleList")};
        uint16_t subslot{0};
        for(auto& submodule : module.submodules)
        {
            subslot++;
            auto virtualSubmoduleItem{virtualSubmoduleList.append_child("VirtualSubmoduleItem")};
            virtualSubmoduleItem.append_attribute("ID").set_value(str_printf("IDSM%u", submodule.GetId()).c_str());
            virtualSubmoduleItem.append_attribute("MayIssueProcessAlarm").set_value("true");
            virtualSubmoduleItem.append_attribute("SubmoduleIdentNumber").set_value(str_printf("%#.8x", submodule.GetId()).c_str());
            virtualSubmoduleItem.append_attribute("FixedInSubslots").set_value(str_printf("%u", subslot).c_str());

            auto ioData{virtualSubmoduleItem.append_child("IOData")};
            
            if(!submodule.outputs.empty())
            {
                auto inputElem{ioData.append_child("Input")};
                inputElem.append_attribute("Consistency").set_value("All items consistency");
                // Note: Inputs and outputs are switched in their meanings in GSDML (perspective of the controller, not of the device)
                unsigned int dataItemNo = 0;
                for(auto& output : submodule.outputs)
                {
                    dataItemNo++;
                    auto dataItem{inputElem.append_child("DataItem")};
                    dataItem.append_attribute("TextId").set_value(addText(str_printf("SUBMODULE%u_INPUT%u", submodule.GetId(), dataItemNo), output.properties.description));
                    //TODO: How to determine data types?
                    dataItem.append_attribute("DataType").set_value(output.properties.dataType.c_str());
                }
            }

            if(!submodule.inputs.empty())
            {
                auto outputElem{ioData.append_child("Output")};
                outputElem.append_attribute("Consistency").set_value("All items consistency");
                // Note: Inputs and outputs are switched in their meanings in GSDML (perspective of the controller, not of the device)
                unsigned int dataItemNo = 0;
                for(auto& input : submodule.inputs)
                {
                    dataItemNo++;
                    auto dataItem{outputElem.append_child("DataItem")};
                    dataItem.append_attribute("TextId").set_value(addText(str_printf("SUBMODULE%u_OUTPUT%u", submodule.GetId(), dataItemNo), input.properties.description));
                    //TODO: How to determine data types?
                    dataItem.append_attribute("DataType").set_value(input.properties.dataType.c_str());
                }
            }

            auto recordDataList{virtualSubmoduleItem.append_child("RecordDataList")};
            for(auto& parameter : submodule.parameters)
            {
                auto parameterRecordDataItem{recordDataList.append_child("ParameterRecordDataItem")};
                parameterRecordDataItem.append_attribute("Length").set_value(str_printf("%u", parameter.GetLengthInBytes()).c_str());
                parameterRecordDataItem.append_attribute("Index").set_value(str_printf("%u", parameter.GetIdx()).c_str());
                parameterRecordDataItem.append_child("Name").append_attribute("TextId").set_value(addText(str_printf("SUBMODULE%u_PARAMETER%u_NAME", submodule.GetId(), parameter.GetIdx()), parameter.properties.name));
                auto ref{parameterRecordDataItem.append_child("Ref")};
                ref.append_attribute("TextId").set_value(addText(str_printf("SUBMODULE%u_PARAMETER%u_REF", submodule.GetId(), parameter.GetIdx()), parameter.properties.description));
                ref.append_attribute("DataType").set_value(parameter.properties.dataType.c_str());
                ref.append_attribute("Visible").set_value(parameter.properties.visible? "true" : "false");
                ref.append_attribute("Changeable").set_value(parameter.properties.changeable? "true" : "false");
                ref.append_attribute("ByteOffset").set_value("0");
                ref.append_attribute("DefaultValue").set_value(parameter.properties.defaultValue.c_str());
            }

            auto submoduleInfo{virtualSubmoduleItem.append_child("ModuleInfo")};
            submoduleInfo.append_child("Name").append_attribute("TextId").set_value(addText(str_printf("SUBMODULE%u_NAME", submodule.GetId()), submodule.properties.name.c_str()));
            submoduleInfo.append_child("InfoText").append_attribute("TextId").set_value(addText(str_printf("SUBMODULE%u_INFOTEXT", submodule.GetId()), submodule.properties.infoText.c_str()));
        }
    }
    
    // ExternalText List
    auto externalTextList{applicationProcess.append_child("ExternalTextList")};
    auto primaryLanguage{externalTextList.append_child("PrimaryLanguage")};
    for(auto& elem : texts)
    {
        auto text{primaryLanguage.append_child("Text")};
        text.append_attribute("TextId").set_value(elem.first.c_str());
        text.append_attribute("Value").set_value(elem.second.c_str());
    }

    return true;
}

bool CreateGsdml(const Profinet& profinet, std::basic_ostream<char>& stream)
{
    pugi::xml_document doc{};
    bool success = CreateDOM(profinet, doc);
    if(!success)
        return false;
    doc.save(stream);
    return true;
}
bool CreateGsdml(const Profinet& profinet, std::basic_ostream<wchar_t>& stream)
{
    pugi::xml_document doc{};
    bool success = CreateDOM(profinet, doc);
    if(!success)
        return false;
    doc.save(stream);
    return true;
}
}