// Switch 2 (Horizon) audio backend over libnx audout. Implementation: ns2recomp/platform/audio_switch2.cpp
#pragma once

#include <memory>

#include <rex/audio/audio_system.h>

namespace rex::audio::switch2 {

class Switch2AudioSystem : public AudioSystem {
 public:
  explicit Switch2AudioSystem(runtime::FunctionDispatcher* function_dispatcher);
  ~Switch2AudioSystem() override;

  static bool IsAvailable() { return true; }
  static std::unique_ptr<AudioSystem> Create(runtime::FunctionDispatcher* function_dispatcher);

  X_STATUS CreateDriver(size_t index, rex::thread::Semaphore* semaphore,
                        AudioDriver** out_driver) override;
  void DestroyDriver(AudioDriver* driver) override;
};

}  // namespace rex::audio::switch2
