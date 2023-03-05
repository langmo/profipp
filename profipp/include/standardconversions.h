#ifndef STANDARDCONVERSIONS_H
#define STANDARDCONVERSIONS_H

#pragma once
// Header for uint32_t and similar
#define __STDC_LIMIT_MACROS // To get macros like INT32_MAX
#include <cstdint>
// Header for memcpy
#include <cstring>
// Header for template overloading
#include <type_traits>

// Uncomment the next line to display the bit patterns of all received inputs
//#define DEBUG_ENDIAN_CONVERSION
#ifdef DEBUG_ENDIAN_CONVERSION
    #include <bitset>
    #include <iostream>
#endif
namespace profinet
{
    /**
     * @brief Returns true if the current machine is big endian.
     * 
     * @return true if big endian.
     * @return false if not big endian (probably small endian).
     */
    constexpr bool isBigEndian()
    {
    #if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
        return true;
    #elif defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
        return false;
    #else
        // If no macros are defined, simply run a test to detect endianess
        uint16_t i{1};
        return ! *((uint8_t*)&i);
    #endif
    }

    /**
     * @brief Template to get the corresponding GSDML (=profinet) name for a given C++ type.
     *        Returns "unknown" if GSDML name does not exist or not known.
     * 
     * @tparam T 
     */
    template<typename T> inline constexpr const char* gsdmlName = "unknown";
#ifdef INT8_MAX
    template<> inline constexpr const char* gsdmlName<int8_t> = "Integer8";
#endif
#ifdef INT16_MAX
    template<> inline constexpr const char* gsdmlName<int16_t> = "Integer16";
#endif
#ifdef INT32_MAX
    template<> inline constexpr const char* gsdmlName<int32_t> = "Integer32";
#endif
#ifdef INT64_MAX
    template<> inline constexpr const char* gsdmlName<int64_t> = "Integer64";
#endif
#ifdef UINT8_MAX
    template<> inline constexpr const char* gsdmlName<uint8_t> = "Unsigned8";
#endif
#ifdef UINT16_MAX
    template<> inline constexpr const char* gsdmlName<uint16_t> = "Unsigned16";
#endif
#ifdef UINT32_MAX
    template<> inline constexpr const char* gsdmlName<uint32_t> = "Unsigned32";
#endif    
#ifdef UINT64_MAX
    template<> inline constexpr const char* gsdmlName<uint64_t> = "Unsigned64";
#endif    
    template<> inline constexpr const char* gsdmlName<float> = "Float32";
    template<> inline constexpr const char* gsdmlName<double> = "Float64";

    /**
     * @brief A complicated way to write false as a macro. However, it depends on T, such that
     *        it can be used in if constexpr (..) static_assert(dependent_false<T>::value),
     *        which only evaluates when the if clause is true as compared to static_assert<false>,
     *        which always evaluates (and thus, cannot be compiled even if the if clause is false).
     * 
     * @tparam T Any dependent type.
     */
    template <class T> struct dependent_false : std::false_type 
    {
    };

    /**
     * @brief Template function which reads in the value of a given C++ data type from
     *        a profinet provided buffer. Performs big to small endian conversion, if necessary.
     *        Only implemented via template specializations for C++ data types which have a corresponding data type in C++.
     *        If not supported, a static compile time assertion fails.
     * 
     * @tparam T Type of the C++ data type which should be read from the profinet buffer.
     * @tparam lengthInBytes Length in bytes of the C++ data type.
     * @param buffer The profinet buffer (in big endian)
     * @param numbytes Number of bytes still available in this buffer. Must be larger than lengthInBytes (=the number of bytes which should be read in). Otherwise, false is returned and value is not changed.
     * @param value Pointer to the C++ variable in which the value stored in the first lengthInBytes bytes of buffer should be read in, using system endianess.
     * @return true If reading was successfull. Then, value was set to the corresponding value.
     * @return false If reading was not successfull. Then, value was not changed.
     */
    template<typename T, std::size_t lengthInBytes=sizeof(T)> 
        bool fromProfinet(const uint8_t* buffer, std::size_t numbytes, T* value)
    {
        if(numbytes < lengthInBytes)
            return false;
        if constexpr (std::is_same_v<T, uint8_t*>)
        {
            memcpy(value, buffer, lengthInBytes);
            return true;
        }
        else if constexpr (std::is_arithmetic_v<T>)
        {
            static_assert(std::is_unsigned_v<T> || lengthInBytes==sizeof(T), "For signed integral types or floating point types, lengthInBytes must equal sizeof(T).");
            static_assert(lengthInBytes<=sizeof(T), "Number of bytes which should be read to an arithmetic type must be smaller or equal to the size of this type.");
            if constexpr (!isBigEndian())
            {
                if constexpr (lengthInBytes<sizeof(T))
                {
                    // only happens when unsigned integer type
                    // initialize all bits to zero, since not all will be overwritten (we might e.g. store an unsigned 8bit integer value in an unsigned 16bit integer).
                    *value = 0;
                }
                uint8_t* ptr = (uint8_t*)value;
                for(int i=0; i<lengthInBytes; i++)
                {
                    ptr[sizeof(T)-i-1]=buffer[i];
                }
                return true;
            }
            else
            {
                memcpy(value, buffer, lengthInBytes);
                if constexpr (lengthInBytes<sizeof(T))
                {
                    // only happens when unsigned integer type
                    *value >>= (lengthInBytes-sizeof(T))*8;
                }
                return true;
            }
        }
        else
        {
            static_assert(dependent_false<T>::value , "No standard conversion function for this type specialized.");
        }
        return false;
    }

    /**
     * @brief Template function which writes the value of a given C++ data type to
     *        a profinet provided buffer. Performs big to small endian conversion, if necessary.
     *        Only implemented via template specializations for C++ data types which have a corresponding data type in C++.
     *        If not supported, a static compile time assertion fails.
     * 
     * @tparam T T Type of the C++ data type which should be written to the profinet buffer.
     * @tparam lengthInBytes Length in bytes of the C++ data type.
     * @param buffer The profinet buffer (in big endian)
     * @param numbytes Number of bytes still available in this buffer. Must be larger than lengthInBytes (=the number of bytes which should be written). Otherwise, false is returned and value is not changed.
     * @param value The C++ variable which stores the value which should be written to the first lengthInBytes bytes of buffer, using system endianess. 
     * @return true If writing was successfull. Then, the corresponding bytes of buffer were set to the provided value.
     * @return false If writing was not successfull. Then, the buffer was not changed.
     */
    template<typename T, std::size_t lengthInBytes=sizeof(T)>
        bool toProfinet(uint8_t* buffer, std::size_t numbytes, const T value)
    {
        if(numbytes < lengthInBytes)
            return false;
        if constexpr (std::is_same_v<T, uint8_t*>)
        {
            memcpy(buffer, &value, lengthInBytes);
            return true;
        }
        else if constexpr (std::is_arithmetic_v<T>)
        {
            static_assert(std::is_unsigned_v<T> || lengthInBytes==sizeof(T), "For signed integral types or floating point types, lengthInBytes must equal sizeof(T).");
            static_assert(lengthInBytes>=sizeof(T), "Number of bytes which should be written to a buffer from an arithmetic type must be greater or equal to the size of this type.");
            if constexpr (!isBigEndian())
            {
                uint8_t* ptr = (uint8_t*)&value;
                for(int i=0; i<sizeof(T); i++)
                {
                    buffer[lengthInBytes-i-1]=ptr[i];
                }
                if constexpr (lengthInBytes>sizeof(T))
                {
                    // only happens when unsigned integer type
                    // initialize all remaining bytes to zero.
                    for(int i=sizeof(T); i<lengthInBytes; i++)
                    {
                        buffer[lengthInBytes-i-1]=0;
                    }
                }
                return true;
            }
            else
            {
                memcpy(buffer+lengthInBytes-sizeof(T), value, lengthInBytes);
                if constexpr (lengthInBytes<sizeof(T))
                {
                    for(int i=0; i<lengthInBytes-sizeof(T); i++)
                    {
                        buffer[i]=0;
                    }
                }
                return true;
            }
        }
        else
        {
            static_assert(dependent_false<T>::value , "No standard conversion function for this type specialized.");
        }
        return false;
    }
    
    /**
     * Support of uint32_t
     */
#ifdef UINT32_MAX
    template<> inline bool fromProfinet<uint32_t, sizeof(uint32_t)>
        (const uint8_t* buffer, std::size_t numbytes, uint32_t* value)
    {
        if(numbytes < sizeof(uint32_t))
            return false;
        if constexpr (!isBigEndian())
        {
            // Note: Doing it this way, instead of
            // a memcpy, ensures that we get the right endianess, 
            // independent of the endianess of the system we are running on.
            *value =     ((static_cast<uint32_t>(buffer[0]) << 24) | 
                        (static_cast<uint32_t>(buffer[1]) << 16) | 
                        (static_cast<uint32_t>(buffer[2]) << 8)  | 
                        (static_cast<uint32_t>(buffer[3])));
        }
        else
        {
            std::memcpy(value, buffer, sizeof(uint32_t));
        }
        #ifdef DEBUG_ENDIAN_CONVERSION
            std::bitset<8> b0(buffer[0]);
            std::bitset<8> b1(buffer[1]);
            std::bitset<8> b2(buffer[2]);
            std::bitset<8> b3(buffer[3]);
            std::cout << "bitvalue int: " << b0 << ' ' << b1 << ' ' << b2 <<' ' << b3 << " -> result=" << *value << '\n';
        #endif

        return true;
    }
    template<> inline bool toProfinet<uint32_t, sizeof(uint32_t)>
        (uint8_t* buffer, std::size_t numbytes, const uint32_t value)
    {
        if(numbytes < sizeof(uint32_t))
            return false;
        if constexpr (!isBigEndian())
        {
            buffer[0] = (value >> 24) & 0xFF;
            buffer[1] = (value >> 16) & 0xFF;
            buffer[2] = (value >> 8 ) & 0xFF;
            buffer[3] = (value      ) & 0xFF;
        }
        else
        {
            std::memcpy(buffer, &value, sizeof(uint32_t));
        }
        return true;
    }
#endif
#ifdef UINT16_MAX
    template<> inline bool fromProfinet<uint16_t, sizeof(uint16_t)>
        (const uint8_t* buffer, std::size_t numbytes, uint16_t* value)
    {
        if(numbytes < sizeof(uint16_t))
            return false;
        if constexpr (!isBigEndian())
        {
            // Note: Doing it this way, instead of
            // a memcpy, ensures that we get the right endianess, 
            // independent of the endianess of the system we are running on.
            *value =     ((static_cast<uint16_t>(buffer[0]) << 8)  | 
                        (static_cast<uint16_t>(buffer[1])));
        }
        else
        {
            std::memcpy(value, buffer, sizeof(uint16_t));
        }
        #ifdef DEBUG_ENDIAN_CONVERSION
            std::bitset<8> b0(buffer[0]);
            std::bitset<8> b1(buffer[1]);
            std::cout << "bitvalue int: " << b0 << ' ' << b1 << " -> result=" << *value << '\n';
        #endif

        return true;
    }
    template<> inline bool toProfinet<uint16_t, sizeof(uint16_t)>
        (uint8_t* buffer, std::size_t numbytes, const uint16_t value)
    {
        if(numbytes < sizeof(uint16_t))
            return false;
        if constexpr (!isBigEndian())
        {
            buffer[0] = (value >> 8 ) & 0xFF;
            buffer[1] = (value      ) & 0xFF;
        }
        else
        {
            std::memcpy(buffer, &value, sizeof(uint16_t));
        }
        return true;
    }
#endif

#ifdef INT32_MAX
    template<> inline bool fromProfinet<int32_t, sizeof(int32_t)>
        (const uint8_t* buffer, std::size_t numbytes, int32_t* value)
    {
        if(numbytes < sizeof(int32_t))
            return false;
        if constexpr (!isBigEndian())
        {
            // Note: Doing it this way, instead of
            // a memcpy, ensures that we get the right endianess, 
            // independent of the endianess of the system we are running on.
            *value =   ((static_cast<int32_t>(buffer[0]) << 24) | 
                        (static_cast<int32_t>(buffer[1]) << 16) | 
                        (static_cast<int32_t>(buffer[2]) << 8)  | 
                        (static_cast<int32_t>(buffer[3])));
        }
        else
        {
            std::memcpy(value, buffer, sizeof(int32_t));
        }
        #ifdef DEBUG_ENDIAN_CONVERSION
            std::bitset<8> b0(buffer[0]);
            std::bitset<8> b1(buffer[1]);
            std::bitset<8> b2(buffer[2]);
            std::bitset<8> b3(buffer[3]);
            std::cout << "bitvalue int: " << b0 << ' ' << b1 << ' ' << b2 <<' ' << b3 << " -> result=" << *value << '\n';
        #endif

        return true;
    }
    template<> inline bool toProfinet<int32_t, sizeof(int32_t)>
        (uint8_t* buffer, std::size_t numbytes, const int32_t value)
    {
        if(numbytes < sizeof(int32_t))
            return false;
        if constexpr (!isBigEndian())
        {
            buffer[0] = (value >> 24) & 0xFF;
            buffer[1] = (value >> 16) & 0xFF;
            buffer[2] = (value >> 8 ) & 0xFF;
            buffer[3] = (value      ) & 0xFF;
        }
        else
        {
            std::memcpy(buffer, &value, sizeof(int32_t));
        }
        return true;
    }
#endif
#ifdef INT16_MAX
    template<> inline bool fromProfinet<int16_t, sizeof(int16_t)>
        (const uint8_t* buffer, std::size_t numbytes, int16_t* value)
    {
        if(numbytes < sizeof(int16_t))
            return false;
        if constexpr (!isBigEndian())
        {
            // Note: Doing it this way, instead of
            // a memcpy, ensures that we get the right endianess, 
            // independent of the endianess of the system we are running on.
            *value =   ((static_cast<int16_t>(buffer[0]) << 8)  | 
                        (static_cast<int16_t>(buffer[1])));
        }
        else
        {
            std::memcpy(value, buffer, sizeof(int16_t));
        }
        #ifdef DEBUG_ENDIAN_CONVERSION
            std::bitset<8> b0(buffer[0]);
            std::bitset<8> b1(buffer[1]);
            std::cout << "bitvalue int: " << b0 << ' ' << b1 << " -> result=" << *value << '\n';
        #endif

        return true;
    }
    template<> inline bool toProfinet<int16_t, sizeof(int16_t)>
        (uint8_t* buffer, std::size_t numbytes, const int16_t value)
    {
        if(numbytes < sizeof(int16_t))
            return false;
        if constexpr (!isBigEndian())
        {
            buffer[0] = (value >> 8 ) & 0xFF;
            buffer[1] = (value      ) & 0xFF;
        }
        else
        {
            std::memcpy(buffer, &value, sizeof(int16_t));
        }
        return true;
    }
#endif

    /**
    * Support of float
    */

    template<> inline bool fromProfinet<float, sizeof(float)>
        (const uint8_t* buffer, std::size_t numbytes, float* value)
    {
        if(numbytes < sizeof(float))
            return false;
        if constexpr (!isBigEndian())
        {
            uint32_t temp =((static_cast<uint32_t>(buffer[0]) << 24) | 
                            (static_cast<uint32_t>(buffer[1]) << 16) | 
                            (static_cast<uint32_t>(buffer[2]) << 8)  | 
                            (static_cast<uint32_t>(buffer[3])));

            *value = *((float*)&temp);
        }
        else
        {
            std::memcpy(value, buffer, sizeof(float));
        }
        return true;
    }

    template<> inline bool toProfinet<float, sizeof(float)>
        (uint8_t* buffer, std::size_t numbytes, float value)
    {
        if(numbytes < sizeof(float))
            return false;
        if constexpr (!isBigEndian())
        {
            uint32_t temp;
            memcpy (&temp, &value, sizeof(float));
            
            buffer[0] = (temp >> 24) & 0xFF;
            buffer[1] = (temp >> 16) & 0xFF;
            buffer[2] = (temp >> 8 ) & 0xFF;
            buffer[3] = (temp      ) & 0xFF;
        }
        else
        {
            std::memcpy(buffer, &value, sizeof(float));
        }
        return true;
    }
}
#endif