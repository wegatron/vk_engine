#include <framework/platform/glfw_window.h>
#include <framework/app_template/app_base.h>
#include <framework/utils/base/error.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/imgui.h>
//#include <imgui/backends/imgui_impl_vulkan.h>

namespace vk_engine {

static MouseButton translateMouseButton(int button) {
  if (button < GLFW_MOUSE_BUTTON_6) {
    return static_cast<MouseButton>(button);
  }

  return MouseButton::Unknown;
}

static MouseAction translateMouseAction(int action) {
  if (action == GLFW_PRESS) {
    return MouseAction::Down;
  } else if (action == GLFW_RELEASE) {
    return MouseAction::Up;
  }

  return MouseAction::Unknown;
}

static void cursorPositionCallback(GLFWwindow *window, double xpos, double ypos) {
  if(ImGui::GetIO().WantCaptureMouse) return;
  if (auto *app =
          reinterpret_cast<AppBase *>(glfwGetWindowUserPointer(window))) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    xpos /= width;
    ypos /= height;    
    app->inputMouseEvent(std::make_shared<MouseInputEvent>(false,
        MouseButton::Unknown, MouseAction::Move, static_cast<float>(xpos),
        static_cast<float>(ypos)));
  }
}

static void mouseButtonCallback(GLFWwindow *window, int button, int action,
                         int /*mods*/) {
  if(ImGui::GetIO().WantCaptureMouse) return;
  MouseAction mouse_action = translateMouseAction(action);

  if (auto *app =
          reinterpret_cast<AppBase *>(glfwGetWindowUserPointer(window))) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    xpos /= width;
    ypos /= height;
    app->inputMouseEvent(std::make_shared<MouseInputEvent>(
        false, translateMouseButton(button), mouse_action,
        static_cast<float>(xpos), static_cast<float>(ypos)));
  }
}

static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
  if(ImGui::GetIO().WantCaptureMouse) return;
  if (AppBase* app = reinterpret_cast<AppBase*>(glfwGetWindowUserPointer(window))) {
    app->inputMouseEvent(std::make_shared<MouseInputEvent>(false,
        MouseButton::Unknown, MouseAction::Scroll, static_cast<float>(xoffset),
        static_cast<float>(yoffset)));    
  }
}

GlfwWindow::GlfwWindow(const std::string &window_title, const int width,
                       const int height)
    : Window(window_title, width, height) {

#if defined(VK_USE_PLATFORM_XLIB_KHR)
  glfwInitHint(GLFW_X11_XCB_VULKAN_SURFACE, false);
#endif
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window_ =
      glfwCreateWindow(width, height, window_title.c_str(), nullptr, nullptr);
}

bool GlfwWindow::shouldClose() { return glfwWindowShouldClose(window_); }

VkSurfaceKHR GlfwWindow::createSurface(VkInstance instance) {
  VkSurfaceKHR surface;
  VK_THROW_IF_ERROR(
      glfwCreateWindowSurface(instance, window_, nullptr, &surface),
      "failed to create window surface");
  return surface;
}

void GlfwWindow::initImgui() {
  ImGui_ImplGlfw_InitForVulkan(window_,
                               true); // init viewport and key/mouse events
}

void GlfwWindow::shutdownImgui()
{
  ImGui_ImplGlfw_Shutdown();
}
void GlfwWindow::imguiNewFrame()
{
  ImGui_ImplGlfw_NewFrame();
}

void GlfwWindow::getExtent(uint32_t &width, uint32_t &height) const {
  glfwGetWindowSize(window_, reinterpret_cast<int *>(&width),
                    reinterpret_cast<int *>(&height));
  width = std::max(width, 1u);
  height = std::max(height, 1u);
}

void GlfwWindow::setupCallback(AppBase *app) {
  glfwSetWindowUserPointer(window_, app);
  glfwSetCursorPosCallback(window_, cursorPositionCallback);
  glfwSetMouseButtonCallback(window_, mouseButtonCallback);
  glfwSetScrollCallback(window_, scrollCallback);
}

GlfwWindow::~GlfwWindow() {
  glfwDestroyWindow(window_);
  glfwTerminate();
}
} // namespace vk_engine