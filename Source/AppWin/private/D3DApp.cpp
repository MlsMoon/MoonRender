#include "../public/D3DApp.h"

#include <cassert>
#include <sstream>
#include <string>

#include <GLFW/glfw3.h>

#include "Source/ThirdParty/ImGui/imgui_impl_glfw.h"

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

#include "../public/D3DUtil.h"
#include "Source/Graphics/public/GraphicsBackendFactory.h"
#include "Source/Logging/public/LogSystem.h"
#include "Source/ThirdParty/ImGui/imgui.h"

namespace
{
    void ApplyMoonEditorStyle(float dpiScale)
    {
        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowPadding = ImVec2(12.0f, 10.0f);
        style.FramePadding = ImVec2(10.0f, 6.0f);
        style.CellPadding = ImVec2(8.0f, 5.0f);
        style.ItemSpacing = ImVec2(8.0f, 8.0f);
        style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);
        style.ScrollbarSize = 13.0f;
        style.GrabMinSize = 10.0f;
        style.WindowBorderSize = 1.0f;
        style.ChildBorderSize = 1.0f;
        style.PopupBorderSize = 1.0f;
        style.FrameBorderSize = 0.0f;
        style.TabBorderSize = 0.0f;
        style.WindowRounding = 6.0f;
        style.ChildRounding = 6.0f;
        style.FrameRounding = 5.0f;
        style.PopupRounding = 6.0f;
        style.ScrollbarRounding = 8.0f;
        style.GrabRounding = 5.0f;
        style.TabRounding = 5.0f;
        style.WindowMenuButtonPosition = ImGuiDir_Right;

        ImVec4* colors = style.Colors;
        colors[ImGuiCol_Text] = ImVec4(0.92f, 0.94f, 0.97f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.48f, 0.52f, 0.58f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.10f, 0.12f, 0.96f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.075f, 0.085f, 0.10f, 1.00f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.11f, 0.13f, 0.98f);
        colors[ImGuiCol_Border] = ImVec4(0.20f, 0.23f, 0.27f, 1.00f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.17f, 0.20f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.26f, 0.31f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.28f, 0.34f, 0.42f, 1.00f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.09f, 0.11f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.14f, 0.17f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.08f, 0.09f, 0.11f, 0.75f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.075f, 0.085f, 0.10f, 1.00f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.07f, 0.08f, 0.09f, 1.00f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.27f, 0.30f, 0.35f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.36f, 0.40f, 0.46f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.46f, 0.51f, 0.58f, 1.00f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.41f, 0.72f, 0.96f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.36f, 0.62f, 0.87f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.46f, 0.74f, 1.00f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(0.17f, 0.20f, 0.24f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.30f, 0.36f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.32f, 0.39f, 0.47f, 1.00f);
        colors[ImGuiCol_Header] = ImVec4(0.18f, 0.23f, 0.29f, 1.00f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.32f, 0.40f, 1.00f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.31f, 0.40f, 0.50f, 1.00f);
        colors[ImGuiCol_Separator] = ImVec4(0.21f, 0.24f, 0.28f, 1.00f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.36f, 0.62f, 0.87f, 1.00f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.46f, 0.74f, 1.00f, 1.00f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.25f, 0.30f, 0.36f, 0.65f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.36f, 0.62f, 0.87f, 0.80f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.46f, 0.74f, 1.00f, 1.00f);
        colors[ImGuiCol_Tab] = ImVec4(0.13f, 0.15f, 0.18f, 1.00f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.25f, 0.32f, 0.40f, 1.00f);
        colors[ImGuiCol_TabActive] = ImVec4(0.18f, 0.23f, 0.29f, 1.00f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.10f, 0.11f, 0.13f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.16f, 0.19f, 1.00f);
        colors[ImGuiCol_DockingPreview] = ImVec4(0.36f, 0.62f, 0.87f, 0.70f);
        colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.06f, 0.07f, 0.08f, 1.00f);
        colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.68f, 0.76f, 1.00f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.41f, 0.72f, 0.96f, 1.00f);
        colors[ImGuiCol_TableHeaderBg] = ImVec4(0.13f, 0.15f, 0.18f, 1.00f);
        colors[ImGuiCol_TableBorderStrong] = ImVec4(0.22f, 0.25f, 0.29f, 1.00f);
        colors[ImGuiCol_TableBorderLight] = ImVec4(0.15f, 0.17f, 0.20f, 1.00f);
        colors[ImGuiCol_TextSelectedBg] = ImVec4(0.36f, 0.62f, 0.87f, 0.35f);
        colors[ImGuiCol_NavHighlight] = ImVec4(0.46f, 0.74f, 1.00f, 0.75f);

        if (dpiScale > 1.0f)
        {
            style.ScaleAllSizes(dpiScale);
        }
    }
}

D3DApp::D3DApp(
    const std::string& windowName,
    int initWidth,
    int initHeight,
    GraphicsBackendType backendType)
    : m_window(nullptr),
      m_AppPaused(false),
      m_Minimized(false),
      m_Maximized(false),
      m_Resizing(false),
      m_Enable4xMsaa(true),
      m_MainWndCaption(windowName),
      m_GraphicsBackendType(backendType),
      ClientWidth(initWidth),
      ClientHeight(initHeight),
      FramebufferWidth(initWidth),
      FramebufferHeight(initHeight)
{
}

D3DApp::~D3DApp()
{
    if (ImGui::GetCurrentContext() != nullptr)
    {
        if (m_GraphicsBackend)
        {
            m_GraphicsBackend->ShutdownImGui();
        }

        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    if (m_window)
    {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
}

GLFWwindow* D3DApp::GetWindow() const
{
    return m_window;
}

float D3DApp::AspectRatio() const
{
    return static_cast<float>(ClientWidth) / ClientHeight;
}

void D3DApp::UpdateFramebufferSize()
{
    if (m_window)
    {
        glfwGetWindowSize(m_window, &ClientWidth, &ClientHeight);
        glfwGetFramebufferSize(m_window, &FramebufferWidth, &FramebufferHeight);
    }
}

GraphicsBackendType D3DApp::GetGraphicsBackendType() const
{
    return m_GraphicsBackendType;
}

int D3DApp::Run()
{
    m_Timer.Reset();

    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();

        glfwGetCursorPos(m_window, &MouseX, &MouseY);
        MouseButtonLeft = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

        m_Timer.Tick();

        if (!m_AppPaused)
        {
            CalculateFrameStats();

            Graphics().BeginImGuiFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            DrawUI();
            UpdateScene(m_Timer.DeltaTime());
            DrawScene();
        }
    }

    return 0;
}

bool D3DApp::Init()
{
    if (!glfwInit())
    {
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    if (!InitMainWindow())
    {
        return false;
    }

    MOON_LOG("WindowSize: " + std::to_string(ClientWidth) + "x" + std::to_string(ClientHeight)
        + " Framebuffer: " + std::to_string(FramebufferWidth) + "x" + std::to_string(FramebufferHeight));

    if (!InitGraphicsBackend())
    {
        return false;
    }

    if (!InitImGui())
    {
        return false;
    }

    return true;
}

void D3DApp::OnResize()
{
    if (m_GraphicsBackend)
    {
        m_GraphicsBackend->Resize(FramebufferWidth, FramebufferHeight);
    }
}

bool D3DApp::InitMainWindow()
{
    m_window = glfwCreateWindow(ClientWidth, ClientHeight, m_MainWndCaption.c_str(), nullptr, nullptr);
    if (!m_window)
    {
        return false;
    }

    UpdateFramebufferSize();

    glfwSetWindowUserPointer(m_window, this);

    glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, int /*width*/, int /*height*/)
    {
        auto* app = static_cast<D3DApp*>(glfwGetWindowUserPointer(window));
        app->UpdateFramebufferSize();
        app->OnResize();
    });

    glfwSetWindowIconifyCallback(m_window, [](GLFWwindow* window, int iconified)
    {
        auto* app = static_cast<D3DApp*>(glfwGetWindowUserPointer(window));
        app->m_Minimized = iconified;
        if (iconified)
        {
            app->m_AppPaused = true;
            app->m_Timer.Stop();
        }
        else
        {
            app->m_AppPaused = false;
            app->m_Timer.Start();
        }
    });

    return true;
}

bool D3DApp::InitGraphicsBackend()
{
    m_GraphicsBackend = CreateGraphicsBackend(m_GraphicsBackendType);
    if (!m_GraphicsBackend)
    {
        return false;
    }

    if (!m_GraphicsBackend->Initialize(m_window, FramebufferWidth, FramebufferHeight, m_Enable4xMsaa))
    {
        return false;
    }

    return true;
}

void D3DApp::CalculateFrameStats()
{
    static int frameCnt = 0;
    static float timeElapsed = 0.0f;

    frameCnt++;
    if ((m_Timer.TotalTime() - timeElapsed) >= 1.0f)
    {
        const float fps = static_cast<float>(frameCnt);
        const float mspf = 1000.0f / fps;

        std::ostringstream outs;
        outs.precision(6);
        outs << m_MainWndCaption << "    "
            << "FPS: " << fps << "    "
            << "Frame Time: " << mspf << " (ms)";
        glfwSetWindowTitle(m_window, outs.str().c_str());

        frameCnt = 0;
        timeElapsed += 1.0f;
    }
}

bool D3DApp::InitImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    float dpiScale = 1.0f;
    float xscale = 1.0f, yscale = 1.0f;
    glfwGetWindowContentScale(m_window, &xscale, &yscale);
    dpiScale = xscale;

    ApplyMoonEditorStyle(dpiScale);
    io.FontGlobalScale = 1.0f;

    MoonEnsureDirectory("Builds/Runtime");
    static std::string imguiIniPath = MoonGetAssetPath("Builds/Runtime/imgui.ini");
    io.IniFilename = imguiIniPath.c_str();

    if (!ImGui_ImplGlfw_InitForOther(m_window, true))
    {
        return false;
    }

    const std::string fontPath = MoonGetAssetPath("Resources/Fonts/hanyiyingsong45jian.ttf");
    ImFontConfig fontConfig = {};
    fontConfig.OversampleH = 3;
    fontConfig.OversampleV = 2;
    fontConfig.PixelSnapH = false;
    ImFont* font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 16.0f * dpiScale, &fontConfig);
    io.FontDefault = font;

    unsigned char* pixels = nullptr;
    int width = 0;
    int height = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    return Graphics().InitializeImGui(m_window);
}

void D3DApp::DrawUI()
{
}

IGraphicsBackend& D3DApp::Graphics()
{
    assert(m_GraphicsBackend != nullptr);
    return *m_GraphicsBackend;
}

const IGraphicsBackend& D3DApp::Graphics() const
{
    assert(m_GraphicsBackend != nullptr);
    return *m_GraphicsBackend;
}
