#define S_PL Serial.println
#define S_P Serial.print

#include <Wire.h>
#include "main_extra.hpp"
#include "rotary_encoder.hpp"
RotaryEncoder rotary_encoder(4, 3, 4);

// buttons
#include <Bounce.h>
#define BTN_ENC 5
#define BTN_REC 6
Bounce button_encoder(BTN_ENC, 50);
Bounce button_rec(BTN_REC, 50);

bool playrec_sw = LOW;  // low is record

std::vector<std::string> sd_wav_filenames;  // Where SD root filenames are held

#include "enums.hpp"

// for tracking whether file is valid WAV
bool valid_audio_sample = false;

int volume = 1;


#include "soundit_ui.hpp"
SounditUI* UI;
std::vector<int> nrs;
void setup() {
  Serial.begin(115200);
  while (!Serial) {}  // Comment this when not on PC USB
  while (!SD.begin()) {}

  Wire.begin();

  pinMode(BTN_ENC, INPUT_PULLUP);
  pinMode(BTN_REC, INPUT_PULLUP);

  UI = new SounditUI();
  // UI->draw_message_screen_blocking("Hallo", button_rec);

  // read SD card for files
  fill_and_purge_filename_list_nrs(sd_wav_filenames);
  // get_lowest_free_nr(sd_wav_filenames);

  get_used_nrs_int(sd_wav_filenames, nrs);

  UI->draw_sample_nr(20);

  for (int i = 0; i < nrs.size(); i++) {
    S_PL(nrs[i]);
  }
}


// track menu choices
int selected_audio_sample_nr = 0;
int list_position = 0;

MAIN_STATES main_state = RECORDER;

void loop() {
  rotary_encoder.update();
  if (rotary_encoder.get_state() == TURNED_CW) {
    list_position++;
  } else if (rotary_encoder.get_state() == TURNED_CCW) {
    list_position--;
  }

  if (list_position < 0) {
    list_position = 0;
  }

  if (list_position < nrs.size()) {
    if ((rotary_encoder.get_state() == TURNED_CW) || (rotary_encoder.get_state() == TURNED_CCW)) {
      S_PL(sd_wav_filenames[list_position].c_str());
      UI->draw_sample_nr(nrs[list_position]);
    }
  }

  if (list_position >= nrs.size()) {
    list_position = nrs.size();
    UI->draw_plus();

    if (button_rec.fallingEdge()) {
      nrs.push_back(get_lowest_free_nr(sd_wav_filenames));
      
    }
  }



  // check buttons for changes
  button_encoder.update();
  button_rec.update();

  if (button_rec.fallingEdge()) {
    S_PL(sd_wav_filenames[0].c_str());
  }
}
