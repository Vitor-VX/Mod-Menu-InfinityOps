LOCAL_PATH := $(call my-dir)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/Includes

include $(CLEAR_VARS)
LOCAL_MODULE := libdobby
LOCAL_SRC_FILES := libs/$(TARGET_ARCH_ABI)/libdobby.a
include $(PREBUILT_STATIC_LIBRARY)

MAIN_LOCAL_PATH := $(call my-dir)
LOCAL_C_INCLUDES += $(MAIN_LOCAL_PATH)

include $(CLEAR_VARS)

LOCAL_MODULE    := vxcheats
LOCAL_CFLAGS := -Wno-error=format-security -fpermissive -DLOG_TAG=\"undetect\"
LOCAL_CFLAGS += -fno-rtti -fno-exceptions
LOCAL_CPPFLAGS += -std=c++14

LOCAL_LDLIBS := -llog -lEGL

LOCAL_STATIC_LIBRARIES := libdobby
LOCAL_C_INCLUDES += $(MAIN_LOCAL_PATH)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/Includes

LOCAL_SRC_FILES :=  main.cpp \
                    Unity/Vector2.hpp \
					Unity/Vector3.hpp \
					Unity/Quaternion.hpp \
					Request/JniHelper/JNIUtils.cpp \
					Request/HttpRequest.cpp \
					And64InlineHook/And64InlineHook.cpp \
					KittyMemory/KittyMemory.cpp \
					KittyMemory/MemoryPatch.cpp \
					KittyMemory/MemoryBackup.cpp \
					KittyMemory/KittyUtils.cpp \
					Includes/ImGui/backends/imgui_impl_opengl3.cpp \
                    Includes/ImGui/backends/imgui_impl_android.cpp \
                    Includes/ImGui/imgui.cpp \
                    Includes/ImGui/imgui_draw.cpp \
                    Includes/ImGui/imgui_demo.cpp \
                    Includes/ImGui/imgui_tables.cpp \
					Includes/ImGui/imgui_widgets.cpp \

LOCAL_LDLIBS := -llog -landroid -lz -lEGL -lGLESv3 -lGLESv2

include $(BUILD_SHARED_LIBRARY)
