#ifdef _WIN32
// for SetConsoleOutput
#include <Windows.h>
#endif
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

const std::string en2ja_version = "0.0.1";
void input_word(std::string &word) {
  std::cout << reinterpret_cast<const char *>(
                   u8"英単語を入力してください(小文字)  \"Exit\":終了, "
                   u8"\"Log\":履歴を表示, "
                   u8"\"Print\":output.txtに記録, \"Help\"：ヘルプを表示")
            << std::endl;
  std::cin >> word;
}
void init(std::string &, std::string &,
          std::unordered_map<std::string, std::string> &);

void not_found() {
  std::cout << reinterpret_cast<const char *>(
                   u8"\n該当する英単語が見つかりませんでした。")
            << std::endl;
}
bool loop(std::string &word, std::vector<std::string> &key_buffer,
          std::ofstream &ofs,
          std::unordered_map<std::string, std::string> &map);

int main(int argc, char **argv) {
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
#endif
  std::string key{};
  std::string value{};
  std::unordered_map<std::string, std::string> map;
  std::vector<std::string> key_buffer;
  std::ofstream ofs("./output.txt", std::ios_base::app | std::ios_base::out);
  if (ofs.bad()) {
    std::cout << "Error" << std::endl;
  }
  init(key, value, map);
  std::string word;
  if (argc > 1) {
    for (int i = 2; i < argc; i++) {
      word = std::string(argv[i]);
      if (!loop(word, key_buffer, ofs, map))
        break;
    }
    return 0;
  }
  do {
    input_word(word);
    std::system("cls");
  } while (loop(word, key_buffer, ofs, map));
  ofs.close();
  return 0;
}

void init(std::string &key, std::string &value,
          std::unordered_map<std::string, std::string> &map) {
  std::ifstream istr{};
  istr.open("./ejdict-hand-utf8.txt");
  if (istr.fail()) {
    std::cout << "error" << std::endl;
    std::exit(-1);
  }
  while (true) {
    char c = 0;
    while (c != '	') {
      c = istr.get();
      key.push_back(c);
    }
    key.pop_back();
    while (c != '\n') {
      c = istr.get();
      value.push_back(c);
      if (c == '/')
        value.push_back('\n');
    }
    map.emplace(key, value);
    key.clear();
    value.clear();
    key.shrink_to_fit();
    value.clear();
    value.shrink_to_fit();
    c = istr.peek();
    if (c == EOF)
      break;
  }
}

bool loop(std::string &word, std::vector<std::string> &key_buffer,
          std::ofstream &ofs,
          std::unordered_map<std::string, std::string> &map) {
  if (word == "Exit")
    return false;
  if (word == "Log") {
    if (key_buffer.empty()) {
      std::cout << reinterpret_cast<const char *>(u8"履歴がありません。")
                << std::endl;
      return true;
    }
    for (auto &i : key_buffer) {
      std::cout << i << ":"
                << "\n"
                << map[i] << "\n";
    }
    return true;
  }
  if (word == "Print") {
    if (key_buffer.empty()) {
      std::cout << reinterpret_cast<const char *>(u8"履歴がありません。")
                << std::endl;
      return true;
    }
    for (auto &i : key_buffer) {
      ofs << i << ":"
          << "\n";
      ofs << map[i];
      ofs << "\n";
    }
    return true;
  }
  if (word == "Help") {
    std::cout << "en2ja Version:" << en2ja_version << std::endl;
    return true;
  }
  if (map.contains(word)) {

    std::cout << word << " :\n" << map[word] << std::endl;
    key_buffer.push_back(word);
    return true;
  }
  not_found();
  return true;
}
