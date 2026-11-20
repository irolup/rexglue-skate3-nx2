#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <vector>

#include <rex/ui/windowed_app_context.h>

namespace rex {
namespace ui {

class WindowSwitch2;

class Switch2WindowedAppContext final : public WindowedAppContext {
 public:
  Switch2WindowedAppContext() = default;
  ~Switch2WindowedAppContext() override;

  bool Initialize();
  int RunMainLoop();

  void NotifyUILoopOfPendingFunctions() override;
  void PlatformQuitFromUIThread() override;

  void RegisterWindow(WindowSwitch2* window);
  void UnregisterWindow(WindowSwitch2* window);
  void RequestPaint();

 private:
  std::mutex wake_mutex_;
  std::condition_variable wake_cv_;
  bool wake_pending_ = false;
  std::atomic<bool> paint_pending_{false};
  std::vector<WindowSwitch2*> windows_;
};

}  // namespace ui
}  // namespace rex
