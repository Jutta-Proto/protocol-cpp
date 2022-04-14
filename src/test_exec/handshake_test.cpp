#include "jutta_proto/JuttaCommands.hpp"
#include "jutta_proto/JuttaConnection.hpp"
#include "logger/Logger.hpp"
#include <cstdint>
#include <string>
#include <thread>
#include <vector>
#include <spdlog/spdlog.h>

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
            SPDLOG_INFO(" Received: {}", connection.vec_to_string(response));
            response.clear();
        }
    }

    return 0;
}
