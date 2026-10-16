#include "captive_portal.h"
#ifdef USE_CAPTIVE_PORTAL
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/core/helpers.h"
#include "esphome/core/string_ref.h"
#include "esphome/components/wifi/scan_list.h"
#include "esphome/components/wifi/wifi_component.h"
#include "esphome/components/api/api_server.h"
#include "esphome/core/alloc_helpers.h"
#ifdef USE_PROVISIONING
#include "esphome/components/provisioning/provisioning.h"
#endif
#include "captive_index.h"
#include "runtime_pairing.h"
#include "display_settings.h"
#include "esphome/components/esp32_improv/esp32_improv_component.h"

namespace esphome::captive_portal {

static const char *const TAG = "captive_portal";

void CaptivePortal::request_scan() {
  if (!this->scanning_.load()) this->scan_requested_.store(true);
}
void CaptivePortal::on_wifi_scan_results(const wifi::wifi_scan_vector_t<wifi::WiFiScanResult> &results) {
  this->scanning_.store(false);
  this->scan_error_.store(false);
  this->scan_revision_.fetch_add(1);
  ESP_LOGI(TAG, "Wi-Fi scan completed: %u network records", unsigned(results.size()));
}
void CaptivePortal::process_scan() {
  auto *wifi=wifi::global_wifi_component;
  const uint32_t now=millis();
  if (this->scanning_.load() && uint32_t(now-this->scan_started_)>15000) {
    this->scanning_.store(false);
    this->scan_error_.store(true);
    this->scan_revision_.fetch_add(1);
    ESP_LOGW(TAG, "Wi-Fi scan did not complete");
  }
  if (!this->scan_requested_.load() || this->scanning_.load() || !this->active_ ||
      wifi->is_connected() || wifi->is_disabled() ||
      (this->last_scan_ && uint32_t(now-this->last_scan_)<15000) ||
      (this->credentials_submitted_ && uint32_t(now-this->credentials_submitted_)<45000)) return;
  auto *ble=esp32_improv::global_improv_component;
  if (ble && ble->get_improv_state()==improv::STATE_PROVISIONING) return;
  this->scan_requested_.store(false);
  this->scanning_.store(true);
  this->scan_error_.store(false);
  this->scan_started_=this->last_scan_=now;
  wifi->start_scanning();
}

void CaptivePortal::handle_config(AsyncWebServerRequest *request) {
  AsyncResponseStream *stream = request->beginResponseStream(ESPHOME_F("application/json"));
  stream->addHeader(ESPHOME_F("cache-control"), ESPHOME_F("no-store"));
  char mac_s[MAC_ADDRESS_PRETTY_BUFFER_SIZE];
  const char *mac_str = get_mac_address_pretty_into_buffer(mac_s);
#ifdef USE_ESP8266
  stream->print(ESPHOME_F("{\"mac\":\""));
  stream->print(mac_str);
  stream->print(ESPHOME_F("\",\"name\":\""));
  stream->print(App.get_name().c_str());
  stream->print(ESPHOME_F("\",\"aps\":[{}"));
#else
  stream->printf(R"({"mac":"%s","name":"%s","aps":[{})", mac_str, App.get_name().c_str());
#endif

  // An SSID can contain a " or \ that would break the JSON, so escape it before writing it out. An SSID is at most
  // 32 bytes (IEEE 802.11), so this is large enough that nothing is ever dropped. Reused for every scan result.
  char escaped_ssid[32 * JSON_ESCAPE_MAX_EXPANSION + 1];
  {
    // Invariant: only bounded in-memory work under the lock; the network send
    // happens later in request->send()
    wifi::ScanResultsLock lock(wifi::global_wifi_component);
    const auto &results = wifi::global_wifi_component->get_scan_result();
    for (const auto &scan : results) {
      bool with_auth = false;
      if (!wifi::should_show_scan_entry(results, scan, with_auth))
        continue;

      json_escape_into_buffer(escaped_ssid, scan.get_ssid());
#ifdef USE_ESP8266
      stream->print(ESPHOME_F(",{\"ssid\":\""));
      stream->print(escaped_ssid);
      stream->print(ESPHOME_F("\",\"rssi\":"));
      stream->print(scan.get_rssi());
      stream->print(ESPHOME_F(",\"lock\":"));
      stream->print(with_auth);
      stream->print(ESPHOME_F("}"));
#else
      stream->printf(R"(,{"ssid":"%s","rssi":%d,"lock":%d})", escaped_ssid, scan.get_rssi(), with_auth);
#endif
    }
  }
  stream->printf(R"(],"display_rotation":%d,"scan":{"revision":%u,"pending":%s,"error":%s}})",
    lab::display_rotation().load(),
    unsigned(this->scan_revision_.load()),
    (this->scan_requested_.load() || this->scanning_.load()) ? "true" : "false",
    this->scan_error_.load() ? "true" : "false");
  request->send(stream);
}
void CaptivePortal::handle_wifisave(AsyncWebServerRequest *request) {
  const auto &ssid = request->arg("ssid");
  const auto &psk = request->arg("psk");
  if (request->method()!=HTTP_POST || !request->hasHeader("X-LabSense-Setup") || !lab::security_ready() ||
      !lab::key_acknowledged().load() || !lab::valid_wifi_lengths(ssid.size(),psk.size())) {
    request->send(400, ESPHOME_F("text/plain"), ESPHOME_F("Check network name and password"));
    return;
  }
  this->defer([this]() { this->credentials_submitted_=millis(); });
  ESP_LOGI(TAG, "Wi-Fi settings received");
#ifdef USE_ESP8266
  // ESP8266 is single-threaded, call directly
  wifi::global_wifi_component->save_wifi_sta(ssid.c_str(), psk.c_str());
#else
  // Defer save to main loop thread to avoid NVS operations from HTTP thread
  this->defer([ssid, psk]() { wifi::global_wifi_component->save_wifi_sta(ssid.c_str(), psk.c_str()); });
#endif
  lab::pairing_consumed().store(true);
  lab::physical_pairing().store(false);
  this->defer([]() { lab::persist_recovery(false); });
  request->send(200, ESPHOME_F("text/plain"), ESPHOME_F("Saved. Connecting..."));
}

void CaptivePortal::handle_pairing_confirm(AsyncWebServerRequest *request) {
  if (request->method()!=HTTP_POST || !request->hasHeader("X-LabSense-Setup") ||
      lab::pairing_consumed().load() || !lab::allow_pairing(this->active_,
      wifi::global_wifi_component->is_connected(),wifi::global_wifi_component->has_sta(),
      lab::physical_pairing().load())) {
    request->send(403, ESPHOME_F("text/plain"), ESPHOME_F("Open device setup again"));
    return;
  }
  auto *server=api::global_api_server;
  noise::psk_t key{};
  if (!server || !server->get_noise_ctx().has_psk()) {
    request->send(503, ESPHOME_F("text/plain"), ESPHOME_F("Key not ready")); return;
  }
  server->get_noise_ctx().load_psk(key);
  if (request->arg("key")!=base64_encode(key.data(),key.size())) {
    request->send(400, ESPHOME_F("text/plain"), ESPHOME_F("Save the displayed key first")); return;
  }
  lab::key_acknowledged().store(true);
  request->send(200, ESPHOME_F("text/plain"), ESPHOME_F("Ready"));
}

void CaptivePortal::handle_pairing(AsyncWebServerRequest *request) {
  // The secret is shown only during initial setup on the local setup hotspot.
  // As soon as Wi-Fi credentials are saved, this route stops exposing it.
  if (lab::pairing_consumed().load() || !lab::allow_pairing(this->active_, wifi::global_wifi_component->is_connected(),
      wifi::global_wifi_component->has_sta(), lab::physical_pairing().load())) {
    request->send(403, ESPHOME_F("text/plain"), ESPHOME_F("Unavailable"));
    return;
  }
  auto *server = api::global_api_server;
  if (server == nullptr || !server->get_noise_ctx().has_psk()) {
    request->send(503, ESPHOME_F("text/plain"), ESPHOME_F("Key not ready"));
    return;
  }
  noise::psk_t key{};
  server->get_noise_ctx().load_psk(key);
  const auto encoded = base64_encode(key.data(), key.size());
  auto *response = request->beginResponseStream(ESPHOME_F("application/json"));
  response->addHeader(ESPHOME_F("Cache-Control"), ESPHOME_F("no-store"));
  response->print(ESPHOME_F("{\"key\":\""));
  response->print(encoded.c_str());
  response->print(ESPHOME_F("\"}"));
  request->send(response);
}

void CaptivePortal::setup() {
  // Disable loop by default - will be enabled when captive portal starts
  this->disable_loop();
#ifdef USE_PROVISIONING
  // The captive portal is a provisioning surface: once the provisioning window
  // has closed, stop serving it. WiFi's own closed-callback shuts down the
  // access point the portal runs on, and the gated fallback in WiFiComponent's
  // loop() ensures neither is started again afterwards.
  if (provisioning::global_provisioning_manager != nullptr) {
    provisioning::global_provisioning_manager->add_on_closed_callback([this]() {
      if (this->active_) {
        ESP_LOGD(TAG, "Provisioning window closed; stopping captive portal");
        this->end();
      }
    });
  }
#endif
}
void CaptivePortal::start() {
  this->base_->init();
  if (!this->initialized_) {
    this->base_->add_handler_without_auth(this);
  }

  network::IPAddress ip = wifi::global_wifi_component->wifi_soft_ap_ip();

#if defined(USE_ESP32)
  // Create DNS server instance for ESP-IDF
  this->dns_server_ = make_unique<DNSServer>();
  this->dns_server_->start(ip);
#elif defined(USE_ARDUINO)
  this->dns_server_ = make_unique<DNSServer>();
  this->dns_server_->setErrorReplyCode(DNSReplyCode::NoError);
  this->dns_server_->start(53, ESPHOME_F("*"), ip);
#endif

  this->initialized_ = true;
  this->active_ = true;

  // Enable loop() now that captive portal is active
  this->enable_loop();

  this->set_timeout("initial-scan", 1500, [this]() { this->request_scan(); });
  ESP_LOGV(TAG, "Captive portal started");
}

void CaptivePortal::handleRequest(AsyncWebServerRequest *req) {
#ifdef USE_ESP32
  char url_buf[AsyncWebServerRequest::URL_BUF_SIZE];
  StringRef url = req->url_to(url_buf);
#else
  const auto &url = req->url();
#endif
  if (url == ESPHOME_F("/scan")) {
    if (req->method()!=HTTP_POST || !req->hasHeader("X-LabSense-Setup")) {
      req->send(400, ESPHOME_F("text/plain"), ESPHOME_F("Invalid request"));
      return;
    }
    this->request_scan();
    req->send(202, ESPHOME_F("text/plain"), ESPHOME_F("Scan requested"));
    return;
  }
  if (url == ESPHOME_F("/display")) {
    if (req->method()!=HTTP_POST || !req->hasHeader("X-LabSense-Setup") || !this->active_) {
      req->send(400, ESPHOME_F("text/plain"), ESPHOME_F("Invalid request"));return;
    }
    const auto &value=req->arg("rotation");
    const int rotation=value=="0"?0:value=="90"?90:value=="180"?180:value=="270"?270:-1;
    if (!lab::valid_rotation(rotation)) {
      req->send(400, ESPHOME_F("text/plain"), ESPHOME_F("Invalid orientation"));return;
    }
    lab::requested_rotation().store(rotation);
    req->send(202, ESPHOME_F("text/plain"), ESPHOME_F("Updating display"));return;
  }
  if (url == ESPHOME_F("/config.json")) {
    this->handle_config(req);
    return;
  } else if (url == ESPHOME_F("/pairing/confirm")) {
    this->handle_pairing_confirm(req);
    return;
  } else if (url == ESPHOME_F("/pairing.json")) {
    this->handle_pairing(req);
    return;
  } else if (url == ESPHOME_F("/wifisave")) {
    this->handle_wifisave(req);
    return;
  }

  // All other requests get the captive portal page
  // This includes OS captive portal detection endpoints which will trigger
  // the captive portal when they don't receive their expected responses
#ifndef USE_ESP8266
  auto *response = req->beginResponse(200, ESPHOME_F("text/html"), INDEX_GZ, sizeof(INDEX_GZ));
#else
  auto *response = req->beginResponse_P(200, ESPHOME_F("text/html"), INDEX_GZ, sizeof(INDEX_GZ));
#endif
#ifdef USE_CAPTIVE_PORTAL_GZIP
  response->addHeader(ESPHOME_F("Content-Encoding"), ESPHOME_F("gzip"));
#else
  response->addHeader(ESPHOME_F("Content-Encoding"), ESPHOME_F("br"));
#endif
  // Local UI: do not cache setup pages across firmware updates.
  response->addHeader(ESPHOME_F("Cache-Control"), ESPHOME_F("no-store"));
  req->send(response);
}

CaptivePortal::CaptivePortal(web_server_base::WebServerBase *base) : base_(base) { global_captive_portal = this; }
float CaptivePortal::get_setup_priority() const {
  // Before WiFi
  return setup_priority::WIFI + 1.0f;
}
void CaptivePortal::dump_config() { ESP_LOGCONFIG(TAG, "Captive Portal:"); }

CaptivePortal *global_captive_portal = nullptr;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

}  // namespace esphome::captive_portal

#endif
