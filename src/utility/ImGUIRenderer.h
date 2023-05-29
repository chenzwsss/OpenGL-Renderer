#ifndef IMGUI_RENDERER_H
#define IMGUI_RENDERER_H

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

class ImGUIRenderer {
    public:
        static auto& getInstance() {
            static ImGUIRenderer instance;
            return instance;
        }

        void setupImGUI(GLFWwindow* window);
        void renderImGUI();
        void destroyImGUI();

        static bool render_wireframe;
};

#endif