// Dear ImGui: standalone example application for SDL2 + OpenGL
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <cstdio>
#include <SDL.h>
#include "json.hpp"
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#include <imgui_internal.h>
#include <fstream>

#endif
#include "droidsansfont.h"
#define IG ImGui

namespace HermesInternal {
    using std::string;
    using nlohmann::json;
    struct AppData {
        bool m_initialized = false;
        ImFont *m_font = nullptr;
        ImGuiIO *m_imgui_io = nullptr;
        bool m_show_setup_window = false;
        SDL_GLContext m_sdl_gl_context = nullptr;
        const int m_mail_list_spacing = 126;
        SDL_Window *m_window;
    };

    struct MailAccount {
        string m_alias;
        std::unordered_map<string, string> m_settings;
    };

    struct AppConfigSerializable {
        std::vector<MailAccount> m_mail_accounts;
        json m_json_data;
        void load_from_file(const char* path) {
            std::ifstream file_reader(path);
//            string file_text;
//            string text;
//            while (file_reader.good()) {
//                std::getline(file_reader, text);
//                file_text += text;
//            }
            m_json_data = file_text;
        }
    };
    //various type conversion functions called by json constructor
    //for to_json functions, and get for from_json functions
    //MailAccount
    void to_json(json& j, MailAccount& m) {
        j["alias"] = m.m_alias;
        j["settings"] = m.m_settings;
    }
    void from_json(json& j, MailAccount& m) {
        j.at("alias").get_to(m.m_alias);
        j.at("settings").get_to(m.m_settings);
    }
    //AppConfig
    void from_json(json& j, std::vector<MailAccount>& m) {

    }
    void to_json(json& j, std::vector<MailAccount>& m) {

    }
}

static HermesInternal::AppData g_data {};

void load_config() {

}

void save_config() {

}

//initialize g_data
void init() {
    // Setup SDL
    // (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
    // depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to latest version of SDL is recommended!)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        exit(-1);
    }
    //g_data.m_show_setup_window = true;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    g_data.m_window = SDL_CreateWindow("Hermes", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(g_data.m_window);
    SDL_GL_MakeCurrent(g_data.m_window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    g_data.m_imgui_io = &ImGui::GetIO(); (void)g_data.m_imgui_io;
    g_data.m_imgui_io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    g_data.m_imgui_io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
//    g_data.m_imgui_io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsClassic();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
//    if (g_data.m_imgui_io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
//    {
//        style.WindowRounding = 0.0f;
//        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
//    }
//style.WindowRounding = 2;

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(g_data.m_window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Our state
    ImFontConfig font_cfg;
    font_cfg.SizePixels = 22.0f;
    g_data.m_font = g_data.m_imgui_io->Fonts->AddFontFromMemoryCompressedBase85TTF(
            DroidSansFont_compressed_data_base85,
            28
    );
    IM_ASSERT(g_data.m_font != NULL);

    // Arbitrary scale-up
    ImGui::GetStyle().ScaleAllSizes(3.0f);

    g_data.m_initialized = true;
}

void draw_mailbox_entry() {
    ImGui::BeginGroup();
//    ImDrawList* draw_list = ImGui::GetWindowDrawList();
//    draw_list->AddRectFilled(ImVec2(0, 0),ImVec2(ImGui::GetWindowWidth(), 300), IM_COL32(0,75,200,255));
    ImGui::Text("title");
    ImGui::Text("subject");
    ImGui::Separator();
    ImGui::EndGroup();
}

void draw_content_viewer() {
    ImGui::BeginGroup();
    ImGui::Text("title");
    ImGui::Separator();
    ImGui::Text("subject");
    ImGui::Separator();
    ImGui::TextWrapped("this is really long message text to test wrapping across multiple lines");
    ImGui::EndGroup();
}

void draw_widgets() {
    const auto main_side_panel_id = 1;
    const auto context_list_id = 2;
    const auto content_viewer_id = 3;
    static bool show_setup_dialog = true;
    ImGui::PushFont(g_data.m_font);
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    //ImGui::SetNextWindowSize(ImVec2(200, ImGui::GetWindowHeight()));
    //ImGui::SetNextWindowDockID(0);
//    ImGui::SameLine(0,0);
    ImGui::SetNextWindowPos(ImVec2(-32,-32));
//    IG::Spacing();
//    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(32,32));
ImVec2 window_size = IG::GetWindowSize();
window_size.x -= 36;
window_size.y -= 64;
ImGui::BeginTable("main_layout_table", 3,
                      ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable
                      , window_size);
    //start of main_layout_table
    ImGui::TableNextRow(ImGuiTableFlags_ScrollY, window_size.y);
    ImGui::TableSetColumnIndex(0);
//    ImGui::BeginChildFrame(main_side_panel_id,ImVec2(ImGui::GetContentRegionAvail().x*0.33, ImGui::GetWindowHeight())
//                           ImGuiWindowFlags_NoTitleBar);
    ImGui::Text("jsmith@gmail.com");
    ImGui::Separator();
    ImGui::PushItemWidth(-100);
    ImGui::Button("Inbox");
    ImGui::Button("Sent Items");
    ImGui::Button("Junk");
    ImGui::Button("Drafts");
    ImGui::PopItemWidth();
    //ImGui::EndChild();
    ImGui::TableSetColumnIndex(1);
    ImRect context_list_cell_rect = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), 1);

    ImGui::SetNextWindowPos(context_list_cell_rect.GetTL());
    ImGui::BeginChild(context_list_id, context_list_cell_rect.GetSize(), true,
                      ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_HorizontalScrollbar);
    for (int i =0; i < 300; i++) { draw_mailbox_entry(); }
    ImGui::EndChild();
    //ImGui::SetNextWindowPos(ImVec2(ImGui::GetContentRegionAvail().x*0.33*2,0));
    ImGui::TableSetColumnIndex(2);
//    ImGui::BeginChild(content_viewer_id, ImVec2(ImGui::GetContentRegionAvail().x*0.33, ImGui::GetWindowHeight()),
//                      true, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_HorizontalScrollbar);
    //ImGui::BeginChild
    draw_content_viewer();
    //ImGui::EndChild();
    if (ImGui::BeginPopup("Account Setup")) {
        ImGui::Text("blah");
        ImGui::EndPopup();
    }
    ImGui::PopFont();
    //end of main_layout_table
    ImGui::EndTable();
//    ImGui::PopStyleVar();
}

void update_frame() {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(0,0));
    ImGui::SetNextWindowSize(g_data.m_imgui_io->DisplaySize);
    ImGui::Begin("Hermes", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
//        ImGui::SetWindowFontScale(3);
    IG::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16,16));
    if (!g_data.m_show_setup_window) { draw_widgets(); }
    IG::PopStyleVar();
    ImGui::End();

    // Rendering
    ImGui::Render();
    glViewport(0, 0, (int)g_data.m_imgui_io->DisplaySize.x, (int)g_data.m_imgui_io->DisplaySize.y);
    glClearColor(0,0,0,255);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
    //  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)

    SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
    SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
//        ImGui::UpdatePlatformWindows();
//        ImGui::RenderPlatformWindowsDefault();
    SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
    SDL_GL_SwapWindow(g_data.m_window);
}

// Main code
int main(int, char**)
{
    init();
    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE
            && event.window.windowID == SDL_GetWindowID(g_data.m_window))
                done = true;
        }
        update_frame();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(g_data.m_sdl_gl_context);
    SDL_DestroyWindow(g_data.m_window);
    SDL_Quit();

    return 0;
}
