#include "ImGuiRenderer.h"

bool ImGuiRenderer::render_wireframe = false;

void ImGuiRenderer::setupImGui(GLFWwindow* window) {
    // Setup Dear ImGui content
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 420 core");
}


void ImGuiRenderer::renderImGui() {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    {
        ImGui::Begin("Opengl Renderer");
        ImGuiIO& io = ImGui::GetIO();
        ImGui::Text("%.3f ms/frame (%d fps)", 1000.0f / io.Framerate, static_cast<int>(io.Framerate));

        if (ImGui::CollapsingHeader("Settings"))
        {
            ImGui::Checkbox("Wireframe", &render_wireframe);
        }

        ImGui::End();
    }

    ImGui::Render();
    // ImGui draw
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiRenderer::destroyImGui() {
    // ImGui Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}