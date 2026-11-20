#pragma once

#include <cstdint>

#include <rex/ui/surface.h>

namespace rex {
namespace ui {

// The process's one native window (libnx/libounce NWindow, from nwindowGetDefault()).
class Switch2ViSurface final : public Surface {
 public:
  explicit Switch2ViSurface(void* nwindow) : nwindow_(nwindow) {}
  TypeIndex GetType() const override { return kTypeIndex_Switch2Vi; }
  void* nwindow() const { return nwindow_; }

 protected:
  bool GetSizeImpl(uint32_t& width_out, uint32_t& height_out) const override;

 private:
  void* nwindow_;
};

}  // namespace ui
}  // namespace rex
