#pragma once

#include <array>
#include <chrono>
#include <mutex>
#include <string>
#include <vector>

#include "serial/SerialConnection.hpp"

//---------------------------------------------------------------------------
namespace jutta_proto {
//---------------------------------------------------------------------------
class JuttaConnection {
 private:
    /**
     * Mutex that prevents multiple threads from accessing the serial connection at the same time.
     * Usefull, when using 'wait_for_ok()' to prevent other threads from manipulating the result.
     **/
    std::mutex actionLock{};
    serial::SerialConnection serial;

 public:
    /**
     * Initializes a new Jutta (UART) connection.
     **/
    explicit JuttaConnection(std::string&& device);

    /**
     * Tries to initializes the Jutta serial (UART) connection.
     * Throws a exception in case something goes wrong.
     * [Thread Safe]
     **/
    void init();

    /**
     * Tries to read a single decoded byte.
     * This requires reading 4 JUTTA bytes and converting them to a single actual data byte.
     * The result will be stored in the given "byte" pointer.
     * Returns true on success.
     * [Thread Safe]
     **/
    bool read_decoded(uint8_t* byte);
    /**
     * Reads as many data bytes, as there are availabel.
     * Each data byte consists of 4 JUTTA bytes which will be decoded into a single data byte.
     * [Thread Safe]
     **/
    bool read_decoded(std::vector<uint8_t>& data);
    /**
     * Waits until the coffee maker responded with a "ok:\r\n".
     * The default timeout for this operation is 5 seconds.
     * To disable the timeout, set the timeout to 0 seconds.
     * Returns true on success.
     * Returns false when a timeout occurred.
     * [Thread Safe]
     **/
    bool wait_for_ok(const std::chrono::milliseconds& timeout = std::chrono::milliseconds{5000});
    /**
     * Writes the given data to the coffee maker and then waits for the given response with an optional timeout.
     * The response has to include the "\r\n" at the end of a message.
     * The default timeout for this operation is 5 seconds.
     * To disable the timeout, set the timeout to 0 seconds.
     * Returns true on success.
     * Returns false when a timeout occurred or writing failed.
     * [Thread Safe]
     **/
    bool write_decoded_wait_for(const std::vector<uint8_t>& data, const std::string& response, const std::chrono::milliseconds& timeout = std::chrono::milliseconds{5000});
    /**
     * Writes the given data to the coffee maker and then waits for the given response with an optional timeout.
     * The response has to include the "\r\n" at the end of a message.
     * The default timeout for this operation is 5 seconds.
     * To disable the timeout, set the timeout to 0 seconds.
     * Returns true on success.
     * Returns false when a timeout occurred or writing failed.
     * [Thread Safe]
     **/
    bool write_decoded_wait_for(const std::string& data, const std::string& response, const std::chrono::milliseconds& timeout = std::chrono::milliseconds{5000});

    /**
     * Encodes the given byte into 4 JUTTA bytes and writes them to the coffee maker.
     * [Thread Safe]
     **/
    bool write_decoded(const uint8_t& byte);
    /**
     * Encodes each byte of the given bytes into 4 JUTTA bytes and writes them to the coffee maker.
     * [Thread Safe]
     **/
    bool write_decoded(const std::vector<uint8_t>& data);
    /**
     * Encodes each character into 4 JUTTA bytes and writes them to the coffee maker.
     *
     * An example call could look like: write_decoded("TY:\r\n");
     * This would request the device type from the coffee maker.
     * [Thread Safe]
     **/
    bool write_decoded(const std::string& data);

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

    /**
     * Converts the given binary vector to a string and returns it.
     **/
    static std::string vec_to_string(const std::vector<uint8_t>& data);

 private:
    /**
     * Encodes the given byte into four bytes that the coffee maker understands.
     * Based on: http://protocoljura.wiki-site.com/index.php/Protocol_to_coffeemaker
     *
     * A full documentation of the process can be found here:
     * https://github.com/Jutta-Proto/protocol-cpp#deobfuscating
     **/
    static std::array<uint8_t, 4> encode(const uint8_t& decData);
    /**
     * Decodes the given four bytes read from the coffee maker into on byte.
     * Based on: http://protocoljura.wiki-site.com/index.php/Protocol_to_coffeemaker
     *
     * A full documentation of the process can be found here:
     * https://github.com/Jutta-Proto/protocol-cpp#deobfuscating
     **/
    static uint8_t decode(const std::array<uint8_t, 4>& encData);
    /**
     * Writes four bytes of encoded data to the coffee maker and then waits 8ms.
     **/
    [[nodiscard]] bool write_encoded_unsafe(const std::array<uint8_t, 4>& encData) const;
    /**
     * Reads four bytes of encoded data which represent one byte of actual data.
     * Returns true on success.
     * Not thread safe!
     **/
    [[nodiscard]] bool read_encoded_unsafe(std::array<uint8_t, 4>& buffer) const;
    /**
     * Reads multiples of four bytes. Every four bytes represent one actual byte.
     * Returns the number of 4 byte tuples read.
     * Not thread safe!
     **/
    [[nodiscard]] size_t read_encoded_unsafe(std::vector<std::array<uint8_t, 4>>& data) const;
    /**
     * Tries to read a single decoded byte.
     * This requires reading 4 JUTTA bytes and converting them to a single actual data byte.
     * The result will be stored in the given "byte" pointer.
     * Returns true on success.
     * Not thread safe!
     **/
    [[nodiscard]] bool read_decoded_unsafe(uint8_t* byte) const;
    /**
     * Reads as many data bytes, as there are availabel.
     * Each data byte consists of 4 JUTTA bytes which will be decoded into a single data byte.
     * Not thread safe!
     **/
    [[nodiscard]] bool read_decoded_unsafe(std::vector<uint8_t>& data) const;

    /**
     * Encodes the given byte into 4 JUTTA bytes and writes them to the coffee maker.
     * Not thread safe!
     **/
    [[nodiscard]] bool write_decoded_unsafe(const uint8_t& byte) const;
    /**
     * Encodes each byte of the given bytes into 4 JUTTA bytes and writes them to the coffee maker.
     * Not thread safe!
     **/
    [[nodiscard]] bool write_decoded_unsafe(const std::vector<uint8_t>& data) const;
    /**
     * Encodes each character into 4 JUTTA bytes and writes them to the coffee maker.
     *
     * An example call could look like: write_decoded("TY:\r\n");
     * This would request the device type from the coffee maker.
     * Not thread safe!
     **/
    [[nodiscard]] bool write_decoded_unsafe(const std::string& data) const;

    /**
     * Waits until the coffee maker responded with the given response.
     * The response has to include the "\r\n" at the end of a message.
     * The default timeout for this operation is 5 seconds.
     * To disable the timeout, set the timeout to 0 seconds.
     * Returns true on success.
     * Returns false when a timeout occurred.
     * Not thread safe!
     **/
    [[nodiscard]] bool wait_for_response_unsafe(const std::string& response, const std::chrono::milliseconds& timeout = std::chrono::milliseconds{5000}) const;
};
//---------------------------------------------------------------------------
}  // namespace jutta_proto
//---------------------------------------------------------------------------
