#pragma once
#include <stdint.h>

namespace board_config {
// External LED with a series resistor, not the board's RGB LED.
inline constexpr uint8_t kLedPin = 4;
// ESP32-C3 ADC1 inputs. Keep analog voltages within 0..3.3 V.
inline constexpr uint8_t kTemperatureAdcPin = 0;
inline constexpr uint8_t kHumidityAdcPin = 1;
inline constexpr uint32_t kPublishIntervalMs = 2000;
}  // namespace board_config
