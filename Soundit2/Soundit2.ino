#define DEBUG

#ifdef DEBUG
#define S_PL Serial.println
#define S_P Serial.print
#endif

#ifndef DEBUG
#define S_PL  //Serial.println
#define S_P   //Serial.print
#endif

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

#include "audio_system.hpp"
AudioSystem* sys;

#include "accelerometer.hpp"
Accelerometer* acc;

std::vector<int> nrs;
void setup() {
  Serial.begin(115200);
#ifdef DEBUG
  while (!Serial) {}  // Comment this when not on PC USB
#endif
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


  for (unsigned int i = 0; i < nrs.size(); i++) {
    S_PL(nrs[i]);
  }

  sys = new AudioSystem(40);

  acc = new Accelerometer();
}


// track menu choices
int selected_audio_sample_nr = 0;
int list_position = 0;

MAIN_STATES main_state = RECORDER;
RECORDER_STATES rec_state = BEFORE_RECORDING;
PLAYBACK_STATES play_state = SELECTING_FILE;

bool encoder_turned = false;
int sample_to_use = 0;
bool ignore_rotary = false;


int sample_menu_option = 0;
bool sample_menu_option_active = false;

void loop() {  // check buttons for changes

  // Check all buttons and wheels for changes:
  button_encoder.update();
  button_rec.update();
  rotary_encoder.update();
  if (rotary_encoder.get_state() == TURNED_CW) {
    if (!ignore_rotary) {
      encoder_turned = true;
      list_position++;
    } else {
      S_PL("Ignoring rotary...");
    }
  } else if (rotary_encoder.get_state() == TURNED_CCW) {
    if (!ignore_rotary) {
      encoder_turned = true;
      list_position--;
    } else {
      S_PL("Ignoring rotary...");
    }
  }


  // Logic for when soundit is in recorder mode
  if (main_state == RECORDER) {
    if (rec_state == BEFORE_RECORDING) {  // file selector logic:

      // Switch to player logic
      if (list_position < -1) {  // don't go below -2'
        list_position = -2;
        UI->draw_text("To player");
        if (button_encoder.fallingEdge()) {
          list_position = 0;
          play_state = SELECTING_FILE;
          ignore_rotary = false;
          main_state = PLAYBACK;
        }
      }

      // gain adjust menu option:
      else if (list_position == -1) {
        UI->draw_mic_icon("Click to adjust gain", sys->get_mic_peak());
        if (button_encoder.fallingEdge()) {
          rec_state = ADJUSTING_MIC_GAIN;
        }
      }

      // Draw all existing sample files
      else if ((list_position < (int)nrs.size()) && (list_position >= 0)) {
        UI->draw_sample_nr(nrs[list_position], false);
        UI->overlay_text_top("Sample nr");
        UI->overlay_fader(sys->get_mic_peak());
      }

      // Draw '+' to add new file for recording
      else if (list_position >= (int)nrs.size()) {
        list_position = nrs.size();  // failsafe that list_position won't go too high

        UI->draw_plus();
        UI->overlay_fader(sys->get_mic_peak());

        // If the button_rec is pressed, start recording
        if (button_rec.fallingEdge()) {
          int file_nr_to_record = get_lowest_free_nr(sd_wav_filenames);  // check what the new filename will become

          sys->start_recording(file_nr_to_record);

          UI->draw_text("Recording");  // maybe swap
          sys->continue_recording();   // these two for recorder/message timing

          rec_state = DURING_RECORDING;

          S_PL("2");
          ignore_rotary = true;
          S_PL("starting rec");
        }
      }
    } else if (rec_state == DURING_RECORDING) {

      sys->continue_recording();  // this MUST BE called continuously to keep recording, otherwise recording will crack or not record at all

      // stop recording if button is let go
      if (button_rec.risingEdge()) {
        sys->stop_recording();
        S_PL("stopping rec");

        fill_and_purge_filename_list_nrs(sd_wav_filenames);  // refresh filename list
        get_used_nrs_int(sd_wav_filenames, nrs);             // refresh sample UI

        ignore_rotary = false;  // ingore wheel turns during recording for UX

        rec_state = BEFORE_RECORDING;
      }
    } else if (rec_state == ADJUSTING_MIC_GAIN) {

      UI->draw_mic_icon("Turn to adjust gain", sys->get_mic_peak());
      if (rotary_encoder.get_state() == TURNED_CW) {
        sys->mic_gain_plus();
        S_PL(sys->get_mic_gain());
      } else if (rotary_encoder.get_state() == TURNED_CCW) {
        sys->mic_gain_min();
        S_PL(sys->get_mic_gain());
      }

      // exit mic gain settings when button is pressed.
      if (button_encoder.fallingEdge()) {
        list_position = -1;
        rec_state = BEFORE_RECORDING;
      }
    }
  }


  if (main_state == PLAYBACK) {
    if (play_state == SELECTING_FILE) {
      if (list_position < -1) {  // don't go below -2
        UI->draw_text("To recorder");
        list_position = -2;
        if (button_encoder.fallingEdge()) {
          list_position = 0;
          rec_state = BEFORE_RECORDING;
          main_state = RECORDER;
        }
      }

      if (list_position == -1) {  // don't go below -1
        list_position = -1;
        UI->draw_speaker_icon("Click knob to adjust", sys->get_volume());
        if (button_encoder.fallingEdge()) {
          play_state = ADJUSTING_VOLUME;
        }
      }

      if ((list_position < (int)nrs.size()) && (list_position >= 0)) {  // draw existing file numbers
        UI->overlay_text_bottom("Click to adjust");
        UI->draw_sample_nr(nrs[list_position], true);

        if (button_rec.fallingEdge()) {
          sys->play_wav(nrs[list_position]);
          sample_to_use = list_position;
          S_PL("acc starting point x = ");
          S_PL(acc->x());
          play_state = PLAYING;
        }

        // If rotary is clicked, switch to sample fx editor
        if (button_encoder.fallingEdge()) {
          sample_to_use = nrs[list_position];
          play_state = CHANGING_SAMPLE_SETTINGS;

          S_P("list_position = ");
          S_PL(list_position);

          S_PL("1");
          ignore_rotary = true;

          sample_menu_option = 0;
          sample_menu_option_active = false;
        }
      }

      if (list_position >= (int)nrs.size()) {
        list_position = (int)nrs.size() - 1;
      }

    } else if (play_state == ADJUSTING_VOLUME) {
      UI->draw_speaker_icon("Turn knob to adjust", sys->get_volume());
      if (rotary_encoder.get_state() == TURNED_CW) {
        sys->set_volume(sys->get_volume() + 5);
      } else if (rotary_encoder.get_state() == TURNED_CCW) {
        sys->set_volume(sys->get_volume() - 5);
      }

      if (button_encoder.fallingEdge()) {
        list_position = -1;
        play_state = SELECTING_FILE;
      }
    } else if (play_state == PLAYING) {
      int x = acc->x();
      if (x < 0) {
        x = x * -1;
        x = map(x, 0, 280, 5000, 0);
        sys->set_lpf(x);
      } else if (x > 0) {
        x = map(x, 0, 280, 0, 8000);
        sys->set_hpf(x);
      }

      int y = acc->y();

      if (y > 50) {
        float speed = sys->mapf(y, 50, 280, 1.0, 2.0);
        sys->set_speed(speed);
      } else if (y < -50) {
        y = y * -1;
        float speed = sys->mapf(y, 50, 280, 1.0, 0.3);
        sys->set_speed(speed);
      } else {
        sys->set_speed(1.0);
      }



      if (!sys->is_playing()) {
        sys->play_wav(nrs[sample_to_use]);
      }
      if (button_rec.risingEdge()) {
        sys->stop_playing();
        list_position = sample_to_use;
        play_state = SELECTING_FILE;
      }
    } else if (play_state == CHANGING_SAMPLE_SETTINGS) {
      // exit mic gain settings when button is pressed.
      switch (sample_menu_option) {
        case DELETE_SAMPLE:
          if (!sample_menu_option_active) {
            UI->draw_text("click to delete");
            if (button_encoder.fallingEdge()) {
              sample_menu_option_active = true;
            }

            if (rotary_encoder.get_state() == TURNED_CW) {
              sample_menu_option = GO_BACK;
            }
          } else if (sample_menu_option_active) {
            UI->draw_text("are you sure?");
            if (button_encoder.fallingEdge()) {
              sys->delete_sample(sample_to_use);
              fill_and_purge_filename_list_nrs(sd_wav_filenames);  // refresh filename list
              get_used_nrs_int(sd_wav_filenames, nrs);             // refresh sample UI

              ignore_rotary = false;
              play_state = SELECTING_FILE;
              list_position = 0;
            }
          }
          break;
        case (12==13):
          UI->draw_text("Back?");

          if (button_encoder.fallingEdge()) {
            ignore_rotary = false;
            play_state = SELECTING_FILE;
          }

          if (rotary_encoder.get_state() == TURNED_CCW) {
            sample_menu_option = DELETE_SAMPLE;
          }
          break;
        default:
          break;
      }
    }
  }

  encoder_turned = false;
}
