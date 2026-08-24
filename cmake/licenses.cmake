function(_sinen_escape_cpp_string input output)
  set(value "${input}")
  string(REPLACE "\\" "\\\\" value "${value}")
  string(REPLACE "\"" "\\\"" value "${value}")
  set(${output} "${value}" PARENT_SCOPE)
endfunction()

function(sinen_generate_license_header output_file)
  set(license_entries)
  set(license_dependencies)

  macro(sinen_embed_license name url relative_path max_lines note)
    set(license_path "${SINEN_SOURCE_DIR}/${relative_path}")
    set(license_text)
    if(NOT "${relative_path}" STREQUAL "" AND EXISTS "${license_path}")
      if(${max_lines} GREATER 0)
        file(STRINGS "${license_path}" license_lines
             LIMIT_COUNT ${max_lines})
        list(JOIN license_lines "\n" license_text)
        string(APPEND license_text "\n")
      else()
        file(READ "${license_path}" license_text)
      endif()
      list(APPEND license_dependencies "${license_path}")
    endif()

    string(HEX "${license_text}" license_hex)
    string(LENGTH "${license_hex}" license_hex_length)
    set(license_hex_chunks)
    foreach(chunk_index RANGE 0 3)
      math(EXPR chunk_offset "${chunk_index} * 8000")
      set(license_hex_chunk)
      if(chunk_offset LESS license_hex_length)
        math(EXPR chunk_length "${license_hex_length} - ${chunk_offset}")
        if(chunk_length GREATER 8000)
          set(chunk_length 8000)
        endif()
        string(SUBSTRING "${license_hex}" ${chunk_offset} ${chunk_length}
                         license_hex_chunk)
      endif()
      string(APPEND license_hex_chunks "\"${license_hex_chunk}\", ")
    endforeach()

    _sinen_escape_cpp_string("${name}" escaped_name)
    _sinen_escape_cpp_string("${url}" escaped_url)
    _sinen_escape_cpp_string("${relative_path}" escaped_source)
    _sinen_escape_cpp_string("${note}" escaped_note)
    string(
      APPEND license_entries
      "    {\"${escaped_name}\", \"${escaped_url}\", \"${escaped_source}\", \"${escaped_note}\", {${license_hex_chunks}}},\n"
    )
  endmacro()

  sinen_embed_license("Sinen" "https://github.com/astomih/sinen"
                      "LICENSE.md" 0 "")
  sinen_embed_license("SDL" "https://github.com/libsdl-org/SDL"
                      "libs/SDL/LICENSE.txt" 0 "")
  sinen_embed_license("Assimp" "https://github.com/assimp/assimp"
                      "libs/assimp/LICENSE" 0 "")
  sinen_embed_license("Box2D" "https://github.com/erincatto/box2d"
                      "libs/box2d/LICENSE" 0 "")
  sinen_embed_license("Box3D" "https://github.com/erincatto/box3d"
                      "libs/box3d/LICENSE" 0 "")
  sinen_embed_license("cppdap" "https://github.com/google/cppdap"
                      "libs/cppdap/LICENSE" 0 "")
  sinen_embed_license("KTX-Software"
                      "https://github.com/KhronosGroup/KTX-Software"
                      "libs/ktx-software/LICENSE.md" 0 "")
  sinen_embed_license("KTX-Software NOTICE"
                      "https://github.com/KhronosGroup/KTX-Software"
                      "libs/ktx-software/NOTICE.md" 0 "")
  sinen_embed_license("Luau" "https://github.com/luau-lang/luau"
                      "libs/luau/LICENSE.txt" 0 "")
  sinen_embed_license(
    "Luau Debugger" "https://github.com/sssooonnnggg/luau-debugger" "" 0
    "No license file is distributed in the pinned source revision.")
  sinen_embed_license("meshoptimizer"
                      "https://github.com/zeux/meshoptimizer"
                      "libs/meshoptimizer/LICENSE.md" 0 "")
  sinen_embed_license("miniaudio" "https://github.com/mackron/miniaudio"
                      "libs/miniaudio/LICENSE" 0 "")
  sinen_embed_license("msdfgen" "https://github.com/Chlumsky/msdfgen"
                      "libs/msdfgen/LICENSE.txt" 0 "")
  sinen_embed_license("RapidJSON" "https://github.com/Tencent/rapidjson"
                      "libs/rapidjson/license.txt" 0 "")
  sinen_embed_license("Slang" "https://github.com/shader-slang/slang"
                      "plugins/shader_compiler/libs/slang/LICENSE" 0 "")
  sinen_embed_license("stb" "https://github.com/nothings/stb"
                      "libs/stb/LICENSE" 0 "")
  sinen_embed_license(
    "TinyEXR" "https://github.com/syoyo/tinyexr"
    "libs/tinyexr/tinyexr.h" 68
    "License notices extracted from the beginning of tinyexr.h.")
  sinen_embed_license(
    "TLSF" "https://github.com/mattconte/tlsf" "libs/tlsf/tlsf.h" 40
    "License notice extracted from the beginning of tlsf.h.")
  sinen_embed_license("Tracy" "https://github.com/wolfpld/tracy"
                      "libs/tracy/LICENSE" 0 "")
  sinen_embed_license("volk" "https://github.com/zeux/volk"
                      "libs/volk/LICENSE.md" 0 "")
  sinen_embed_license(
    "Vulkan Memory Allocator"
    "https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator"
    "libs/VulkanMemoryAllocator/LICENSE.txt" 0 "")
  sinen_embed_license(
    "WebGPU-distribution"
    "https://github.com/eliemichel/WebGPU-distribution"
    "libs/WebGPU-distribution/LICENSE.txt" 0 "")

  get_filename_component(output_directory "${output_file}" DIRECTORY)
  file(MAKE_DIRECTORY "${output_directory}")
  file(
    WRITE "${output_file}"
    "#pragma once\n\n#include <array>\n#include <string_view>\n\nnamespace sinen::license_data {\nstruct EmbeddedLicense {\n  std::string_view name;\n  std::string_view url;\n  std::string_view source;\n  std::string_view note;\n  std::array<std::string_view, 4> textHexChunks;\n};\n\ninline constexpr EmbeddedLicense entries[] = {\n${license_entries}};\n} // namespace sinen::license_data\n"
  )

  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
                                          ${license_dependencies})
endfunction()
