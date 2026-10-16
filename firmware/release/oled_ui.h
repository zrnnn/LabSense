#pragma once
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
namespace lab::ui {
constexpr uint32_t LOGO_MS=2400, BOOT_MS=7000;
enum class Screen { LOGO, CHECKS, SETUP, RELEASE_BUTTON, READINGS, BLANK };
enum class Air { LOW, ELEVATED, HIGH, WARMUP, SENSOR_FAULT, COMPENSATION_FAULT, CONFIG_FAULT };
enum class Font { SMALL, LABEL, VALUE, COMPACT, CLIMATE };
enum class Align { LEFT, CENTER, RIGHT };
struct Model {
  uint64_t now{0},started{0};
  uint32_t warmup_ms{0};
  bool portrait{false},display_enabled{true},recovery_armed{false},setup{false},key_saved{false};
  bool wifi{false},api{false},ap{false},bme_id_ok{false},ccs_id_ok{false},bme_failed{false},ccs_failed{false},gas_fresh{false};
  Air air{Air::WARMUP};
  float voc{NAN},temperature{NAN},humidity{NAN};
  const char *ssid{"LabSense"},*password{""};
};
inline uint64_t age(const Model &m) { return m.now-m.started; }
inline bool fault(Air a) { return a==Air::SENSOR_FAULT || a==Air::COMPENSATION_FAULT || a==Air::CONFIG_FAULT; }
inline Screen screen(const Model &m) {
  if(m.recovery_armed)return Screen::RELEASE_BUTTON;
  if(m.air==Air::HIGH)return Screen::READINGS;
  if(age(m)<LOGO_MS)return Screen::LOGO;
  if(age(m)<BOOT_MS)return Screen::CHECKS;
  if(m.setup)return Screen::SETUP;
  if(!m.display_enabled)return Screen::BLANK;
  return Screen::READINGS;
}
// The status reacts to current values as well as sustained, latched warnings.
inline Air assessed_air(bool ready, bool sensor_fault, bool compensation_fault, bool config_fault,
                        bool high_latched, bool warning_latched,float value,float warning,float high) {
  if(sensor_fault)return Air::SENSOR_FAULT;
  if(compensation_fault)return Air::COMPENSATION_FAULT;
  if(config_fault)return Air::CONFIG_FAULT;
  if(!ready)return Air::WARMUP;
  if(!std::isfinite(value) || !std::isfinite(warning) || !std::isfinite(high) || warning<=0 || high<=warning)
    return Air::CONFIG_FAULT;
  if(high_latched || value>=high)return Air::HIGH;
  if(warning_latched || value>=warning)return Air::ELEVATED;
  return Air::LOW;
}
template<class C> void render_portrait(C &c,const Model &m,Screen page) {
  char text[48];
  if(page==Screen::BLANK)return;
  if(page==Screen::RELEASE_BUTTON) {
    c.text(32,38,Font::SMALL,Align::CENTER,"Release");
    c.text(32,51,Font::LABEL,Align::CENTER,"BOOT");
    c.text(32,76,Font::SMALL,Align::CENTER,"for setup");return;
  }
  if(page==Screen::LOGO) {
    c.text(32,26,Font::SMALL,Align::CENTER,"LabSense");
    c.text(32,53,Font::SMALL,Align::CENTER,"STARTING");
    for(int i=0;i<7;i++)c.fill_circle(11+i*7,88,int((age(m)/180)%7)==i?2:1);
    return;
  }
  if(page==Screen::CHECKS) {
    c.text(32,0,Font::SMALL,Align::CENTER,"STARTUP");
    const char *labels[]={"BME280","CCS811","Wi-Fi","API"};
    const char *values[]={m.bme_id_ok&&!m.bme_failed?"DETECTED":"FAULT",
      !m.ccs_id_ok||m.ccs_failed?"FAULT":m.gas_fresh?"WARM-UP":"STARTING",
      m.wifi?"ONLINE":m.ap?"SETUP":"WAITING",m.api?"ONLINE":"WAITING"};
    for(int i=0;i<4;i++) {
      c.text(4,17+i*27,Font::SMALL,Align::LEFT,labels[i]);
      c.text(4,29+i*27,Font::SMALL,Align::LEFT,values[i]);
    }return;
  }
  if(page==Screen::SETUP) {
    c.text(32,0,Font::LABEL,Align::CENTER,"SETUP");
    c.text(32,23,Font::SMALL,Align::CENTER,"LabSense");
    const char *suffix=std::strrchr(m.ssid,' ');
    c.text(32,35,Font::SMALL,Align::CENTER,suffix?suffix+1:"");
    c.text(32,52,Font::SMALL,Align::CENTER,"Password");
    c.text(32,65,Font::SMALL,Align::CENTER,m.password);
    c.text(32,84,Font::SMALL,Align::CENTER,"192.168.4.1");
    c.text(32,102,Font::SMALL,Align::CENTER,m.key_saved?"Connect":"Save key");return;
  }
  c.text(4,3,Font::SMALL,Align::LEFT,"VOC");c.text(60,3,Font::SMALL,Align::RIGHT,"ppb");
  const bool numeric=m.air==Air::LOW||m.air==Air::ELEVATED||m.air==Air::HIGH;
  if(numeric&&std::isfinite(m.voc))snprintf(text,sizeof(text),"%.0f",m.voc);else snprintf(text,sizeof(text),"---");
  const bool compact=numeric&&m.voc>=1000;
  c.text(32,compact?21:10,compact?Font::CLIMATE:Font::COMPACT,Align::CENTER,text);
  const char *status=nullptr,*detail=nullptr;
  switch(m.air) {
    case Air::HIGH:status="VOC HIGH";detail="VENTILATE";break;
    case Air::ELEVATED:status="ELEVATED";break;
    case Air::SENSOR_FAULT:status="VOC FAULT";detail="CHECK";break;
    case Air::COMPENSATION_FAULT:status="BME FAULT";detail="CHECK";break;
    case Air::CONFIG_FAULT:status="SETTINGS";detail="CHECK";break;
    case Air::WARMUP: {
      status="WARM-UP";const uint32_t mins=(m.warmup_ms+59999UL)/60000UL;
      if(mins>=60)snprintf(text,sizeof(text),"%luh",static_cast<unsigned long>((mins+59)/60));
      else snprintf(text,sizeof(text),"%lum",static_cast<unsigned long>(mins));
      detail=text;break;
    }
    default:break;
  }
  if(m.air==Air::HIGH)c.fill_rect(2,47,60,25);
  if(status)c.text(32,47,Font::SMALL,Align::CENTER,status,m.air!=Air::HIGH);
  if(detail)c.text(32,60,Font::SMALL,Align::CENTER,detail,m.air!=Air::HIGH);
  c.line(4,75,59,75);
  c.text(5,83,Font::SMALL,Align::LEFT,"T");
  if(std::isfinite(m.temperature))snprintf(text,sizeof(text),"%.0f",m.temperature);else snprintf(text,sizeof(text),"--");
  c.text(43,79,Font::CLIMATE,Align::RIGHT,text);c.text(47,83,Font::SMALL,Align::LEFT,"°C");
  c.text(4,107,Font::SMALL,Align::LEFT,"RH");
  if(std::isfinite(m.humidity))snprintf(text,sizeof(text),"%.0f",m.humidity);else snprintf(text,sizeof(text),"--");
  c.text(49,103,Font::CLIMATE,Align::RIGHT,text);c.text(54,107,Font::SMALL,Align::LEFT,"%");
}
template<class C> void render(C &c,const Model &m) {
  const auto page=screen(m);char text[48];
  if(m.portrait){render_portrait(c,m,page);return;}
  if(page==Screen::BLANK)return;
  if(page==Screen::RELEASE_BUTTON) {
    c.text(64,15,Font::SMALL,Align::CENTER,"Release BOOT");
    c.text(64,34,Font::SMALL,Align::CENTER,"to open setup");return;
  }
  if(page==Screen::LOGO) {
    c.text(64,14,Font::LABEL,Align::CENTER,"LabSense");
    c.text(64,37,Font::SMALL,Align::CENTER,"STARTING");
    for(int i=0;i<7;i++) {const int r=int((age(m)/180)%7)==i?3:1;c.fill_circle(28+i*12,57,r);}return;
  }
  if(page==Screen::CHECKS) {
    c.text(3,0,Font::SMALL,Align::LEFT,"SYSTEM CHECK");
    const char *labels[]={"BME280","CCS811","Wi-Fi","API"};
    const char *values[]={m.bme_id_ok&&!m.bme_failed?"DETECTED":"FAULT",
      !m.ccs_id_ok||m.ccs_failed?"FAULT":m.gas_fresh?"WARM-UP":"STARTING",
      m.wifi?"CONNECTED":m.ap?"SETUP":"CONNECTING",m.api?"CONNECTED":"WAITING"};
    for(int i=0;i<4;i++){c.text(3,15+i*12,Font::SMALL,Align::LEFT,labels[i]);c.text(125,15+i*12,Font::SMALL,Align::RIGHT,values[i]);}return;
  }
  if(page==Screen::SETUP) {
    c.text(0,0,Font::SMALL,Align::LEFT,m.key_saved?"BLE / WI-FI SETUP":"SAVE HA KEY FIRST");
    c.text(0,13,Font::SMALL,Align::LEFT,m.ssid);
    snprintf(text,sizeof(text),"Pass: %s",m.password);c.text(0,26,Font::SMALL,Align::LEFT,text);
    c.text(0,39,Font::SMALL,Align::LEFT,"192.168.4.1");
    c.text(0,52,Font::SMALL,Align::LEFT,"Join with phone");return;
  }
  // Fixed instrument layout: all three measurements stay visible.
  c.text(0,5,Font::SMALL,Align::LEFT,"VOC");
  c.text(0,17,Font::SMALL,Align::LEFT,"ppb");
  const bool numeric=m.air==Air::LOW||m.air==Air::ELEVATED||m.air==Air::HIGH;
  if(numeric&&std::isfinite(m.voc))snprintf(text,sizeof(text),"%.0f",m.voc);
  else snprintf(text,sizeof(text),"---");
  const bool compact=numeric&&m.voc>=10000;
  c.text(128,compact?1:-9,compact?Font::COMPACT:Font::VALUE,Align::RIGHT,text);

  const char *status=nullptr;
  switch(m.air) {
    case Air::HIGH:status="VOC HIGH - VENTILATE";break;
    case Air::ELEVATED:status="VOC ELEVATED";break;
    case Air::SENSOR_FAULT:status="VOC SENSOR FAULT";break;
    case Air::COMPENSATION_FAULT:status="BME DATA MISSING";break;
    case Air::CONFIG_FAULT:status="CHECK THRESHOLDS";break;
    case Air::WARMUP: {
      const uint32_t mins=(m.warmup_ms+59999UL)/60000UL;
      if(mins>=60)snprintf(text,sizeof(text),"WARM-UP %luh",static_cast<unsigned long>((mins+59)/60));
      else snprintf(text,sizeof(text),"WARM-UP %lum",static_cast<unsigned long>(mins));
      status=text;break;
    }
    default:break;
  }
  if(status) {
    if(m.air==Air::HIGH)c.fill_rect(0,34,128,10);
    c.text(64,33,Font::SMALL,Align::CENTER,status,m.air!=Air::HIGH);
  }
  c.text(0,52,Font::SMALL,Align::LEFT,"T");
  if(std::isfinite(m.temperature))snprintf(text,sizeof(text),"%.0f",m.temperature);
  else snprintf(text,sizeof(text),"--");
  c.text(14,45,Font::CLIMATE,Align::LEFT,text);
  c.text(43,52,Font::SMALL,Align::LEFT,"°C");
  c.text(66,52,Font::SMALL,Align::LEFT,"RH");
  if(std::isfinite(m.humidity))snprintf(text,sizeof(text),"%.0f",m.humidity);
  else snprintf(text,sizeof(text),"--");
  c.text(112,45,Font::CLIMATE,Align::RIGHT,text);
  c.text(117,52,Font::SMALL,Align::LEFT,"%");
}
}  // namespace lab::ui
