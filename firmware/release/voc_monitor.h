#pragma once
#include <cmath>
#include <cstdint>
#include <limits>

// Device-independent logic. Thresholds are user-defined technical indicators,
// not occupational exposure limits and never a declaration that air is safe.
namespace lab {
enum class AirState { WARMUP, SENSOR_FAULT, COMPENSATION_FAULT, CONFIG_FAULT, READY };

struct Timer {
  bool active{false};
  uint32_t since{0};
  void reset() { active = false; }
  bool held(bool condition, uint32_t now, uint32_t duration) {
    if (!condition) { reset(); return false; }
    if (!active) { active = true; since = now; }
    return uint32_t(now - since) >= duration;
  }
};

class VocMonitor {
 public:
  void begin(uint32_t warmup_ms) { warmup_ms_ = warmup_ms; }
  void configure(float warning, float high, uint32_t warn_delay=120000,
                 uint32_t high_delay=30000, uint32_t clear_delay=300000) {
    config_ok_ = std::isfinite(warning) && std::isfinite(high) && warning > 0 &&
                 high > warning && high <= 32768 && warn_delay > 0 && high_delay > 0 && clear_delay > 0;
    if (warning != warning_ || high != high_ || warn_delay != warn_delay_ || high_delay != high_delay_ || clear_delay != clear_delay_)
      reset_timers_();
    warning_ = warning; high_ = high; warn_delay_ = warn_delay;
    high_delay_ = high_delay; clear_delay_ = clear_delay;
  }
  void sample(uint32_t now, float value) {
    sample_valid_ = std::isfinite(value) && value >= 0 && value <= 32768;
    if (!sample_valid_) return;
    if (!conditioning_started_) { conditioning_started_=true; conditioning_start_=now; }
    last_sample_=now; seen_=true; value_=value;
  }
  void settle(uint32_t now, uint32_t ms) { settling_=true; settle_start_=now; settle_ms_=ms; reset_timers_(); }
  bool fresh(uint32_t now) const { return seen_ && sample_valid_ && uint32_t(now-last_sample_) <= 15000; }
  uint32_t age(uint32_t now) const { return seen_ ? uint32_t(now-last_sample_) : UINT32_MAX; }
  uint32_t conditioning_elapsed(uint32_t now) const {
    return conditioning_started_ ? uint32_t(now-conditioning_start_) : 0;
  }
  uint32_t remaining(uint32_t now) const {
    uint32_t base=0;
    if (!warmed_) { uint32_t elapsed=conditioning_elapsed(now); base=elapsed<warmup_ms_ ? warmup_ms_-elapsed : 0; }
    uint32_t extra=0;
    if (settling_) { uint32_t elapsed=uint32_t(now-settle_start_); extra=elapsed<settle_ms_ ? settle_ms_-elapsed : 0; }
    return base>extra ? base : extra;
  }
  AirState tick(uint32_t now, bool hardware_ok, bool compensation_ok, bool baseline_ok=true) {
    if (!fresh(now) || !hardware_ok || !baseline_ok) {
      // A failed/stale gas source cannot leave a pending timer counting.
      reset_timers_();
      if (!fresh(now) || !hardware_ok) { conditioning_started_=false; warmed_=false; }
      state_=AirState::SENSOR_FAULT; return state_;
    }
    if (!warmed_ && conditioning_elapsed(now)>=warmup_ms_) warmed_=true;
    if (settling_ && uint32_t(now-settle_start_)>=settle_ms_) settling_=false;
    if (!compensation_ok) { reset_timers_(); state_=AirState::COMPENSATION_FAULT; return state_; }
    if (!config_ok_) { reset_timers_(); state_=AirState::CONFIG_FAULT; return state_; }
    if (remaining(now)>0) { reset_timers_(); state_=AirState::WARMUP; return state_; }
    state_=AirState::READY;
    if (warn_on_.held(value_>=warning_,now,warn_delay_)) warning_active_=true;
    if (high_on_.held(value_>=high_,now,high_delay_)) { high_active_=true; warning_active_=true; }
    if (high_off_.held(high_active_ && value_<high_*0.8f,now,clear_delay_)) high_active_=false;
    if (warn_off_.held(warning_active_ && !high_active_ && value_<warning_*0.8f,now,clear_delay_)) warning_active_=false;
    return state_;
  }
  bool ready() const { return state_==AirState::READY; }
  bool warning() const { return warning_active_; }
  bool high() const { return high_active_; }
  float value() const { return value_; }
  AirState state() const { return state_; }
  const char *label() const {
    switch (state_) {
      case AirState::SENSOR_FAULT:return "VOC sensor fault / no data";
      case AirState::COMPENSATION_FAULT:return "Temperature/humidity compensation missing";
      case AirState::CONFIG_FAULT:return "Invalid indication thresholds";
      case AirState::WARMUP:return "VOC warm-up";
      default:return high_active_ ? "VOC high" : warning_active_ ? "VOC elevated" : "Below VOC indication threshold";
    }
  }
 private:
  void reset_timers_() { warn_on_.reset();high_on_.reset();warn_off_.reset();high_off_.reset(); }
  uint32_t warmup_ms_{3600000},conditioning_start_{0},last_sample_{0},settle_start_{0},settle_ms_{0};
  uint32_t warn_delay_{120000},high_delay_{30000},clear_delay_{300000};
  bool conditioning_started_{false},warmed_{false},settling_{false},seen_{false},sample_valid_{false};
  bool config_ok_{true},warning_active_{false},high_active_{false};
  float value_{NAN},warning_{500},high_{1500};
  AirState state_{AirState::SENSOR_FAULT};
  Timer warn_on_,high_on_,warn_off_,high_off_;
};

// Access from ESPHome lambdas without declaring a custom C++ type in globals
// (user includes are emitted after ESPHome's generated global declarations).
inline VocMonitor &monitor() { static VocMonitor instance; return instance; }
}  // namespace lab
