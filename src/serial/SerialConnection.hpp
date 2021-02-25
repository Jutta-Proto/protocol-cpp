#pragma once

#include <array>
#include <string>
#include <vector>

//---------------------------------------------------------------------------
namespace serial {
//---------------------------------------------------------------------------
enum SerialConnectionState { SC_DISABLED = 0,
                             SC_OPENED = 1,
                             SC_READY = 2,
                             SC_ERROR = 3 };

/**
 * Based on: https://en.wikibooks.org/wiki/Serial_Programming/termios
 **/
class SerialConnection {
 private:
    int fd = -1;
    SerialConnectionState state{SC_DISABLED};

 public:
    explicit SerialConnection(const std::string& device);
    ~SerialConnection();

    /**
     * Reads at maximum four bytes.
     * Returns how many bytes have been actually read.
     **/
    size_t read(std::array<uint8_t, 4>& buffer);
    /**
     * Writes the given data buffer to the serial connection.
     **/
    size_t write(const std::array<uint8_t, 4>& data);
    void flush() const;
    /**
     * Flushes the read buffer.
     **/
    void flush_read_buffer();

 private:
    void openTty(const std::string& device);
    void configureTty();
    void closeTty();
};
//---------------------------------------------------------------------------
}  // namespace serial
//---------------------------------------------------------------------------
