#pragma once
#include "oled_ui.h"
#include "esphome/components/display/display.h"
#include "esphome/components/font/font.h"
namespace lab::ui {
struct OledCanvas {
  esphome::display::Display &d;
  esphome::font::Font *small,*label,*value,*compact,*climate;
  void text(int x,int y,Font f,Align a,const char *s,bool white=true) {
    auto *font=f==Font::SMALL?small:f==Font::LABEL?label:f==Font::VALUE?value:f==Font::COMPACT?compact:climate;
    const auto align=a==Align::LEFT?esphome::display::TextAlign::TOP_LEFT:a==Align::RIGHT?esphome::display::TextAlign::TOP_RIGHT:esphome::display::TextAlign::TOP_CENTER;
    d.print(x,y,font,white?esphome::display::COLOR_ON:esphome::display::COLOR_OFF,align,s);
  }
  void line(int x,int y,int x2,int y2){d.line(x,y,x2,y2);}
  void circle(int x,int y,int r){d.circle(x,y,r);}
  void fill_circle(int x,int y,int r){d.filled_circle(x,y,r);}
  void fill_rect(int x,int y,int w,int h){if(w>0&&h>0)d.filled_rectangle(x,y,w,h);}
};
}  // namespace lab::ui
