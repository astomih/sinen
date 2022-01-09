#pragma once

namespace nen {
class json_reader {
public:
  json_reader();
  ~json_reader();

  bool Open();
  void Close();

private:
};
} // namespace nen