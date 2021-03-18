#include "serial/SerialConnection.hpp"
#include "logger/Logger.hpp"
#include <cassert>
#include <cstddef>
#include <sstream>

extern "C" {
#include <fcntl.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
}

//---------------------------------------------------------------------------
namespace serial {
//---------------------------------------------------------------------------
SerialConnection::SerialConnection(const std::string& device) {
    assert(state == SC_DISABLED || state == SC_ERROR);
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
    assert(fd != -1);  // Ensure opening was successfull
    tcflush(fd, TCIOFLUSH);
    state = SC_OPENED;
    SPDLOG_INFO("Successfully opened serial device: {}", device);
}

void SerialConnection::configureTty() {
    assert(state == SC_OPENED);
    assert(fd != -1);  // Ensure opening was successfull
    // Ensure the device is a TTY:
    assert(isatty(fd));

    termios config{};
    if (tcgetattr(fd, &config) < 0) {
        assert(false);
    }

    config.c_iflag = 0;
    config.c_oflag = 0;
    config.c_cflag = CS8 | CREAD | CLOCAL | CSTOPB;
    config.c_lflag = 0;
    config.c_cc[VTIME] = static_cast<cc_t>(15);
    config.c_cc[VMIN] = 10;
    if (cfsetispeed(&config, B115200) < 0 || cfsetospeed(&config, B115200) < 0) {
        assert(false);
    }

    if (tcsetattr(fd, TCSANOW, &config) < 0) {
        assert(false);
    }
    state = SC_READY;
    SPDLOG_INFO("Successfully configured serial device.");
}

void SerialConnection::closeTty() {
    close(fd);
    state = SC_DISABLED;
    SPDLOG_INFO("Serial device closed.");
}

SerialConnection::~SerialConnection() {
    closeTty();
}

size_t SerialConnection::read_serial(std::array<uint8_t, 4>& buffer) {
    assert(state == SC_READY);
    return read(fd, buffer.data(), buffer.size());
}

size_t SerialConnection::write_serial(const std::array<uint8_t, 4>& data) {
    assert(state == SC_READY);
    size_t result = write(fd, data.data(), data.size());
    return result;
}

void SerialConnection::flush() const {
    // Wait until everything has been send:
    tcdrain(fd);
}

void SerialConnection::flush_read_buffer() {
    std::array<uint8_t, 4> buffer{};
    while (read_serial(buffer) > 0) {}
}

std::vector<std::string> SerialConnection::get_available_ports() {
    std::vector<std::string> ports{};
    return ports;
}
//---------------------------------------------------------------------------
}  // namespace serial
//---------------------------------------------------------------------------
