#ifndef SINEN_SINGLETON_HPP
#define SINEN_SINGLETON_HPP
#include <functional>
#include <list>
#include <memory>
namespace sinen {
class singleton_finalizer {
public:
  static void add(const std::function<void(void)> &func);
  static void finalize();

private:
  static std::list<std::function<void(void)>> finalizers;
};
template <class T> class singleton {
public:
  static T &get() {
    if (!once) {
      create();
      once = true;
    }
    return *m_instance.get();
  }

private:
  static void create() {
    m_instance = std::make_unique<T>();
    singleton_finalizer::add(singleton<T>::destroy);
  }
  static void destroy() { m_instance = nullptr; }
  static std::unique_ptr<T> m_instance;
  static bool once;
};
template <class T> std::unique_ptr<T> singleton<T>::m_instance = nullptr;
template <class T> bool singleton<T>::once = false;
} // namespace sinen
#endif // !SINEN_SINGLETON_HPP
