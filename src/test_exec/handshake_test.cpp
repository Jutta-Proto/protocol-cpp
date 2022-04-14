#include "jutta_proto/JuttaCommands.hpp"
#include "jutta_proto/JuttaConnection.hpp"
#include "logger/Logger.hpp"
#include <cstdint>
#include <string>
#include <thread>
#include <vector>
#include <spdlog/spdlog.h>

const std::array<uint8_t, 16> numbers3 = {0x08, 0x0E, 0x0C, 0x04, 0x03, 0x0D, 0x0A, 0x0B, 0x00, 0x0F, 0x06, 0x07, 0x02, 0x05, 0x01, 0x09};

//--------------------------------------------------------------------------------------------------------------------------
// const std::array<uint8_t, 16> numbers1 = {14, 4, 3, 2, 1, 13, 8, 11, 6, 15, 12, 7, 10, 5, 0, 9};
// const std::array<uint8_t, 16> numbers2 = {10, 6, 13, 12, 14, 11, 1, 9, 15, 7, 0, 5, 3, 2, 4, 8};

uint8_t mod256(int i) {
    while (i > 255) {
        i -= 256;
    }
    while (i < 0) {
        i += 256;
    }
    return static_cast<uint8_t>(i);
}

uint8_t shuffle(int dataNibble, int nibbleCount, int keyLeftNibbel, int keyRightNibbel) {
    uint8_t i5 = mod256(nibbleCount >> 4);
    uint8_t tmp1 = numbers3[mod256(dataNibble + nibbleCount + keyLeftNibbel) % 16];
    uint8_t tmp2 = numbers3[mod256(tmp1 + keyRightNibbel + i5 - nibbleCount - keyLeftNibbel) % 16];
    uint8_t tmp3 = numbers3[mod256(tmp2 + keyLeftNibbel + nibbleCount - keyRightNibbel - i5) % 16];
    return mod256(tmp3 - nibbleCount - keyLeftNibbel) % 16;
}

std::vector<uint8_t> encDecBytes(const std::vector<uint8_t>& data, uint8_t key) {
    std::vector<uint8_t> result;
    result.resize(data.size());
    uint8_t keyLeftNibbel = key >> 4;
    uint8_t keyRightNibbel = key & 15;
    int nibbelCount = 0;
    for (size_t offset = 0; offset < data.size(); offset++) {
        uint8_t d = data[offset];
        uint8_t dataLeftNibbel = d >> 4;
        uint8_t dataRightNibbel = d & 15;
        uint8_t resultLeftNibbel = shuffle(dataLeftNibbel, nibbelCount++, keyLeftNibbel, keyRightNibbel);
        uint8_t resultRightNibbel = shuffle(dataRightNibbel, nibbelCount++, keyLeftNibbel, keyRightNibbel);
        result[offset] = (resultLeftNibbel << 4) | resultRightNibbel;
    }
    return result;
}
//--------------------------------------------------------------------------------------------------------------------------

void discBasedDecryption(uint8_t* dst, uint8_t* src) {
    uint8_t curchar = 0;
    uint8_t key = 0;
    uint8_t* data = nullptr;
    uint8_t* pcVar1 = nullptr;
    uint8_t nibbelCount = 0;
    uint8_t keyLeftNibble = 0;
    uint8_t keyRightNibble = 0;
    uint8_t bVar1 = 0;
    uint8_t cVar1 = 0;
    const uint8_t* decodeDisc = nullptr;

    key = src[1];
    if (key == 0x1b) {
        key = static_cast<char>(src[2] ^ 0x80);
        data = src + 2;
        src[2] = key;
    } else {
        data = src + 1;
    }
    decodeDisc = numbers3.data();
    keyLeftNibble = key >> 4;
    keyRightNibble = key & 0xf;
    curchar = data[1];
    nibbelCount = 0;
    while (curchar != 0xd) {
        pcVar1 = data + 1;
        if (curchar == 0x1b) {
            curchar = data[2] ^ 0x80;
            data[2] = curchar;
            pcVar1 = data + 2;
        }
        bVar1 = nibbelCount + 1;
        uint8_t discOff1 = ((curchar >> 4) + nibbelCount + keyLeftNibble) & 0xf;
        uint8_t discOff2 = (((((decodeDisc[discOff1] + keyRightNibble) - nibbelCount) - keyLeftNibble) + (nibbelCount >> 4)) & 0xf) + 0x10;
        uint8_t discOff3 = (((keyLeftNibble + nibbelCount + decodeDisc[discOff2]) - keyRightNibble) - (nibbelCount >> 4)) & 0xf;
        cVar1 = decodeDisc[discOff3] - nibbelCount;
        nibbelCount = nibbelCount + 2;

        uint8_t discOff4 = ((curchar & 0xf) + bVar1 + keyLeftNibble) & 0xf;
        uint8_t discOff5 = ((((decodeDisc[discOff4] + keyRightNibble + (bVar1 >> 4)) - keyLeftNibble) - bVar1) & 0xf) + 0x10;
        uint8_t discOff6 = (((keyLeftNibble + bVar1 + decodeDisc[discOff5]) - keyRightNibble) - (bVar1 >> 4)) & 0xf;
        *dst = ((cVar1 - keyLeftNibble) << 4) | (((decodeDisc[discOff6] - bVar1) - keyLeftNibble) & 0xf);
        dst++;
        data = pcVar1;
        curchar = pcVar1[1];
    }
    *dst = 0;
}

std::vector<uint8_t> decode(const std::vector<uint8_t>& data) {
    std::vector<uint8_t> input;
    input.insert(input.begin(), data.begin(), data.end());
    discBasedDecryption(input.data(), input.data());
    return input;
}

int main(int /*argc*/, char** /*argv*/) {
    logger::setup_logger(spdlog::level::debug);
    SPDLOG_INFO("Starting handshake test...");

    jutta_proto::JuttaConnection connection("/dev/serial/by-id/usb-FTDI_FT232R_USB_UART_A5047JSK-if00-port0");
    connection.init();
    while (true) {
        std::shared_ptr<std::string> coffeeMakerType = nullptr;
        while (!coffeeMakerType || coffeeMakerType->find("ty:") == std::string::npos) {
            coffeeMakerType = connection.write_decoded_with_response(jutta_proto::JUTTA_GET_TYPE, std::chrono::milliseconds{1000});
            if (!coffeeMakerType) {
                std::this_thread::sleep_for(std::chrono::milliseconds{500});
            }
        }
        SPDLOG_INFO("Found coffee maker: {}", *coffeeMakerType);

        // Handshake:
        SPDLOG_INFO("Continuing with the handshake...");
        SPDLOG_INFO("Sending '@T1'...");
        if (!connection.write_decoded_wait_for("@T1\r\n", "@t1\r\n")) {
            SPDLOG_WARN("Failed to receive '@t1'");
            continue;
        }

        SPDLOG_INFO("Waiting for '@T2:...'...");
        std::vector<uint8_t> buf;
        // NOLINTNEXTLINE (abseil-string-find-str-contains)
        while (connection.vec_to_string(buf).find("@T2") == std::string::npos) {
            connection.read_decoded(buf);
        }

        SPDLOG_INFO("Sending '@t2:...'...");
        buf.clear();
        connection.write_decoded("@t2:8120000000\r\n");
        // NOLINTNEXTLINE (abseil-string-find-str-contains)
        while (connection.vec_to_string(buf).find("@T3") == std::string::npos) {
            connection.read_decoded(buf);
        }

        SPDLOG_INFO("Sending '@t3'...");
        connection.write_decoded("@t3\r\n");
        SPDLOG_INFO("Handshake done!");
        break;
    }

    std::vector<uint8_t> response;
    while (true) {
        connection.read_decoded(response);
        if (!response.empty()) {
            if (response[0] == '&') {
                SPDLOG_INFO("Received: {}", connection.vec_to_string(decode(response)));
            }
            response.clear();
        }
    }

    return 0;
}
