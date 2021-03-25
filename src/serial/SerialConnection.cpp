#include "serial/SerialConnection.hpp"
#include "logger/Logger.hpp"
#include <cassert>
#include <cstddef>
#include <exception>
#include <sstream>
#include <stdexcept>
#include <system_error>

extern "C" {
#include <fcntl.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
}

//---------------------------------------------------------------------------
namespace serial {
//---------------------------------------------------------------------------
SerialConnection::SerialConnection(std::string&& device) : device(std::move(device)) {}

SerialConnection::~SerialConnection() {
    closeTty();
}

void SerialConnection::init() {
    assert(state == SC_DISABLED);
    openTty(device);
    configureTty();
    assert(state == SC_READY);
}

void SerialConnection::openTty(const std::string& device) {
    assert(state == SC_DISABLED || state == SC_ERROR);
    // Open with:
    // O_RDWR - Open for read and write.
    // O_NOCTTY - The device never becomes the controlling terminal of the process.
    // O_NDELAY - Use non-blocking I/O.
    // NOLINTNEXTLINE (hicpp-signed-bitwise)
    fd = open(device.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        throw std::runtime_error("Failed to open '" + device + "' with: " + strerror(errno));
    }
    tcflush(fd, TCIOFLUSH);
    state = SC_OPENED;
    SPDLOG_INFO("Successfully opened serial device: {}", device);
}

void SerialConnection::configureTty() {
    assert(state == SC_OPENED);
    assert(fd != -1);  // Ensure opening was successfull
    // Ensure the device is a TTY:
    if (!isatty(fd)) {
        throw std::runtime_error("Failed to configure '" + device + "'. The device is no TTY.");
    }

    termios config{};
    if (tcgetattr(fd, &config) < 0) {
        throw std::runtime_error("Failed to get configuration for '" + device + "' with: " + strerror(errno));
    }

    config.c_iflag = 0;
    config.c_oflag = 0;
    config.c_cflag = CS8 | CREAD | CLOCAL;
    config.c_lflag = 0;
    /**
     * Max time in tenth of seconds between characters allowed.
     * We abuse this since the coffee maker will make a 8ms break between each byte it sends.
     * For fail save reasons we allow 2 ms between the individual bytes.
     * http://unixwiz.net/techtips/termios-vmin-vtime.html
     **/
    config.c_cc[VTIME] = 2;
    /**
     * Number of characters have been received, with no more data available.
     * http://unixwiz.net/techtips/termios-vmin-vtime.html
     **/
    config.c_cc[VMIN] = 4;
    if (cfsetispeed(&config, B9600) < 0 || cfsetospeed(&config, B9600) < 0) {
        throw std::runtime_error("Failed to set the baud rate for '" + device + "' with: " + strerror(errno));
    }

    if (tcsetattr(fd, TCSANOW, &config) < 0) {
        throw std::runtime_error("Failed to set configuration for '" + device + "' with: " + strerror(errno));
    }
    state = SC_READY;
    SPDLOG_INFO("Successfully configured serial device.");
}

void SerialConnection::closeTty() {
    if (state != SC_DISABLED) {
        close(fd);
        fd = -1;
        SPDLOG_INFO("Serial device closed.");
        state = SC_DISABLED;
    }
}

size_t SerialConnection::read_serial(std::array<uint8_t, 4>& buffer) const {
    assert(state == SC_READY);
    return read(fd, buffer.data(), buffer.size());
}

size_t SerialConnection::write_serial(const std::array<uint8_t, 4>& data) const {
    assert(state == SC_READY);
    size_t result = write(fd, data.data(), data.size());
    return result;
}

void SerialConnection::flush() const {
    // Wait until everything has been send:
    tcdrain(fd);
}

std::vector<std::string> SerialConnection::get_available_ports() {
    std::vector<std::string> ports{};
    return ports;
}
//---------------------------------------------------------------------------
}  // namespace serial
//---------------------------------------------------------------------------
