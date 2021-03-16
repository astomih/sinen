#include <Engine.hpp>
#include <SDL_image.h>

std::string TextureAsset::texname = "default";
std::unordered_map<std::string, std::shared_ptr<Texture>> TextureAsset::mTexture;
Texture::Texture()
	: m_surface(nullptr), width(0), height(0)
{
}

Texture::~Texture()
{
}

bool Texture::Load(std::string_view fileName)
{
	id = fileName.data();
	if (m_surface != nullptr)
	{
		m_surface.reset();
	}
	m_surface = std::unique_ptr<SDL_Surface, SDLObjectCloser>(IMG_Load(fileName.data()));
	if (m_surface)
	{
		width = m_surface->w;
		height = m_surface->h;
		return true;
	}
	SDL_Log("%s", IMG_GetError());
	return false;
}
bool Texture::LoadFromMemory(std::vector<char> &buffer, std::string_view ID)
{
	id = ID.data();
	auto rw = std::unique_ptr<SDL_RWops, SDLObjectCloser>(SDL_RWFromMem(reinterpret_cast<void *>(buffer.data()), buffer.size()));
	if (!rw)
	{
		SDL_Log("%s", SDL_GetError());
		return false;
	}

	auto temp = std::unique_ptr<SDL_Surface, SDLObjectCloser>(IMG_Load_RW(rw.get(), 1));
	if (!temp)
	{
		SDL_Log("%s", IMG_GetError());
		return false;
	}

	SDL_LockSurface(temp.get());

	auto formatbuf = SDL_AllocFormat(SDL_PIXELFORMAT_ABGR8888);
	formatbuf->BytesPerPixel = 4;
	if (m_surface != nullptr)
	{
		m_surface.reset();
	}

	m_surface = std::unique_ptr<SDL_Surface, SDLObjectCloser>(SDL_ConvertSurface(temp.get(), formatbuf, 0));
	if (!m_surface)
	{
		SDL_Log("%s", SDL_GetError());
		return false;
	}

	SDL_UnlockSurface(temp.get());

	width = m_surface->w;
	height = m_surface->h;

	return true;
}
void Texture::SetSurface(std::unique_ptr<SDL_Surface, SDLObjectCloser> surface)
{
	width = surface->w;
	height = surface->h;
	m_surface = std::move(surface);
}
const SDL_Surface &Texture::GetSurface()
{
	if (m_surface)
		return *m_surface.get();
	m_surface = std::unique_ptr<SDL_Surface, SDLObjectCloser>(SDL_CreateRGBSurface(NULL, 32, 32, 32, 0, 0, 0, 0));
	SDL_FillRect(m_surface.get(), NULL, SDL_MapRGBA(m_surface->format, 0, 0, 0, 0));
	if (m_surface)
		return *m_surface.get();
	std::cout << SDL_GetError() << std::endl;
}