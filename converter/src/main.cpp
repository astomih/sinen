#include "assimp_model/assimp_model.hpp"
#include <fstream>
#include <iostream>
#include <string>

const std::string version = "0.0.1";
bool z_mode = false;
bool cpp_mode = false;
inline std::string get_file_name(const std::string &path) {
  std::size_t pos;
  pos = path.rfind('\\');
  if (pos != std::string::npos) {
    return path.substr(pos + 1, path.size() - pos - 1);
  }
  pos = path.rfind('/');
  if (pos != std::string::npos) {
    return path.substr(pos + 1, path.size() - pos - 1);
  }

  return path;
}
inline std::string extract_path_without_extension(const std::string &fn) {
  std::string::size_type pos;
  if ((pos = fn.find_last_of(".")) == std::string::npos) {
    return fn;
  }

  return fn.substr(0, pos);
}
std::string to_string(float val) {
  char buffer[std::numeric_limits<float>::max_exponent10 + 10];
  std::sprintf(buffer, "%f", val);
  std::string str = buffer;
  str.erase(str.find_last_not_of('0') + 1, std::string::npos);
  str.erase(str.find_last_not_of('.') + 1, std::string::npos);
  return str;
}

void write_version(std::string &write_data) {
  write_data += "version " + version + "\n";
}
void write_vertex_cpp(const std::string &name, std::string &write_data,
                      sinen::assimp_model &model);
void write_indices_cpp(std::string &write_data, sinen::assimp_model &model);

void write_vertex(std::string &write_data, sinen::assimp_model &model);
void write_indices(std::string &write_data, sinen::assimp_model &model);
#ifdef main
#undef main
#endif
void show_help() {
  std::cout << "Usage: model_converter.exe model_path [options]" << std::endl;
  std::cout << "Options:" << std::endl;
  std::cout << "  z: convert to z-up mode" << std::endl;
  std::cout << "  cpp: convert to cpp mode" << std::endl;
  std::string s;
  std::getline(std::cin, s);
}
int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Please input model path." << std::endl;
    show_help();
    return -1;
  }
  sinen::assimp_model model;
  if (!model.load_from_file(argv[1])) {
    show_help();
  }
  for (int i = 2; i < argc; i++) {
    if (std::string(argv[i]) == "z") {
      z_mode = true;
    } else if (std::string(argv[i]) == "cpp") {
      cpp_mode = true;
    } else {
      std::cerr << "Unknown option: " << argv[i] << std::endl;
      show_help();
      return -1;
    }
  }

  std::string write_data;
  std::string file_name =
      extract_path_without_extension(get_file_name(std::string(argv[1])));

  if (cpp_mode) {
    write_vertex_cpp(file_name, write_data, model);
    write_indices_cpp(write_data, model);

  } else {
    write_version(write_data);
    write_vertex(write_data, model);
    write_indices(write_data, model);
  }

  if (cpp_mode)
    file_name += ".cpp";
  else
    file_name += ".sim";
  std::ofstream out(file_name);
  if (!out.is_open()) {
    std::cerr << "Can't open file " << file_name << std::endl;
    return -1;
  }
  out.write(write_data.data(), write_data.size());
  out.close();
  return 0;
}
void write_vertex(std::string &write_data, sinen::assimp_model &model) {
  write_data += "begin vertex\n";
  for (auto &i : model.node_list) {
    for (auto &j : i->mesh) {
      for (auto &v : j.body.vertices) {
        if (z_mode) {
          write_data +=
              to_string(v.position.x) + " " + to_string(v.position.z) + " " +
              to_string(v.position.y) + " " + to_string(v.normal.x) + " " +
              to_string(v.normal.z) + " " + to_string(v.normal.x) + " " +
              to_string(v.uv.x) + " " + to_string(v.uv.y) + " " +
              to_string(v.rgba.r) + " " + to_string(v.rgba.g) + " " +
              to_string(v.rgba.b) + " " + to_string(v.rgba.a) + "\n";

        } else {
          write_data +=
              to_string(v.position.x) + " " + to_string(v.position.y) + " " +
              to_string(v.position.z) + " " + to_string(v.normal.x) + " " +
              to_string(v.normal.y) + " " + to_string(v.normal.z) + " " +
              to_string(v.uv.x) + " " + to_string(v.uv.y) + " " +
              to_string(v.rgba.r) + " " + to_string(v.rgba.g) + " " +
              to_string(v.rgba.b) + " " + to_string(v.rgba.a) + "\n";
        }
      }
    }
  }
  write_data += "end vertex\n";
}
void write_indices(std::string &write_data, sinen::assimp_model &model) {
  write_data += "begin indices\n";
  for (auto &i : model.node_list) {
    for (auto &j : i->mesh) {
      for (auto &v : j.body.indices) {
        write_data += std::to_string(v) + "\n";
      }
    }
  }
  write_data += "end indices\n";
}
void write_vertex_cpp(const std::string &name, std::string &write_data,
                      sinen::assimp_model &model) {
  write_data += "#include <sinen.hpp>\n";
  write_data += "sinen::vertex_array create_" + name + "_vertices(){\n";
  write_data += "sinen::vertex_array v_array;\n";
  for (auto &i : model.node_list) {
    for (auto &j : i->mesh) {
      for (auto &v : j.body.vertices) {
        write_data += "v_array.vertices.push_back({sinen::vector3(" +
                      to_string(v.position.x) + "," + to_string(v.position.y) +
                      "," + to_string(v.position.z) + "),sinen::vector3(" +
                      to_string(v.normal.x) + "," + to_string(v.normal.y) +
                      "," + to_string(v.normal.z) + "),sinen::vector2(" +
                      to_string(v.uv.x) + "," + to_string(v.uv.y) +
                      "),sinen::color{" + to_string(v.rgba.r) + "," +
                      to_string(v.rgba.g) + "," + to_string(v.rgba.b) + "," +
                      to_string(v.rgba.a) + "}});\n";
      }
    }
  }
}
void write_indices_cpp(std::string &write_data, sinen::assimp_model &model) {
  write_data += "uint32_t indices[] = {\n";
  for (auto &i : model.node_list) {
    for (auto &j : i->mesh) {
      for (auto &v : j.body.indices) {
        write_data += std::to_string(v) + ",\n";
      }
    }
  }
  write_data +=
      "};\n v_array.indexCount = sizeof(indices) / sizeof(uint32_t);\n "
      "v_array.push_indices(indices, v_array.indexCount);return v_array;}\n";
}