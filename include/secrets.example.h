#pragma once
#include <stdint.h>

// Copy to secrets.h and fill in local lab settings. Never put real values here.
namespace local_config {
inline constexpr char kWifiSsid[] = "";
inline constexpr char kWifiPassword[] = "";
inline constexpr char kMqttHost[] = "";
inline constexpr uint16_t kMqttPort = 1883;
inline constexpr char kMqttUsername[] = "";
inline constexpr char kMqttPassword[] = "";
inline constexpr char kTelemetryTopic[] = "";
inline constexpr char kLedCommandTopic[] = "";
inline constexpr char kStudentName[] = "";
inline constexpr char kStudentId[] = "";
}  // namespace local_config
