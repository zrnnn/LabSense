#pragma once
#include <cstddef>
#include <cstdint>
namespace lab {
inline bool allow_ble_setup(bool ready, bool opened, bool connected, uint32_t elapsed) {
  return ready && opened && !connected && elapsed < 600000UL;
}
inline bool valid_wifi_lengths(size_t ssid, size_t password) {
  return ssid >= 1 && ssid <= 32 && (password == 0 || (password >= 8 && password <= 63));
}
inline bool allow_pairing(bool active, bool connected, bool saved_network, bool physically_opened) {
  return active && !connected && (!saved_network || physically_opened);
}
}  // namespace lab
