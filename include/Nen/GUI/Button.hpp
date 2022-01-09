#pragma once
#include "../Math/Vector2.hpp"
#include "../Texture/Texture.hpp"
#include <functional>
namespace nen {
class button {
public:
  button(const std::string &name, std::shared_ptr<class font> font,
         std::function<void()> onClick, const vector2 &pos,
         const vector2 &dims);
  ~button();

  // Set the name of the button
  void SetName(const std::string &name);

  // Getters/setters
  std::shared_ptr<class texture> GetNameTex() { return mNameTex; }
  const vector2 &GetPosition() const { return mPosition; }
  void SetHighlighted(bool sel) { mHighlighted = sel; }
  bool GetHighlighted() const { return mHighlighted; }

  // Returns true if the point is within the button's bounds
  bool ContainsPoint(const vector2 &pt) const;
  // Called when button is clicked
  void OnClick();

private:
  std::function<void()> mOnClick;
  std::string mName;
  std::shared_ptr<texture> mNameTex;
  std::shared_ptr<class font> mFont;
  vector2 mPosition;
  vector2 mDimensions;
  bool mHighlighted;
};
} // namespace nen