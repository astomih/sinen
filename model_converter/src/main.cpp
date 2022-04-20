#include "Model/Model.hpp"
#include <SDL2/SDL_rwops.h>
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

void write_version(std::string &write_data) {
  write_data += "version " + version + "\n";
}
void write_vertex_cpp(const std::string &name, std::string &write_data,
                      nen::Model &model);
void write_indices_cpp(std::string &write_data, nen::Model &model);

void write_vertex(std::string &write_data, nen::Model &model);
void write_indices(std::string &write_data, nen::Model &model);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "Please input model." << std::endl;
    return -1;
  }
  nen::Model model;
  model.LoadFromFile(argv[1]);
  if (argc > 2 && std::string(argv[2]) == "z") {
    z_mode = true;
  }
  if (argc > 2 && std::string(argv[2]) == "cpp") {
    cpp_mode = true;
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
  auto *file = SDL_RWFromFile(file_name.data(), "w");
  if (file != NULL) {
    auto len = write_data.size();
    if (SDL_RWwrite(file, write_data.data(), 1, len) != len) {
      std::cout << "Error: Failed to write data." << std::endl;
    } else {
      std::cout << std::to_string(len) << " 1-byte block wrote.";
    }
    SDL_RWclose(file);
  }
  return 0;
}
void write_vertex(std::string &write_data, nen::Model &model) {
  write_data += "begin vertex\n";
  for (auto &i : model.node_list) {
    for (auto &j : i->mesh) {
      for (auto &v : j.body.vertices) {
        if (z_mode) {
          write_data +=
              std::to_string(v.position.x) + " " +
              std::to_string(v.position.z) + " " +
              std::to_string(v.position.y) + " " + std::to_string(v.normal.x) +
              " " + std::to_string(v.normal.z) + " " +
              std::to_string(v.normal.x) + " " + std::to_string(v.uv.x) + " " +
              std::to_string(v.uv.y) + " " + std::to_string(v.rgba.r) + " " +
              std::to_string(v.rgba.g) + " " + std::to_string(v.rgba.b) + " " +
              std::to_string(v.rgba.a) + "\n";

        } else {
          write_data +=
              std::to_string(v.position.x) + " " +
              std::to_string(v.position.y) + " " +
              std::to_string(v.position.z) + " " + std::to_string(v.normal.x) +
              " " + std::to_string(v.normal.y) + " " +
              std::to_string(v.normal.z) + " " + std::to_string(v.uv.x) + " " +
              std::to_string(v.uv.y) + " " + std::to_string(v.rgba.r) + " " +
              std::to_string(v.rgba.g) + " " + std::to_string(v.rgba.b) + " " +
              std::to_string(v.rgba.a) + "\n";
        }
      }
    }
  }
  write_data += "end vertex\n";
}
void write_indices(std::string &write_data, nen::Model &model) {
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
                      nen::Model &model) {
  write_data += "#include <Nen.hpp>\n";
  write_data += "nen::vertex_array create_" + name + "_vertices(){\n";
  write_data += "nen::vertex_array v_array;\n";
  for (auto &i : model.node_list) {
    for (auto &j : i->mesh) {
      for (auto &v : j.body.vertices) {
        write_data +=
            "v_array.vertices.push_back({nen::vector3(" +
            std::to_string(v.position.x) + "," + std::to_string(v.position.y) +
            "," + std::to_string(v.position.z) + "),nen::vector3(" +
            std::to_string(v.normal.x) + "," + std::to_string(v.normal.y) +
            "," + std::to_string(v.normal.z) + "),nen::vector2(" +
            std::to_string(v.uv.x) + "," + std::to_string(v.uv.y) +
            "),nen::color{" + std::to_string(v.rgba.r) + "," +
            std::to_string(v.rgba.g) + "," + std::to_string(v.rgba.b) + "," +
            std::to_string(v.rgba.a) + "}});\n";
      }
    }
  }
}
void write_indices_cpp(std::string &write_data, nen::Model &model) {
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