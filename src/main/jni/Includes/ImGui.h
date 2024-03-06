#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <asm-generic/mman.h>
#include <sys/mman.h>

#include "ImGui/imgui.h"
#include "Includes/Roboto-Regular.h"
#include "ImGui/backends/imgui_impl_opengl3.h"
#include "ImGui/backends/imgui_impl_android.h"
#include "ImGui/backends/android_native_app_glue.h"

#include "Utils.h"
#include "Dobby/dobby.h"

#include "obfuscate.h"
#include "Logger.h"

using swapbuffers_orig = EGLBoolean (*)(EGLDisplay dpy, EGLSurface surf);

EGLBoolean swapbuffers_hook(EGLDisplay dpy, EGLSurface surf);

using inject_event_orig = int (*)(JNIEnv *env, jobject thiz, jobject inputEvent);

swapbuffers_orig o_swapbuffers = nullptr;

inject_event_orig o_inject_event = nullptr;

bool isInitialized = false;

int glWidth = 400;
int glHeight = 250;

int inject_event_hook(JNIEnv *env, jobject __this, jobject input_event) {
    jclass motion_event = env->FindClass((OBFUSCATE("android/view/MotionEvent")));

    if (!motion_event) {
        LOGI("Can't find MotionEvent!");
        return o_inject_event(env, __this, input_event);
    }

    if (env->IsInstanceOf(input_event, motion_event)) {
        jmethodID get_action = env->GetMethodID(motion_event, ("getActionMasked"), ("()I"));
        jmethodID get_x = env->GetMethodID(motion_event, ("getX"), ("()F"));
        jmethodID get_y = env->GetMethodID(motion_event, ("getY"), ("()F"));
        jmethodID get_pointer_count = env->GetMethodID(motion_event, ("getPointerCount"), ("()I"));

        ImGui_ImplAndroid_HandleInputEvent(env->CallIntMethod(input_event, get_action),
                                           env->CallFloatMethod(input_event, get_x),
                                           env->CallFloatMethod(input_event, get_y),
                                           env->CallIntMethod(input_event, get_pointer_count));

        ImGuiIO &io = ImGui::GetIO();
        if (io.WantCaptureMouse) {
            return true;
        }
    }

    jclass key_event = env->FindClass(OBFUSCATE("android/view/KeyEvent"));

    if (!key_event) {
        LOGI("Can't find KeyEvent!");
        return o_inject_event(env, __this, input_event);
    }

    if (env->IsInstanceOf(input_event, key_event)) {
        jmethodID get_unicode_char = env->GetMethodID(key_event, ("getUnicodeChar"), ("()I"));
        jmethodID get_meta_state = env->GetMethodID(key_event, ("getMetaState"), ("()I"));

        ImGuiIO &io = ImGui::GetIO();
        io.AddInputCharacter(env->CallIntMethod(input_event, get_unicode_char,
                                                env->CallIntMethod(input_event, get_meta_state)));
    }
    return o_inject_event(env, __this, input_event);
}

void *(*orig_Input)(void *, void *, const void *);

void *hook_Input(void *inst, void *motionEvent, const void *inputMessage) {
    void *result = orig_Input(inst, motionEvent, inputMessage);
    ImGui_ImplAndroid_HandleInputEvent((AInputEvent *) inst);
    return result;
}

void SetupImgui() {
    //IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    io.DisplaySize = ImVec2((float) glWidth, (float) glHeight);

    ImGuiStyle *style = &ImGui::GetStyle();

    style->WindowBorderSize = 0;
    style->WindowRounding = 10.0f;
    style->WindowTitleAlign = ImVec2(0.5, 0.5);
    style->WindowPadding = ImVec2(0, 0);
    style->WindowMinSize = ImVec2(100, 100);

    ImGui_ImplOpenGL3_Init("#version 100");

    ImFontConfig fontConfig;

    fontConfig.SizePixels = 24.0f;
    fontConfig.FontDataOwnedByAtlas = false;
    ImFont *font = io.Fonts->AddFontFromMemoryTTF((void *) Roboto_Regular, 168260, 24.0f,
                                                  &fontConfig,
                                                  io.Fonts->GetGlyphRangesChineseFull());
    IM_ASSERT(font != NULL);
    ImGui::GetStyle().ScaleAllSizes(2.0f);
}