#include <pthread.h>
#include "jni.h"
#include <iostream>
#include <fstream>
#include <string>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <array>
#include <unistd.h>
#include <vector>
#include <stdio.h>
#include <map>
#include <sstream>
#include <obfuscate.h>
#include "KittyMemory/KittyMemory.h"
#include "KittyMemory/MemoryPatch.h"
#include <android/log.h>
#include <jni.h>
#include <ctime>
#include <random>
#include <stdlib.h>
#include <Includes/Dobby/dobby.h>
#include "Includes/Imgui/imgui.h"
#include "Includes/Imgui/imgui_internal.h"
#include "Includes/Imgui.h"

#define LOG_TAG "VxCheats"

#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef unsigned char BYTE;  // 1byte
typedef unsigned short WORD; // 2bytes
typedef unsigned long DWORD; //4bytes

uintptr_t location;
string path;
using namespace std;

extern "C"
{
static JavaVM *gJavaVM;
static jobject jcontext;
static jstring jtoken;

void jniSetJavaVM(JavaVM *vm) {
    gJavaVM = vm;
}

JavaVM *jniGetJavaVM() {
    return gJavaVM;
}

extern "C"
JNIEnv *jniGetEnv() {
    JNIEnv *env;
    if (gJavaVM == 0) {
        return 0;
    }

    int status;
    status = gJavaVM->GetEnv((void **) &env, JNI_VERSION_1_6);
    if (status < 0) {
        status = gJavaVM->AttachCurrentThread(&env, 0);
        if (status < 0) {
            return 0;
        }
    }

    return env;
}

}

uintptr_t getRealOffset(uintptr_t offset) {
    if (location == 0) {
        KittyMemory::ProcMap map;
        do {
            map = KittyMemory::getLibraryMap(OBFUSCATE("libil2cpp.so"));
            sleep(1);
        } while (!map.isValid());

        location = reinterpret_cast<uintptr_t>(map.startAddr);

    }

    return location + offset;
}

struct Funcoes {
    bool tryShoot = false;
    bool smoothref = false;
    bool speedDrive = false;
    bool speed = false;
    bool esp = false;
    bool invulnerable = false;
    bool autoKillTest = false;
    bool bulletTimeSpeed = false;
    bool delayBulletImpact = false;
    bool suicidePlayers = false;
};

Funcoes funcoes;

void Menu() {
    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_Appearing);
    ImGui::Begin("Vx Plugin", 0, ImGuiWindowFlags_NoResize);

    float buttonWidth = ImGui::GetWindowWidth() - ImGui::GetStyle().FramePadding.x * 2;

    if (ImGui::Button(funcoes.speed ? "SPEED ON" : "SPEED OFF", ImVec2(buttonWidth, 0))) {
        funcoes.speed = !funcoes.speed;
    }

    if (ImGui::Button(funcoes.smoothref ? "SMOOTH FAST ON" : "SMOOTH FAST OFF",
                      ImVec2(buttonWidth, 0))) {
        funcoes.smoothref = !funcoes.smoothref;
    }

    if (ImGui::Button(funcoes.tryShoot ? "TRY SHOOT ON" : "TRY SHOOT OFF",
                      ImVec2(buttonWidth, 0))) {
        funcoes.tryShoot = !funcoes.tryShoot;
    }

    if (ImGui::Button(funcoes.suicidePlayers ? "SUICIDE PLAYERS ON" : "SUICIDE PLAYERS OFF",
                      ImVec2(buttonWidth, 0))) {
        funcoes.suicidePlayers = !funcoes.suicidePlayers;
    }

    ImGui::End();
}

EGLBoolean swapbuffers_hook(EGLDisplay dpy, EGLSurface surf) {

    eglQuerySurface(dpy, surf, EGL_WIDTH, &glWidth);
    eglQuerySurface(dpy, surf, EGL_HEIGHT, &glHeight);

    if (!isInitialized) {
        SetupImgui();
        isInitialized = true;
    };

    ImGuiIO &io = ImGui::GetIO();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame(glWidth, glHeight);
    ImGui::NewFrame();

    Menu();

    ImGui::Render();
    ImGui::EndFrame();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glViewport(0, 0, (int) io.DisplaySize.x, (int) io.DisplaySize.y);
    glClearColor(0.0, 0.0, 0, 0.0);

    return o_swapbuffers(dpy, surf);
}

void *plugin_init(void *) {
    LOGI("plugin init");

    do {
        sleep(1);
    } while (!isLibraryLoaded(OBFUSCATE("libEGL.so")));

    auto swapBuffers = ((uintptr_t) DobbySymbolResolver(OBFUSCATE("libEGL.so"),
                                                        OBFUSCATE("eglSwapBuffers")));
    mprotect((void *) swapBuffers, 8, PROT_READ | PROT_WRITE | PROT_EXEC);
    DobbyHook((void *) swapBuffers, (void *) swapbuffers_hook, (void **) &o_swapbuffers);

    KittyMemory::ProcMap libunity;

    do {
        sleep(1);
        libunity = KittyMemory::getLibraryMap("libunity.so");
    } while (!libunity.isValid());

    uintptr_t inject_event = reinterpret_cast<uintptr_t>(libunity.startAddr) + 0x293ECC + 1;

    DobbyHook((void *) inject_event, (void *) inject_event_hook, (void **) &o_inject_event);

    return NULL;
}

void TryShoot(void *a1) {
    void (*TryShoot)(void *a1) = (void (*)(void *)) getRealOffset(0x9614C0);
    TryShoot(a1);

    return;
}

void SucidePlayers(void *a1) {
    void (*SucidePlayers)(void *a1) = (void (*)(void *)) getRealOffset(0x83D568);
    SucidePlayers(a1);

    return;
}

void *(*orig_NetworkPlayer)(void *a1);
void *hook_NetworkPlayer(void *a1) {

    if (funcoes.suicidePlayers) {

        bool (*IsAllyOfPlayer)(bool *a1) = (bool (*)(bool *)) getRealOffset(0x82A5D8);

        if (IsAllyOfPlayer((bool *) a1) == 0) {
            SucidePlayers(a1);
        }
    }

    return orig_NetworkPlayer(a1);
}

void *(*orig_UpdateInfinityOps)(void *a1);

void *hook_UpdateInfinityOps(void *a1) {

    if (funcoes.speed) {
        *(float *) ((uint64_t) a1 + 0x2B0) = 45.0f;
    }

    if (funcoes.smoothref) {
        *(float *) ((uint64_t) a1 + 0x2F0) = 800.0f;
    }

    if (funcoes.tryShoot) {
        TryShoot(a1);
    }

    return orig_UpdateInfinityOps(a1);
}

void *Starting(void *) {
    LOGI("pthread init");

    KittyMemory::ProcMap libil2cpp;

    do {
        libil2cpp = KittyMemory::getLibraryMap(OBFUSCATE("libil2cpp.so"));
        sleep(2);
    } while (!libil2cpp.isValid());

    LOGI("libil2cpp encontrada");

    pthread_t ptid;
    pthread_create(&ptid, NULL, plugin_init, NULL);

    DobbyHook((void *) getRealOffset(0x83D424), (void *) hook_NetworkPlayer, (void **) &orig_NetworkPlayer);
    DobbyHook((void *) getRealOffset(0x95A4DC), (void *) hook_UpdateInfinityOps, (void **) &orig_UpdateInfinityOps);

    return NULL;
}


extern "C"
JNIEXPORT jint

JNICALL JNI_OnLoad(JavaVM *vm, void *) {
    JNIEnv *env;
    vm->GetEnv((void **) &env, JNI_VERSION_1_6);

    jniSetJavaVM(vm);

    return JNI_VERSION_1_6;
}

__attribute__((constructor))
void initializer() {
    pthread_t ptid;
    pthread_create(&ptid, NULL, Starting, NULL);
}