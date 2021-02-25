#pragma once

#include <chrono>
#include <map>
#include <string>
#include <vector>

#include "JuttaConnection.hpp"

//---------------------------------------------------------------------------
namespace jutta {
//---------------------------------------------------------------------------
class CoffeeMaker {
 public:
    /**
     * All available coffee types.
     **/
    enum coffee_t { ESPRESSO = 0,
                    COFFEE = 1,
                    CAPPUCCINO = 2,
                    MILK_FOAM = 3,
                    CAFFE_BARISTA = 4,
                    LUNGO_BARISTA = 5,
                    ESPRESSO_DOPPIO = 6,
                    MACCHIATO = 7 };
    enum jutta_button_t {
        BUTTON_1 = 1,
        BUTTON_2 = 2,
        BUTTON_3 = 3,
        BUTTON_4 = 4,
        BUTTON_5 = 5,
        BUTTON_6 = 6,
    };

    JuttaConnection connection;

 private:
    static constexpr size_t NUM_PAGES = 2;
    /**
     * Mapping of all coffee types to page.
     **/
    std::map<coffee_t, size_t> coffee_page_map{{coffee_t::ESPRESSO, 0}, {coffee_t::COFFEE, 0}, {coffee_t::CAPPUCCINO, 0}, {coffee_t::MILK_FOAM, 0}, {coffee_t::CAFFE_BARISTA, 1}, {coffee_t::LUNGO_BARISTA, 1}, {coffee_t::ESPRESSO_DOPPIO, 1}, {coffee_t::MACCHIATO, 1}};
    /**
     * Mapping of all coffee types to their button.
     **/
    std::map<coffee_t, jutta_button_t> coffee_button_map{{coffee_t::ESPRESSO, jutta_button_t::BUTTON_1}, {coffee_t::COFFEE, jutta_button_t::BUTTON_2}, {coffee_t::CAPPUCCINO, jutta_button_t::BUTTON_4}, {coffee_t::MILK_FOAM, jutta_button_t::BUTTON_5}, {coffee_t::CAFFE_BARISTA, jutta_button_t::BUTTON_1}, {coffee_t::LUNGO_BARISTA, jutta_button_t::BUTTON_2}, {coffee_t::ESPRESSO_DOPPIO, jutta_button_t::BUTTON_4}, {coffee_t::MACCHIATO, jutta_button_t::BUTTON_5}};

    /**
     * The current page we are on.
     **/
    size_t pageNum{0};

    /**
     * True in case we are currently making a something like a cup of coffee. 
     **/
    bool locked{false};

 public:
    CoffeeMaker();

    /**
     * Initializes UART and coffee maker connection.
     **/
    void init();
    /**
     * Switches to the next page.
     * 0 -> 1
     * 1 -> 0
     **/
    void switch_page();
    /**
     * Switches to the given page number.
     * Does nothing, in case the page number is the same as the current one.
     **/
    void switch_page(size_t pageNum);
    /**
     * Brews the given coffee and switches to the appropriate page for this.
     **/
    void brew_coffee(coffee_t coffee);
    /**
     * Brews a custom coffee with the given grind and water times.
     * A default coffee on a JUTTA E6 (2019) grinds for 3.6 seconds and then lets the water run for 40 seconds (200 ml).
     * This corresponds to a water flow rate of 5 ml/s.
     **/
    void brew_custom_coffee(const std::chrono::milliseconds& grindTime = std::chrono::milliseconds{3600}, const std::chrono::milliseconds& waterTime = std::chrono::milliseconds{40000});
    /**
     * Simulates a button press of the given button.
     **/
    void press_button(jutta_button_t button);

    /**
     * Returns true in case the coffee maker is locked due to it currently interacting with the coffee maker e.g. brewing a coffee.
     **/
    [[nodiscard]] bool is_locked() const;

 private:
    /**
     * Returns the page number for the given coffee type.
     **/
    size_t get_page_num(coffee_t coffee);

    /**
     * Returns the button number for the given coffee type.
     **/
    jutta_button_t get_button_num(coffee_t coffee);
    /**
     * Writes the given string to the coffee maker and waits for an "ok:\r\n"
     **/
    bool write_and_wait(const std::string& s);
    /**
     * Turns on the water pump and heater for the given amount of time.
     **/
    void pump_hot_water(const std::chrono::milliseconds& waterTime);
};
//---------------------------------------------------------------------------
}  // namespace jutta
//---------------------------------------------------------------------------
