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
    const std::string device;
    int fd = -1;
    SerialConnectionState state{SC_DISABLED};

 public:
    explicit SerialConnection(std::string&& device);
    ~SerialConnection();

    /**
     * Tries to initializes the serial (UART) connection.
     * Throws a exception in case something goes wrong.
     **/
    void init();

    /**
     * Reads at maximum four bytes.
     * Returns how many bytes have been actually read.
     **/
    [[nodiscard]] size_t read_serial(std::array<uint8_t, 4>& buffer) const;
    /**
     * Writes the given data buffer to the serial connection.
     **/
    [[nodiscard]] size_t write_serial(const std::array<uint8_t, 4>& data) const;
    void flush() const;

    /**
     * Returns all available serial port paths for this device.
     **/
    static std::vector<std::string> get_available_ports();

 private:
    /**
     * Tries to open the given serial port and stores the resulting file descriptor in fd.
     * Throws a exception in case something goes wrong.
     **/
    void openTty(const std::string& device);
    /**
     * Tries to configure the current file descriptor fd as serial (UART) device.
     * Throws a exception in case something goes wrong.
     **/
    void configureTty();
    void closeTty();
};
//---------------------------------------------------------------------------
}  // namespace serial
//---------------------------------------------------------------------------
