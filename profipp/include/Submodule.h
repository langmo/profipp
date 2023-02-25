#ifndef SUBMODULE_H
#define SUBMODULE_H

#pragma once

#include "Parameter.h"
#include "Input.h"
#include "Output.h"
#include "helperfunctions.h"
#include "SubmoduleProperties.h"
#include <map>
#include <cstdint>
#include <functional>
#include <memory>
#include <sstream>
namespace profinet
{
class Submodule
{
public:
    Submodule(uint16_t id_);
    uint16_t GetId() const;
    
private:
    const uint16_t id;

public:
    class Inputs : public tools::VectorView<Input>
    {
    public:
        Inputs() : tools::VectorView<Input>{}
        {
        }
        Input* Create(const Input::SetCallbackType& setCallback, std::size_t lengthInBytes);
        template<typename T, std::size_t lengthInBytes=sizeof(T)> Input* Create(
            std::function<void(const T value)> setCallback)
        {
            auto wrapperSet = [setCallback](const uint8_t * buffer, std::size_t numbytes) -> bool
            {
                if(numbytes < lengthInBytes)
                    return false;
                T value;
                bool result = fromProfinet<T, lengthInBytes>(buffer, numbytes, &value);
                if(!result)
                    return false;
                setCallback(value);
                return true;
            };
            auto result{Create(wrapperSet, lengthInBytes)};
            if(result)
            {
                result->properties.dataType = gsdmlName<T>;
            }
            return result;
        }  
        std::size_t GetLengthInBytes() const;
    } inputs;
    class Outputs : public tools::VectorView<Output>
    {
    public:
        Outputs() : tools::VectorView<Output>{}
        {
        }
        std::size_t GetLengthInBytes() const;
        Output* Create(const Output::GetCallbackType& getCallback, std::size_t lengthInBytes);
        template<typename T, std::size_t lengthInBytes=sizeof(T)>
            Output* Create(std::function<T()> getCallback)
        {
            auto wrapperGet = [getCallback](uint8_t* buffer, std::size_t numbytes) -> bool
            {
                if(numbytes < lengthInBytes)
                    return false;
                T value = getCallback();
                return toProfinet<T, lengthInBytes>(buffer, numbytes, value);
            };
            auto result{Create(wrapperGet, lengthInBytes)};
            if(result)
            {
                result->properties.dataType = gsdmlName<T>;
            }
            return result;
        }
    } outputs;
    class Parameters : public tools::MapView<uint16_t, Parameter>
    {
    public:
        Parameters() : tools::MapView<uint16_t, Parameter>{}
        {
        }
        Parameter* Create(uint16_t parameterIdx, const Parameter::SetCallbackType& setCallback_, const Parameter::GetCallbackType& getCallback_, std::size_t lengthInBytes);
        template<typename T, std::size_t lengthInBytes=sizeof(T)> Parameter* Create(
            uint16_t parameterIdx, 
            const std::function<void(const T value)>& setCallback, const std::function<T()>& getCallback, T defaultValue=T{})
        {
            auto wrapperSet = [setCallback](const uint8_t * buffer, std::size_t numbytes) -> bool
            {
                if(numbytes < lengthInBytes)
                    return false;
                T value;
                bool result = fromProfinet<T, lengthInBytes>(buffer, numbytes, &value);
                if(!result)
                    return false;
                setCallback(value);
                return true;
            };
            auto wrapperGet = [getCallback](uint8_t* buffer, std::size_t numbytes) -> bool
            {
                if(numbytes < lengthInBytes)
                    return false;
                T value = getCallback();
                return toProfinet<T, lengthInBytes>(buffer, numbytes, value);
            };
            auto retVal = Create(parameterIdx, wrapperSet, wrapperGet, lengthInBytes);
            if(retVal)
            {
                std::stringstream ss;
                ss << defaultValue;
                retVal->properties.defaultValue = ss.str();
                retVal->properties.dataType = gsdmlName<T>;
            }
            return retVal;
        }
    } parameters;

    SubmoduleProperties properties;

};
}
#endif