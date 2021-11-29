LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := nen

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
                    $(LOCAL_PATH)/../include \
                    $(LOCAL_PATH)/../include/nen \
                    $(LOCAL_PATH)/../libs/ImGui/include \
                    $(LOCAL_PATH)/../libs/Lua/include \
                    $(LOCAL_PATH)/../libs/mojoAL \
                    $(LOCAL_PATH)/../libs/rapidjson/include \
                    $(LOCAL_PATH)/../libs/sol2/include \

LOCAL_CFLAGS := -DSOL_NO_EXCEPTIONS
SRC_PATH := $(LOCAL_PATH)/../src/nen \
            $(LOCAL_PATH)/../libs/ImGui/source \
            $(LOCAL_PATH)/../libs/Lua/source \
            $(LOCAL_PATH)/../libs/mojoAL \

LOCAL_SRC_FILES := \
    $(wildcard $(LOCAL_PATH)/../src/nen/*/*.c) \
    $(wildcard $(LOCAL_PATH)/../src/nen/Render/OpenGLES/*.c) \
    $(wildcard $(LOCAL_PATH)/../libs/ImGui/source/*.c) \
    $(wildcard $(LOCAL_PATH)/../libs/Lua/source/*.c) \
    $(wildcard $(LOCAL_PATH)/../libs/mojoAL/mojoal.c) \

LOCAL_SHARED_LIBRARIES := SDL2 SDL2_image SDL2_mixer SDL2_ttf SDL2_net
LOCAL_LDLIBS := -lGLESv3 -llog
LOCAL_EXPORT_C_INCLUDES += $(LOCAL_C_INCLUDES) \

include $(BUILD_SHARED_LIBRARY)
