#pragma once
#include <memory>
#include <utility>

namespace Reactor::Common
{
	template<class Handle, class Raw, Raw Empty>
	class HandlePtr
	{
	protected:
		Raw ptr = Empty;

	public:
		HandlePtr() = default;

		template<class ...Args>
		HandlePtr(Args &&...args)
		{
			ptr = Handle::Ctor(std::forward<Args>(args)...);
		}

		HandlePtr(HandlePtr &&other) noexcept : HandlePtr()
		{
			using std::swap;
			swap(*this, other);
		}

		HandlePtr &operator =(HandlePtr &&other)
		{
			using std::swap;
			swap(*this, other);
			return *this;
		}

		~HandlePtr()
		{
			Handle::Dtor(ptr);
		}

		operator Raw() const
		{
			return ptr;
		}

		friend void swap(HandlePtr &first, HandlePtr &second) 
		{
			using std::swap;
			swap(first.ptr, second.ptr);
		}
	};
}
