#pragma once

#include <array>
#include <chrono>
#include <string>
#include <vector>

#include "serial/SerialConnection.hpp"

//---------------------------------------------------------------------------
namespace jutta_proto {
//---------------------------------------------------------------------------
class JuttaConnection {
 private:
    serial::SerialConnection serial;

 public:
    /**
     * Initializes a new Jutta (UART) connection.
     **/
    explicit JuttaConnection(const std::string& device);

    /**
     * Tries to read a single decoded byte.
     * This requires reading 4 JUTTA bytes and converting them to a single actual data byte.
     * The result will be stored in the given "byte" pointer.
     * Returns true on success.
     **/
    [[nodiscard]] bool read_decoded(uint8_t* byte) const;
    /**
     * Reads as many data bytes, as there are availabel.
     * Each data byte consists of 4 JUTTA bytes which will be decoded into a single data byte.
     **/
    [[nodiscard]] bool read_decoded(std::vector<uint8_t>& data) const;
    /**
     * Waits until the coffee maker responded with a "ok:\r\n".
     * The default timeout for this operation is 5 seconds.
     * To disable the timeout, set the timeout to 0 seconds.
     * Returns true on success.
     * Returns false when a timeout occurred.
     **/
    [[nodiscard]] bool wait_for_ok(const std::chrono::milliseconds& timeout = std::chrono::milliseconds{5000}) const;

    /**
     * Encodes the given byte into 4 JUTTA bytes and writes them to the coffee maker.
     **/
    [[nodiscard]] bool write_decoded(const uint8_t& byte) const;
    /**
     * Encodes each byte of the given bytes into 4 JUTTA bytes and writes them to the coffee maker.
     **/
    [[nodiscard]] bool write_decoded(const std::vector<uint8_t>& data) const;
    /**
     * Encodes each character into 4 JUTTA bytes and writes them to the coffee maker.
     *
     * An example call could look like: write_decoded("TY:\r\n");
     * This would request the device type from the coffee maker.
     **/
    [[nodiscard]] bool write_decoded(const std::string& data) const;

    /**
     * Helper function used for debugging.
     * Prints the given byte in binary, hex and as a char.
     * Does not append a new line at the end!
     *
     * Example output:
     * 0 1 0 1 0 1 0 0 -> 84	54	T
     **/
    static void print_byte(const uint8_t& byte);
    /**
     * Prints each byte in the given vector in binary, hex and as a char
     *
     * Example output:
     * 0 1 0 1 0 1 0 0 -> 84	54	T
     * 0 1 0 1 1 0 0 1 -> 89	59	Y
     * 0 0 1 1 1 0 1 0 -> 58	3a	:
     * 0 0 0 0 1 1 0 1 -> 13	0d
     * 0 0 0 0 1 0 1 0 -> 10	0a
     **/
    static void print_bytes(const std::vector<uint8_t>& data);

    /**
     * Runs the encode and decode test.
     * Ensures encoding and decoding is reversable.
     * Should be run at least once per session to ensure proper functionality.
     **/
    static void run_encode_decode_test();

 private:
    /**
     * Encodes the given byte into four bytes that the coffee maker understands.
     * Based on: http://protocoljura.wiki-site.com/index.php/Protocol_to_coffeemaker
     *
     * A full documentation of the process can be found here:
     * https://github.com/COM8/esp32-jutta#deobfuscating
     **/
    static std::array<uint8_t, 4> encode(const uint8_t& decData);
    /**
     * Decodes the given four bytes read from the coffee maker into on byte.
     * Based on: http://protocoljura.wiki-site.com/index.php/Protocol_to_coffeemaker
     *
     * A full documentation of the process can be found here:
     * https://github.com/COM8/esp32-jutta#deobfuscating
     **/
    static uint8_t decode(const std::array<uint8_t, 4>& encData);
    /**
     * Writes four bytes of encoded data to the coffee maker and then waits 8ms.
     **/
    [[nodiscard]] bool write_encoded(const std::array<uint8_t, 4>& encData) const;
    /**
     * Reads four bytes of encoded data which represent one byte of actual data.
     * Returns true on success.
     **/
    [[nodiscard]] bool read_encoded(std::array<uint8_t, 4>& buffer) const;
    /**
     * Reads multiples of four bytes. Every four bytes represent one actual byte.
     * Returns the number of 4 byte tuples read.
     **/
    [[nodiscard]] size_t read_encoded(std::vector<std::array<uint8_t, 4>>& data) const;
};
//---------------------------------------------------------------------------
}  // namespace jutta_proto
//---------------------------------------------------------------------------
