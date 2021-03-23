#include "jutta_proto/CoffeeMaker.hpp"

#include <cassert>
#include <iomanip>
#include <limits>
#include <string>
#include <thread>
#include "logger/Logger.hpp"

#include "jutta_proto/JuttaCommands.hpp"

//---------------------------------------------------------------------------
namespace jutta_proto {
//---------------------------------------------------------------------------
CoffeeMaker::CoffeeMaker() : connection("/dev/ttyS1") {}

// NOLINTNEXTLINE (readability-convert-member-functions-to-static)
void CoffeeMaker::init() {
    SPDLOG_INFO("Initializing coffee maker...");
    connection.init();
    SPDLOG_INFO("Coffee maker initialized.");
}

void CoffeeMaker::switch_page() {
    press_button(jutta_button_t::BUTTON_6);
    if (++pageNum >= NUM_PAGES) {
        pageNum = 0;
    }
}

void CoffeeMaker::switch_page(size_t pageNum) {
    if (this->pageNum == pageNum) {
        return;
    }

    press_button(jutta_button_t::BUTTON_6);
    if (++pageNum >= NUM_PAGES) {
        pageNum = 0;
    }
    switch_page(pageNum);
}

void CoffeeMaker::brew_coffee(coffee_t coffee) {
    assert(!locked);
    locked = true;

    size_t pageNum = get_page_num(coffee);
    switch_page(pageNum);
    jutta_button_t button = get_button_num(coffee);
    press_button(button);
    locked = false;
}

size_t CoffeeMaker::get_page_num(coffee_t coffee) const {
    for (const std::pair<const coffee_t, size_t>& c : coffee_page_map) {
        if (c.first == coffee) {
            return c.second;
        }
    }
    assert(false);  // Should not happen
    return std::numeric_limits<size_t>::max();
}

CoffeeMaker::jutta_button_t CoffeeMaker::get_button_num(coffee_t coffee) const {
    for (const std::pair<const coffee_t, jutta_button_t>& c : coffee_button_map) {
        if (c.first == coffee) {
            return c.second;
        }
    }
    assert(false);  // Should not happen
    return jutta_button_t::BUTTON_6;
}

void CoffeeMaker::press_button(jutta_button_t button) const {
    switch (button) {
        case jutta_button_t::BUTTON_1:
            static_cast<void>(write_and_wait(JUTTA_BUTTON_1));
            break;

        case jutta_button_t::BUTTON_2:
            static_cast<void>(write_and_wait(JUTTA_BUTTON_2));
            break;

        case jutta_button_t::BUTTON_3:
            static_cast<void>(write_and_wait(JUTTA_BUTTON_3));
            break;

        case jutta_button_t::BUTTON_4:
            static_cast<void>(write_and_wait(JUTTA_BUTTON_4));
            break;

        case jutta_button_t::BUTTON_5:
            static_cast<void>(write_and_wait(JUTTA_BUTTON_5));
            break;

        case jutta_button_t::BUTTON_6:
            static_cast<void>(write_and_wait(JUTTA_BUTTON_6));
            break;

        default:
            assert(false);  // Should not happen
            break;
    }

    // Give the coffee maker time to react:
    std::this_thread::sleep_for(std::chrono::milliseconds{500});
}

void CoffeeMaker::brew_custom_coffee(const std::chrono::milliseconds& grindTime, const std::chrono::milliseconds& waterTime) {
    assert(!locked);
    locked = true;
    SPDLOG_INFO("Brewing custom coffee with {} ms grind time and {} ms ms water time...", std::to_string(grindTime.count()), std::to_string(waterTime.count()));

    // Grind:
    SPDLOG_INFO("Custom coffee grinding...");
    static_cast<void>(write_and_wait(JUTTA_GRINDER_ON));
    std::this_thread::sleep_for(grindTime);
    static_cast<void>(write_and_wait(JUTTA_GRINDER_OFF));
    static_cast<void>(write_and_wait(JUTTA_BREW_GROUP_TO_BREWING_POSITION));

    // Compress:
    SPDLOG_INFO("Custom coffee compressing...");
    static_cast<void>(write_and_wait(JUTTA_COFFEE_PRESS_ON));
    std::this_thread::sleep_for(std::chrono::milliseconds{500});
    static_cast<void>(write_and_wait(JUTTA_COFFEE_PRESS_OFF));

    // Brew step 1:
    SPDLOG_INFO("Custom coffee brewing...");
    static_cast<void>(write_and_wait(JUTTA_COFFEE_WATER_PUMP_ON));
    std::this_thread::sleep_for(std::chrono::milliseconds{2000});
    static_cast<void>(write_and_wait(JUTTA_COFFEE_WATER_PUMP_OFF));
    std::this_thread::sleep_for(std::chrono::milliseconds{2000});

    // Brew setp 2:
    pump_hot_water(waterTime);

    // Reset:
    SPDLOG_INFO("Custom coffee finishing up...");
    static_cast<void>(write_and_wait(JUTTA_BREW_GROUP_RESET));
    SPDLOG_INFO("Custom coffee done.");

    locked = false;
}

bool CoffeeMaker::write_and_wait(const std::string& s) const {
    static_cast<void>(connection.write_decoded(s));
    return connection.wait_for_ok();
}

void CoffeeMaker::pump_hot_water(const std::chrono::milliseconds& waterTime) const {
    static_cast<void>(write_and_wait(JUTTA_COFFEE_WATER_PUMP_ON));
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now() + waterTime;
    // NOLINTNEXTLINE (hicpp-use-nullptr, modernize-use-nullptr)
    while (std::chrono::steady_clock::now() < end) {
        static_cast<void>(write_and_wait(JUTTA_COFFEE_WATER_HEATER_ON));
        SPDLOG_INFO("Heater turned on.");
        std::this_thread::sleep_for(waterTime / 8);
        static_cast<void>(write_and_wait(JUTTA_COFFEE_WATER_HEATER_OFF));
        SPDLOG_INFO("Heater turned off.");
        std::this_thread::sleep_for(waterTime / 20);
    }
    static_cast<void>(write_and_wait(JUTTA_COFFEE_WATER_PUMP_OFF));
}

bool CoffeeMaker::is_locked() const { return locked; }

//---------------------------------------------------------------------------
}  // namespace jutta_proto
//---------------------------------------------------------------------------
