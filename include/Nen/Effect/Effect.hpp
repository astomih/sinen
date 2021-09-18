#pragma once
#include "../Math/Vector3.hpp"
#include "../Math/Quaternion.hpp"
#include <string>

namespace nen
{
	class Effect
	{
	public:
		Effect(std::string_view path);
		~Effect();
		const Vector3 &GetPosition() { return this->position; }
		const Quaternion &GetRotation() { return this->rotate; }
		void SetPosition(const Vector3 &pos) { this->position = pos; }
		void SetRotation(const Quaternion &rot) { this->rotate = rot; }
		const std::u16string &GetPath() { return path; }
		int handle;

		bool isLoop() { return loop; }
		void SetLoop(bool loop, Timer interval)
		{
			this->loop = loop;
			this->interval = interval;
		}

		Timer &GetTimer() { return interval; }

		bool first = true;

		enum class State
		{
			Active,
			Dead
		};

		State state = State::Active;

	private:
		Vector3 position;
		Quaternion rotate;
		std::u16string path;
		/* 
			ループ再生させるのか？ 
		*/
		bool loop;
		/*
		 * ループの間隔
		*/
		Timer interval;
	};
}