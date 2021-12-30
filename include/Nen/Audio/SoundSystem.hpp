#pragma once
#include "../Math/Math.hpp"
#include "../Math/Vector3.hpp"
#include "../Math/Quaternion.hpp"
#include <memory>
#include <unordered_map>
#include <string>
#include <string_view>
#include "SoundEvent.hpp"

namespace
{
	struct ALCdevice;
	struct ALCcontext;
}

namespace nen
{
	class SoundSystem : public std::enable_shared_from_this<SoundSystem>
	{
	public:
		SoundSystem();
		~SoundSystem();

		bool Initialize();
		void Shutdown();
		void Update(float deltaTime);

		SoundEvent PlayEvent(std::string_view name, uint32_t sourceID = 0);

		void LoadAudioFile(std::string_view fileName);
		void UnloadAudioFile(std::string_view fileName);

		uint32_t NewSource(std::string_view name);
		void DeleteSource(uint32_t sourceID);

		// For positional audio
		void SetListener(const Vector3 &pos, const Quaternion &direction);

	protected:
		friend class SoundEvent;

	private:
		std::unordered_map<std::string, uint32_t> buffers;
		::ALCdevice *device;
		::ALCcontext *context;
	};
}
