#ifdef _WIN32
// for SetConsoleOutput
#include <Windows.h>
#endif
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <tchar.h>
#include <unordered_map>
#include <vector>

const std::string En2JaVersion = "0.0.1";
void inputWord(std::string &word) {
  std::cout << reinterpret_cast<const char *>(
                   u8"英単語を入力してください(小文字)  \"Exit\":終了, "
                   u8"\"Print\":output.txtに記録, \"Help\"：ヘルプを表示")
            << std::endl;
  std::cin >> word;
}
bool findMap(std::unordered_map<std::string, std::string> &map,
             const std::string &word) {
  if (map.find(word) != map.end()) {
    std::cout << map[word];
    return true;
  }
  return false;
}

void init(std::string &, std::string &,
          std::unordered_map<std::string, std::string> &);

void not_found() {
  std::cout << reinterpret_cast<const char *>(
                   u8"\n該当する英単語が見つかりませんでした。")
            << std::endl;
}

int main(int argc, char **argv) {
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
#endif
  std::string key{};
  std::string value{};
  std::unordered_map<std::string, std::string> map;
  std::vector<std::string> keyBuffer;
  std::ofstream ofs("./output.txt", std::ios_base::app | std::ios_base::out);
  if (ofs.bad()) {
    std::cout << "Error" << std::endl;
  }
  init(key, value, map);

  std::string word;
  if (argc < 2)
    inputWord(word);
  else
    word = std::string(argv[1]);
  if (word == "Exit")
    return 0;
  else if (word == "Print")
    std::cout << reinterpret_cast<const char *>(u8"まだ何も入力されていません")
              << std::endl;
  else if (word == "Help") {
    std::cout << "En2Ja Version:" << En2JaVersion << std::endl;
    std::cout << "" << std::endl;
  } else if (findMap(map, word)) {
    keyBuffer.push_back(word);
  } else
    not_found();
  std::cout << std::endl;
  for (int i = 2; i < argc; i++) {
    word = std::string(argv[i]);
    if (word == "Exit")
      return 0;
    if (word == "Print") {
      for (auto &i : keyBuffer) {
        ofs << i << ":"
            << "\n";
        ofs << map[i];
        ofs << "\n";
      }
    } else if (findMap(map, word)) {
      keyBuffer.push_back(word);
    } else
      not_found();
    std::cout << std::endl;
  }
  while (true) {
    inputWord(word);
    if (word == "Exit")
      break;
    if (word == "Print") {
      for (auto &i : keyBuffer) {
        ofs << i << ":"
            << "\n";
        ofs << map[i];
        ofs << "\n";
      }
    } else if (findMap(map, word)) {
      keyBuffer.push_back(word);
    } else
      not_found();
    std::cout << std::endl;
  }
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
