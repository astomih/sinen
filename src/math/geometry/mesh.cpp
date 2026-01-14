#include "mesh.hpp"
namespace sinen {
Mesh::Mesh() { _data = makePtr<Data>(); }
Mesh::Mesh(const Data &data) {
  _data = makePtr<Data>();
  *_data = data;
}
Mesh::Mesh(const Ptr<Data> &data) : _data(data) {}
} // namespace sinen
