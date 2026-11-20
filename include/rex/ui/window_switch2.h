#pragma once

#include <memory>
#include <string>

#include <rex/ui/window.h>

namespace rex {
namespace ui {

class Switch2WindowedAppContext;

// Nintendo Switch 2: the window is the screen (the default NWindow).
class WindowSwitch2 final : public Window {
 public:
  WindowSwitch2(WindowedAppContext& app_context, const std::string_view title,
                uint32_t desired_logical_width, uint32_t desired_logical_height);
  ~WindowSwitch2() override;

  void HandlePaint();
  void HandleCloseRequest();
  // Recreates the surface (and so the presenter's swapchain connection), e.g. after returning
  // from the HOME menu, where the presenter would otherwise wait for a surface event that the
  // Switch never sends.
  void HandleSurfaceReset();

 protected:
  bool OpenImpl() override;
  void RequestCloseImpl() override;
  std::unique_ptr<Surface> CreateSurfaceImpl(Surface::TypeFlags allowed_types) override;
  void RequestPaintImpl() override;

 private:
  Switch2WindowedAppContext& switch2_context() const;
  bool open_ = false;
};

}  // namespace ui
}  // namespace rex
