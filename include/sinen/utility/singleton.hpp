#ifndef NEN_SINGLETON_HPP
#define NEN_SINGLETON_HPP
#include <functional>
#include <list>
#include <memory>

namespace nen {

class singleton_finalizer {
public:
  static void AddFinalizer(const std::function<void(void)> &func);
  static void Finalize();

private:
  static std::list<std::function<void(void)>> finalizers;
};
template <class T> class Singleton {
public:
  static T &Get() {
    static auto once = []() {
      Create();
      return true;
    }();
    return *mInstance.get();
  }

private:
  static void Create() {
    mInstance = std::make_unique<T>();
    singleton_finalizer::AddFinalizer(Singleton<T>::Destroy);
  }
  static void Destroy() { mInstance = nullptr; }
  static std::unique_ptr<T> mInstance;
};
} // namespace nen
#endif
