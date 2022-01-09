#pragma once

namespace nen {
class EffectManager {
public:
  EffectManager(){};
  virtual ~EffectManager() {}
  virtual void Init() {}
  virtual void Shutdown();

protected:
};
} // namespace nen