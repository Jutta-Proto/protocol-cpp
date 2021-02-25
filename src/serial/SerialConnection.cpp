#include "SerialConnection.hpp"
#include <cassert>
#include <cstddef>
#include <iostream>
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
    state = SC_OPENED;
    std::cout << "Successfully opened serial device: " << device << '\n';
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
    // NOLINTNEXTLINE (hicpp-signed-bitwise)
    config.c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON);
    config.c_oflag = 0;
    // NOLINTNEXTLINE (hicpp-signed-bitwise)
    config.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
    // NOLINTNEXTLINE (hicpp-signed-bitwise)
    config.c_cflag &= ~(CSIZE | PARENB);
    // NOLINTNEXTLINE (hicpp-signed-bitwise)
    config.c_cflag |= CS8;

    // One byte is enough to return from read:
    config.c_cc[VMIN] = 1;
    config.c_cc[VTIME] = 0;

    if (cfsetispeed(&config, B115200) < 0 || cfsetospeed(&config, B9600) < 0) {
        assert(false);
    }

    if (tcsetattr(fd, TCSAFLUSH, &config) < 0) {
        assert(false);
    }
    state = SC_READY;
    std::cout << "Successfully configured serial device\n";
}

void SerialConnection::closeTty() {
    close(fd);
    state = SC_DISABLED;
}

SerialConnection::~SerialConnection() {
    closeTty();
}

size_t SerialConnection::read(std::array<uint8_t, 4>& buffer) {
    assert(state == SC_READY);
    return read(fd, buffer.data(), buffer.size());
}

size_t SerialConnection::write(const std::array<uint8_t, 4>& data) {
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
    while (read(buffer) >= 0) {}
}
//---------------------------------------------------------------------------
}  // namespace serial
//---------------------------------------------------------------------------
