#define NODEBUG

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
Bounce button_rec(BTN_REC, 25);


#include <EEPROM.h>

bool playrec_sw = LOW;  // low is record

std::vector<std::string> sd_wav_filenames;  // Where SD root filenames are held

#include "enums.hpp"

// for tracking whether file is valid WAV
bool valid_audio_sample = false;



#include "soundit_ui.hpp"
SounditUI* UI;

#include "audio_system.hpp"
AudioSystem* sys;

#include "accelerometer.hpp"
Accelerometer* acc;

std::vector<int> nrs;

int list_position = 0;

EFFECTS assigned_effects[4] = { LPF, LPF, LPF, LPF };  // X+, X-, Y+, Y-

//settings:
const int fx_deadzone = 30;

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

  // read SD card for files
  fill_and_purge_filename_list_nrs(sd_wav_filenames);
  get_used_nrs_int(sd_wav_filenames, nrs);

  // for (unsigned int i = 0; i < nrs.size(); i++) {
  //   S_PL(nrs[i]);
  // }

  sys = new AudioSystem(40);
  acc = new Accelerometer();

  list_position = nrs.size();

  for (int i = 0; i < 1080; i++) {
    if (EEPROM.read(i) >= amount_of_effects) {
      EEPROM.write(i, 0x00);
    }
  }


  for (int i = 0; i < 4; i++) {
    assigned_effects[i] = static_cast<EFFECTS>(EEPROM.read(i));
  }
}


// track menu choices
int selected_audio_sample_nr = 0;

MAIN_STATES main_state = PLAYBACK;
RECORDER_STATES rec_state = BEFORE_RECORDING;
PLAYBACK_STATES play_state = SELECTING_FILE;

bool encoder_turned = false;
int sample_to_use = 0;
bool ignore_rotary = false;

int sample_menu_option = 0;
bool sample_menu_option_active = false;



int fx_to_change = 0;

bool x_flat = false;
bool y_flat = false;



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
  switch (main_state) {
    case RECORDER:
      {
        switch (rec_state) {
          case BEFORE_RECORDING:
            {
              // Switch to player logic
              if (list_position < -1) {  // don't go below -2'
                list_position = -2;
                UI->Clear();
                UI->draw_text("To player");
                UI->Write();
                if (button_encoder.fallingEdge()) {
                  list_position = 0;
                  play_state = SELECTING_FILE;
                  ignore_rotary = false;
                  main_state = PLAYBACK;
                }
              }

              // gain adjust menu option:
              else if (list_position == -1) {
                UI->Clear();
                UI->draw_mic_icon("Click to adjust gain", sys->get_mic_peak());
                UI->Write();
                if (button_encoder.fallingEdge()) {
                  rec_state = ADJUSTING_MIC_GAIN;
                }
              }

              // Draw all existing sample files
              else if ((list_position < (int)nrs.size()) && (list_position >= 0)) {
                UI->Clear();
                UI->overlay_rec_icon();
                UI->draw_sample_nr(nrs[list_position]);
                UI->overlay_text_top("Sample nr");
                UI->overlay_fader(sys->get_mic_peak());
                UI->Write();
              }

              // Draw '+' to add new file for recording
              else if (list_position >= (int)nrs.size()) {
                list_position = nrs.size();  // failsafe that list_position won't go too high
                UI->Clear();
                UI->draw_plus();
                UI->overlay_rec_icon();
                UI->overlay_fader(sys->get_mic_peak());
                UI->Write();

                // If the button_rec is pressed, start recording
                if (button_rec.fallingEdge()) {
                  sample_to_use = get_lowest_free_nr(sd_wav_filenames);  // check what the new filename will become

                  sys->start_recording(sample_to_use);
                  UI->Clear();
                  UI->draw_text("Recording");  // maybe swap
                  UI->Write();
                  sys->continue_recording();  // these two for recorder/message timing

                  rec_state = DURING_RECORDING;

                  S_PL("2");
                  ignore_rotary = true;
                  S_PL("starting rec");
                }
              }
              break;
            }

          case DURING_RECORDING:
            {
              sys->continue_recording();  // this MUST BE called continuously to keep recording, otherwise recording will crack or not record at all

              // stop recording if button is let go
              if (button_rec.risingEdge()) {
                sys->stop_recording();
                S_PL("stopping rec");

                fill_and_purge_filename_list_nrs(sd_wav_filenames);  // refresh filename list
                get_used_nrs_int(sd_wav_filenames, nrs);             // refresh sample UI

                ignore_rotary = false;  // ingore wheel turns during recording for UX

                rec_state = AFTER_RECORDING;
              }
              break;
            }
          case AFTER_RECORDING:
            {
              UI->Clear();
              UI->draw_sample_nr(sample_to_use);
              UI->overlay_text_top("Saved to");
              UI->Write();
              delay(3000);
              rec_state = BEFORE_RECORDING;
              break;
            }

          case ADJUSTING_MIC_GAIN:
            {
              UI->Clear();
              UI->draw_mic_icon("Turn to adjust gain", sys->get_mic_peak());
              UI->Write();
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
              break;
            }
          default:
            break;
        }
        break;
      }

    case PLAYBACK:
      {

        switch (play_state) {
          case SELECTING_FILE:
            {
              if (list_position <= -6) {  // don't go below -2
                UI->Clear();
                UI->draw_text("To recordr");
                UI->Write();
                list_position = -6;
                if (button_encoder.fallingEdge()) {
                  list_position = 0;
                  rec_state = BEFORE_RECORDING;
                  main_state = RECORDER;
                }
              }

              else if (list_position == -5) {  // y axis positive
                UI->Clear();
                UI->overlay_text_top(EFFECTS_STRINGS[assigned_effects[3]]);
                UI->draw_text(AVAILABLE_AXES[3]);
                UI->overlay_text_bottom("Click to change");
                UI->Write();

                if (button_encoder.fallingEdge()) {
                  fx_to_change = 3;
                  play_state = CHANGING_FX_SETTINGS;
                  ignore_rotary = true;
                }
              }

              else if (list_position == -4) {  // y axis positive
                UI->Clear();
                UI->overlay_text_top(EFFECTS_STRINGS[assigned_effects[2]]);
                UI->draw_text(AVAILABLE_AXES[2]);
                UI->overlay_text_bottom("Click to change");
                UI->Write();

                if (button_encoder.fallingEdge()) {
                  fx_to_change = 2;
                  play_state = CHANGING_FX_SETTINGS;
                  ignore_rotary = true;
                }
              }

              else if (list_position == -3) {  // y axis positive
                UI->Clear();
                UI->overlay_text_top(EFFECTS_STRINGS[assigned_effects[1]]);
                UI->draw_text(AVAILABLE_AXES[1]);
                UI->overlay_text_bottom("Click to change");
                UI->Write();

                if (button_encoder.fallingEdge()) {
                  fx_to_change = 1;
                  play_state = CHANGING_FX_SETTINGS;
                  ignore_rotary = true;
                }
              }

              else if (list_position == -2) {  // y axis positive
                UI->Clear();
                UI->overlay_text_top(EFFECTS_STRINGS[assigned_effects[0]]);
                UI->draw_text(AVAILABLE_AXES[0]);
                UI->overlay_text_bottom("Click to change");
                UI->Write();

                if (button_encoder.fallingEdge()) {
                  fx_to_change = 0;
                  play_state = CHANGING_FX_SETTINGS;
                  ignore_rotary = true;
                }
              }


              // else if (list_position == -1) {  // don't go below -1
              // else if (list_position == -1) {  // don't go below -1
              // else if (list_position == -1) {  // don't go below -1



              else if (list_position == -1) {
                UI->Clear();
                UI->draw_speaker_icon("Click knob to adjust", sys->get_volume());
                UI->Write();
                if (button_encoder.fallingEdge()) {
                  play_state = ADJUSTING_VOLUME;
                }
              }

              else if ((list_position < (int)nrs.size()) && (list_position >= 0)) {  // draw existing file numbers
                UI->Clear();
                UI->overlay_text_bottom("Click to adjust");
                UI->overlay_play_icon();
                UI->draw_sample_nr(nrs[list_position]);
                UI->Write();

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

              else if (list_position >= (int)nrs.size()) {
                list_position = (int)nrs.size() - 1;
              }
              break;
            }
          case ADJUSTING_VOLUME:
            {
              UI->Clear();
              UI->draw_speaker_icon("Turn knob to adjust", sys->get_volume());
              UI->Write();
              if (rotary_encoder.get_state() == TURNED_CW) {
                sys->set_volume(sys->get_volume() + 5);
              } else if (rotary_encoder.get_state() == TURNED_CCW) {
                sys->set_volume(sys->get_volume() - 5);
              }

              if (button_encoder.fallingEdge()) {
                list_position = -1;
                play_state = SELECTING_FILE;
              }
            }
            break;
          case PLAYING:
            {
              // BEGIN NEW SENSOR HANDLING CODE
              x_flat = false;
              y_flat = false;

              int x = acc->x();
              int y = acc->y();

              if (x > fx_deadzone) {  // X+
                x = x - fx_deadzone;

                sys->handle_effect(assigned_effects[0], x);

              } else if (x < -fx_deadzone) {  //X-
                x = x * -1;
                x = x - fx_deadzone;

                sys->handle_effect(assigned_effects[1], x);
              } else {
                x_flat = true;
              }

              if (y > fx_deadzone) {  //Y+
                y = y - fx_deadzone;

                sys->handle_effect(assigned_effects[2], y);

              } else if (y < -fx_deadzone) {  //Y-
                y = y * -1;
                y = y - fx_deadzone;

                sys->handle_effect(assigned_effects[3], y);
              } else {
                y_flat = true;
              }

              if (x_flat && y_flat) {
                S_PL("resetting fx");
                sys->reset_fx();
              }




              // END NEW SENSOR HANDLING CODE


              if (!sys->is_playing()) {
                sys->play_wav(nrs[sample_to_use]);
              }
              if (button_rec.risingEdge()) {
                sys->stop_playing();
                list_position = sample_to_use;
                play_state = SELECTING_FILE;
              }
              break;
            }
          case CHANGING_SAMPLE_SETTINGS:
            {
              // // exit mic gain settings when button is pressed.
              switch (sample_menu_option) {
                case DELETE_SAMPLE:
                  {
                    if (!sample_menu_option_active) {
                      UI->Clear();
                      UI->draw_text("Delete");
                      UI->Write();
                      if (button_encoder.fallingEdge()) {
                        sample_menu_option_active = true;
                      }

                      if (rotary_encoder.get_state() == TURNED_CW) {
                        sample_menu_option = GO_BACK;
                      }
                    } else if (sample_menu_option_active) {
                      UI->Clear();
                      UI->draw_text("U sure?");
                      UI->Write();
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
                  }
                case GO_BACK:
                  {
                    UI->Clear();
                    UI->draw_text("Back?");
                    UI->Write();

                    if (button_encoder.fallingEdge()) {
                      ignore_rotary = false;
                      play_state = SELECTING_FILE;
                    }

                    if (rotary_encoder.get_state() == TURNED_CCW) {
                      sample_menu_option = DELETE_SAMPLE;
                    }
                    break;
                  }
                default:
                  break;
              }
              break;
            }
          case CHANGING_FX_SETTINGS:
            {
              int current_effect = assigned_effects[fx_to_change];
              if (rotary_encoder.get_state() == TURNED_CW) {
                current_effect++;
                if (current_effect >= amount_of_effects) {
                  current_effect = 0;
                }
              } else if (rotary_encoder.get_state() == TURNED_CCW) {
                current_effect--;
                if (current_effect < 0) {
                  current_effect = amount_of_effects - 1;
                }
              }
              assigned_effects[fx_to_change] = current_effect;
              UI->Clear();
              UI->overlay_text_top(EFFECTS_STRINGS[assigned_effects[fx_to_change]]);
              UI->draw_text(AVAILABLE_AXES[fx_to_change]);
              UI->overlay_text_bottom("Turn to change");
              UI->Write();

              assigned_effects[fx_to_change] = static_cast<EFFECTS>(current_effect);

              if (button_encoder.fallingEdge()) {
                ignore_rotary = false;
                play_state = SELECTING_FILE;
                EEPROM.write(fx_to_change, static_cast<uint8_t>(current_effect));
              }
              break;
            }
          default:
            break;
        }
      }
    default:
      break;
  }
  encoder_turned = false;
}