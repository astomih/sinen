#ifndef SINEN_MATH_RECT_HPP
#define SINEN_MATH_RECT_HPP
namespace sinen {
class rect {
public:
  float x, y, w, h;
  rect() : x(0.f), y(0.f), w(0.f), h(0.f) {}

  /**
   * @brief Construct a new rect object
   *
   * @param x_ x position
   * @param y_ y position
   * @param w_ width
   * @param h_ height
   */
  void set(float x_, float y_, float w_, float h_) {
    x = x_;
    y = y_;
    w = w_;
    h = h_;
  }

  /**
   * @brief Calculate the area of the rect
   *
   * @return float
   */
  float area() const { return w * h; }

  /**
   * @brief Check if the rect contains a point
   *
   * @param x_
   * @param y_
   * @return true
   * @return false
   */
  bool contains(float x_, float y_) const {
    return x_ >= x && x_ <= x + w && y_ >= y && y_ <= y + h;
  }

  /**
   * @brief Check if the rect contains another rect
   *
   * @param r
   * @return true
   * @return false
   */
  bool contains(const rect &r) const {
    return contains(r.x, r.y) && contains(r.x + r.w, r.y + r.h);
  }
};
} // namespace sinen
#endif // !SINEN_MATH_RECT_HPP