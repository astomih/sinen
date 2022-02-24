#include "SurfaceHandle.hpp"
#include <Nen.hpp>
#include <SDL.h>
#include <SDL_image.h>

namespace nen {

void SDLObjectCloser::operator()(::SDL_Surface *surface) {
  if (surface != nullptr) {
    ::SDL_FreeSurface(surface);
    surface = nullptr;
  }
}
void SDLObjectCloser::operator()(::SDL_RWops *rw) {
  if (rw != nullptr) {
    ::SDL_FreeRW(rw);
    rw = nullptr;
  }
}
std::unordered_map<std::string, std::unique_ptr<::SDL_Surface, SDLObjectCloser>>
    surface_handler::surfaces =
        std::unordered_map<std::string,
                           std::unique_ptr<::SDL_Surface, SDLObjectCloser>>{};
void surface_handler::Register(
    std::string_view string,
    std::unique_ptr<::SDL_Surface, SDLObjectCloser> surface) {
  surfaces.insert_or_assign(std::string(string), std::move(surface));
}
bool surface_handler::Contain(std::string_view string) {
  return surfaces.contains(std::string(string));
}
SDL_Surface &surface_handler::Load(std::string_view string) {
  return *surfaces[std::string(string)].get();
}
class texture::Impl {};
texture::texture() : impl(nullptr), width(0), height(0) {}
texture::texture(std::string_view file_name)
    : impl(nullptr), width(0), height(0) {
  Load(file_name);
}

texture::~texture() = default;

bool texture::Load(std::string_view fileName) {
  id = fileName.data();
  auto *surface = ::IMG_Load_RW(
      (SDL_RWops *)asset_reader::LoadAsRWops(asset_type::Texture, fileName), 0);
  if (surface) {
    width = surface->w;
    height = surface->h;
  } else {
    logger::Error("%s", ::IMG_GetError());
    return false;
  }
  surface_handler::Register(
      id, std::move(std::unique_ptr<::SDL_Surface, SDLObjectCloser>(surface)));
  return true;
}
bool texture::LoadFromMemory(std::vector<char> &buffer, std::string_view ID) {
  id = ID.data();
  auto rw = std::unique_ptr<::SDL_RWops, SDLObjectCloser>(
      ::SDL_RWFromMem(reinterpret_cast<void *>(buffer.data()), buffer.size()));
  if (!rw) {
    logger::Error("%s", IMG_GetError());
    return false;
  }

  auto surface =
      std::unique_ptr<::SDL_Surface, SDLObjectCloser>(IMG_Load_RW(rw.get(), 1));
  if (!surface) {
    logger::Error("%s", IMG_GetError());
    return false;
  }
  width = surface->w;
  height = surface->h;

  surface_handler::Register(id, std::move(surface));

  return true;
}

void texture::CreateFromColor(const color &color, std::string_view ID) {
  id = ID.data();
  auto surface = ::SDL_CreateRGBSurface(0, 1, 1, 32, 0, 0, 0, 0);
  ::SDL_FillRect(surface, NULL,
                 ::SDL_MapRGBA(surface->format, color.r * 255, color.g * 255,
                               color.b * 255, color.a * 255));
  width = surface->w;
  height = surface->h;
  auto s = std::unique_ptr<::SDL_Surface, SDLObjectCloser>(surface);
  surface_handler::Register(ID, std::move(s));
}
} // namespace nen
