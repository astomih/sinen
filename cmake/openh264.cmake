# Build the portable C++ implementation; no assembler or external build tool
# is required. Keep the third-party checkout unmodified.
set(SINEN_OPENH264_DIR ${SINEN_SOURCE_DIR}/libs/openh264)
file(GLOB SINEN_OPENH264_COMMON CONFIGURE_DEPENDS
     ${SINEN_OPENH264_DIR}/codec/common/src/*.cpp)
file(GLOB SINEN_OPENH264_ENCODER CONFIGURE_DEPENDS
     ${SINEN_OPENH264_DIR}/codec/encoder/core/src/*.cpp
     ${SINEN_OPENH264_DIR}/codec/encoder/plus/src/*.cpp)
# DllEntry.cpp is Windows DLL glue, unused by our static library.
list(FILTER SINEN_OPENH264_ENCODER EXCLUDE REGEX "/DllEntry\\.cpp$")
file(GLOB_RECURSE SINEN_OPENH264_PROCESSING CONFIGURE_DEPENDS
     ${SINEN_OPENH264_DIR}/codec/processing/src/*.cpp)
add_library(sinen_openh264 STATIC ${SINEN_OPENH264_COMMON}
            ${SINEN_OPENH264_ENCODER} ${SINEN_OPENH264_PROCESSING})
target_include_directories(sinen_openh264
  PUBLIC ${SINEN_OPENH264_DIR}/codec/api
  PRIVATE ${SINEN_OPENH264_DIR}/codec/api/wels
          ${SINEN_OPENH264_DIR}/codec/common/inc
          ${SINEN_OPENH264_DIR}/codec/encoder/core/inc
          ${SINEN_OPENH264_DIR}/codec/encoder/plus/inc
          ${SINEN_OPENH264_DIR}/codec/processing/interface
          ${SINEN_OPENH264_DIR}/codec/processing/src/common)
set_target_properties(sinen_openh264 PROPERTIES POSITION_INDEPENDENT_CODE ON
                                               CXX_STANDARD 11)
if(NOT WIN32)
  find_package(Threads REQUIRED)
  target_link_libraries(sinen_openh264 PRIVATE Threads::Threads ${CMAKE_DL_LIBS})
endif()
