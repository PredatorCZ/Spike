/*	std::allocator_hybrid for container classes, allows to link external buffer.
	more info in README for PreCore Project

	Copyright 2018-2019 Lukas Cone

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

#pragma once
#pragma warning(push)
#pragma warning(disable:4293)
namespace std {

	// TEMPLATE CLASS allocator_hybrid
	template<class _Ty>
	class allocator_hybrid
	{	// generic allocator for objects of class _Ty
	public:
		static_assert(!is_const<_Ty>::value,
			"The C++ Standard forbids containers of const elements "
			"because allocator<const T> is ill-formed.");

		typedef void _Not_user_specialized;

		typedef _Ty value_type;

		typedef value_type *pointer;
		typedef const value_type *const_pointer;

		typedef value_type& reference;
		typedef const value_type& const_reference;

		typedef size_t size_type;
		typedef ptrdiff_t difference_type;

		typedef true_type propagate_on_container_move_assignment;
		typedef true_type is_always_equal;

		template<class _Other>
		struct rebind
		{	// convert this type to allocator<_Other>
			typedef allocator_hybrid<_Other> other;
		};

		pointer address(reference _Val) const _NOEXCEPT
		{	// return address of mutable _Val
			return (_STD addressof(_Val));
		}

		const_pointer address(const_reference _Val) const _NOEXCEPT
		{	// return address of nonmutable _Val
			return (_STD addressof(_Val));
		}

		allocator_hybrid() _THROW0()
		{	// construct default allocator (do nothing)
		}

		allocator_hybrid(const allocator_hybrid<_Ty>&) _THROW0()
		{	// construct by copying (do nothing)
		}

		template<class _Other>
		allocator_hybrid(const allocator_hybrid<_Other>&) _THROW0()
		{	// construct from a related allocator (do nothing)
		}

		template<class _Other>
		allocator_hybrid<_Ty>& operator=(const allocator_hybrid<_Other>&)
		{	// assign from a related allocator (do nothing)
			return (*this);
		}

		void deallocate(pointer _Ptr, size_type _Count)
		{	// deallocate object at _Ptr
			if (!_buffer)
				_Deallocate(_Ptr, _Count, sizeof(_Ty));
			else
				_buffer = nullptr;

		}

		_DECLSPEC_ALLOCATOR pointer allocate(size_type _Count)
		{	// allocate array of _Count elements
			if (!CheckBuffer())
				return (static_cast<pointer>(_Allocate(_Count, sizeof(_Ty))));
			else
				return _buffer;
		}

		_DECLSPEC_ALLOCATOR pointer allocate(size_type _Count, const void *)
		{	// allocate array of _Count elements, ignore hint
			return (allocate(_Count));
		}

		template<class _Objty, class... _Types>
		void construct(_Objty *_Ptr, _Types&&... _Args)
		{	// construct _Objty(_Types...) at _Ptr
			if (!CheckBuffer())
				::new ((void *)_Ptr) _Objty(_STD forward<_Types>(_Args)...);
		}


		template<class _Uty>
		void destroy(_Uty *_Ptr)
		{	// destroy object at _Ptr
			if (!_buffer)
				_Ptr->~_Uty();
		}

		size_t max_size() const _NOEXCEPT
		{	// estimate maximum array size
			return ((size_t)(-1) / sizeof(_Ty));
		}
	private:
		pointer _buffer = nullptr;
		const static uintptr_t _reallocflag = static_cast<uintptr_t>(1) << (sizeof(uintptr_t)*8-1);
		const static uintptr_t _reallocmask = ~_reallocflag;
		inline bool CheckBuffer()
		{
			return (reinterpret_cast<uintptr_t&>(_buffer)&_reallocmask)!=0;
		}
	public:
		void SetBuffer(pointer buffer)
		{
			_buffer = buffer;
		}
		void FreeBuffer()
		{
			reinterpret_cast<uintptr_t&>(_buffer) = _reallocflag;
		}
	};
	template<class _Ty,
		class _Other> inline
		bool operator==(const allocator_hybrid<_Ty>&,
			const allocator_hybrid<_Other>&) _THROW0()
	{	// test for allocator equality
		return (true);
	}

	template<class _Ty,
		class _Other> inline
		bool operator!=(const allocator_hybrid<_Ty>& _Left,
			const allocator_hybrid<_Other>& _Right) _THROW0()
	{	// test for allocator inequality
		return (false);
	}
}
#pragma warning(pop)