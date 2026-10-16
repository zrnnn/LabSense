#pragma once
#include "esphome/core/defines.h"
#ifdef USE_CAPTIVE_PORTAL
#include <memory>
#include <atomic>
#include "esphome/components/wifi/wifi_component.h"
#if defined(USE_ESP32)
#include "dns_server_esp32_idf.h"
#elif defined(USE_ARDUINO)
#include <DNSServer.h>
#endif
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/preferences.h"
#include "esphome/components/web_server_base/web_server_base.h"

namespace esphome::captive_portal {

class CaptivePortal final : public AsyncWebHandler, public Component, public wifi::WiFiScanResultsListener {
 public:
  CaptivePortal(web_server_base::WebServerBase *base);
  void setup() override;
  void dump_config() override;
  void loop() override {
    this->process_scan();
#if defined(USE_ESP32)
    if (this->dns_server_ != nullptr) {
      this->dns_server_->process_next_request();
    }
#elif defined(USE_ARDUINO)
    if (this->dns_server_ != nullptr) {
      this->dns_server_->processNextRequest();
    }
#endif
  }
  float get_setup_priority() const override;
  void start();
  bool is_active() const { return this->active_; }
  void end() {
    this->active_ = false;
    this->disable_loop();  // Stop processing DNS requests
    this->base_->deinit();
    if (this->dns_server_ != nullptr) {
      this->dns_server_->stop();
      this->dns_server_ = nullptr;
    }
  }

  bool canHandle(AsyncWebServerRequest *request) const override {
    // Handle all GET requests when captive portal is active
    // This allows us to respond with the portal page for any URL,
    // triggering OS captive portal detection
    return this->active_ && (request->method() == HTTP_GET || request->method() == HTTP_POST);
  }

  void request_scan();
  void process_scan();
  void on_wifi_scan_results(const wifi::wifi_scan_vector_t<wifi::WiFiScanResult> &results) override;
  void handle_config(AsyncWebServerRequest *request);

  void handle_pairing_confirm(AsyncWebServerRequest *request);
  void handle_pairing(AsyncWebServerRequest *request);

  void handle_wifisave(AsyncWebServerRequest *request);

  void handleRequest(AsyncWebServerRequest *req) override;

 protected:
  web_server_base::WebServerBase *base_;
  std::atomic<bool> scan_requested_{false};
  std::atomic<bool> scanning_{false};
  std::atomic<bool> scan_error_{false};
  std::atomic<uint32_t> scan_revision_{0};
  uint32_t scan_started_{0};
  uint32_t last_scan_{0};
  uint32_t credentials_submitted_{0};
  bool initialized_{false};
  bool active_{false};
#if defined(USE_ARDUINO) || defined(USE_ESP32)
  std::unique_ptr<DNSServer> dns_server_{nullptr};
#endif
};

extern CaptivePortal *global_captive_portal;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

}  // namespace esphome::captive_portal

#endif
