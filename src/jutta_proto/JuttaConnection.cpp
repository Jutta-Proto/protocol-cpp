#include "jutta_proto/JuttaConnection.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <spdlog/spdlog.h>

//---------------------------------------------------------------------------
namespace jutta_proto {
//---------------------------------------------------------------------------
JuttaConnection::JuttaConnection(std::string&& device) : serial(std::move(device)) {}

void JuttaConnection::init() {
    actionLock.lock();
    serial.init();
    actionLock.unlock();
}

bool JuttaConnection::read_decoded(std::vector<uint8_t>& data) {
    actionLock.lock();
    bool result = read_decoded_unsafe(data);
    actionLock.unlock();
    return result;
}

bool JuttaConnection::read_decoded(uint8_t* byte) {
    actionLock.lock();
    bool result = read_decoded_unsafe(byte);
    actionLock.unlock();
    return result;
}

bool JuttaConnection::read_decoded_unsafe(uint8_t* byte) const {
    std::array<uint8_t, 4> buffer{};
    if (!read_encoded_unsafe(buffer)) {
        return false;
    }
    *byte = decode(buffer);
    return true;
}

bool JuttaConnection::read_decoded_unsafe(std::vector<uint8_t>& data) const {
    // Read encoded data:
    std::vector<std::array<uint8_t, 4>> dataBuffer;
    if (read_encoded_unsafe(dataBuffer) <= 0) {
        return false;
    }

    // Decode all:
    for (const std::array<uint8_t, 4>& buffer : dataBuffer) {
        data.push_back(decode(buffer));
    }
    return true;
}

bool JuttaConnection::write_decoded_unsafe(const uint8_t& byte) const {
    return write_encoded_unsafe(encode(byte));
}

bool JuttaConnection::write_decoded_unsafe(const std::vector<uint8_t>& data) const {
    // Bad compiler support:
    // return std::ranges::all_of(data.begin(), data.end(), [this](uint8_t byte) { return write_decoded_unsafe(byte); });
    // So we use this until it gets better:
    bool result = true;
    for (uint8_t byte : data) {
        if (!write_decoded_unsafe(byte)) {
            result = false;
        }
    }
    return result;
}

bool JuttaConnection::write_decoded_unsafe(const std::string& data) const {
    // Bad compiler support:
    // return std::ranges::all_of(data.begin(), data.end(), [this](char c) { return write_decoded_unsafe(static_cast<uint8_t>(c)); });
    // So we use this until it gets better:
    bool result = true;
    for (char c : data) {
        if (!write_decoded_unsafe(static_cast<uint8_t>(c))) {
            result = false;
        }
    }
    return result;
}

bool JuttaConnection::write_decoded(const uint8_t& byte) {
    actionLock.lock();
    bool result = write_decoded_unsafe(byte);
    actionLock.unlock();
    return result;
}

bool JuttaConnection::write_decoded(const std::vector<uint8_t>& data) {
    actionLock.lock();
    bool result = write_decoded_unsafe(data);
    actionLock.unlock();
    return result;
}

bool JuttaConnection::write_decoded(const std::string& data) {
    actionLock.lock();
    bool result = write_decoded_unsafe(data);
    actionLock.unlock();
    return result;
}

void JuttaConnection::print_byte(const uint8_t& byte) {
    for (size_t i = 0; i < 8; i++) {
        SPDLOG_INFO("{} ", ((byte >> (7 - i)) & 0b00000001));
    }
    // printf("-> %d\t%02x\t%c", byte, byte, byte);
    printf("-> %d\t%02x", byte, byte);
}

void JuttaConnection::print_bytes(const std::vector<uint8_t>& data) {
    for (const uint8_t& byte : data) {
        print_byte(byte);
    }
}

void JuttaConnection::run_encode_decode_test() {
    bool success = true;

    for (uint16_t i = 0b00000000; i <= 0b11111111; i++) {
        if (i != decode(encode(i))) {
            success = false;
            SPDLOG_ERROR("data:");
            print_byte(i);

            std::array<uint8_t, 4> dataEnc = encode(i);
            for (size_t i = 0; i < 4; i++) {
                SPDLOG_ERROR("dataEnc[{}]", i);
                print_byte(dataEnc.at(i));
            }

            uint8_t dataDec = decode(dataEnc);
            SPDLOG_ERROR("dataDec:");
            print_byte(dataDec);
        }
    }
    // Flush the stdout to ensure the result gets printed when assert(success) fails:
    SPDLOG_INFO("Encode decode test: {}", success);
    assert(success);
}

std::array<uint8_t, 4> JuttaConnection::encode(const uint8_t& decData) {
    // 1111 0000 -> 0000 1111:
    uint8_t tmp = ((decData & 0xF0) >> 4) | ((decData & 0x0F) << 4);

    // 1100 1100 -> 0011 0011:
    tmp = ((tmp & 0xC0) >> 2) | ((tmp & 0x30) << 2) | ((tmp & 0x0C) >> 2) | ((tmp & 0x03) << 2);

    // The base bit layout for all send bytes:
    constexpr uint8_t BASE = 0b01011011;

    std::array<uint8_t, 4> encData{};
    encData[0] = BASE | ((tmp & 0b10000000) >> 2);
    encData[0] |= ((tmp & 0b01000000) >> 4);

    encData[1] = BASE | (tmp & 0b00100000);
    encData[1] |= ((tmp & 0b00010000) >> 2);

    encData[2] = BASE | ((tmp & 0b00001000) << 2);
    encData[2] |= (tmp & 0b00000100);

    encData[3] = BASE | ((tmp & 0b00000010) << 4);
    encData[3] |= ((tmp & 0b00000001) << 2);

    return encData;
}

uint8_t JuttaConnection::decode(const std::array<uint8_t, 4>& encData) {
    // Bit mask for the 2. bit from the left:
    constexpr uint8_t B2_MASK = (0b10000000 >> 2);
    // Bit mask for the 5. bit from the left:
    constexpr uint8_t B5_MASK = (0b10000000 >> 5);

    uint8_t decData = 0;
    decData |= (encData[0] & B2_MASK) << 2;
    decData |= (encData[0] & B5_MASK) << 4;

    decData |= (encData[1] & B2_MASK);
    decData |= (encData[1] & B5_MASK) << 2;

    decData |= (encData[2] & B2_MASK) >> 2;
    decData |= (encData[2] & B5_MASK);

    decData |= (encData[3] & B2_MASK) >> 4;
    decData |= (encData[3] & B5_MASK) >> 2;

    // 1111 0000 -> 0000 1111:
    decData = ((decData & 0xF0) >> 4) | ((decData & 0x0F) << 4);

    // 1100 1100 -> 0011 0011:
    decData = ((decData & 0xC0) >> 2) | ((decData & 0x30) << 2) | ((decData & 0x0C) >> 2) | ((decData & 0x03) << 2);

    return decData;
}

bool JuttaConnection::write_encoded_unsafe(const std::array<uint8_t, 4>& encData) const {
    bool result = serial.write_serial(encData);
    serial.flush();
    std::this_thread::sleep_for(std::chrono::milliseconds{8});
    return result;
}

bool JuttaConnection::read_encoded_unsafe(std::array<uint8_t, 4>& buffer) const {
    size_t size = serial.read_serial(buffer);
    if (size <= 0 || size > 4) {
        SPDLOG_TRACE("No serial data found.");
        return false;
    }
    if (size < 4) {
        SPDLOG_WARN("Invalid amount of UART data found ({} byte) - ignoring.", size);
        return false;
    }
    SPDLOG_TRACE("Read 4 encoded bytes.");
    return true;
}

size_t JuttaConnection::read_encoded_unsafe(std::vector<std::array<uint8_t, 4>>& data) const {
    while (true) {
        std::array<uint8_t, 4> buffer{};
        if (!read_encoded_unsafe(buffer)) {
            // Wait 100 ms for the next bunch of data to arrive:
            std::this_thread::sleep_for(std::chrono::milliseconds{100});
            if (!read_encoded_unsafe(buffer)) {
                break;
            }
        }
        data.push_back(buffer);
    }
    return data.size();
}

bool JuttaConnection::wait_for_ok(const std::chrono::milliseconds& timeout) {
    actionLock.lock();
    bool result = wait_for_response_unsafe("ok:\r\n", timeout);
    actionLock.unlock();
    return result;
}

std::shared_ptr<std::string> JuttaConnection::write_decoded_with_response(const std::vector<uint8_t>& data, const std::chrono::milliseconds& timeout) {
    std::shared_ptr<std::string> result{nullptr};
    actionLock.lock();
    if (write_decoded_unsafe(data)) {
        result = wait_for_str_unsafe(timeout);
    }
    actionLock.unlock();
    return result;
}

std::shared_ptr<std::string> JuttaConnection::write_decoded_with_response(const std::string& data, const std::chrono::milliseconds& timeout) {
    std::shared_ptr<std::string> result{nullptr};
    actionLock.lock();
    if (write_decoded_unsafe(data)) {
        result = wait_for_str_unsafe(timeout);
    }
    actionLock.unlock();
    return result;
}

std::shared_ptr<std::string> JuttaConnection::wait_for_str_unsafe(const std::chrono::milliseconds& timeout) const {
    std::shared_ptr<std::string> result{nullptr};
    std::vector<uint8_t> buffer;
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    // NOLINTNEXTLINE (hicpp-use-nullptr, modernize-use-nullptr)
    while ((timeout.count() <= 0) || ((std::chrono::steady_clock::now() - start) < timeout)) {
        if (read_decoded_unsafe(buffer)) {
            result = std::make_shared<std::string>(vec_to_string(buffer));
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds{250});
    }
    return result;
}

bool JuttaConnection::wait_for_response_unsafe(const std::string& response, const std::chrono::milliseconds& timeout) const {
    std::vector<uint8_t> buffer;
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    // NOLINTNEXTLINE (hicpp-use-nullptr, modernize-use-nullptr)
    while ((timeout.count() <= 0) || ((std::chrono::steady_clock::now() - start) < timeout)) {
        if (read_decoded_unsafe(buffer)) {
            for (size_t i = 0; (buffer.size() >= response.size()) && (i < buffer.size() - (response.size() - 1)); i++) {
                bool success = true;
                for (size_t e = 0; e < response.size(); e++) {
                    if (static_cast<char>(buffer[i + e]) != response[e]) {
                        success = false;
                        break;
                    }
                }
                if (success) {
                    return true;
                }
            }
            buffer.clear();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds{250});
    }
    return false;
}

bool JuttaConnection::write_decoded_wait_for(const std::vector<uint8_t>& data, const std::string& response, const std::chrono::milliseconds& timeout) {
    actionLock.lock();
    bool result = write_decoded_unsafe(data);
    if (result) {
        result = wait_for_response_unsafe(response, timeout);
    }
    actionLock.unlock();
    return result;
}

std::string JuttaConnection::vec_to_string(const std::vector<uint8_t>& data) {
    if (data.empty()) {
        return "";
    }

    std::ostringstream sstream;
    for (unsigned char i : data) {
        sstream << static_cast<char>(i);
    }
    return sstream.str();
}

//---------------------------------------------------------------------------
}  // namespace jutta_proto
//---------------------------------------------------------------------------
