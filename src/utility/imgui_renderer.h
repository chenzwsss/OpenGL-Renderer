#ifndef IMGUI_RENDERER_H
#define IMGUI_RENDERER_H

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

class imgui_renderer {
    public:
        static auto& get_instance() {
            static imgui_renderer instance;
            return instance;
        }

        void setup_imgui(GLFWwindow* window);
        void render_imgui();
        void destroy_imgui();

        static bool render_wireframe;
};

#endif