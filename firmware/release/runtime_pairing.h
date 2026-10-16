#pragma once
#include <atomic>
#include "esp_random.h"
#include "nvs.h"
#include "bootloader_random.h"
#include "esphome/components/api/api_server.h"
#include "esphome/components/wifi/wifi_component.h"
#include "esphome/core/preferences.h"
#include "esphome/core/helpers.h"
#include "esphome/core/application.h"
#include "commissioning_policy.h"
namespace lab {
inline std::atomic<bool> &key_acknowledged() { static std::atomic<bool> value{false}; return value; }
inline std::atomic<bool> &ble_window() { static std::atomic<bool> value{false}; return value; }
inline std::atomic<bool> &recovery_armed() { static std::atomic<bool> value{false}; return value; }
inline std::atomic<bool> &physical_pairing() { static std::atomic<bool> value{false}; return value; }
inline std::atomic<bool> &security_ready() { static std::atomic<bool> value{false}; return value; }
inline std::atomic<bool> &pairing_consumed() { static std::atomic<bool> value{false}; return value; }
inline bool ble_setup_allowed() {
  if (esphome::millis() >= 600000UL) ble_window().store(false);
  return allow_ble_setup(security_ready().load(),ble_window().load(),
      esphome::wifi::global_wifi_component->is_connected(),esphome::millis());
}
inline bool persist_recovery(bool enabled) {
  auto pref=esphome::global_preferences->make_preference<bool>(0x4C535243UL,true);
  return pref.save(&enabled) && esphome::global_preferences->sync();
}
inline bool erase_saved_wifi() {
  // ESPHome 2026.9.1 key for runtime Wi-Fi (no YAML station entries).
  // Delete the record: an empty SSID is a wildcard, not a safe reset value.
  if (!esphome::global_preferences->sync()) return false;
  const auto handle=esphome::global_preferences->nvs_handle;
  for (const char *key : {"88491487", "88491488"}) {
    const auto result=nvs_erase_key(handle,key);
    if (result!=ESP_OK && result!=ESP_ERR_NVS_NOT_FOUND) return false;
  }
  return nvs_commit(handle)==ESP_OK;
}
inline void request_recovery() {
  if (!persist_recovery(true)) return;
  if (!erase_saved_wifi()) return;
  esphome::App.safe_reboot();
}
inline void configure_setup_ap() {
  bool recovery=false;
  esphome::global_preferences->make_preference<bool>(0x4C535243UL,true).load(&recovery);
  physical_pairing().store(recovery);
  if (recovery && !erase_saved_wifi()) {
    esphome::wifi::global_wifi_component->set_enable_on_boot(false);
    return;
  }
  struct SetupSecret { uint32_t version; char password[9]; } secret{};
  auto pref=esphome::global_preferences->make_preference<SetupSecret>(0x4C534150UL,true);
  bool valid=pref.load(&secret) && secret.version==1 && secret.password[8]=='\0';
  for (int i=0;valid && i<8;i++) valid=secret.password[i]>='0' && secret.password[i]<='9';
  if (!valid) {
    // This hook runs before BLE/Wi-Fi/ADC starts. Enable the IDF entropy source
    // only while generating this independently random commissioning password.
    bootloader_random_enable();
    uint32_t random=esp_random();
    bootloader_random_disable();
    secret.version=1;
    snprintf(secret.password,sizeof(secret.password),"%08u",unsigned(random%100000000UL));
    if (!pref.save(&secret) || !esphome::global_preferences->sync()) {
      esphome::wifi::global_wifi_component->set_enable_on_boot(false);
      return;  // No networking if credentials cannot be persisted.
    }
  }
  auto *wifi=esphome::wifi::global_wifi_component;
  auto ap=wifi->get_ap();
  const auto mac=esphome::get_mac_address();
  ap.set_ssid("LabSense "+mac.substr(mac.size()-6));
  ap.set_password(secret.password);ap.set_hidden(false);wifi->set_ap(ap);
  security_ready()=true;
}
inline void ensure_pairing_key() {
  auto *server=esphome::api::global_api_server;
  if (!security_ready() || server==nullptr) return;
  if (server->get_noise_ctx().has_psk()) return;
  esphome::noise::psk_t key{};
  esp_fill_random(key.data(),key.size()); // Wi-Fi is now active: hardware entropy is available.
  if (esphome::noise::NoiseContext::is_all_zeros(key) || !server->save_noise_psk(key,true) ||
      !esphome::global_preferences->sync()) {
    security_ready()=false;
    esphome::wifi::global_wifi_component->disable();
  }
}
} // namespace lab
