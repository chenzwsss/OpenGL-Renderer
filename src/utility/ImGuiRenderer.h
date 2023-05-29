#ifndef IMGUI_RENDERER_H
#define IMGUI_RENDERER_H

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

class ImGuiRenderer {
    public:
        static auto& getInstance() {
            static ImGuiRenderer instance;
            return instance;
        }

        void setupImGui(GLFWwindow* window);
        void renderImGui();
        void destroyImGui();

        static bool render_wireframe;
};

#endif