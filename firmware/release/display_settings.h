#pragma once
#include <atomic>
#include "esphome/core/preferences.h"
namespace lab {
inline bool valid_rotation(int degrees) { return degrees==0 || degrees==90 || degrees==180 || degrees==270; }
inline const char *rotation_name(int degrees) {
  switch(degrees) {case 90:return "USB bottom";case 180:return "USB right";case 270:return "USB top";default:return "USB left";}
}
inline std::atomic<int> &display_rotation() { static std::atomic<int> value{0};return value; }
inline std::atomic<int> &requested_rotation() { static std::atomic<int> value{-1};return value; }
inline void load_display_settings() {
  int value=0;
  esphome::global_preferences->make_preference<int>(0x4C53524FUL,true).load(&value);
  display_rotation().store(valid_rotation(value)?value:0);
}
// Called only from the main loop; HTTP queues a request instead of writing NVS.
inline bool save_display_rotation(int degrees) {
  if(!valid_rotation(degrees))return false;
  if(display_rotation().load()==degrees)return true;
  auto pref=esphome::global_preferences->make_preference<int>(0x4C53524FUL,true);
  if(!pref.save(&degrees) || !esphome::global_preferences->sync())return false;
  display_rotation().store(degrees);return true;
}
} // namespace lab
