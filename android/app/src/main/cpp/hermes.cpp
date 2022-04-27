// hermes
// based on imgui example for android code

#include "imgui.h"
#include "imgui_impl_android.h"
#include "imgui_impl_opengl3.h"
#include <android/log.h>
#include <android_native_app_glue.h>
#include <android/asset_manager.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include "misc/fonts/droidsansfont.h"

// Data

const char HERMESAPP_LOGTAG[] = "HermesApp";

struct HermesAppInternal {
    EGLDisplay m_EglDisplay = EGL_NO_DISPLAY;
    EGLSurface m_EglSurface = EGL_NO_SURFACE;
    EGLContext m_EglContext = EGL_NO_CONTEXT;
    struct android_app *m_AndroidApp = NULL;
    bool m_Initialized = false;
    ImFont *m_Font = nullptr;
    const int m_MailListSpacing = 126;
};

static HermesAppInternal g_Data;

// Forward declarations of helper functions
static int ShowSoftKeyboardInput();

static int PollUnicodeChars();

static int GetAssetData(const char *filename, void **out_data);

void init(struct android_app *app) {
    if (g_Data.m_Initialized)
        return;

    g_Data.m_AndroidApp = app;
    ANativeWindow_acquire(g_Data.m_AndroidApp->window);

    // Initialize EGL
    // This is mostly boilerplate code for EGL...
    {
        g_Data.m_EglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (g_Data.m_EglDisplay == EGL_NO_DISPLAY)
            __android_log_print(ANDROID_LOG_ERROR, HERMESAPP_LOGTAG, "%s",
                                "eglGetDisplay(EGL_DEFAULT_DISPLAY) returned EGL_NO_DISPLAY");

        if (eglInitialize(g_Data.m_EglDisplay, 0, 0) != EGL_TRUE)
            __android_log_print(ANDROID_LOG_ERROR, HERMESAPP_LOGTAG, "%s",
                                "eglInitialize() returned with an error");

        const EGLint egl_attributes[] = {EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8,
                                         EGL_DEPTH_SIZE, 24, EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                                         EGL_NONE};
        EGLint num_configs = 0;
        if (eglChooseConfig(g_Data.m_EglDisplay, egl_attributes, nullptr, 0, &num_configs) !=
            EGL_TRUE)
            __android_log_print(ANDROID_LOG_ERROR, HERMESAPP_LOGTAG, "%s",
                                "eglChooseConfig() returned with an error");
        if (num_configs == 0)
            __android_log_print(ANDROID_LOG_ERROR, HERMESAPP_LOGTAG, "%s",
                                "eglChooseConfig() returned 0 matching config");

        // Get the first matching config
        EGLConfig egl_config;
        eglChooseConfig(g_Data.m_EglDisplay, egl_attributes, &egl_config, 1, &num_configs);
        EGLint egl_format;
        eglGetConfigAttrib(g_Data.m_EglDisplay, egl_config, EGL_NATIVE_VISUAL_ID, &egl_format);
        ANativeWindow_setBuffersGeometry(g_Data.m_AndroidApp->window, 0, 0, egl_format);

        const EGLint egl_context_attributes[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
        g_Data.m_EglContext = eglCreateContext(g_Data.m_EglDisplay, egl_config, EGL_NO_CONTEXT,
                                               egl_context_attributes);

        if (g_Data.m_EglContext == EGL_NO_CONTEXT)
            __android_log_print(ANDROID_LOG_ERROR, HERMESAPP_LOGTAG, "%s",
                                "eglCreateContext() returned EGL_NO_CONTEXT");

        g_Data.m_EglSurface = eglCreateWindowSurface(g_Data.m_EglDisplay, egl_config,
                                                     g_Data.m_AndroidApp->window, NULL);
        eglMakeCurrent(g_Data.m_EglDisplay, g_Data.m_EglSurface, g_Data.m_EglSurface,
                       g_Data.m_EglContext);
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    // Disable loading/saving of .ini file from disk.
    // FIXME: Consider using LoadIniSettingsFromMemory() / SaveIniSettingsToMemory() to save in appropriate location for Android.
    io.IniFilename = NULL;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplAndroid_Init(g_Data.m_AndroidApp->window);
    ImGui_ImplOpenGL3_Init("#version 300 es");

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Android: The TTF files have to be placed into the assets/ directory (android/app/src/main/assets), we use our GetAssetData() helper to retrieve them.

    // We load the default font with increased size to improve readability on many devices with "high" DPI.
    // FIXME: Put some effort into DPI awareness.
    // Important: when calling AddFontFromMemoryTTF(), ownership of font_data is transfered by Dear ImGui by default (deleted is handled by Dear ImGui), unless we set FontDataOwnedByAtlas=false in ImFontConfig
    ImFontConfig font_cfg;
    font_cfg.SizePixels = 22.0f;
    g_Data.m_Font = io.Fonts->AddFontFromMemoryCompressedBase85TTF(
            DroidSansFont_compressed_data_base85,
            24
    );
    IM_ASSERT(g_Data.m_Font != NULL);

    // Arbitrary scale-up
    // FIXME: Put some effort into DPI awareness
    ImGui::GetStyle().ScaleAllSizes(3.0f);

    g_Data.m_Initialized = true;
}

void drawWidgets() {
    ImGui::PushFont(g_Data.m_Font);
    ImGui::Dummy(ImVec2(0, 16));
    ImGui::Text("joebloggs@windowslive.com");
    ImGui::Dummy(ImVec2(0, g_Data.m_MailListSpacing));
    ImGui::Text("earnings report summary");
    ImGui::Dummy(ImVec2(0, g_Data.m_MailListSpacing));
    ImGui::Text("verify login");
    ImGui::PopFont();
}

void tick() {
    ImGuiIO &io = ImGui::GetIO();
    if (g_Data.m_EglDisplay == EGL_NO_DISPLAY)
        return;

    // Our state
    static ImVec4 clear_color = ImVec4(0.7f, 0.7f, 0.7f, 1.00f);

    // Poll Unicode characters via JNI
    // FIXME: do not call this every frame because of JNI overhead
    PollUnicodeChars();

    // Open on-screen (soft) input if requested by Dear ImGui
    static bool WantTextInputLast = false;
    if (io.WantTextInput && !WantTextInputLast)
        ShowSoftKeyboardInput();
    WantTextInputLast = io.WantTextInput;

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame();
    ImGui::NewFrame();
#ifdef IMGUI_HAS_VIEWPORT
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
#else
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
#endif
//    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("Hermes", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
    ImGui::SetWindowFontScale(3);
    drawWidgets();
    ImGui::End();
//    ImGui::PopStyleVar(2);

    // Rendering
    ImGui::Render();
    glViewport(0, 0, (int) io.DisplaySize.x, (int) io.DisplaySize.y);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                 clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    eglSwapBuffers(g_Data.m_EglDisplay, g_Data.m_EglSurface);
}

void shutdown() {
    if (!g_Data.m_Initialized)
        return;

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplAndroid_Shutdown();
    ImGui::DestroyContext();

    if (g_Data.m_EglDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(g_Data.m_EglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE,
                       EGL_NO_CONTEXT);

        if (g_Data.m_EglContext != EGL_NO_CONTEXT)
            eglDestroyContext(g_Data.m_EglDisplay, g_Data.m_EglContext);

        if (g_Data.m_EglSurface != EGL_NO_SURFACE)
            eglDestroySurface(g_Data.m_EglDisplay, g_Data.m_EglSurface);

        eglTerminate(g_Data.m_EglDisplay);
    }

    g_Data.m_EglDisplay = EGL_NO_DISPLAY;
    g_Data.m_EglContext = EGL_NO_CONTEXT;
    g_Data.m_EglSurface = EGL_NO_SURFACE;
    ANativeWindow_release(g_Data.m_AndroidApp->window);

    g_Data.m_Initialized = false;
}

static void handleAppCmd(struct android_app *app, int32_t appCmd) {
    switch (appCmd) {
        case APP_CMD_SAVE_STATE:
            break;
        case APP_CMD_INIT_WINDOW:
            init(app);
            break;
        case APP_CMD_TERM_WINDOW:
            shutdown();
            break;
        case APP_CMD_GAINED_FOCUS:
            break;
        case APP_CMD_LOST_FOCUS:
            break;
    }
}

static int32_t handleInputEvent(struct android_app *app, AInputEvent *inputEvent) {
    return ImGui_ImplAndroid_HandleInputEvent(inputEvent);
}

void android_main(struct android_app *app) {
    app->onAppCmd = handleAppCmd;
    app->onInputEvent = handleInputEvent;

    while (true) {
        int out_events;
        struct android_poll_source *out_data;

        // Poll all events. If the app is not visible, this loop blocks until g_Initialized == true.
        while (ALooper_pollAll(g_Data.m_Initialized ? 0 : -1, NULL, &out_events,
                               (void **) &out_data) >= 0) {
            // Process one event
            if (out_data != NULL)
                out_data->process(app, out_data);

            // Exit the app by returning from within the infinite loop
            if (app->destroyRequested != 0) {
                // shutdown() should have been called already while processing the
                // app command APP_CMD_TERM_WINDOW. But we play save here
                if (!g_Data.m_Initialized)
                    shutdown();

                return;
            }
        }

        // Initiate a new frame
        tick();
    }
}

// Unfortunately, there is no way to show the on-screen input from native code.
// Therefore, we call ShowSoftKeyboardInput() of the main activity implemented in MainActivity.kt
// via JNI.
static int ShowSoftKeyboardInput() {
    JavaVM *java_vm = g_Data.m_AndroidApp->activity->vm;
    JNIEnv *java_env = NULL;

    jint jni_return = java_vm->GetEnv((void **) &java_env, JNI_VERSION_1_6);
    if (jni_return == JNI_ERR)
        return -1;

    jni_return = java_vm->AttachCurrentThread(&java_env, NULL);
    if (jni_return != JNI_OK)
        return -2;

    jclass native_activity_clazz = java_env->GetObjectClass(g_Data.m_AndroidApp->activity->clazz);
    if (native_activity_clazz == NULL)
        return -3;

    jmethodID method_id = java_env->GetMethodID(native_activity_clazz, "showSoftInput",
                                                "()V");
    if (method_id == NULL)
        return -4;

    java_env->CallVoidMethod(g_Data.m_AndroidApp->activity->clazz, method_id);

    jni_return = java_vm->DetachCurrentThread();
    if (jni_return != JNI_OK)
        return -5;

    return 0;
}

// Unfortunately, the native KeyEvent implementation has no getUnicodeChar() function.
// Therefore, we implement the processing of KeyEvents in MainActivity.kt and poll
// the resulting Unicode characters here via JNI and send them to Dear ImGui.
static int PollUnicodeChars() {
    JavaVM *java_vm = g_Data.m_AndroidApp->activity->vm;
    JNIEnv *java_env = NULL;

    jint jni_return = java_vm->GetEnv((void **) &java_env, JNI_VERSION_1_6);
    if (jni_return == JNI_ERR)
        return -1;

    jni_return = java_vm->AttachCurrentThread(&java_env, NULL);
    if (jni_return != JNI_OK)
        return -2;

    jclass native_activity_clazz = java_env->GetObjectClass(g_Data.m_AndroidApp->activity->clazz);
    if (native_activity_clazz == NULL)
        return -3;

    jmethodID method_id = java_env->GetMethodID(native_activity_clazz, "pollUnicodeChar",
                                                "()I");
    if (method_id == NULL)
        return -4;

    // Send the actual characters to Dear ImGui
    ImGuiIO &io = ImGui::GetIO();
    jint unicode_character;
    while ((unicode_character = java_env->CallIntMethod(g_Data.m_AndroidApp->activity->clazz,
                                                        method_id)) != 0)
        io.AddInputCharacter(unicode_character);

    jni_return = java_vm->DetachCurrentThread();
    if (jni_return != JNI_OK)
        return -5;

    return 0;
}

// Helper to retrieve data placed into the assets/ directory (android/app/src/main/assets)
static int GetAssetData(const char *filename, void **outData) {
    int num_bytes = 0;
    AAsset *asset_descriptor = AAssetManager_open(g_Data.m_AndroidApp->activity->assetManager,
                                                  filename, AASSET_MODE_BUFFER);
    if (asset_descriptor) {
        num_bytes = AAsset_getLength(asset_descriptor);
        *outData = IM_ALLOC(num_bytes);
        int64_t num_bytes_read = AAsset_read(asset_descriptor, *outData, num_bytes);
        AAsset_close(asset_descriptor);
        IM_ASSERT(num_bytes_read == num_bytes);
    }
    return num_bytes;
}
