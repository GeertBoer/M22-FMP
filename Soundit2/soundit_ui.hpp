#pragma once

#include <U8g2lib.h>
#include <SimpleRotary.h>
#include <Wire.h>

#include <SD.h>

#include <string>
#include <vector>
#include <algorithm>
#include <math.h>

#include <Bounce.h>

#define ONELINE 15

using string = std::string;
using strVec = std::vector<string>;

class SounditUI {
private:
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C* u8g2;

public:
  void draw_message_screen(std::string message, bool selected);
  void draw_message_screen_blocking(std::string message, Bounce& button);
  void draw_sample_nr(int samplenr);
  void draw_plus(); 

  SounditUI();
};


void SounditUI::draw_sample_nr(int samplenr) {
  u8g2->clearBuffer();

  u8g2->setFont(u8g2_font_fur20_tf);  //Font height = 20px, so centering vertically = (64-20)/2 = 22px
  int width = u8g2->getUTF8Width(String(samplenr).c_str());
  int margin = (128 - width) / 2;
  u8g2->drawStr(margin, 42, String(samplenr).c_str());

  u8g2->sendBuffer();
}

void SounditUI::draw_plus() {
  u8g2->clearBuffer();

  u8g2->setFont(u8g2_font_fur20_tf);  //Font height = 20px, so centering vertically = (64-20)/2 = 22px
  int width = u8g2->getUTF8Width("+");
  int margin = (128 - width) / 2;
  u8g2->drawStr(margin, 42, "+");

  u8g2->sendBuffer();
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
