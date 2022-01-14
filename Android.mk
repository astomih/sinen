
include $(CLEAR_VARS)
LOCAL_PATH := $(call my-dir)

LOCAL_MODULE := sinen

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
                    $(LOCAL_PATH)/include \
                    $(LOCAL_PATH)/include/Nen \
                    $(LOCAL_PATH)/libs/ImGui/include \
                    $(LOCAL_PATH)/libs/Lua/include \
                    $(LOCAL_PATH)/libs/mojoAL \
                    $(LOCAL_PATH)/libs/rapidjson/include \
                    $(LOCAL_PATH)/libs/sol2/include \

LOCAL_CFLAGS := -DSOL_NO_EXCEPTIONS -DNEN_NO_EXCEPTION -DMOBILE
SRC_PATH := $(LOCAL_PATH)/src/Nen \
            $(LOCAL_PATH)/libs/ImGui/source \
            $(LOCAL_PATH)/libs/Lua/source \
            $(LOCAL_PATH)/libs/mojoAL

LOCAL_SRC_FILES := \
    $(wildcard $(LOCAL_PATH)/src/nen/*/*.cpp) \
    $(wildcard $(LOCAL_PATH)/src/nen/Render/OpenGLES/*.cpp) \
    $(wildcard $(LOCAL_PATH)/libs/ImGui/source/gles/*.cpp) \
    $(wildcard $(LOCAL_PATH)/libs/Lua/source/*.c) \
    $(wildcard $(LOCAL_PATH)/libs/mojoAL/mojoal.c)
LOCAL_STATIC_LIBRARIES :=
LOCAL_SHARED_LIBRARIES := SDL2 \
SDL2_image \
SDL2_mixer \
SDL2_ttf \
SDL2_net
LOCAL_LDLIBS := -lGLESv3 -llog
LOCAL_EXPORT_C_INCLUDES += $(LOCAL_C_INCLUDES)

include $(BUILD_SHARED_LIBRARY)
