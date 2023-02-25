#ifndef SERIALCONNECTION_H
#define SERIALCONNECTION_H

#pragma once

#include <string>
class SerialConnection final
{
public:
    SerialConnection();
    ~SerialConnection();
    
    // Forbid to make copies, but move is OK.
    SerialConnection(const  SerialConnection&) = delete;
    SerialConnection( SerialConnection&&) = default;
    SerialConnection& operator= (const  SerialConnection&) = delete;
    SerialConnection& operator= ( SerialConnection&&) = default;

public:
    bool Connect(std::string serialPort="/dev/ttyACM0");
    bool IsConnected();
    /**
     * @brief Disconnects the serial port connection. Automatically called in destructor.
     * 
     * @return true If connection was successfully disconnected.
     * @return false If no connection existed/connection was already disconnected.
     */
    bool Disconnect();

    bool Send(const uint8_t* buffer, std::size_t numBytes);

    bool Read(uint8_t* buffer, std::size_t maxBytes, std::size_t* numBytes);
private:
    // -1 if serial port connection was not established or is closed.
    int connection{-1};
};
#endif