#include "SerialConnection.h"

#include <cstdint>

// C library headers
#include <cstdio>
#include <cstring>
#include <string>

// Linux headers
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <errno.h>   // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()

SerialConnection::SerialConnection()
{
}

SerialConnection::~SerialConnection()
{
    Disconnect();
}

bool SerialConnection::Connect(std::string serialPort)
{
    // Open serial port
    int serial_port = open(serialPort.c_str(), O_RDWR);
    if(serial_port < 0)
    {
        return false;
    }

    // Read in serial port settings
    struct termios tty;
    if (tcgetattr(serial_port, &tty) != 0)
    {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        close(serial_port);
        return false;
    }

    // Change settings
    tty.c_cflag &= ~PARENB;        // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB;        // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag &= ~CSIZE;         // Clear all bits that set the data size
    tty.c_cflag |= CS8;            // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS;       // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag &= ~ICANON;                                                      // Disable canonical mode (=processing data only upon newline)
    tty.c_lflag &= ~ECHO;                                                        // Disable echo
    tty.c_lflag &= ~ECHOE;                                                       // Disable erasure
    tty.c_lflag &= ~ECHONL;                                                      // Disable new-line echo
    tty.c_lflag &= ~ISIG;                                                        // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);                                      // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
    // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

    tty.c_cc[VTIME] = 0; // (deciseconds), return immediately if no data is available.
    tty.c_cc[VMIN] = 0;

    // Set in/out baud rate to be 9600
    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);

    // Save tty settings, also checking for error
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
    {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        close(serial_port);
        return false;
    }
    connection = serial_port;
    return true;
}
bool SerialConnection::IsConnected()
{
    return connection > 0;
}
bool SerialConnection::Disconnect()
{
    if(!IsConnected())
        return false;
    close(connection);
    connection = -1;
    return true;
}
bool SerialConnection::Send(const uint8_t* buffer, std::size_t numBytes)
{
    if(!IsConnected())
        return false;
    ssize_t length = write(connection, buffer, numBytes);
    if (length < 0)
    {
        printf("Error writing: %s", strerror(errno));
        return false;
    }
    fsync(connection);
    return true;
}
bool SerialConnection::Read(uint8_t* buffer, std::size_t maxBytes, std::size_t* numBytes)
{
    // Read bytes. The behaviour of read() (e.g. does it block?,
    // how long does it block for?) depends on the configuration
    // settings above, specifically VMIN and VTIME
    ssize_t length = read(connection, buffer, maxBytes);

    // n is the number of bytes read. n may be 0 if no bytes were received, and can also be -1 to signal an error.
    if (length < 0)
    {
        printf("Error reading: %s", strerror(errno));
        *numBytes = 0;
        return false;
    }
    *numBytes = length;
    return true; // success
};