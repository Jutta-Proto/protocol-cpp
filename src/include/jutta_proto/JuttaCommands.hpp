#pragma once

#include <string>

//---------------------------------------------------------------------------
namespace jutta_proto {
//---------------------------------------------------------------------------
const std::string JUTTA_POWER_OFF = "AN:01\r\n";
const std::string JUTTA_TEST_MODE_ON = "AN:20\r\n";
const std::string JUTTA_TEST_MODE_OFF = "AN:21\r\n";

const std::string JUTTA_GET_TYPE = "TY:\r\n";

const std::string JUTTA_BUTTON_1 = "FA:04\r\n";
const std::string JUTTA_BUTTON_2 = "FA:05\r\n";
const std::string JUTTA_BUTTON_3 = "FA:06\r\n";
const std::string JUTTA_BUTTON_4 = "FA:07\r\n";
const std::string JUTTA_BUTTON_5 = "FA:08\r\n";
const std::string JUTTA_BUTTON_6 = "FA:09\r\n";

const std::string JUTTA_BREW_GROUP_TO_BREWING_POSITION = "FN:22\r\n";
const std::string JUTTA_BREW_GROUP_RESET = "FN:0D\r\n";

const std::string JUTTA_GRINDER_ON = "FN:07\r\n";
const std::string JUTTA_GRINDER_OFF = "FN:08\r\n";
const std::string JUTTA_COFFEE_PRESS_ON = "FN:0B\r\n";
const std::string JUTTA_COFFEE_PRESS_OFF = "FN:0C\r\n";
const std::string JUTTA_COFFEE_WATER_HEATER_ON = "FN:03\r\n";
const std::string JUTTA_COFFEE_WATER_HEATER_OFF = "FN:04\r\n";
const std::string JUTTA_COFFEE_WATER_PUMP_ON = "FN:01\r\n";
const std::string JUTTA_COFFEE_WATER_PUMP_OFF = "FN:02\r\n";
//---------------------------------------------------------------------------
}  // namespace jutta_proto
//---------------------------------------------------------------------------
