
#include <string>
#include <cctype>

#include <vector>
#include <algorithm>
#include <cstring>

#include <SD.h>

#include <stdlib.h>     /* atoi */

bool alphanumericCompare(const std::string& a, const std::string& b) {
  size_t i = 0, j = 0;

  while (i < a.length() && j < b.length()) {
    char charA = std::tolower(a[i]);
    char charB = std::tolower(b[j]);

    if (std::isdigit(charA) && std::isdigit(charB)) {
      // Both are numeric, compare as numbers
      int numA = 0, numB = 0;

      while (i < a.length() && std::isdigit(a[i])) {
        numA = numA * 10 + (a[i] - '0');
        i++;
      }

      while (j < b.length() && std::isdigit(b[j])) {
        numB = numB * 10 + (b[j] - '0');
        j++;
      }

      if (numA < numB) return true;
      else if (numA > numB) return false;
    } else {
      // Compare as characters (case-insensitive)
      if (charA < charB) return true;
      else if (charA > charB) return false;

      i++;
      j++;
    }
  }

  // If one string is a prefix of the other, shorter string comes first
  return i == a.length();
}

void fill_and_purge_filename_list(std::vector<std::string>& list) {
  list.clear();

  SdFile dir("/Soundit/", O_RDONLY);
  SdFile file;
  while (file.openNext(&dir, O_RDONLY)) {
    constexpr size_t maxFileLen = 30;
    char buf[maxFileLen];
    file.getName(buf, maxFileLen);
    list.emplace_back(buf);  // directly construct the string in the vector to avoid copies
  }

  // std::sort(list.begin(), list.end(), [](std::string a, std::string b) {
  //   return a < b;
  // });
  std::sort(list.begin(), list.end(), alphanumericCompare);

  for (unsigned int i = 0; i < list.size(); i++) {  // remove files that start with a . from filenames
    if (list[i][0] == '.') {
      list.erase(list.begin() + i);
      i--;
    }
  }

  for (unsigned int i = 0; i < list.size(); i++) {  // Remove filenames that are 0
    if (list[i].length() == 0) {
      Serial.print("Dingske te kort ");
      Serial.println(i);
      list.erase(list.begin() + i);
      i--;
    }
  }

  // Max filelength is 01234567890123456789012345.wav

  // for (int n = 0; n < 100; n++) {
  //   for (int i = 0; i < list.size(); i++) {  // display all filenames
  //     Serial.print(list[i].c_str());
  //     Serial.print(", ");
  //     Serial.println(list[i].length());
  //   }
  // }
}

int get_lowest_free_nr(std::vector<std::string>& list) {
  int nr_to_check = 0;
  bool found_available = false;
  bool nr_in_list = false;

  while (!found_available) {
    nr_to_check++;
    nr_in_list = false;
    for (unsigned int i = 0; i < list.size(); i++) {
      std::string comparison = std::to_string(nr_to_check) + ".wav";
      if (list[i].compare(comparison) == 0) {
        nr_in_list = true;
      }
    }

    if (nr_in_list == false) {
      found_available = true;
    }
  }

  S_PL(nr_to_check);

  return nr_to_check;
}


void fill_and_purge_filename_list_nrs(std::vector<std::string>& list) {
  list.clear();

  SdFile dir("/Soundit/", O_RDONLY);
  SdFile file;
  while (file.openNext(&dir, O_RDONLY)) {
    constexpr size_t maxFileLen = 30;
    char buf[maxFileLen];
    file.getName(buf, maxFileLen);
    list.emplace_back(buf);  // directly construct the string in the vector to avoid copies
  }

  // std::sort(list.begin(), list.end(), [](std::string a, std::string b) {
  //   return a < b;
  // });
  std::sort(list.begin(), list.end(), alphanumericCompare);

  for (unsigned int i = 0; i < list.size(); i++) {  // remove files that start with a . from filenames
    if (list[i][0] == '.') {
      list.erase(list.begin() + i);
      i--;
    }
  }

  for (unsigned int i = 0; i < list.size(); i++) {  // Remove filenames that are 0
    if (list[i].length() == 0) {
      Serial.print("Dingske te kort ");
      Serial.println(i);
      list.erase(list.begin() + i);
      i--;
    }
  }

  for (unsigned int i = 0; i < list.size(); i++) {  // Remove filenames that are not .WAV
    int result = list[i].find(".wav");
    // S_P(list[i].c_str());
    // S_P(" : ");
    // S_PL(result);
    if (result <= 0) {
      list.erase(list.begin() + i);
    }
  }

  for (unsigned int i = 0; i < list.size(); i++) {  // Remove filenames contain letters before .WAV
    if (list[i].length() == 0) {
      // Serial.print("Dingske te kort ");
      // Serial.println(i);
      list.erase(list.begin() + i);
      i--;
    }
  }


  // Max filelength is 01234567890123456789012345.wav

  // for (int n = 0; n < 100; n++) {
  //   for (int i = 0; i < list.size(); i++) {  // display all filenames
  //     Serial.print(list[i].c_str());
  //     Serial.print(", ");
  //     Serial.println(list[i].length());
  //   }
  // }
}

bool get_used_nrs_int(std::vector<std::string> filenames_list, std::vector<int>& numbers_list) {
  numbers_list.clear();

  for (unsigned int i = 0; i < filenames_list.size(); i++) {  // Remove filenames contain letters before .WAV
    std::string::size_type pos = filenames_list[i].find('.');
    std::string nr_str = filenames_list[i].substr(0, pos);
    int present_nr = atoi(nr_str.c_str());
    numbers_list.push_back(present_nr);
  }

  return true;
}
