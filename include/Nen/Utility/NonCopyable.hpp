#pragma once
#ifndef NEN_NONCOPYABLE_HPP
#define NEN_NONCOPYABLE_HPP

namespace nen
{

	class NonCopyable
	{
	protected:
		NonCopyable() {}
		~NonCopyable() {}

	private:
		NonCopyable(NonCopyable const&) = delete;
		NonCopyable& operator=(NonCopyable const&) = delete;
		NonCopyable(NonCopyable&&) = delete;
		NonCopyable& operator=(NonCopyable&&) = delete;
	};

}

#endif
