#pragma once
#include "esp_wifi.h"
#include "esp_err.h"
#include "esphome/core/log.h"
#include "esphome/components/captive_portal/captive_portal.h"
#include "esphome/components/wifi/wifi_component.h"

// Bounded boot diagnostics: report the radio's actual AP configuration, never
// home-network credentials or the API key. No radio state is changed here.
inline void lab_wifi_boot_report() {
  static unsigned count=0;
  if(count++>=6)return;
  auto *wifi=esphome::wifi::global_wifi_component;
  wifi_mode_t mode=WIFI_MODE_NULL;
  wifi_config_t config{};
  const auto mode_error=esp_wifi_get_mode(&mode);
  const auto config_error=esp_wifi_get_config(WIFI_IF_AP,&config);
  wifi_sta_list_t stations{};
  const auto stations_error=esp_wifi_ap_get_sta_list(&stations);
  ESP_LOGI("lab_wifi","mode=%d mode_result=%s AP_active=%s connected=%s STA_saved=%s portal=%s",
    int(mode),esp_err_to_name(mode_error),wifi->is_ap_active()?"yes":"no",
    wifi->is_connected()?"yes":"no",wifi->has_sta()?"yes":"no",
    esphome::captive_portal::global_captive_portal->is_active()?"yes":"no");
  if(config_error==ESP_OK)
    ESP_LOGI("lab_wifi","AP SSID=%.*s hidden=%u channel=%u clients=%d",
      int(config.ap.ssid_len?config.ap.ssid_len:32),config.ap.ssid,
      unsigned(config.ap.ssid_hidden),unsigned(config.ap.channel),
      stations_error==ESP_OK?int(stations.num):-1);
  else ESP_LOGW("lab_wifi","AP configuration unavailable: %s",esp_err_to_name(config_error));
}
