LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := nen

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
                    $(LOCAL_PATH)/include \
                    $(LOCAL_PATH)/include/nen \
                    $(LOCAL_PATH)/libs/Effekseer/include \
                    $(LOCAL_PATH)/libs/Effekseer/include/Effekseer \
                    $(LOCAL_PATH)/libs/Effekseer/include/EffekseerRendererGL \
                    $(LOCAL_PATH)/libs/ImGui/include \
                    $(LOCAL_PATH)/libs/Lua/include \
                    $(LOCAL_PATH)/libs/mojoAL \
                    $(LOCAL_PATH)/libs/rapidjson/include \
                    $(LOCAL_PATH)/libs/sol2/include \
                    $(LOCAL_PATH)/libs/tinyobjloader

LOCAL_CFLAGS := -D__EFFEKSEER_RENDERER_GLES3_ -DSOL_NO_EXCEPTIONS
SRC_PATH := $(LOCAL_PATH)/src/nen \
            $(LOCAL_PATH)/libs/ImGui/source \
            $(LOCAL_PATH)/libs/Lua/source \
            $(LOCAL_PATH)/libs/mojoAL \
            $(LOCAL_PATH)/libs/tinyobjloader

LOCAL_SRC_FILES := \
    src/nen/actor/Actor.cpp \
    src/nen/actor/Animation2D.cpp \
    src/nen/actor/PlaneActor.cpp \
    src/nen/actor/Text.cpp \
    src/nen/component/AudioComponent.cpp \
    src/nen/component/CircleComponent.cpp \
    src/nen/component/EffectComponent.cpp \
    src/nen/component/Component.cpp \
    src/nen/component/FontComponent.cpp \
    src/nen/component/InputComponent.cpp \
    src/nen/component/MoveComponent.cpp \
    src/nen/component/ScriptComponent.cpp \
    src/nen/component/Sprite2DComponent.cpp \
    src/nen/component/Sprite3DComponent.cpp \
    src/nen/ES/ESRenderer.cpp \
    src/nen/OpenGL/ShaderGL.cpp \
    src/nen/scene/Scene.cpp \
    src/nen/AudioEvent.cpp \
    src/nen/AudioSystem.cpp \
    src/nen/Effect.cpp \
    src/nen/InputSystem.cpp \
    src/nen/Font.cpp \
    src/nen/Logger.cpp \
    src/nen/Math.cpp \
    src/nen/ObjLoader.cpp \
    src/nen/Random.cpp \
    src/nen/Renderer.cpp \
    src/nen/Script.cpp \
    src/nen/StreamReader.cpp \
    src/nen/Texture.cpp \
    src/nen/Transform.cpp \
    src/nen/Transition.cpp \
    src/nen/Vertex.cpp \
    src/nen/VertexArray.cpp \
    src/nen/Window.cpp \
    libs/ImGui/source/imgui.cpp \
    libs/ImGui/source/imgui_draw.cpp \
    libs/ImGui/source/imgui_impl_opengl3.cpp \
    libs/ImGui/source/imgui_impl_sdl.cpp \
    libs/ImGui/source/imgui_widgets.cpp \
    $(wildcard $(LOCAL_PATH)/libs/Lua/source/*.c) \
    libs/mojoAL/mojoal.c \
    libs/tinyobjloader/tiny_obj_loader.cc \

LOCAL_SHARED_LIBRARIES := SDL2 SDL2_image SDL2_mixer SDL2_ttf SDL2_net Effekseer
LOCAL_LDLIBS := -lGLESv3 -llog
LOCAL_EXPORT_C_INCLUDES += $(LOCAL_C_INCLUDES) \

include $(BUILD_SHARED_LIBRARY)
