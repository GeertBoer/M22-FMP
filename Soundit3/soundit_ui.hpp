#pragma once

#include <U8g2lib.h>

#include <Wire.h>

#include <SD.h>

#include <string>
#include <vector>
#include <algorithm>
#include <math.h>

#include <Bounce.h>

#define ONELINE 15

#include "micicon.hpp"
#include "volume_icon.hpp"

using string = std::string;
using strVec = std::vector<string>;

class SounditUI {
private:
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C* u8g2;

  float mapf(float x, float in_min, float in_max, float out_min, float out_max);

public:
  void Clear();
  void Write();
  void draw_message_screen(std::string message, bool selected);
  void draw_message_screen_blocking(std::string message, Bounce& button);
  void draw_sample_nr(int samplenr);
  void draw_text(std::string text);
  void draw_plus();
  void draw_mic_icon(std::string text, float position);
  void draw_speaker_icon(std::string text, int position);

  void overlay_fader(float position);
  void overlay_text_bottom(std::string text);
  void overlay_text_top(std::string text);
  void overlay_play_icon();
  void overlay_rec_icon();

  SounditUI();
};


float SounditUI::mapf(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void SounditUI::overlay_fader(float position) {
  if (position > 1.0) {
    position = 1.0;
  } else if (position < 0.0) {
    position = 0.0;
  }

  int faderpos = (int)((float)128 * position);

  u8g2->setDrawColor(2);
  u8g2->drawBox(0, 0, faderpos, 64);

  u8g2->sendBuffer();
}

void SounditUI::Clear() {
  u8g2->clearBuffer();
}

void SounditUI::Write() {
  u8g2->sendBuffer();
}

void SounditUI::overlay_rec_icon() {
  u8g2->setFontMode(1);
  u8g2->setBitmapMode(1);
  u8g2->setDrawColor(1);
  u8g2->drawFilledEllipse(10, 10, 7, 7);
  u8g2->setDrawColor(0);
  u8g2->setFont(u8g2_font_6x10_tr);
  u8g2->drawStr(8, 14, "R");

  u8g2->setBitmapMode(0);
}

void SounditUI::overlay_play_icon() {
  u8g2->setFontMode(1);
  u8g2->setDrawColor(2);
  u8g2->setBitmapMode(1);

  u8g2->drawLine(4, 3, 4, 17);
  u8g2->drawLine(5, 4, 5, 16);
  u8g2->drawLine(6, 5, 6, 15);
  u8g2->drawLine(7, 6, 7, 14);
  u8g2->drawLine(8, 7, 8, 13);
  u8g2->drawLine(9, 8, 9, 12);
  u8g2->drawLine(10, 9, 10, 11);
  u8g2->drawPixel(11, 10);

  u8g2->setBitmapMode(0);
}

void SounditUI::overlay_text_bottom(std::string text) {
  u8g2->setDrawColor(1);
  u8g2->setFont(u8g2_font_crox2h_tf);  //Font height = 20px, so centering vertically = (64-20)/2 = 22px
  int width = u8g2->getUTF8Width(text.c_str());
  int margin = (128 - width) / 2;
  u8g2->drawStr(margin, 60, text.c_str());
}

void SounditUI::overlay_text_top(std::string text) {
  u8g2->setDrawColor(1);
  u8g2->setFont(u8g2_font_crox2h_tf);  //Font height = 20px, so centering vertically = (64-20)/2 = 22px
  int width = u8g2->getUTF8Width(text.c_str());
  int margin = (128 - width) / 2;
  u8g2->drawStr(margin, 12, text.c_str());
}

void SounditUI::draw_mic_icon(std::string text, float position) {
  u8g2->clearBuffer();
  u8g2->setDrawColor(1);
  u8g2->drawXBM(0, 0, 128, 64, microphone_20gain_20adjust_bits);

  u8g2->setFont(u8g2_font_crox2h_tf);  //Font height = 20px, so centering vertically = (64-20)/2 = 22px
  int width = u8g2->getUTF8Width(text.c_str());
  int margin = (128 - width) / 2;
  u8g2->drawStr(margin, 62, text.c_str());

  overlay_fader(position);
}

void SounditUI::draw_speaker_icon(std::string text, int position) {
  u8g2->clearBuffer();
  u8g2->setDrawColor(1);
  u8g2->drawXBM(0, 0, 128, 64, volume_icon_bits);

  u8g2->setFont(u8g2_font_crox2h_tf);  //Font height = 20px, so centering vertically = (64-20)/2 = 22px
  int width = u8g2->getUTF8Width(text.c_str());
  int margin = (128 - width) / 2;
  u8g2->drawStr(margin, 62, text.c_str());

  float position_f = mapf((float)position, 0, 100, 0.0, 1.0);

  overlay_fader(position_f);
}

void SounditUI::draw_sample_nr(int samplenr) {
  u8g2->setDrawColor(1);
  u8g2->setFont(u8g2_font_fur20_tf);  //Font height = 20px, so centering vertically = (64-20)/2 = 22px
  int width = u8g2->getUTF8Width(String(samplenr).c_str());
  int margin = (128 - width) / 2;
  u8g2->drawStr(margin, 42, String(samplenr).c_str());
}

void SounditUI::draw_text(std::string text) {
  u8g2->setFont(u8g2_font_fur20_tf);  //Font height = 20px, so centering vertically = (64-20)/2 = 22px
  int width = u8g2->getUTF8Width(text.c_str());
  int margin = (128 - width) / 2;
  u8g2->drawStr(margin, 42, text.c_str());
}

void SounditUI::draw_plus() {
  u8g2->setFontMode(1);
  u8g2->setBitmapMode(1);
  u8g2->setDrawColor(1);
  u8g2->drawFilledEllipse(62, 30, 14, 14);
  u8g2->setDrawColor(0);
  u8g2->drawLine(62, 21, 62, 39);
  u8g2->drawLine(53, 30, 71, 30);
  u8g2->drawLine(53, 29, 71, 29);
  u8g2->drawLine(53, 31, 71, 31);
  u8g2->drawLine(61, 21, 61, 39);
  u8g2->drawLine(63, 21, 63, 39);
  u8g2->drawLine(61, 29, 61, 31);
  u8g2->drawLine(63, 29, 63, 31);
  u8g2->drawLine(62, 29, 62, 31);
  u8g2->setDrawColor(1);
  u8g2->setFont(u8g2_font_6x10_tr);
  u8g2->drawStr(4, 57, "Create new recording");
}


SounditUI::SounditUI() {
  // u8g2 = new U8G2_SSD1306_128X64_NONAME_F_SW_I2C(U8G2_R2, screen_scl, screen_sda, U8X8_PIN_NONE);
  u8g2 = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R2);

  u8g2->begin();
  u8g2->setFont(u8g2_font_helvR08_tf);  // regular

  while (!SD.begin()) {
    u8g2->clearBuffer();
    u8g2->drawStr(23, 35, "Please insert SD");
    u8g2->sendBuffer();
  }

  Serial.println("Finished UI instantiation");
}

void SounditUI::draw_message_screen(std::string message, bool selected) {
  u8g2->clearBuffer();

  u8g2->setFont(u8g2_font_helvB08_tf);  //bold
  u8g2->drawStr(8, ONELINE + 4, message.c_str());

  u8g2->setFont(u8g2_font_helvR08_tf);  // regular
  int x = 82;
  int y = 45;
  if (!selected) {
    u8g2->drawButtonUTF8(x, y, U8G2_BTN_BW2 | U8G2_BTN_INV | U8G2_BTN_XFRAME, 0, 2, 2, "OK");
  } else {
    u8g2->drawButtonUTF8(x, y, U8G2_BTN_BW2, 0, 2, 2, "OK");
  }

  u8g2->sendBuffer();
}


void SounditUI::draw_message_screen_blocking(std::string message, Bounce& button) {
  draw_message_screen(message, false);
  do {
    button.update();
  } while (!button.fallingEdge());
  draw_message_screen(message, true);
  delay(100);
}
