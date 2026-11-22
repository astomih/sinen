#ifndef SINEN_TEXT_MODEL_CACHE_HPP
#define SINEN_TEXT_MODEL_CACHE_HPP

#include <graphics/model/model.hpp>

namespace sinen {
// Keys are weak CPU mesh identities, so font reloads, destruction and pointer
// reuse cannot select stale geometry. Graphics clears this on backend teardown.
class TextModelCache {
public:
  static constexpr Size maxEntries = 128;
  static constexpr Size maxBytes = 8 * 1024 * 1024;

  Ptr<Model> acquire(const Mesh &mesh,
                     const Ptr<gpu::CommandBuffer> &commands) {
    const auto data = mesh.data();
    const Size bytes = 2 * (data->vertices.size() * sizeof(Vertex) +
                            data->indices.size() * sizeof(UInt32));
    auto it = entries.find(data.get());
    if (it != entries.end() && it->second.source.lock() != data) {
      erase(it);
      it = entries.end();
    }
    if (it != entries.end()) {
      it->second.lastUsed = ++clock;
      if (it->second.model) {
        return it->second.model;
      }
      // Promote repeated text to persistent GPU buffers; one-off text streams.
      auto model = makePtr<Model>(GlobalAllocator::get());
      model->loadFromVertexArray(mesh, false);
      it->second.model = model;
      return model;
    }
    for (auto expired = entries.begin(); expired != entries.end();) {
      if (expired->second.source.expired()) {
        auto victim = expired++;
        erase(victim);
      } else {
        ++expired;
      }
    }
    auto model = makePtr<Model>(GlobalAllocator::get());
    const bool streamed = model->loadTransientFromVertexArray(mesh, commands);
    if (!streamed) {
      model->loadFromVertexArray(mesh, false);
    }
    if (bytes <= maxBytes) {
      while (!entries.empty() &&
             (entries.size() >= maxEntries || usedBytes + bytes > maxBytes)) {
        auto oldest = entries.begin();
        for (auto candidate = entries.begin(); candidate != entries.end();
             ++candidate) {
          if (candidate->second.lastUsed < oldest->second.lastUsed) {
            oldest = candidate;
          }
        }
        erase(oldest);
      }
      entries.emplace(data.get(),
                      Entry{data, streamed ? nullptr : model, ++clock, bytes});
      usedBytes += bytes;
    }
    return model;
  }

  void clear() {
    entries.clear();
    usedBytes = 0;
  }
  Size size() const { return entries.size(); }
  Size byteSize() const { return usedBytes; }

private:
  struct Entry {
    Ref<Mesh::Data> source;
    Ptr<Model> model;
    UInt64 lastUsed;
    Size bytes;
  };
  using Entries = Hashmap<const Mesh::Data *, Entry>;
  void erase(Entries::iterator it) {
    usedBytes -= it->second.bytes;
    entries.erase(it);
  }
  Entries entries;
  Size usedBytes = 0;
  UInt64 clock = 0;
};
} // namespace sinen

#endif
