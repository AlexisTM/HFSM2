﻿// HFSM2 (hierarchical state machine for games and interactive applications)
// Created by Andrew Gresyk
//
// Licensed under the MIT License;
// you may not use this file except in compliance with the License.
//
//
// MIT License
//
// Copyright (c) 2020
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <stdint.h>			// uint32_t, uint64_t
#include <string.h>			// memcpy_s()

#include <new>
#include <typeindex>
#include <utility>			// std::conditional<>, move(), forward()

#if defined _DEBUG && _MSC_VER
	#include <intrin.h>		// __debugbreak()
#endif

#define HFSM_INLINE														  //inline

//------------------------------------------------------------------------------

namespace hfsm2 {

struct EmptyContext {};
struct EmptyPayload {};

}

#ifdef _MSC_VER
	#pragma warning(push)
	#pragma warning(disable: 4324) // structure was padded due to alignment specifier
#endif
#ifdef __clang__
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wextra-semi" // error : extra ';' inside a class
#endif
#if defined(__GNUC__) || defined(__GNUG__)
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wpedantic" // error: extra ‘;’
#endif


//------------------------------------------------------------------------------

#if INTPTR_MAX == INT64_MAX
	#define HFSM_64BIT_OR_32BIT(p64, p32)		p64
#else
	#define HFSM_64BIT_OR_32BIT(p64, p32)		p32
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if defined _DEBUG && defined _MSC_VER
	#define HFSM_BREAK()						__debugbreak()
#else
	#define HFSM_BREAK()						((void) 0)
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef _DEBUG
	#define HFSM_IF_DEBUG(...)					__VA_ARGS__
	#define HFSM_UNLESS_DEBUG(...)
	#define HFSM_DEBUG_OR(y, n)					y
#else
	#define HFSM_IF_DEBUG(...)
	#define HFSM_UNLESS_DEBUG(...)				__VA_ARGS__
	#define HFSM_DEBUG_OR(y, n)					n
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef HFSM_ENABLE_ASSERT
	#define HFSM_IF_ASSERT(...)					__VA_ARGS__
	#define HFSM_CHECKED(x)						(!!(x) || (HFSM_BREAK(), 0))
	#define HFSM_ASSERT(x)						(!!(x) || (HFSM_BREAK(), 0))
	#define HFSM_ASSERT_OR(y, n)				y
#else
	#define HFSM_IF_ASSERT(...)
	#define HFSM_CHECKED(x)						x
	#define HFSM_ASSERT(x)						((void) 0)
	#define HFSM_ASSERT_OR(y, n)				n
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if defined HFSM_ENABLE_LOG_INTERFACE || defined HFSM_ENABLE_VERBOSE_DEBUG_LOG

	#define HFSM_IF_LOGGER(...)										  __VA_ARGS__

	#define HFSM_LOGGER_OR(Y, N)												Y

	#define HFSM_LOG_TRANSITION(CONTEXT, ORIGIN, TYPE, DESTINATION)		\
		if (_logger)															\
			_logger->recordTransition(CONTEXT, ORIGIN, TYPE, DESTINATION)

	#define HFSM_LOG_TASK_STATUS(CONTEXT, REGION, ORIGIN, STATUS)				\
		if (_logger)															\
			_logger->recordTaskStatus(CONTEXT, REGION, ORIGIN, STATUS)

	#define HFSM_LOG_PLAN_STATUS(CONTEXT, REGION, STATUS)						\
		if (_logger)															\
			_logger->recordPlanStatus(CONTEXT, REGION, STATUS)

	#define HFSM_LOG_CANCELLED_PENDING(CONTEXT, ORIGIN)							\
		if (_logger)															\
			_logger->recordCancelledPending(CONTEXT, ORIGIN)

	#define HFSM_LOG_UTILITY_RESOLUTION(CONTEXT, HEAD, PRONG, UTILITY)			\
		if (auto* const logger = control.logger())								\
			logger->recordUtilityResolution(CONTEXT, HEAD, PRONG, UTILITY)

	#define HFSM_LOG_RANDOM_RESOLUTION(CONTEXT, HEAD, PRONG, UTILITY)			\
		if (auto* const logger = control.logger())								\
			logger->recordRandomResolution(CONTEXT, HEAD, PRONG, UTILITY)

#else

	#define HFSM_IF_LOGGER(...)
	#define HFSM_LOGGER_OR(Y, N)												N

	#define HFSM_LOG_TRANSITION(CONTEXT, ORIGIN, TYPE, DESTINATION)
	#define HFSM_LOG_TASK_STATUS(CONTEXT, REGION, ORIGIN, STATUS)
	#define HFSM_LOG_PLAN_STATUS(CONTEXT, REGION, STATUS)
	#define HFSM_LOG_CANCELLED_PENDING(CONTEXT, ORIGIN)
	#define HFSM_LOG_UTILITY_RESOLUTION(CONTEXT, HEAD, PRONG, UTILITY)
	#define HFSM_LOG_RANDOM_RESOLUTION(CONTEXT, HEAD, PRONG, UTILITY)

#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if defined HFSM_ENABLE_VERBOSE_DEBUG_LOG

	#define HFSM_LOG_STATE_METHOD(METHOD, CONTEXT, METHOD_ID)					\
		if (auto* const logger = control.logger())								\
			logger->recordMethod(CONTEXT, STATE_ID, METHOD_ID)

#elif defined HFSM_ENABLE_LOG_INTERFACE

	#define HFSM_LOG_STATE_METHOD(METHOD, CONTEXT, METHOD_ID)					\
		if (auto* const logger = control.logger())								\
			log<decltype(METHOD)>(*logger, CONTEXT, METHOD_ID)

#else
	#define HFSM_LOG_STATE_METHOD(METHOD, CONTEXT, METHOD_ID)
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef HFSM_ENABLE_STRUCTURE_REPORT
	#define HFSM_IF_STRUCTURE(...)									  __VA_ARGS__
#else
	#define HFSM_IF_STRUCTURE(...)
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef HFSM_ENABLE_TRANSITION_HISTORY
	#define HFSM_IF_TRANSITION_HISTORY(...)							  __VA_ARGS__
#else
	#define HFSM_IF_TRANSITION_HISTORY(...)
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef HFSM_ENABLE_SERIALIZATION
	#define HFSM_IF_SERIALIZATION(...)								  __VA_ARGS__
#else
	#define HFSM_IF_SERIALIZATION(...)
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if defined _MSC_VER || defined __clang_major__ && __clang_major__ >= 7
	#define HFSM_EXPLICIT_MEMBER_SPECIALIZATION
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

namespace hfsm2 {

//------------------------------------------------------------------------------

using ShortIndex = uint8_t;
static constexpr ShortIndex	INVALID_SHORT_INDEX = UINT8_MAX;

using RegionID	 = ShortIndex;
static constexpr RegionID	INVALID_REGION_ID	= INVALID_SHORT_INDEX;

using ForkID	 = int8_t;
static constexpr ForkID		INVALID_FORK_ID		= INT8_MIN;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

using LongIndex  = uint16_t;
static constexpr LongIndex	INVALID_LONG_INDEX	= UINT16_MAX;

using StateID	 = LongIndex;
static constexpr StateID	INVALID_STATE_ID	= INVALID_LONG_INDEX;

//------------------------------------------------------------------------------

namespace detail {

////////////////////////////////////////////////////////////////////////////////

template <typename T>
HFSM_INLINE
void
fill(T& a, const char value) {
	memset(&a, (int) value, sizeof(a));
}

////////////////////////////////////////////////////////////////////////////////

template <typename T, unsigned NCount>
constexpr
unsigned
count(const T(&)[NCount]) {
	return NCount;
}

////////////////////////////////////////////////////////////////////////////////

template <int N1, int N2>
struct Min {
	static constexpr auto VALUE = N1 < N2 ? N1 : N2;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <int N1, int N2>
struct Max {
	static constexpr auto VALUE = N1 > N2 ? N1 : N2;
};

//------------------------------------------------------------------------------

template <typename T>
constexpr
T
min(const T t1, const T t2) { return t1 < t2 ? t1 : t2; }


//------------------------------------------------------------------------------

template <unsigned NCapacity>
struct UnsignedCapacityT {
	static constexpr LongIndex CAPACITY = NCapacity;

	using Type = typename std::conditional<CAPACITY <= UINT8_MAX,  uint8_t,
				 typename std::conditional<CAPACITY <= UINT16_MAX, uint16_t,
				 typename std::conditional<CAPACITY <= UINT32_MAX, uint32_t,
																   uint64_t>::type>::type>::type;

	static_assert(CAPACITY <= UINT64_MAX, "STATIC ASSERT");
};

template <unsigned NCapacity>
using UnsignedCapacity = typename UnsignedCapacityT<NCapacity>::Type;

//------------------------------------------------------------------------------

template <unsigned NBitWidth>
struct UnsignedBitWidthT {
	static constexpr ShortIndex BIT_WIDTH = NBitWidth;

	using Type = typename std::conditional<BIT_WIDTH <= 8,  uint8_t,
				 typename std::conditional<BIT_WIDTH <= 16, uint16_t,
				 typename std::conditional<BIT_WIDTH <= 32, uint32_t,
															uint64_t>::type>::type>::type;

	static_assert(BIT_WIDTH <= 64, "STATIC ASSERT");
};

template <unsigned NCapacity>
using UnsignedBitWidth = typename UnsignedBitWidthT<NCapacity>::Type;

//------------------------------------------------------------------------------

constexpr
LongIndex
roundUp(const LongIndex x,
		const LongIndex to)
{
	return (x + (to - 1)) / to;
}

//------------------------------------------------------------------------------

constexpr
ShortIndex
bitWidth(const ShortIndex x) {
	return x <=   2 ? 1 :
		   x <=   4 ? 2 :
		   x <=   8 ? 3 :
		   x <=  16 ? 4 :
		   x <=  32 ? 5 :
		   x <=  64 ? 6 :
		   x <= 128 ? 7 :
					  8 ;
}

//------------------------------------------------------------------------------

template <typename TTo, typename TFrom>
void
overwrite(TTo& to, const TFrom& from) {
	static_assert(sizeof(TTo) == sizeof(TFrom), "STATIC ASSERT");

#if defined(__GNUC__) || defined(__GNUG__)
	memcpy  (&to,			  &from, sizeof(from));
#else
	memcpy_s(&to, sizeof(to), &from, sizeof(from));
#endif
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TO, typename TI>
TO convert(const TI& in) {
	TO out;

	overwrite(out, in);

	return out;
}

////////////////////////////////////////////////////////////////////////////////

template <int>
struct StaticPrintConst;

template <typename>
struct StaticPrintType;

//------------------------------------------------------------------------------

template <unsigned V1, unsigned V2>
struct StaticAssertEquality;

template <unsigned V1>
struct StaticAssertEquality<V1, V1> {};

////////////////////////////////////////////////////////////////////////////////

}
}

namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

template <typename TContainer>
class Iterator {
public:
	using Container = TContainer;
	using Item = typename Container::Item;

	template <typename T, LongIndex NCapacity>
	friend class Array;

private:
	HFSM_INLINE Iterator(Container& container, const LongIndex cursor)
		: _container(container)
		, _cursor(cursor)
	{}

public:
	HFSM_INLINE bool operator != (const Iterator<Container>& dummy) const;

	HFSM_INLINE Iterator& operator ++();

	HFSM_INLINE		  Item& operator *()	   { return  _container[_cursor]; }
	HFSM_INLINE const Item& operator *() const { return  _container[_cursor]; }

	HFSM_INLINE		  Item* operator->()	   { return &_container[_cursor]; }
	HFSM_INLINE const Item* operator->() const { return &_container[_cursor]; }

private:
	Container& _container;

	LongIndex _cursor;
};

//------------------------------------------------------------------------------

template <typename TContainer>
class Iterator<const TContainer> {
public:
	using Container = TContainer;
	using Item = typename Container::Item;

	template <typename T, LongIndex NCapacity>
	friend class Array;

private:
	HFSM_INLINE Iterator(const Container& container, const LongIndex cursor)
		: _container(container)
		, _cursor(cursor)
	{}

public:
	HFSM_INLINE bool operator != (const Iterator<const Container>& dummy) const;

	HFSM_INLINE Iterator& operator ++();

	HFSM_INLINE const Item& operator *() const { return _container[_cursor]; }

	HFSM_INLINE const Item* operator->() const { return &operator *();		 }

private:
	const Container& _container;

	LongIndex _cursor;
};

////////////////////////////////////////////////////////////////////////////////

}
}

namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

template <typename T>
bool
Iterator<T>::operator != (const Iterator<T>& HFSM_IF_ASSERT(dummy)) const {
	HFSM_ASSERT(&_container == &dummy._container);

	return _cursor != _container.limit();
}

//------------------------------------------------------------------------------

template <typename T>
Iterator<T>&
Iterator<T>::operator ++() {
	_cursor = _container.next(_cursor);

	return *this;
}

////////////////////////////////////////////////////////////////////////////////

template <typename T>
bool
Iterator<const T>::operator != (const Iterator<const T>& HFSM_IF_ASSERT(dummy)) const {
	HFSM_ASSERT(&_container == &dummy._container);

	return _cursor != _container.limit();
}

//------------------------------------------------------------------------------

template <typename T>
Iterator<const T>&
Iterator<const T>::operator ++() {
	_cursor = _container.next(_cursor);

	return *this;
}

////////////////////////////////////////////////////////////////////////////////

}
}

namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

template <typename T, LongIndex NCapacity>
class StaticArray {
public:
	static constexpr LongIndex CAPACITY = NCapacity;
	static constexpr LongIndex DUMMY	= INVALID_LONG_INDEX;

	using Item  = T;
	using Index = UnsignedCapacity<CAPACITY>;

public:
	HFSM_INLINE StaticArray() = default;
	HFSM_INLINE StaticArray(const Item filler);

	template <typename N>
	HFSM_INLINE		  Item& operator[] (const N i);

	template <typename N>
	HFSM_INLINE const Item& operator[] (const N i) const;

	HFSM_INLINE LongIndex count() const						{ return CAPACITY;									}

	HFSM_INLINE void fill(const Item filler);
	HFSM_INLINE void clear()								{ fill(INVALID_SHORT_INDEX);						}

	HFSM_INLINE Iterator<	   StaticArray>  begin()		{ return Iterator<		StaticArray>(*this, 0);		}
	HFSM_INLINE Iterator<const StaticArray>  begin() const	{ return Iterator<const StaticArray>(*this, 0);		}
	HFSM_INLINE Iterator<const StaticArray> cbegin() const	{ return Iterator<const StaticArray>(*this, 0);		}

	HFSM_INLINE Iterator<	   StaticArray>	 end()			{ return Iterator<		StaticArray>(*this, DUMMY);	}
	HFSM_INLINE Iterator<const StaticArray>	 end() const	{ return Iterator<const StaticArray>(*this, DUMMY);	}
	HFSM_INLINE Iterator<const StaticArray> cend() const	{ return Iterator<const StaticArray>(*this, DUMMY);	}

private:
	Item _items[CAPACITY];
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename T>
struct StaticArray<T, 0> {
	using Item  = T;

	HFSM_INLINE StaticArray() = default;
	HFSM_INLINE StaticArray(const Item)											{}
};

//------------------------------------------------------------------------------

template <typename T, LongIndex NCapacity>
class Array {
	template <typename>
	friend class Iterator;

public:
	static constexpr LongIndex CAPACITY = NCapacity;
	static constexpr LongIndex DUMMY	= INVALID_LONG_INDEX;

	using Item = T;
	using Index = UnsignedCapacity<CAPACITY>;

public:
	HFSM_INLINE void clear()														{ _count = 0;		}

	template <typename TValue>
	HFSM_INLINE LongIndex append(TValue&& value);

	template <typename N>
	HFSM_INLINE		  Item& operator[] (const N i);

	template <typename N>
	HFSM_INLINE const Item& operator[] (const N i) const;

	HFSM_INLINE LongIndex count() const												{ return _count;	}

	HFSM_INLINE Iterator<	   Array>  begin()			{ return Iterator<		Array>(*this, 0);		}
	HFSM_INLINE Iterator<const Array>  begin() const	{ return Iterator<const Array>(*this, 0);		}
	HFSM_INLINE Iterator<const Array> cbegin() const	{ return Iterator<const Array>(*this, 0);		}

	HFSM_INLINE Iterator<	   Array>	 end()			{ return Iterator<		Array>(*this, DUMMY);	}
	HFSM_INLINE Iterator<const Array>	 end() const	{ return Iterator<const Array>(*this, DUMMY);	}
	HFSM_INLINE Iterator<const Array>   cend() const	{ return Iterator<const Array>(*this, DUMMY);	}

private:
	HFSM_INLINE LongIndex next(const LongIndex i) const								{ return i + 1;		}
	HFSM_INLINE LongIndex limit() const												{ return _count;	}

private:
	LongIndex _count = 0;
	Item _items[CAPACITY];
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename T>
class Array<T, 0> {};

////////////////////////////////////////////////////////////////////////////////

}
}

namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

template <typename T, LongIndex NC>
StaticArray<T, NC>::StaticArray(const Item filler) {
	fill(filler);
}

//------------------------------------------------------------------------------

template <typename T, LongIndex NC>
template <typename N>
T&
StaticArray<T, NC>::operator[] (const N i) {
	HFSM_ASSERT(0 <= i && i < CAPACITY);

	return _items[(Index) i];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename T, LongIndex NC>
template <typename N>
const T&
StaticArray<T, NC>::operator[] (const N i) const {
	HFSM_ASSERT(0 <= i && i < CAPACITY);

	return _items[(Index) i];
}

//------------------------------------------------------------------------------

template <typename T, LongIndex NC>
void
StaticArray<T, NC>::fill(const Item filler) {
	for (LongIndex i = 0; i < CAPACITY; ++i)
		_items[i] = filler;
}

////////////////////////////////////////////////////////////////////////////////

template <typename T, LongIndex NC>
template <typename TValue>
LongIndex
Array<T, NC>::append(TValue&& value) {
	HFSM_ASSERT(_count < CAPACITY);

	new (&_items[_count]) Item{std::move(value)};

	return _count++;
}

//------------------------------------------------------------------------------

template <typename T, LongIndex NC>
template <typename N>
T&
Array<T, NC>::operator[] (const N i) {
	HFSM_ASSERT(0 <= i && i < CAPACITY);

	return _items[(Index) i];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename T, LongIndex NC>
template <typename N>
const T&
Array<T, NC>::operator[] (const N i) const {
	HFSM_ASSERT(0 <= i && i < CAPACITY);

	return _items[(Index) i];
}

////////////////////////////////////////////////////////////////////////////////

}
}

namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

struct Units {
	ShortIndex unit;
	ShortIndex width;
};

//------------------------------------------------------------------------------

template <typename TIndex, ShortIndex NCapacity>
class BitArray final {
public:
	using Index	= TIndex;
	using Unit	= unsigned char;

	static constexpr Index CAPACITY	= NCapacity;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class Bits {
		template <typename, ShortIndex>
		friend class BitArray;

	private:
		HFSM_INLINE explicit Bits(Unit* const storage,
								  const Index width)
			: _storage{storage}
			, _width{width}
		{}

	public:
		HFSM_INLINE explicit operator bool() const;

		HFSM_INLINE void clear();

		template <ShortIndex NIndex>
		HFSM_INLINE bool get() const;

		template <ShortIndex NIndex>
		HFSM_INLINE void set();

		template <ShortIndex NIndex>
		HFSM_INLINE void reset();

		HFSM_INLINE bool get  (const Index index) const;
		HFSM_INLINE void set  (const Index index);
		HFSM_INLINE void reset(const Index index);

	private:
		Unit* const _storage;
		const Index _width;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class ConstBits {
		template <typename, ShortIndex>
		friend class BitArray;

	private:
		HFSM_INLINE explicit ConstBits(const Unit* const storage,
									   const Index width)
			: _storage{storage}
			, _width{width}
		{}

	public:
		HFSM_INLINE explicit operator bool() const;

		template <ShortIndex NIndex>
		HFSM_INLINE bool get() const;

		HFSM_INLINE bool get(const Index index) const;

	private:
		const Unit* const _storage;
		const Index _width;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:
	BitArray() {
		clear();
	}

	HFSM_INLINE void clear();

	template <ShortIndex NIndex>
	HFSM_INLINE bool get() const;

	template <ShortIndex NIndex>
	HFSM_INLINE void set();

	template <ShortIndex NIndex>
	HFSM_INLINE void reset();

	HFSM_INLINE bool get  (const Index index) const;
	HFSM_INLINE void set  (const Index index);
	//HFSM_INLINE void reset(const Index index);

	template <ShortIndex NUnit, ShortIndex NWidth>
	HFSM_INLINE		 Bits bits();

	template <ShortIndex NUnit, ShortIndex NWidth>
	HFSM_INLINE ConstBits bits() const;


	HFSM_INLINE		 Bits bits(const Units& units);
	HFSM_INLINE ConstBits bits(const Units& units) const;

private:
	Unit _storage[CAPACITY];
};

//------------------------------------------------------------------------------

template <typename TIndex>
class BitArray<TIndex, 0> final {
public:
	HFSM_INLINE void clear() {}
};

////////////////////////////////////////////////////////////////////////////////

}
}

namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

template <typename TI, ShortIndex NC>
BitArray<TI, NC>::Bits::operator bool() const {
	const ShortIndex fullUnits = _width / (sizeof(Unit) * 8);

	// TODO: cover this case
	for (Index i = 0; i < fullUnits; ++i)
		if (_storage[i])
			return true;

	const ShortIndex bit = _width % (sizeof(Unit) * 8);
	const Unit mask = (1 << bit) - 1;
	const Unit& u = _storage[fullUnits];

	return (u & mask) != 0;
}

//------------------------------------------------------------------------------

template <typename TI, ShortIndex NC>
void
BitArray<TI, NC>::Bits::clear() {
	const Index count = (_width + 7) / (sizeof(Unit) * 8);

	for (Index i = 0; i < count; ++i)
		_storage[i] = Unit{0};
}

//------------------------------------------------------------------------------

template <typename TI, ShortIndex NC>
template <ShortIndex NIndex>
bool
BitArray<TI, NC>::Bits::get() const {
	constexpr Index INDEX = NIndex;
	HFSM_ASSERT(INDEX < _width);

	constexpr Index BIT_UNIT  = INDEX / (sizeof(Unit) * 8);
	constexpr Index BIT_INDEX = INDEX % (sizeof(Unit) * 8);
	constexpr Unit MASK = 1 << BIT_INDEX;

	return (_storage[BIT_UNIT] & MASK) != 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TI, ShortIndex NC>
template <ShortIndex NIndex>
void
BitArray<TI, NC>::Bits::set() {
	constexpr Index INDEX = NIndex;
	HFSM_ASSERT(INDEX < _width);

	constexpr Index BIT_UNIT  = INDEX / (sizeof(Unit) * 8);
	constexpr Index BIT_INDEX = INDEX % (sizeof(Unit) * 8);
	constexpr Unit MASK = 1 << BIT_INDEX;

	_storage[BIT_UNIT] |= MASK;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TI, ShortIndex NC>
template <ShortIndex NIndex>
void
BitArray<TI, NC>::Bits::reset() {
	constexpr Index INDEX = NIndex;
	HFSM_ASSERT(INDEX < _width);

	constexpr Index BIT_UNIT  = INDEX / (sizeof(Unit) * 8);
	constexpr Index BIT_INDEX = INDEX % (sizeof(Unit) * 8);
	constexpr Unit MASK = 1 << BIT_INDEX;

	_storage[BIT_UNIT] &= ~MASK;
}

//------------------------------------------------------------------------------

template <typename TI, ShortIndex NC>
bool
BitArray<TI, NC>::Bits::get(const Index index) const {
	HFSM_ASSERT(index < _width);

	const Index unit = index / (sizeof(Unit) * 8);
	const Index bit  = index % (sizeof(Unit) * 8);
	const Unit mask = 1 << bit;

	return (_storage[unit] & mask) != 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TI, ShortIndex NC>
void
BitArray<TI, NC>::Bits::set(const Index index) {
	HFSM_ASSERT(index < _width);

	const Index unit = index / (sizeof(Unit) * 8);
	const Index bit  = index % (sizeof(Unit) * 8);
	const Unit mask = 1 << bit;

	_storage[unit] |= mask;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TI, ShortIndex NC>
void
BitArray<TI, NC>::Bits::reset(const Index index) {
	HFSM_ASSERT(index < _width);

	const Index unit = index / (sizeof(Unit) * 8);
	const Index bit  = index % (sizeof(Unit) * 8);
	const Unit mask = 1 << bit;

	_storage[unit] &= ~mask;
}

////////////////////////////////////////////////////////////////////////////////

template <typename TI, ShortIndex NC>
BitArray<TI, NC>::ConstBits::operator bool() const {
	const ShortIndex fullUnits = _width / (sizeof(Unit) * 8);

	for (Index i = 0; i < fullUnits; ++i)
		if (_storage[i])
			return true;

	const ShortIndex bit = _width % (sizeof(Unit) * 8);
	const Unit mask = (1 << bit) - 1;
	const Unit& u = _storage[fullUnits];

	return (u & mask) != 0;
}

//------------------------------------------------------------------------------

template <typename TI, ShortIndex NC>
template <ShortIndex NIndex>
bool
BitArray<TI, NC>::ConstBits::get() const {
	constexpr Index INDEX = NIndex;
	static_assert(INDEX < _width, "");

	constexpr Index BIT_UNIT  = INDEX / (sizeof(Unit) * 8);
	constexpr Index BIT_INDEX = INDEX % (sizeof(Unit) * 8);
	constexpr Unit MASK = 1 << BIT_INDEX;

	return (_storage[BIT_UNIT] & MASK) != 0;
}

//------------------------------------------------------------------------------

template <typename TI, ShortIndex NC>
bool
BitArray<TI, NC>::ConstBits::get(const Index index) const {
	HFSM_ASSERT(index < _width);

	const Index unit = index / (sizeof(Unit) * 8);
	const Index bit  = index % (sizeof(Unit) * 8);
	const Unit mask = 1 << bit;

	return (_storage[unit] & mask) != 0;
}

////////////////////////////////////////////////////////////////////////////////

template <typename TI, ShortIndex NC>
void
BitArray<TI, NC>::clear() {
	for (Unit& unit: _storage)
		unit = Unit{0};
}

////////////////////////////////////////////////////////////////////////////////

template <typename TI, ShortIndex NC>
template <ShortIndex NIndex>
bool
BitArray<TI, NC>::get() const {
	constexpr Index INDEX = NIndex;
	static_assert(INDEX < CAPACITY, "");

	constexpr Index BIT_UNIT  = INDEX / (sizeof(Unit) * 8);
	constexpr Index BIT_INDEX = INDEX % (sizeof(Unit) * 8);
	constexpr Unit MASK = 1 << BIT_INDEX;

	return (_storage[BIT_UNIT] & MASK) != 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TI, ShortIndex NC>
template <ShortIndex NIndex>
void
BitArray<TI, NC>::set() {
	constexpr Index INDEX = NIndex;
	static_assert(INDEX < CAPACITY, "");

	constexpr Index BIT_UNIT  = INDEX / (sizeof(Unit) * 8);
	constexpr Index BIT_INDEX = INDEX % (sizeof(Unit) * 8);
	constexpr Unit MASK = 1 << BIT_INDEX;

	_storage[BIT_UNIT] |= MASK;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TI, ShortIndex NC>
template <ShortIndex NIndex>
void
BitArray<TI, NC>::reset() {
	constexpr Index INDEX = NIndex;
	static_assert(INDEX < CAPACITY, "");

	constexpr Index BIT_UNIT  = INDEX / (sizeof(Unit) * 8);
	constexpr Index BIT_INDEX = INDEX % (sizeof(Unit) * 8);
	constexpr Unit MASK = 1 << BIT_INDEX;

	_storage[BIT_UNIT] &= ~MASK;
}

//------------------------------------------------------------------------------

template <typename TI, ShortIndex NC>
bool
BitArray<TI, NC>::get(const Index index) const {
	HFSM_ASSERT(index < CAPACITY);

	const Index unit = index / (sizeof(Unit) * 8);
	const Index bit  = index % (sizeof(Unit) * 8);
	const Unit mask = 1 << bit;

	return (_storage[unit] & mask) != 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TI, ShortIndex NC>
void
BitArray<TI, NC>::set(const Index index) {
	HFSM_ASSERT(index < CAPACITY);

	const Index unit = index / (sizeof(Unit) * 8);
	const Index bit  = index % (sizeof(Unit) * 8);
	const Unit mask = 1 << bit;

	_storage[unit] |= mask;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//template <typename TI, ShortIndex NC>
//void
//BitArray<TI, NC>::reset(const Index index) {
//	HFSM_ASSERT(index < CAPACITY);
//
//	const Index unit = index / (sizeof(Unit) * 8);
//	const Index bit  = index % (sizeof(Unit) * 8);
//	const Unit mask = 1 << bit;
//
//	_storage[unit] &= ~mask;
//}

//------------------------------------------------------------------------------

template <typename TI, ShortIndex NC>
template <ShortIndex NUnit, ShortIndex NWidth>
typename BitArray<TI, NC>::Bits
BitArray<TI, NC>::bits() {
	constexpr ShortIndex UNIT  = NUnit;
	constexpr ShortIndex WIDTH = NWidth;
	static_assert(UNIT + (WIDTH + 7) / (sizeof(Unit) * 8) <= CAPACITY, "");

	return Bits{_storage + UNIT, WIDTH};
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TI, ShortIndex NC>
template <ShortIndex NUnit, ShortIndex NWidth>
typename BitArray<TI, NC>::ConstBits
BitArray<TI, NC>::bits() const {
	constexpr ShortIndex UNIT  = NUnit;
	constexpr ShortIndex WIDTH = NWidth;
	static_assert(UNIT + (WIDTH + 7) / (sizeof(Unit) * 8) <= CAPACITY, "");

	return ConstBits{_storage + UNIT, WIDTH};
}

//------------------------------------------------------------------------------

template <typename TI, ShortIndex NC>
typename BitArray<TI, NC>::Bits
BitArray<TI, NC>::bits(const Units& units) {
	HFSM_ASSERT(units.unit + (units.width + 7) / (sizeof(Unit) * 8) <= CAPACITY);

	return Bits{_storage + units.unit, units.width};
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TI, ShortIndex NC>
typename BitArray<TI, NC>::ConstBits
BitArray<TI, NC>::bits(const Units& units) const {
	HFSM_ASSERT(units.unit + (units.width + 7) / (sizeof(Unit) * 8) <= CAPACITY);

	return ConstBits{_storage + units.unit, units.width};
}

////////////////////////////////////////////////////////////////////////////////

}
}

namespace hfsm2 {
namespace detail {

//------------------------------------------------------------------------------

template <LongIndex NBitCapacity>
struct StreamBuffer {
	static constexpr LongIndex BIT_CAPACITY	= NBitCapacity;
	static constexpr LongIndex BYTE_COUNT	= roundUp(BIT_CAPACITY, 8);

	using Size = UnsignedCapacity<BIT_CAPACITY>;
	using Data = uint8_t[BYTE_COUNT];

	void clear();

	//Size write(const uint8_t byte);

	Size bitSize;
	Data payload;
};

////////////////////////////////////////////////////////////////////////////////

template <LongIndex NBitCapacity>
class BitWriteStream final {
public:
	static constexpr LongIndex BIT_CAPACITY = NBitCapacity;

	using Buffer = StreamBuffer<BIT_CAPACITY>;

public:
	BitWriteStream(Buffer& _buffer);

	template <ShortIndex NBitWidth>
	void write(const UnsignedBitWidth<NBitWidth> item);

private:
	Buffer& _buffer;
};

//------------------------------------------------------------------------------

template <LongIndex NBitCapacity>
class BitReadStream final {
public:
	static constexpr LongIndex BIT_CAPACITY = NBitCapacity;

	using Buffer = StreamBuffer<BIT_CAPACITY>;

public:
	BitReadStream(const Buffer& buffer)
		: _buffer{buffer}
	{}

	template <ShortIndex NBitWidth>
	UnsignedBitWidth<NBitWidth> read();

	LongIndex cursor() const								{ return _cursor;	}

private:
	const Buffer& _buffer;

	LongIndex _cursor = 0;
};

////////////////////////////////////////////////////////////////////////////////

}
}


namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

template <LongIndex NBC>
void
StreamBuffer<NBC>::clear() {
	bitSize = 0;
	fill(payload, 0);
}

////////////////////////////////////////////////////////////////////////////////

template <LongIndex NBC>
BitWriteStream<NBC>::BitWriteStream(Buffer& buffer)
	: _buffer{buffer}
{
	_buffer.clear();
}

//------------------------------------------------------------------------------

template <LongIndex NBC>
template <ShortIndex NBitWidth>
void
BitWriteStream<NBC>::write(const UnsignedBitWidth<NBitWidth> item) {
	constexpr ShortIndex BIT_WIDTH = NBitWidth;
	static_assert(BIT_WIDTH > 0, "STATIC ASSERT");

	HFSM_ASSERT(_buffer.bitSize + BIT_WIDTH < BIT_CAPACITY);

	using Item = UnsignedBitWidth<BIT_WIDTH>;

	Item itemBits = item;

	for (ShortIndex itemWidth = BIT_WIDTH; itemWidth; ) {
		const LongIndex	 byteIndex		= _buffer.bitSize >> 3;
		uint8_t& byte = _buffer.payload[byteIndex];

		const ShortIndex byteChunkStart	= _buffer.bitSize & 0x7;
		const ShortIndex byteDataWidth	= 8 - byteChunkStart;
		const ShortIndex byteChunkWidth	= detail::min(byteDataWidth, itemWidth);
		const Item		 byteChunk		= itemBits << byteChunkStart;
		byte |= byteChunk;

		itemBits	>>= byteChunkWidth;
		itemWidth	 -= byteChunkWidth;
		_buffer.bitSize += byteChunkWidth;
	}
}

////////////////////////////////////////////////////////////////////////////////

template <LongIndex NBC>
template <ShortIndex NBitWidth>
UnsignedBitWidth<NBitWidth>
BitReadStream<NBC>::read() {
	constexpr ShortIndex BIT_WIDTH = NBitWidth;
	static_assert(BIT_WIDTH > 0, "STATIC ASSERT");

	using Item = UnsignedBitWidth<BIT_WIDTH>;

	Item item = 0;
	ShortIndex itemCursor = 0;

	for (ShortIndex itemWidth = BIT_WIDTH; itemWidth; )
		if (HFSM_CHECKED(_cursor + itemWidth <= _buffer.bitSize)) {
			const LongIndex	 byteIndex		= _cursor >> 3;
			const uint8_t& byte = _buffer.payload[byteIndex];

			const ShortIndex byteChunkStart	= _cursor & 0x7;
			const ShortIndex byteDataWidth	= 8 - byteChunkStart;
			const ShortIndex byteChunkWidth	= detail::min(byteDataWidth, itemWidth);
			const ShortIndex byteChunkMask	= (1 << byteChunkWidth) - 1;
			const Item		 byteChunk		= (byte >> byteChunkStart) & byteChunkMask;

			const Item		 itemChunk		= byteChunk << itemCursor;
			item |= itemChunk;

			_cursor	   += byteChunkWidth;
			itemCursor += byteChunkWidth;
			itemWidth  -= byteChunkWidth;
		}

	return item;
}

////////////////////////////////////////////////////////////////////////////////

}
}

namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

template <typename TItem, LongIndex NCapacity>
class List {
public:
	using Index = LongIndex;

	static constexpr Index CAPACITY = NCapacity;

	static constexpr Index INVALID = Index (-1);

private:
	using Item  = TItem;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct Links {
		Index prev;
		Index next;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	union Cell {
		Item item;
		Links links;

		HFSM_INLINE Cell()
			: links{}
		{}
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:
	template <typename... TArgs>
	Index emplace(TArgs... args);

	void remove(const Index i);

	HFSM_INLINE		  Item& operator[] (const Index i);
	HFSM_INLINE const Item& operator[] (const Index i) const;

	HFSM_INLINE Index count() const							{ return _count;	}

private:
	HFSM_IF_ASSERT(void verifyStructure(const Index occupied = INVALID) const);

private:
	Cell _cells[CAPACITY];
	Index _vacantHead = 0;
	Index _vacantTail = 0;
	Index _last = 0;
	Index _count = 0;
};

//------------------------------------------------------------------------------

template <typename TItem>
class List<TItem, 0> {};

////////////////////////////////////////////////////////////////////////////////

}
}

namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

template <typename T, LongIndex NC>
template <typename... TA>
LongIndex
List<T, NC>::emplace(TA... args) {
	if (_count < CAPACITY) {
		HFSM_ASSERT(_vacantHead < CAPACITY);
		HFSM_ASSERT(_vacantTail < CAPACITY);

		const Index result = _vacantHead;
		auto& cell = _cells[result];
		++_count;

		if (_vacantHead != _vacantTail) {
			// recycle
			HFSM_ASSERT(cell.links.prev == INVALID);
			HFSM_ASSERT(cell.links.next != INVALID);

			_vacantHead = cell.links.next;

			auto& head = _cells[_vacantHead];
			HFSM_ASSERT(head.links.prev == result);
			head.links.prev = INVALID;
		} else if (_last < CAPACITY) {
			// grow
			++_last;
			_vacantHead = _last;
			_vacantTail = _last;

			auto& nextVacant = _cells[_vacantHead];
			nextVacant.links.prev = INVALID;
			nextVacant.links.next = INVALID;
		} else {
			HFSM_ASSERT(_count == CAPACITY);

			_vacantHead = INVALID;
			_vacantTail = INVALID;
		}

		HFSM_IF_ASSERT(verifyStructure());

		new (&cell.item) Item{std::forward<TA>(args)...};

		return result;
	} else {
		// full
		HFSM_ASSERT(_vacantHead == INVALID);
		HFSM_ASSERT(_vacantTail == INVALID);
		HFSM_ASSERT(_count == CAPACITY);
		HFSM_BREAK();

		return INVALID;
	}
}

//------------------------------------------------------------------------------

template <typename T, LongIndex NC>
void
List<T, NC>::remove(const Index i) {
	HFSM_ASSERT(i < CAPACITY && _count);

	auto& fresh = _cells[i];

	if (_count < CAPACITY) {
		HFSM_ASSERT(_vacantHead < CAPACITY);
		HFSM_ASSERT(_vacantTail < CAPACITY);

		fresh.links.prev = INVALID;
		fresh.links.next = _vacantHead;

		auto& head = _cells[_vacantHead];
		head.links.prev = i;

		_vacantHead = i;
	} else {
		// 0 -> 1
		HFSM_ASSERT(_count == CAPACITY);
		HFSM_ASSERT(_vacantHead == INVALID);
		HFSM_ASSERT(_vacantTail == INVALID);

		fresh.links.prev = INVALID;
		fresh.links.next = INVALID;

		_vacantHead = i;
		_vacantTail = i;
	}

	--_count;

	HFSM_IF_ASSERT(verifyStructure());
}

//------------------------------------------------------------------------------

template <typename T, LongIndex NC>
T&
List<T, NC>::operator[] (const Index i) {
	HFSM_IF_ASSERT(verifyStructure());

	return _cells[i].item;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename T, LongIndex NC>
const T&
List<T, NC>::operator[] (const Index i) const {
	HFSM_IF_ASSERT(verifyStructure());

	return _cells[i].item;
}

//------------------------------------------------------------------------------

#ifdef HFSM_ENABLE_ASSERT

template <typename T, LongIndex NC>
void
List<T, NC>::verifyStructure(const Index occupied) const {
	if (_count < CAPACITY) {
		HFSM_ASSERT(_vacantHead < CAPACITY);
		HFSM_ASSERT(_vacantTail < CAPACITY);

		HFSM_ASSERT(_cells[_vacantHead].links.prev == INVALID);
		HFSM_ASSERT(_cells[_vacantTail].links.next == INVALID);

		auto emptyCount = 1;

		for (auto c = _vacantHead; c != _vacantTail; ) {
			HFSM_ASSERT(occupied != c);

			const auto& current = _cells[c];

			const auto f = current.links.next;
			if (f != INVALID) {
				// next
				const auto& following = _cells[f];

				HFSM_ASSERT(following.links.prev == c);

				c = f;
				continue;
			} else {
				// end
				HFSM_ASSERT(_vacantTail == c);
				HFSM_ASSERT(_count == CAPACITY - emptyCount);

				break;
			}
		}
	} else {
		HFSM_ASSERT(_vacantHead == INVALID);
		HFSM_ASSERT(_vacantTail == INVALID);
	}
}

#endif

////////////////////////////////////////////////////////////////////////////////

}
}

namespace hfsm2 {

////////////////////////////////////////////////////////////////////////////////
// http://xoshiro.di.unimi.it/splitmix64.c

class SplitMix64 {
public:
	inline SplitMix64() {}
	inline SplitMix64(const uint64_t seed);

	inline uint64_t next();

private:
	inline uint64_t raw();

private:
	uint64_t _state;
};

//------------------------------------------------------------------------------
// xoshiro256+ (http://xoshiro.di.unimi.it/xoshiro256plus.c)

class XoShiRo256Plus {
public:
	inline XoShiRo256Plus();
	inline XoShiRo256Plus(const uint64_t s);
	inline XoShiRo256Plus(const uint64_t(& s)[4]);

	inline void seed(const uint64_t s);
	inline void seed(const uint64_t(& s)[4]);

	inline float next();
	inline void jump();

private:
	inline uint64_t raw();

private:
	uint64_t _state[4];
};

////////////////////////////////////////////////////////////////////////////////
// https://groups.google.com/forum/#!topic/prng/VFjdFmbMgZI

class SplitMix32 {
public:
	inline SplitMix32() {}
	inline SplitMix32(const uint32_t seed);

	inline uint32_t next();

private:
	inline uint32_t raw();

private:
	uint32_t _state;
};

//------------------------------------------------------------------------------
// http://xoshiro.di.unimi.it/xoshiro128plus.c

class XoShiRo128Plus {
public:
	inline XoShiRo128Plus();
	inline XoShiRo128Plus(const uint32_t s);
	inline XoShiRo128Plus(const uint32_t(& s)[4]);

	inline void seed(const uint32_t s);
	inline void seed(const uint32_t(& s)[4]);

	inline float next();
	inline void jump();

private:
	inline uint32_t raw();

private:
	uint32_t _state[4];
};

//------------------------------------------------------------------------------

template <typename T>
class RandomT;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <>
class RandomT<float>
	: public HFSM_64BIT_OR_32BIT(XoShiRo256Plus, XoShiRo128Plus)
{
public:
	using HFSM_64BIT_OR_32BIT(XoShiRo256Plus::XoShiRo256Plus,
							  XoShiRo128Plus::XoShiRo128Plus);
};

////////////////////////////////////////////////////////////////////////////////

}

namespace hfsm2 {

////////////////////////////////////////////////////////////////////////////////

namespace detail {

inline
float
uniformReal(const uint32_t uint) {
	const auto real = convert<float>(UINT32_C(0x7F) << 23 | uint >> 9);

	return real - 1.0f;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

inline
double
uniformReal(const uint64_t uint) {
	const auto real = convert<double>(UINT64_C(0x3FF) << 52 | uint >> 12);

	return real - 1.0;
}

//------------------------------------------------------------------------------

inline
uint32_t
rotl(const uint32_t x, const uint32_t k) {
	return (x << k) | (x >> (32 - k));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

inline
uint64_t
rotl(const uint64_t x, const uint64_t k) {
	return (x << k) | (x >> (64 - k));
}

}

////////////////////////////////////////////////////////////////////////////////

SplitMix64::SplitMix64(const uint64_t seed)
	: _state{seed}
{}

//------------------------------------------------------------------------------

uint64_t
SplitMix64::next() {
	for (;;)
		if (const uint64_t number = raw())
			return number;
}

//------------------------------------------------------------------------------

uint64_t
SplitMix64::raw() {
	uint64_t z = (_state += 0x9e3779b97f4a7c15);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
	z = (z ^ (z >> 27)) * 0x94d049bb133111eb;

	return z ^ (z >> 31);
}

////////////////////////////////////////////////////////////////////////////////

XoShiRo256Plus::XoShiRo256Plus() {
	SplitMix64 generator;

	_state[0] = generator.next();
	_state[1] = generator.next();
	_state[2] = generator.next();
	_state[3] = generator.next();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

XoShiRo256Plus::XoShiRo256Plus(const uint64_t s) {
	seed(s);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

XoShiRo256Plus::XoShiRo256Plus(const uint64_t(& s)[4]) {
	seed(s);
}

//------------------------------------------------------------------------------

void
XoShiRo256Plus::seed(const uint64_t s) {
	SplitMix64 generator{s};

	_state[0] = generator.next();
	_state[1] = generator.next();
	_state[2] = generator.next();
	_state[3] = generator.next();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void
XoShiRo256Plus::seed(const uint64_t(& s)[4]) {
	_state[0] = s[0];
	_state[1] = s[1];
	_state[2] = s[2];
	_state[3] = s[3];
}

//------------------------------------------------------------------------------

float
XoShiRo256Plus::next() {
	return detail::uniformReal((uint32_t) raw());
}

//------------------------------------------------------------------------------

void
XoShiRo256Plus::jump() {
	static const uint64_t JUMP[] = { 0x180ec6d33cfd0aba, 0xd5a61266f0c9392c, 0xa9582618e03fc9aa, 0x39abdc4529b1661c };

	uint64_t s0 = 0;
	uint64_t s1 = 0;
	uint64_t s2 = 0;
	uint64_t s3 = 0;

	for (unsigned i = 0; i < detail::count(JUMP); ++i)
		for (int b = 0; b < 64; ++b) {
			if (JUMP[i] & UINT64_C(1) << b) {
				s0 ^= _state[0];
				s1 ^= _state[1];
				s2 ^= _state[2];
				s3 ^= _state[3];
			}
			raw();
		}

	_state[0] = s0;
	_state[1] = s1;
	_state[2] = s2;
	_state[3] = s3;
}

//------------------------------------------------------------------------------

uint64_t
XoShiRo256Plus::raw() {
	const uint64_t result_plus = _state[0] + _state[3];

	const uint64_t t = _state[1] << 17;

	_state[2] ^= _state[0];
	_state[3] ^= _state[1];
	_state[1] ^= _state[2];
	_state[0] ^= _state[3];

	_state[2] ^= t;

	_state[3] = detail::rotl(_state[3], 45);

	return result_plus;
}

////////////////////////////////////////////////////////////////////////////////

SplitMix32::SplitMix32(const uint32_t seed)
	: _state{seed}
{}

//------------------------------------------------------------------------------

uint32_t
SplitMix32::next() {
	for (;;)
		if (const uint32_t number = raw())
			return number;
}

//------------------------------------------------------------------------------

uint32_t
SplitMix32::raw() {
	uint32_t z = (_state += 0x9E3779B9);
	z = (z ^ (z >> 16)) * 0x85ebca6b;
	z = (z ^ (z >> 13)) * 0xc2b2ae35;

	return z ^ (z >> 16);
}

////////////////////////////////////////////////////////////////////////////////

XoShiRo128Plus::XoShiRo128Plus() {
	SplitMix32 generator;

	_state[0] = generator.next();
	_state[1] = generator.next();
	_state[2] = generator.next();
	_state[3] = generator.next();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

XoShiRo128Plus::XoShiRo128Plus(const uint32_t s) {
	seed(s);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

XoShiRo128Plus::XoShiRo128Plus(const uint32_t(& s)[4]) {
	seed(s);
}

//------------------------------------------------------------------------------

void
XoShiRo128Plus::seed(const uint32_t s) {
	SplitMix32 generator{s};

	_state[0] = generator.next();
	_state[1] = generator.next();
	_state[2] = generator.next();
	_state[3] = generator.next();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void
XoShiRo128Plus::seed(const uint32_t(& s)[4]) {
	_state[0] = s[0];
	_state[1] = s[1];
	_state[2] = s[2];
	_state[3] = s[3];
}

//------------------------------------------------------------------------------

float
XoShiRo128Plus::next() {
	return detail::uniformReal(raw());
}

//------------------------------------------------------------------------------

void
XoShiRo128Plus::jump() {
	static const uint32_t JUMP[] = { 0x8764000b, 0xf542d2d3, 0x6fa035c3, 0x77f2db5b };

	uint32_t s0 = 0;
	uint32_t s1 = 0;
	uint32_t s2 = 0;
	uint32_t s3 = 0;

	for (unsigned i = 0; i < detail::count(JUMP); ++i)
		for (unsigned b = 0; b < 32; ++b) {
			if (JUMP[i] & UINT32_C(1) << b) {
				s0 ^= _state[0];
				s1 ^= _state[1];
				s2 ^= _state[2];
				s3 ^= _state[3];
			}
			raw();
		}

	_state[0] = s0;
	_state[1] = s1;
	_state[2] = s2;
	_state[3] = s3;
}

//------------------------------------------------------------------------------

uint32_t
XoShiRo128Plus::raw() {
	const uint32_t result_plus = _state[0] + _state[3];

	const uint32_t t = _state[1] << 9;

	_state[2] ^= _state[0];
	_state[3] ^= _state[1];
	_state[1] ^= _state[2];
	_state[0] ^= _state[3];

	_state[2] ^= t;

	_state[3] = detail::rotl(_state[3], 11);

	return result_plus;
}

////////////////////////////////////////////////////////////////////////////////

}

namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

template<LongIndex N>
struct IndexConstant {};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template<LongIndex... Ns>
struct IndexSequence {
	using Type = IndexSequence<Ns...>;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template<typename, typename>
struct MakeIndexSequenceT {};

template<LongIndex N, LongIndex... Ns>
struct MakeIndexSequenceT<IndexConstant<N>,
						  IndexSequence<Ns...>>
	: MakeIndexSequenceT<IndexConstant<N - 1>,
						 IndexSequence<N - 1, Ns...>>
{};

template<LongIndex... Ns>
struct MakeIndexSequenceT<IndexConstant<0>,
							  IndexSequence<Ns...>>
	: IndexSequence<Ns...>
{};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template<LongIndex N>
using MakeIndexSequence = typename MakeIndexSequenceT<IndexConstant<N>,
													  IndexSequence<>>::Type;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template<class... Ts>
using IndexSequenceFor = MakeIndexSequence<sizeof...(Ts)>;

////////////////////////////////////////////////////////////////////////////////

template <typename T>
struct ITL_EntryT {};

template <typename T, LongIndex N>
struct ITL_EntryN
	: ITL_EntryT<T>
{};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename...>
struct ITL_Impl;

template <LongIndex... Ns, typename... Ts>
struct ITL_Impl<IndexSequence<Ns...>, Ts...>
	: ITL_EntryN<Ts, Ns>...
{
	template <typename T, LongIndex N>
	static constexpr LongIndex select(ITL_EntryN<T, N>) { return (LongIndex) N; }
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename... Ts>
struct ITL_
	: private ITL_Impl<IndexSequenceFor<Ts...>, Ts...>
{
	using Base = ITL_Impl<IndexSequenceFor<Ts...>, Ts...>;

	static constexpr LongIndex SIZE = sizeof...(Ts);

	template <typename T>
	static constexpr bool contains() { return std::is_base_of<ITL_EntryT<T>, ITL_>::value; }

	template <typename T>
	static constexpr
	typename std::enable_if< std::is_base_of<ITL_EntryT<T>, ITL_>::value, LongIndex>::type
	index() {
		return Base::template select<T>(ITL_{});
	}

	template <typename T>
	static constexpr
	typename std::enable_if<!std::is_base_of<ITL_EntryT<T>, ITL_>::value, LongIndex>::type
	index() {
		return INVALID_LONG_INDEX;
	}
};

////////////////////////////////////////////////////////////////////////////////

template <typename...>
struct PrependT;

template <typename T, typename... Ts>
struct PrependT<T, ITL_<Ts...>> {
	using Type = ITL_<T, Ts...>;
};

template <typename... Ts>
using Prepend = typename PrependT<Ts...>::Type;

//------------------------------------------------------------------------------

template <typename...>
struct MergeT;

template <typename... Ts1, typename... Ts2>
struct MergeT<ITL_<Ts1...>, ITL_<Ts2...>> {
	using Type = ITL_<Ts1..., Ts2...>;
};

template <typename... Ts>
using Merge = typename MergeT<Ts...>::Type;

//------------------------------------------------------------------------------

template <LongIndex, LongIndex, typename...>
struct LesserT;

template <LongIndex H, LongIndex I, typename TFirst, typename... TRest>
struct LesserT<H, I, TFirst, TRest...> {
	using Type = typename std::conditional<(I < H),
										   Prepend<TFirst, typename LesserT<H, I + 1, TRest...>::Type>,
										   typename LesserT<H, I + 1, TRest...>::Type>::type;
};

template <LongIndex H, LongIndex I>
struct LesserT<H, I> {
	using Type = ITL_<>;
};

template <typename... Ts>
using SplitL = typename LesserT<sizeof...(Ts) / 2, 0, Ts...>::Type;

//------------------------------------------------------------------------------

template <LongIndex, LongIndex, typename...>
struct GreaterT;

template <LongIndex H, LongIndex I, typename TFirst, typename... TRest>
struct GreaterT<H, I, TFirst, TRest...> {
	using Type = typename std::conditional<(I < H),
										   typename GreaterT<H, I + 1, TRest...>::Type,
										   ITL_<TFirst, TRest...>>::type;
};

template <LongIndex H, LongIndex I>
struct GreaterT<H, I> {
	using Type = ITL_<>;
};

template <typename... Ts>
using SplitR = typename GreaterT<sizeof...(Ts) / 2, 0, Ts...>::Type;

////////////////////////////////////////////////////////////////////////////////

}
}


////////////////////////////////////////////////////////////////////////////////

namespace hfsm2 {

enum class Method : uint8_t {
	NONE,

	RANK,
	UTILITY,
	ENTRY_GUARD,
	CONSTRUCT,
	ENTER,
	REENTER,
	UPDATE,
	REACT,
	EXIT_GUARD,
	EXIT,
	DESTRUCT,
	PLAN_SUCCEEDED,
	PLAN_FAILED,

	COUNT
};

enum class TransitionType : uint8_t {
	CHANGE,
	RESTART,
	RESUME,
	UTILIZE,
	RANDOMIZE,
	SCHEDULE,

	COUNT
};

enum class StatusEvent : uint8_t {
	SUCCEEDED,
	FAILED,

	COUNT
};

//------------------------------------------------------------------------------

static inline
const char*
stateName(const std::type_index stateType) {
	const char* const raw = stateType.name();

	#if defined(_MSC_VER)

		auto first =
			raw[0] == 's' ? 7 : // Struct
			raw[0] == 'c' ? 6 : // Class
							0;
		return raw + first;

	#elif defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)

		return raw;

	#else

		return raw;

	#endif
}

//------------------------------------------------------------------------------

static inline
const char*
methodName(const Method method) {
	switch (method) {
	case Method::RANK:			 return "rank";
	case Method::UTILITY:		 return "utility";
	case Method::ENTRY_GUARD:	 return "entryGuard";
	case Method::ENTER:			 return "enter";
	case Method::CONSTRUCT:		 return "construct";
	case Method::REENTER:		 return "reenter";
	case Method::UPDATE:		 return "update";
	case Method::REACT:			 return "react";
	case Method::EXIT_GUARD:	 return "exitGuard";
	case Method::EXIT:			 return "exit";
	case Method::DESTRUCT:		 return "destruct";
	case Method::PLAN_SUCCEEDED: return "planSucceeded";
	case Method::PLAN_FAILED:	 return "planFailed";

	default:
		HFSM_BREAK();
		return nullptr;
	}
}

//------------------------------------------------------------------------------

static inline
const char*
transitionName(const TransitionType transitionType) {
	switch (transitionType) {
	case TransitionType::CHANGE:	return "changeTo";
	case TransitionType::RESTART:	return "restart";
	case TransitionType::RESUME:	return "resume";
	case TransitionType::UTILIZE:	return "utilize";
	case TransitionType::RANDOMIZE:	return "randomize";
	case TransitionType::SCHEDULE:	return "schedule";

	default:
		HFSM_BREAK();
		return nullptr;
	}
}

////////////////////////////////////////////////////////////////////////////////

}

namespace hfsm2 {

#if defined HFSM_ENABLE_LOG_INTERFACE || defined HFSM_ENABLE_VERBOSE_DEBUG_LOG

////////////////////////////////////////////////////////////////////////////////

template <typename TContext = EmptyContext,
		  typename TUtilty = float>
struct LoggerInterfaceT {
	using Context		 = TContext;
	using Utilty		 = TUtilty;

	using Method		 = ::hfsm2::Method;
	using StateID		 = ::hfsm2::StateID;
	using RegionID		 = ::hfsm2::RegionID;
	using TransitionType = ::hfsm2::TransitionType;
	using StatusEvent	 = ::hfsm2::StatusEvent;

	virtual void recordMethod(Context& /*context*/,
							  const StateID /*origin*/,
							  const Method /*method*/)
	{}

	virtual void recordTransition(Context& /*context*/,
								  const StateID /*origin*/,
								  const TransitionType /*transitionType*/,
								  const StateID /*target*/)
	{}

	virtual void recordTaskStatus(Context& /*context*/,
								  const RegionID /*region*/,
								  const StateID /*origin*/,
								  const StatusEvent /*event*/)
	{}

	virtual void recordPlanStatus(Context& /*context*/,
								  const RegionID /*region*/,
								  const StatusEvent /*event*/)
	{}

	virtual void recordCancelledPending(Context& /*context*/,
										const StateID /*origin*/)
	{}

	virtual void recordUtilityResolution(Context& /*context*/,
										 const StateID /*head*/,
										 const StateID /*prong*/,
										 const Utilty /*utilty*/)
	{}

	virtual void recordRandomResolution(Context& /*context*/,
										const StateID /*head*/,
										const StateID /*prong*/,
										const Utilty /*utilty*/)
	{}
};

////////////////////////////////////////////////////////////////////////////////

#else

template <typename TContext = EmptyContext,
		  typename TUtilty = float>
using LoggerInterfaceT = void;

#endif

using LoggerInterface = LoggerInterfaceT<>;

}

namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

#pragma pack(push, 2)

struct TaskLink {
	HFSM_INLINE TaskLink(const TransitionType transitionType_,
						 const StateID origin_,
						 const StateID destination_)
		: transitionType{transitionType_}
		, origin{origin_}
		, destination{destination_}
		, next{INVALID_LONG_INDEX}
	{}

	TransitionType transitionType;
	StateID origin		= INVALID_STATE_ID;
	StateID destination	= INVALID_STATE_ID;

	LongIndex prev		= INVALID_LONG_INDEX;
	LongIndex next		= INVALID_LONG_INDEX;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct Bounds {
	LongIndex first		= INVALID_LONG_INDEX;
	LongIndex last		= INVALID_LONG_INDEX;
};

#pragma pack(pop)

//------------------------------------------------------------------------------

template <typename,
		  typename,
		  typename,
		  typename,
		  LongIndex,
		  LongIndex,
		  LongIndex,
		  LongIndex,
		  LongIndex>
struct ArgsT;

template <typename>
struct PlanDataT;

//------------------------------------------------------------------------------

template <typename TContext,
		  typename TConfig,
		  typename TStateList,
		  typename TRegionList,
		  LongIndex NCompoCount,
		  LongIndex NOrthoCount,
		  LongIndex NOrthoUnits,
		  LongIndex NSerialBits,
		  LongIndex NTaskCapacity>
struct PlanDataT<ArgsT<TContext,
					   TConfig,
					   TStateList,
					   TRegionList,
					   NCompoCount,
					   NOrthoCount,
					   NOrthoUnits,
					   NSerialBits,
					   NTaskCapacity>>
{
	using StateList		= TStateList;
	using RegionList	= TRegionList;

	static constexpr ShortIndex REGION_COUNT  = RegionList::SIZE;
	static constexpr ShortIndex TASK_CAPACITY = NTaskCapacity;

	using TaskLinks		= List<TaskLink, TASK_CAPACITY>;
	using TasksBounds	= Array<Bounds, RegionList::SIZE>;
	using TasksBits		= BitArray<StateID, StateList::SIZE>;
	using RegionBits	= BitArray<RegionID, RegionList::SIZE>;

	TaskLinks taskLinks;
	TasksBounds tasksBounds;
	TasksBits tasksSuccesses;
	TasksBits tasksFailures;
	RegionBits planExists;

#ifdef HFSM_ENABLE_ASSERT
	void verifyPlans() const;
	LongIndex verifyPlan(const RegionID stateId) const;
#endif
};

//------------------------------------------------------------------------------

template <typename TContext,
		  typename TConfig,
		  typename TStateList,
		  typename TRegionList,
		  LongIndex NOrthoCount,
		  LongIndex NOrthoUnits,
		  LongIndex NTaskCapacity>
struct PlanDataT<ArgsT<TContext,
					   TConfig,
					   TStateList,
					   TRegionList,
					   0,
					   NOrthoCount,
					   NOrthoUnits,
					   0,
					   NTaskCapacity>>
{
#ifdef HFSM_ENABLE_ASSERT
	void verifyPlans() const													{}
	LongIndex verifyPlan(const RegionID) const					{ return 0;		}
#endif
};

////////////////////////////////////////////////////////////////////////////////

}
}

namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

#ifdef HFSM_ENABLE_ASSERT

template <typename TC, typename TG, typename TSL, typename TRL, LongIndex NCC, LongIndex NOC, LongIndex NOU, LongIndex NSB, LongIndex NTC>
void
PlanDataT<ArgsT<TC, TG, TSL, TRL, NCC, NOC, NOU, NSB, NTC>>::verifyPlans() const {
	LongIndex planCount = 0;
	for (RegionID id = 0; id < REGION_COUNT; ++id)
		planCount += verifyPlan(id);

	HFSM_ASSERT(taskLinks.count() == planCount);
}

//------------------------------------------------------------------------------

template <typename TC, typename TG, typename TSL, typename TRL, LongIndex NCC, LongIndex NOC, LongIndex NOU, LongIndex NSB, LongIndex NTC>
LongIndex
PlanDataT<ArgsT<TC, TG, TSL, TRL, NCC, NOC, NOU, NSB, NTC>>::verifyPlan(const RegionID regionId) const {
	LongIndex length = 0;
	const Bounds& bounds = tasksBounds[regionId];

	if (bounds.first != INVALID_LONG_INDEX) {
		HFSM_ASSERT(bounds.last != INVALID_LONG_INDEX);

		for (auto slow = bounds.first, fast = slow; ; ) {
			++length;
			const TaskLink& task = taskLinks[slow];

			if (slow != bounds.last) {
				HFSM_ASSERT(task.next != INVALID_LONG_INDEX);
				slow = task.next;

				// loop check
				if (fast != INVALID_LONG_INDEX) {
					fast = taskLinks[fast].next;

					if (fast != INVALID_LONG_INDEX) {
						fast = taskLinks[fast].next;
					}

					HFSM_ASSERT(fast == INVALID_LONG_INDEX || slow != fast);
				}
			} else {
				HFSM_ASSERT(task.next == INVALID_LONG_INDEX);

				break;
			}
		};
	} else
		HFSM_ASSERT(bounds.last == INVALID_LONG_INDEX);

	return length;
}

#endif

////////////////////////////////////////////////////////////////////////////////

}
}
namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

#pragma pack(push, 1)

struct Status {
	enum Result {
		NONE,
		SUCCESS,
		FAILURE
	};

	Result result = NONE;
	bool outerTransition = false;

	inline Status(const Result result_ = NONE,
				  const bool outerTransition_ = false);

	inline explicit operator bool() const { return result || outerTransition; }

	inline void clear();
};

#pragma pack(pop)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

inline Status
combine(const Status lhs, const Status rhs) {
	const Status::Result result = lhs.result > rhs.result ?
		lhs.result : rhs.result;

	return {result, lhs.outerTransition || rhs.outerTransition};
}

////////////////////////////////////////////////////////////////////////////////

template <typename TArgs>
class ConstPlanT {
	template <typename>
	friend class ControlT;

	template <typename>
	friend class PlanControlT;

	template <typename>
	friend class FullControlT;

	template <typename>
	friend class GuardControlT;

	using Args			= TArgs;
	using Context		= typename Args::Context;
	using StateList		= typename Args::StateList;
	using RegionList	= typename Args::RegionList;

	static constexpr LongIndex TASK_CAPACITY = Args::TASK_CAPACITY;

public:
	using PlanData		= PlanDataT<Args>;
	using TaskLinks		= typename PlanData::TaskLinks;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct Iterator {
		HFSM_INLINE Iterator(const ConstPlanT& plan);

		HFSM_INLINE explicit operator bool() const;

		HFSM_INLINE void operator ++();

		HFSM_INLINE const TaskLink& operator  *() const { return  _plan._planData.taskLinks[_curr];	}
		HFSM_INLINE const TaskLink* operator ->() const { return &_plan._planData.taskLinks[_curr];	}

		HFSM_INLINE LongIndex next() const;

		const ConstPlanT& _plan;
		LongIndex _curr;
		LongIndex _next;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:
	HFSM_INLINE ConstPlanT(const PlanData& planData,
						   const RegionID regionId);

	template <typename T>
	static constexpr StateID  stateId()		{ return			StateList ::template index<T>();	}

	template <typename T>
	static constexpr RegionID regionId()	{ return (RegionID) RegionList::template index<T>();	}

public:
	HFSM_INLINE explicit operator bool() const;

	HFSM_INLINE Iterator first()			{ return Iterator{*this};								}

private:
	const PlanData& _planData;
	const Bounds& _bounds;
};

////////////////////////////////////////////////////////////////////////////////

template <typename TArgs>
class PlanT {
	template <typename>
	friend class PlanControlT;

	template <typename>
	friend class FullControlT;

	template <typename>
	friend class GuardControlT;

	using Args			= TArgs;
	using Context		= typename Args::Context;
	using StateList		= typename Args::StateList;
	using RegionList	= typename Args::RegionList;

	static constexpr LongIndex  TASK_CAPACITY	= Args::TASK_CAPACITY;

public:
	using PlanData		= PlanDataT<Args>;
	using TaskLinks		= typename PlanData::TaskLinks;
	using TaskIndex		= typename TaskLinks::Index;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct Iterator {
		HFSM_INLINE Iterator(PlanT& plan);

		HFSM_INLINE explicit operator bool() const;

		HFSM_INLINE void operator ++();

		HFSM_INLINE		  TaskLink& operator  *()	    { return  _plan._planData.taskLinks[_curr];	}
		HFSM_INLINE const TaskLink& operator  *() const { return  _plan._planData.taskLinks[_curr];	}

		HFSM_INLINE		  TaskLink* operator ->()	    { return &_plan._planData.taskLinks[_curr];	}
		HFSM_INLINE const TaskLink* operator ->() const { return &_plan._planData.taskLinks[_curr];	}

		HFSM_INLINE void remove();

		HFSM_INLINE LongIndex next() const;

		PlanT& _plan;
		LongIndex _curr;
		LongIndex _next;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:
	HFSM_INLINE PlanT(PlanData& planData,
					  const RegionID regionId);

	template <typename T>
	static constexpr StateID  stateId()		{ return			StateList ::template index<T>();	}

	template <typename T>
	static constexpr RegionID regionId()	{ return (RegionID) RegionList::template index<T>();	}

	bool append(const TransitionType transitionType,
				const StateID origin,
				const StateID destination);

public:
	HFSM_INLINE explicit operator bool() const;

	HFSM_INLINE void clear();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE bool change   (const StateID origin, const StateID destination)	{ return append(TransitionType::CHANGE,	   origin, destination); }
	HFSM_INLINE bool restart  (const StateID origin, const StateID destination)	{ return append(TransitionType::RESTART,   origin, destination); }
	HFSM_INLINE bool resume   (const StateID origin, const StateID destination)	{ return append(TransitionType::RESUME,	   origin, destination); }
	HFSM_INLINE bool utilize  (const StateID origin, const StateID destination)	{ return append(TransitionType::UTILIZE,   origin, destination); }
	HFSM_INLINE bool randomize(const StateID origin, const StateID destination)	{ return append(TransitionType::RANDOMIZE, origin, destination); }
	HFSM_INLINE bool schedule (const StateID origin, const StateID destination)	{ return append(TransitionType::SCHEDULE,  origin, destination); }

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	template <typename TOrigin>
	HFSM_INLINE bool change   (const StateID destination)					{ return change   (stateId<TOrigin>(), destination);				}

	template <typename TOrigin>
	HFSM_INLINE bool restart  (const StateID destination)					{ return restart  (stateId<TOrigin>(), destination);				}

	template <typename TOrigin>
	HFSM_INLINE bool resume   (const StateID destination)					{ return resume   (stateId<TOrigin>(), destination);				}

	template <typename TOrigin>
	HFSM_INLINE bool utilize  (const StateID destination)					{ return utilize  (stateId<TOrigin>(), destination);				}

	template <typename TOrigin>
	HFSM_INLINE bool randomize(const StateID destination)					{ return randomize(stateId<TOrigin>(), destination);				}

	template <typename TOrigin>
	HFSM_INLINE bool schedule (const StateID destination)					{ return schedule (stateId<TOrigin>(), destination);				}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	template <typename TOrigin, typename TDestination>
	HFSM_INLINE bool change   ()											{ return change   (stateId<TOrigin>(), stateId<TDestination>());	}

	template <typename TOrigin, typename TDestination>
	HFSM_INLINE bool restart  ()											{ return restart  (stateId<TOrigin>(), stateId<TDestination>());	}

	template <typename TOrigin, typename TDestination>
	HFSM_INLINE bool resume   ()											{ return resume   (stateId<TOrigin>(), stateId<TDestination>());	}

	template <typename TOrigin, typename TDestination>
	HFSM_INLINE bool utilize  ()											{ return utilize  (stateId<TOrigin>(), stateId<TDestination>());	}

	template <typename TOrigin, typename TDestination>
	HFSM_INLINE bool randomize()											{ return randomize(stateId<TOrigin>(), stateId<TDestination>());	}

	template <typename TOrigin, typename TDestination>
	HFSM_INLINE bool schedule ()											{ return schedule (stateId<TOrigin>(), stateId<TDestination>());	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE Iterator first()			{ return Iterator{*this};								}

private:
	void remove(const LongIndex task);

private:
	PlanData& _planData;
	const RegionID _regionId;
	Bounds& _bounds;
};

////////////////////////////////////////////////////////////////////////////////

}
}

namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

Status::Status(const Result result_,
			   const bool outerTransition_)
	: result{result_}
	, outerTransition{outerTransition_}
{}

//------------------------------------------------------------------------------

void
Status::clear() {
	result = NONE;
	outerTransition = false;
}

////////////////////////////////////////////////////////////////////////////////

template <typename TArgs>
ConstPlanT<TArgs>::Iterator::Iterator(const ConstPlanT& plan)
	: _plan{plan}
	, _curr{plan._bounds.first}
{
	_next = next();
}

//------------------------------------------------------------------------------

template <typename TArgs>
ConstPlanT<TArgs>::Iterator::operator bool() const {
	HFSM_ASSERT(_curr < ConstPlanT::TASK_CAPACITY || _curr == INVALID_LONG_INDEX);

	return _curr < ConstPlanT::TASK_CAPACITY;
}

//------------------------------------------------------------------------------

template <typename TArgs>
void
ConstPlanT<TArgs>::Iterator::operator ++() {
	_curr = _next;
	_next = next();
}

//------------------------------------------------------------------------------

template <typename TArgs>
LongIndex
ConstPlanT<TArgs>::Iterator::next() const {
	if (_curr < ConstPlanT::TASK_CAPACITY) {
		const TaskLink& task = _plan._planData.taskLinks[_curr];

		return task.next;
	} else {
		HFSM_ASSERT(_curr == INVALID_LONG_INDEX);

		return INVALID_LONG_INDEX;
	}
}

////////////////////////////////////////////////////////////////////////////////

template <typename TArgs>
ConstPlanT<TArgs>::ConstPlanT(const PlanData& planData,
							  const RegionID regionId)

	: _planData{planData}
	, _bounds{planData.tasksBounds[regionId]}
{}

//------------------------------------------------------------------------------

template <typename TArgs>
ConstPlanT<TArgs>::operator bool() const {
	if (_bounds.first < TASK_CAPACITY) {
		HFSM_ASSERT(_bounds.last < TASK_CAPACITY);
		return true;
	} else {
		HFSM_ASSERT(_bounds.last == INVALID_LONG_INDEX);
		return false;
	}
}

////////////////////////////////////////////////////////////////////////////////

template <typename TArgs>
PlanT<TArgs>::Iterator::Iterator(PlanT& plan)
	: _plan{plan}
	, _curr{plan._bounds.first}
{
	_next = next();
}

//------------------------------------------------------------------------------

template <typename TArgs>
PlanT<TArgs>::Iterator::operator bool() const {
	HFSM_ASSERT(_curr < PlanT::TASK_CAPACITY || _curr == INVALID_LONG_INDEX);

	return _curr < PlanT::TASK_CAPACITY;
}

//------------------------------------------------------------------------------

template <typename TArgs>
void
PlanT<TArgs>::Iterator::operator ++() {
	_curr = _next;
	_next = next();
}

//------------------------------------------------------------------------------

template <typename TArgs>
void
PlanT<TArgs>::Iterator::remove() {
	_plan.remove(_curr);
}

//------------------------------------------------------------------------------

template <typename TArgs>
LongIndex
PlanT<TArgs>::Iterator::next() const {
	if (_curr < PlanT::TASK_CAPACITY) {
		const TaskLink& task = _plan._planData.taskLinks[_curr];

		return task.next;
	} else {
		HFSM_ASSERT(_curr == INVALID_LONG_INDEX);

		return INVALID_LONG_INDEX;
	}
}

////////////////////////////////////////////////////////////////////////////////

template <typename TArgs>
PlanT<TArgs>::PlanT(PlanData& planData,
					const RegionID regionId)

	: _planData{planData}
	, _regionId{regionId}
	, _bounds{planData.tasksBounds[regionId]}
{}

//------------------------------------------------------------------------------

template <typename TArgs>
PlanT<TArgs>::operator bool() const {
	if (_bounds.first < TASK_CAPACITY) {
		HFSM_ASSERT(_bounds.last < TASK_CAPACITY);
		return true;
	} else {
		HFSM_ASSERT(_bounds.last == INVALID_LONG_INDEX);
		return false;
	}
}

//------------------------------------------------------------------------------

template <typename TArgs>
bool
PlanT<TArgs>::append(const TransitionType transitionType,
					 const StateID origin,
					 const StateID destination)
{
	_planData.planExists.set(_regionId);

	const TaskIndex index = _planData.taskLinks.emplace(transitionType, origin, destination);
	if (index == TaskLinks::INVALID)
		return false;

	if (_bounds.first < TaskLinks::CAPACITY) {
		HFSM_ASSERT(_bounds.last < TaskLinks::CAPACITY);

		auto& last  = _planData.taskLinks[_bounds.last];
		last.next = index;

		auto& next = _planData.taskLinks[index];
		next.prev  = _bounds.last;

		_bounds.last = index;
	} else {
		HFSM_ASSERT(_bounds.first == INVALID_LONG_INDEX &&
					_bounds.last  == INVALID_LONG_INDEX);

		_bounds.first = index;
		_bounds.last  = index;
	}

	return true;
}

//------------------------------------------------------------------------------

template <typename TArgs>
void
PlanT<TArgs>::clear() {
	if (_bounds.first < TaskLinks::CAPACITY) {
		HFSM_ASSERT(_bounds.last < TaskLinks::CAPACITY);

		for (LongIndex index = _bounds.first;
			 index != INVALID_LONG_INDEX;
			 )
		{
			HFSM_ASSERT(index < TaskLinks::CAPACITY);

			const auto& task = _planData.taskLinks[index];
			HFSM_ASSERT(index == _bounds.first ?
				   task.prev == INVALID_LONG_INDEX :
				   task.prev <  TaskLinks::CAPACITY);

			const LongIndex next = task.next;

			_planData.taskLinks.remove(index);

			index = next;
		}

		_bounds.first = INVALID_LONG_INDEX;
		_bounds.last  = INVALID_LONG_INDEX;
	} else
		HFSM_ASSERT(_bounds.first == INVALID_LONG_INDEX &&
			   _bounds.last  == INVALID_LONG_INDEX);
}

//------------------------------------------------------------------------------

template <typename TArgs>
void
PlanT<TArgs>::remove(const LongIndex task) {
	HFSM_ASSERT(_planData.planExists.get(_regionId) &&
				_bounds.first < TaskLinks::CAPACITY &&
				_bounds.last  < TaskLinks::CAPACITY);

	HFSM_ASSERT(task < TaskLinks::CAPACITY);

	const TaskLink& curr = _planData.taskLinks[task];

	if (curr.prev < TaskLinks::CAPACITY) {
		TaskLink& prev = _planData.taskLinks[curr.prev];
		prev.next = curr.next;
	} else {
		HFSM_ASSERT(_bounds.first == task);
		_bounds.first = curr.next;
	}

	if (curr.next < TaskLinks::CAPACITY) {
		TaskLink& next = _planData.taskLinks[curr.next];
		next.prev = curr.prev;
	} else {
		HFSM_ASSERT(_bounds.last == task);
		_bounds.last = curr.prev;
	}

	_planData.taskLinks.remove(task);
}

////////////////////////////////////////////////////////////////////////////////

}
}

namespace hfsm2 {
namespace detail {

//------------------------------------------------------------------------------

enum Strategy {
	Composite,
	Resumable,
	Utilitarian,
	RandomUtil,
};

////////////////////////////////////////////////////////////////////////////////

#pragma pack(push, 1)

struct alignas(2 * sizeof(ShortIndex)) Parent {
	HFSM_INLINE Parent() = default;

	HFSM_INLINE Parent(const ForkID forkId_)
		: forkId{forkId_}
	{}

	HFSM_INLINE Parent(const ForkID forkId_,
					   const ShortIndex prong_)
		: forkId{forkId_}
		, prong{prong_}
	{}

	HFSM_INLINE explicit operator bool() const {
		return forkId != INVALID_FORK_ID &&
			   prong  != INVALID_SHORT_INDEX;
	}

	ForkID	   forkId = INVALID_FORK_ID;
	ShortIndex prong  = INVALID_SHORT_INDEX;
};

#pragma pack(pop)

////////////////////////////////////////////////////////////////////////////////

struct Request {
	enum Type {
		REMAIN,
		CHANGE,
		RESTART,
		RESUME,
		UTILIZE,
		RANDOMIZE,
		SCHEDULE,

		COUNT
	};

	HFSM_INLINE Request() = default;

	HFSM_INLINE Request(const Type type_,
						const StateID stateId_)
		: type{type_}
		, stateId{stateId_}
	{
		HFSM_ASSERT(type_ < Type::COUNT);
	}

	Type type = CHANGE;
	StateID stateId = INVALID_STATE_ID;
};

template <ShortIndex NCount>
using RequestsT = Array<Request, NCount>;

////////////////////////////////////////////////////////////////////////////////

template <typename TRegistry>
struct BackUpT {
	using CompoForks = typename TRegistry::CompoForks;
	using OrthoForks = typename TRegistry::OrthoForks;

	CompoForks compoRequested;
	OrthoForks orthoRequested;
	CompoForks compoResumable;
};

////////////////////////////////////////////////////////////////////////////////

template <typename,
		  typename,
		  typename,
		  typename,
		  LongIndex,
		  LongIndex,
		  LongIndex,
		  LongIndex,
		  LongIndex>
struct ArgsT;

template <typename>
struct RegistryT;

//------------------------------------------------------------------------------

template <typename TContext,
		  typename TConfig,
		  typename TStateList,
		  typename TRegionList,
		  LongIndex NCompoCount,
		  LongIndex NOrthoCount,
		  LongIndex NOrthoUnits,
		  LongIndex NSerialBits,
		  LongIndex NTaskCapacity>
struct RegistryT<ArgsT<TContext,
					   TConfig,
					   TStateList,
					   TRegionList,
					   NCompoCount,
					   NOrthoCount,
					   NOrthoUnits,
					   NSerialBits,
					   NTaskCapacity>>
{
	using StateList		= TStateList;
	using RegionList	= TRegionList;

	static constexpr LongIndex  STATE_COUNT		= StateList::SIZE;
	static constexpr ShortIndex COMPO_REGIONS	= NCompoCount;
	static constexpr ShortIndex ORTHO_REGIONS	= NOrthoCount;
	static constexpr ShortIndex ORTHO_UNITS		= NOrthoUnits;

	using StateParents	= StaticArray<Parent, STATE_COUNT>;

	using CompoParents	= StaticArray<Parent, COMPO_REGIONS>;
	using OrthoParents	= StaticArray<Parent, ORTHO_REGIONS>;
	using OrthoUnits	= StaticArray<Units,  ORTHO_UNITS>;

	using CompoForks	= StaticArray<ShortIndex, COMPO_REGIONS>;
	using OrthoForks	= BitArray	 <ShortIndex, ORTHO_UNITS>;
	using OrthoBits		= typename OrthoForks::Bits;
	using CompoRemains	= BitArray	 <ShortIndex, COMPO_REGIONS>;

	using BackUp		= BackUpT<RegistryT>;

	bool isActive		(const StateID stateId) const;
	bool isResumable	(const StateID stateId) const;

	bool isPendingChange(const StateID stateId) const;
	bool isPendingEnter	(const StateID stateId) const;
	bool isPendingExit	(const StateID stateId) const;

	HFSM_INLINE const Parent&	  forkParent(const ForkID forkId) const;

	HFSM_INLINE OrthoBits requestedOrthoFork(const ForkID forkId);

	bool requestImmediate(const Request request);
	void requestScheduled(const StateID stateId);

	void clearRequests();
	void reset();

	StateParents stateParents;
	CompoParents compoParents;
	OrthoParents orthoParents;
	OrthoUnits orthoUnits;

	CompoForks compoRequested{INVALID_SHORT_INDEX};
	OrthoForks orthoRequested;
	CompoForks compoActive	 {INVALID_SHORT_INDEX};
	CompoForks compoResumable{INVALID_SHORT_INDEX};

	CompoRemains compoRemains;
};

//------------------------------------------------------------------------------

template <typename TContext,
		  typename TConfig,
		  typename TStateList,
		  typename TRegionList,
		  LongIndex NCompoCount,
		  LongIndex NSerialBits,
		  LongIndex NTaskCapacity>
struct RegistryT<ArgsT<TContext,
					   TConfig,
					   TStateList,
					   TRegionList,
					   NCompoCount,
					   0,
					   0,
					   NSerialBits,
					   NTaskCapacity>>
{
	using StateList		= TStateList;
	using RegionList	= TRegionList;

	static constexpr LongIndex  STATE_COUNT		= StateList::SIZE;
	static constexpr ShortIndex COMPO_REGIONS	= NCompoCount;

	using StateParents	= StaticArray<Parent, STATE_COUNT>;
	using CompoParents	= StaticArray<Parent, COMPO_REGIONS>;

	using CompoForks	= StaticArray<ShortIndex, COMPO_REGIONS>;
	using OrthoForks	= BitArray	 <ShortIndex, 0>;
	using CompoRemains	= BitArray	 <ShortIndex, COMPO_REGIONS>;

	using BackUp		= BackUpT<RegistryT>;

	bool isActive		(const StateID stateId) const;
	bool isResumable	(const StateID stateId) const;

	bool isPendingChange(const StateID stateId) const;
	bool isPendingEnter	(const StateID stateId) const;
	bool isPendingExit	(const StateID stateId) const;

	HFSM_INLINE const Parent& forkParent(const ForkID forkId) const;

	bool requestImmediate(const Request request);
	void requestScheduled(const StateID stateId);

	void clearRequests();
	void reset();

	StateParents stateParents;
	CompoParents compoParents;

	CompoForks compoRequested{INVALID_SHORT_INDEX};
	OrthoForks orthoRequested;
	CompoForks compoActive	 {INVALID_SHORT_INDEX};
	CompoForks compoResumable{INVALID_SHORT_INDEX};

	CompoRemains compoRemains;
};

////////////////////////////////////////////////////////////////////////////////

template <typename TRegistry>
void
backup(const TRegistry& registry, BackUpT<TRegistry>& copy) {
	overwrite(copy.compoRequested, registry.compoRequested);
	overwrite(copy.orthoRequested, registry.orthoRequested);
	overwrite(copy.compoResumable, registry.compoResumable);
}

//------------------------------------------------------------------------------

template <typename TRegistry>
void
restore(TRegistry& registry, const BackUpT<TRegistry>& copy) {
	overwrite(registry.compoRequested, copy.compoRequested);
	overwrite(registry.orthoRequested, copy.orthoRequested);
	overwrite(registry.compoResumable, copy.compoResumable);
}

////////////////////////////////////////////////////////////////////////////////

}
}


namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

template <typename TC, typename TG, typename TSL, typename TRL, LongIndex NCC, LongIndex NOC, LongIndex NOU, LongIndex NSB, LongIndex NTC>
bool
RegistryT<ArgsT<TC, TG, TSL, TRL, NCC, NOC, NOU, NSB, NTC>>::isActive(const StateID stateId) const {
	if (HFSM_CHECKED(stateId < STATE_COUNT))
		for (Parent parent = stateParents[stateId];
			 parent;
			 parent = forkParent(parent.forkId))
		{
			HFSM_ASSERT(parent.forkId != 0);

			if (parent.forkId > 0)
				return parent.prong == compoActive[parent.forkId - 1];
		}

	return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TC, typename TG, typename TSL, typename TRL, LongIndex NCC, LongIndex NOC, LongIndex NOU, LongIndex NSB, LongIndex NTC>
bool
RegistryT<ArgsT<TC, TG, TSL, TRL, NCC, NOC, NOU, NSB, NTC>>::isResumable(const StateID stateId) const {
	if (HFSM_CHECKED(stateId < STATE_COUNT))
		for (Parent parent = stateParents[stateId];
			 parent;
			 parent = forkParent(parent.forkId))
		{
			HFSM_ASSERT(parent.forkId != 0);

			if (parent.forkId > 0)
				return parent.prong == compoResumable[parent.forkId - 1];
		}

	return false;
}

//------------------------------------------------------------------------------

template <typename TC, typename TG, typename TSL, typename TRL, LongIndex NCC, LongIndex NOC, LongIndex NOU, LongIndex NSB, LongIndex NTC>
bool
RegistryT<ArgsT<TC, TG, TSL, TRL, NCC, NOC, NOU, NSB, NTC>>::isPendingChange(const StateID stateId) const {
	if (HFSM_CHECKED(stateId < STATE_COUNT))
		for (Parent parent = stateParents[stateId];
			 parent;
			 parent = forkParent(parent.forkId))
		{
			HFSM_ASSERT(parent.forkId != 0);

			if (parent.forkId > 0)
				return compoRequested[parent.forkId - 1] !=
					   compoActive	 [parent.forkId - 1];
		}

	return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TC, typename TG, typename TSL, typename TRL, LongIndex NCC, LongIndex NOC, LongIndex NOU, LongIndex NSB, LongIndex NTC>
bool
RegistryT<ArgsT<TC, TG, TSL, TRL, NCC, NOC, NOU, NSB, NTC>>::isPendingEnter(const StateID stateId) const {
	if (HFSM_CHECKED(stateId < STATE_COUNT))
		for (Parent parent = stateParents[stateId];
			 parent;
			 parent = forkParent(parent.forkId))
		{
			HFSM_ASSERT(parent.forkId != 0);

			if (parent.forkId > 0)
				return parent.prong != compoActive	 [parent.forkId - 1] &&
					   parent.prong == compoRequested[parent.forkId - 1];
		}

	return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TC, typename TG, typename TSL, typename TRL, LongIndex NCC, LongIndex NOC, LongIndex NOU, LongIndex NSB, LongIndex NTC>
bool
RegistryT<ArgsT<TC, TG, TSL, TRL, NCC, NOC, NOU, NSB, NTC>>::isPendingExit(const StateID stateId) const {
	if (HFSM_CHECKED(stateId < STATE_COUNT))
		for (Parent parent = stateParents[stateId];
			 parent;
			 parent = forkParent(parent.forkId))
		{
			HFSM_ASSERT(parent.forkId != 0);

			if (parent.forkId > 0)
				return parent.prong == compoActive	 [parent.forkId - 1] &&
					   parent.prong != compoRequested[parent.forkId - 1];
		}

	return true;
}

//------------------------------------------------------------------------------

template <typename TC, typename TG, typename TSL, typename TRL, LongIndex NCC, LongIndex NOC, LongIndex NOU, LongIndex NSB, LongIndex NTC>
const Parent&
RegistryT<ArgsT<TC, TG, TSL, TRL, NCC, NOC, NOU, NSB, NTC>>::forkParent(const ForkID forkId) const {
	HFSM_ASSERT(forkId != 0);

	return forkId > 0 ?
		compoParents[ forkId - 1] :
		orthoParents[-forkId - 1];
}

//------------------------------------------------------------------------------

template <typename TC, typename TG, typename TSL, typename TRL, LongIndex NCC, LongIndex NOC, LongIndex NOU, LongIndex NSB, LongIndex NTC>
typename RegistryT<ArgsT<TC, TG, TSL, TRL, NCC, NOC, NOU, NSB, NTC>>::OrthoBits
RegistryT<ArgsT<TC, TG, TSL, TRL, NCC, NOC, NOU, NSB, NTC>>::requestedOrthoFork(const ForkID forkId) {
	HFSM_ASSERT(forkId < 0);
	const Units& units = orthoUnits[-forkId - 1];

	return orthoRequested.bits(units);
}

//------------------------------------------------------------------------------

template <typename TC, typename TG, typename TSL, typename TRL, LongIndex NCC, LongIndex NOC, LongIndex NOU, LongIndex NSB, LongIndex NTC>
bool
RegistryT<ArgsT<TC, TG, TSL, TRL, NCC, NOC, NOU, NSB, NTC>>::requestImmediate(const Request request) {
	if (request.stateId == 0)
		return false;
	else if (HFSM_CHECKED(request.stateId < STATE_COUNT)) {
		Parent parent;

		for (parent = stateParents[request.stateId];
			 parent;
			 parent = forkParent(parent.forkId))
		{
			if (parent.forkId > 0) {
				compoRequested[parent.forkId - 1] = parent.prong;
				parent = forkParent(parent.forkId);

				break;
			} else if (parent.forkId < 0)
				requestedOrthoFork(parent.forkId).set(parent.prong);
			else
				HFSM_BREAK();
		}

		for (; parent; parent = forkParent(parent.forkId)) {
			if (parent.forkId > 0) {
				compoRemains.set(parent.forkId - 1);

				if (compoActive	  [parent.forkId - 1] != parent.prong)
					compoRequested[parent.forkId - 1]  = parent.prong;
				else {
					parent = forkParent(parent.forkId);
					break;
				}
			} else if (parent.forkId < 0)
				requestedOrthoFork(parent.forkId).set(parent.prong);
			else
				HFSM_BREAK();
		}

		for (; parent; parent = forkParent(parent.forkId)) {
			if (parent.forkId > 0)
				compoRemains.set(parent.forkId - 1);
			else if (parent.forkId < 0)
				requestedOrthoFork(parent.forkId).set(parent.prong);
			else
				HFSM_BREAK();
		}
	}

	return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TC, typename TG, typename TSL, typename TRL, LongIndex NCC, LongIndex NOC, LongIndex NOU, LongIndex NSB, LongIndex NTC>
void
RegistryT<ArgsT<TC, TG, TSL, TRL, NCC, NOC, NOU, NSB, NTC>>::requestScheduled(const StateID stateId) {
	if (HFSM_CHECKED(stateId < STATE_COUNT)) {
		const Parent parent = stateParents[stateId];

		HFSM_ASSERT(parent.forkId != 0);
		if (parent.forkId > 0)
			compoResumable[parent.forkId - 1] = parent.prong;
	}
}

//------------------------------------------------------------------------------

template <typename TC, typename TG, typename TSL, typename TRL, LongIndex NCC, LongIndex NOC, LongIndex NOU, LongIndex NSB, LongIndex NTC>
void
RegistryT<ArgsT<TC, TG, TSL, TRL, NCC, NOC, NOU, NSB, NTC>>::clearRequests() {
	compoRequested.clear();
	orthoRequested.clear();
	compoRemains  .clear();
}

//------------------------------------------------------------------------------

template <typename TC, typename TG, typename TSL, typename TRL, LongIndex NCC, LongIndex NOC, LongIndex NOU, LongIndex NSB, LongIndex NTC>
void
RegistryT<ArgsT<TC, TG, TSL, TRL, NCC, NOC, NOU, NSB, NTC>>::reset() {
	compoRequested.clear();
	orthoRequested.clear();
	compoActive	  .clear();
	compoResumable.clear();
	compoRemains  .clear();
}

////////////////////////////////////////////////////////////////////////////////

template <typename TC, typename TG, typename TSL, typename TRL, LongIndex NCC, LongIndex NSB, LongIndex NTC>
bool
RegistryT<ArgsT<TC, TG, TSL, TRL, NCC, 0, 0, NSB, NTC>>::isActive(const StateID stateId) const {
	if (HFSM_CHECKED(stateId < STATE_COUNT)) {
		if (Parent parent = stateParents[stateId]) {
			HFSM_ASSERT(parent.forkId > 0);

			return parent.prong == compoActive[parent.forkId - 1];
		} else
			return true;
	}

	return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TC, typename TG, typename TSL, typename TRL, LongIndex NCC, LongIndex NSB, LongIndex NTC>
bool
RegistryT<ArgsT<TC, TG, TSL, TRL, NCC, 0, 0, NSB, NTC>>::isResumable(const StateID stateId) const {
	if (HFSM_CHECKED(stateId < STATE_COUNT))
		if (Parent parent = stateParents[stateId]) {
			HFSM_ASSERT(parent.forkId > 0);

			return parent.prong == compoResumable[parent.forkId - 1];
		}

	return false;
}

//------------------------------------------------------------------------------

template <typename TC, typename TG, typename TSL, typename TRL, LongIndex NCC, LongIndex NSB, LongIndex NTC>
bool
RegistryT<ArgsT<TC, TG, TSL, TRL, NCC, 0, 0, NSB, NTC>>::isPendingChange(const StateID stateId) const {
	if (HFSM_CHECKED(stateId < STATE_COUNT))
		if (Parent parent = stateParents[stateId]) {
			HFSM_ASSERT(parent.forkId > 0);

			return compoRequested[parent.forkId - 1] !=
				   compoActive	 [parent.forkId - 1];
		}

	return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TC, typename TG, typename TSL, typename TRL, LongIndex NCC, LongIndex NSB, LongIndex NTC>
bool
RegistryT<ArgsT<TC, TG, TSL, TRL, NCC, 0, 0, NSB, NTC>>::isPendingEnter(const StateID stateId) const {
	if (HFSM_CHECKED(stateId < STATE_COUNT))
		if (Parent parent = stateParents[stateId]) {
			HFSM_ASSERT(parent.forkId > 0);

			return parent.prong != compoActive	 [parent.forkId - 1] &&
				   parent.prong == compoRequested[parent.forkId - 1];
		}

	return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TC, typename TG, typename TSL, typename TRL, LongIndex NCC, LongIndex NSB, LongIndex NTC>
bool
RegistryT<ArgsT<TC, TG, TSL, TRL, NCC, 0, 0, NSB, NTC>>::isPendingExit(const StateID stateId) const {
	if (HFSM_CHECKED(stateId < STATE_COUNT))
		if (Parent parent = stateParents[stateId]) {
			HFSM_ASSERT(parent.forkId > 0);

			return parent.prong == compoActive	 [parent.forkId - 1] &&
				   parent.prong != compoRequested[parent.forkId - 1];
		}

	return false;
}

//------------------------------------------------------------------------------

template <typename TC, typename TG, typename TSL, typename TRL, LongIndex NCC, LongIndex NSB, LongIndex NTC>
const Parent&
RegistryT<ArgsT<TC, TG, TSL, TRL, NCC, 0, 0, NSB, NTC>>::forkParent(const ForkID forkId) const {
	HFSM_ASSERT(forkId > 0);

	return compoParents[forkId - 1];
}

//------------------------------------------------------------------------------

template <typename TC, typename TG, typename TSL, typename TRL, LongIndex NCC, LongIndex NSB, LongIndex NTC>
bool
RegistryT<ArgsT<TC, TG, TSL, TRL, NCC, 0, 0, NSB, NTC>>::requestImmediate(const Request request) {
	if (request.stateId == 0)
		return false;
	else if (HFSM_CHECKED(request.stateId < STATE_COUNT)) {
		Parent parent = stateParents[request.stateId];

		if (HFSM_CHECKED(parent)) {
			HFSM_ASSERT(parent.forkId > 0);

			compoRequested[parent.forkId - 1] = parent.prong;

			for (parent = forkParent(parent.forkId);
				 parent;
				 parent = forkParent(parent.forkId))
			{
				HFSM_ASSERT(parent.forkId > 0);
				compoRemains.set(parent.forkId - 1);

				if (compoActive	  [parent.forkId - 1] != parent.prong)
					compoRequested[parent.forkId - 1]  = parent.prong;
				else {
					parent = forkParent(parent.forkId);
					break;
				}
			}

			for (; parent; parent = forkParent(parent.forkId)) {
				HFSM_ASSERT(parent.forkId > 0);
				compoRemains.set(parent.forkId - 1);
			}
		}
	}

	return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TC, typename TG, typename TSL, typename TRL, LongIndex NCC, LongIndex NSB, LongIndex NTC>
void
RegistryT<ArgsT<TC, TG, TSL, TRL, NCC, 0, 0, NSB, NTC>>::requestScheduled(const StateID stateId) {
	if (HFSM_CHECKED(stateId < STATE_COUNT)) {
		const Parent parent = stateParents[stateId];

		HFSM_ASSERT(parent.forkId > 0);
		compoResumable[parent.forkId - 1] = parent.prong;
	}
}

//------------------------------------------------------------------------------

template <typename TC, typename TG, typename TSL, typename TRL, LongIndex NCC, LongIndex NSB, LongIndex NTC>
void
RegistryT<ArgsT<TC, TG, TSL, TRL, NCC, 0, 0, NSB, NTC>>::clearRequests() {
	compoRequested.clear();
	orthoRequested.clear();
	compoRemains  .clear();
}

//------------------------------------------------------------------------------

template <typename TC, typename TG, typename TSL, typename TRL, LongIndex NCC, LongIndex NSB, LongIndex NTC>
void
RegistryT<ArgsT<TC, TG, TSL, TRL, NCC, 0, 0, NSB, NTC>>::reset() {
	compoRequested.clear();
	orthoRequested.clear();
	compoActive	  .clear();
	compoResumable.clear();
	compoRemains  .clear();
}

////////////////////////////////////////////////////////////////////////////////

}
}

namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

template <typename TArgs>
class ControlT {
	template <typename, typename, typename>
	friend struct S_;

	template <typename, typename, Strategy, typename, typename...>
	friend struct C_;

	template <typename, typename, typename, typename...>
	friend struct O_;

	template <typename, typename>
	friend class R_;

	using Args			= TArgs;

protected:
	using Context		= typename Args::Context;
	using RNG			= typename Args::RNG;
	using Logger		= typename Args::Logger;
	using StateList		= typename Args::StateList;
	using RegionList	= typename Args::RegionList;

	using Registry		= RegistryT<Args>;
	using PlanData		= PlanDataT<Args>;
	using ConstPlan		= ConstPlanT<Args>;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct Region {
		HFSM_INLINE Region(ControlT& control,
						   const RegionID id);

		HFSM_INLINE ~Region();

		ControlT& control;
		const RegionID prevId;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE ControlT(Context& context,
						 RNG& rng,
						 Registry& registry,
						 PlanData& planData,
						 Logger* const HFSM_IF_LOGGER(logger))
		: _context{context}
		, _rng{rng}
		, _registry{registry}
		, _planData{planData}
		HFSM_IF_LOGGER(, _logger{logger})
	{}


	HFSM_INLINE void setRegion(const RegionID id);
	HFSM_INLINE void resetRegion(const RegionID id);

public:
	template <typename T>
	static constexpr StateID  stateId()					{ return			StateList ::template index<T>();	}

	template <typename T>
	static constexpr RegionID regionId()				{ return (RegionID) RegionList::template index<T>();	}

	HFSM_INLINE Context& _()								{ return _context;									}
	HFSM_INLINE Context& context()							{ return _context;									}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE bool isActive   (const StateID id) const	{ return _registry.isActive   (id);					}
	HFSM_INLINE bool isResumable(const StateID id) const	{ return _registry.isResumable(id);					}

	HFSM_INLINE bool isScheduled(const StateID id) const	{ return isResumable(id);							}

	template <typename TState>
	HFSM_INLINE bool isActive() const						{ return isActive	(stateId<TState>());			}

	template <typename TState>
	HFSM_INLINE bool isResumable() const					{ return isResumable(stateId<TState>());			}

	template <typename TState>
	HFSM_INLINE bool isScheduled() const					{ return isResumable(stateId<TState>());			}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE ConstPlan plan() const						{ return ConstPlan{_planData, _regionId};			}
	HFSM_INLINE ConstPlan plan(const RegionID id) const		{ return ConstPlan{_planData, id};					}

	template <typename TRegion>
	HFSM_INLINE ConstPlan plan()							{ return ConstPlan{_planData, regionId<TRegion>()};	}

	template <typename TRegion>
	HFSM_INLINE ConstPlan plan() const						{ return ConstPlan{_planData, regionId<TRegion>()};	}

protected:
#if defined HFSM_ENABLE_LOG_INTERFACE || defined HFSM_ENABLE_VERBOSE_DEBUG_LOG
	HFSM_INLINE Logger* logger()							{ return _logger;									}
#endif

protected:
	Context& _context;
	RNG& _rng;
	Registry& _registry;
	PlanData& _planData;
	RegionID _regionId = 0;
	HFSM_IF_LOGGER(Logger* _logger);
};

//------------------------------------------------------------------------------

template <typename TArgs>
class PlanControlT
	: public ControlT<TArgs>
{
	template <typename, typename, typename>
	friend struct S_;

	template <typename, typename, Strategy, typename, typename...>
	friend struct C_;

	template <typename, typename, typename, typename...>
	friend struct O_;

	template <typename, typename>
	friend class R_;

	using Args			= TArgs;

protected:
	using Control		= ControlT<Args>;

	using typename Control::StateList;
	using typename Control::RegionList;
	using typename Control::PlanData;
	using typename Control::ConstPlan;

	using Plan			= PlanT<Args>;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct Origin {
		HFSM_INLINE Origin(PlanControlT& control_,
						   const StateID id);

		HFSM_INLINE ~Origin();

		PlanControlT& control;
		const StateID prevId;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct Region {
		HFSM_INLINE Region(PlanControlT& control,
						   const RegionID id,
						   const StateID index,
						   const LongIndex size);

		HFSM_INLINE ~Region();

		PlanControlT& control;
		const RegionID prevId;
		const LongIndex prevIndex;
		const LongIndex prevSize;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	using Control::Control;

	HFSM_INLINE void setOrigin  (const StateID id);
	HFSM_INLINE void resetOrigin(const StateID id);

	HFSM_INLINE void setRegion  (const RegionID id, const StateID index, const LongIndex size);
	HFSM_INLINE void resetRegion(const RegionID id, const StateID index, const LongIndex size);

public:
	using Control::stateId;
	using Control::regionId;

	using Control::_;
	using Control::context;

	using Control::isActive;
	using Control::isResumable;
	using Control::isScheduled;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE		 Plan plan()							{ return	  Plan{_planData, _regionId};			}
	HFSM_INLINE ConstPlan plan() const						{ return ConstPlan{_planData, _regionId};			}

	HFSM_INLINE		 Plan plan(const RegionID id)			{ return	  Plan{_planData, id};					}
	HFSM_INLINE ConstPlan plan(const RegionID id) const		{ return ConstPlan{_planData, id};					}

	template <typename TRegion>
	HFSM_INLINE		 Plan plan()		{ return	  Plan{_planData, Control::template regionId<TRegion>()};	}

	template <typename TRegion>
	HFSM_INLINE ConstPlan plan() const	{ return ConstPlan{_planData, Control::template regionId<TRegion>()};	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

protected:
	using Control::_planData;
	using Control::_regionId;
	HFSM_IF_LOGGER(using Control::_logger);

	StateID _originId = 0;
	StateID _regionIndex = 0;
	LongIndex _regionSize = StateList::SIZE;
	Status _status;
};

//------------------------------------------------------------------------------

template <typename TArgs>
class FullControlT
	: public PlanControlT<TArgs>
{
	template <typename, typename, typename>
	friend struct S_;

	template <typename, typename, Strategy, typename, typename...>
	friend struct C_;

	template <typename, typename, typename, typename...>
	friend struct O_;

	template <typename, typename>
	friend class R_;

	using Args			= TArgs;

protected:
	using PlanControl	= PlanControlT<Args>;

	using typename PlanControl::Context;
	using typename PlanControl::RNG;
	using typename PlanControl::Logger;
	using typename PlanControl::StateList;
	using typename PlanControl::RegionList;
	using typename PlanControl::PlanData;

	using typename PlanControl::Plan;
	using typename PlanControl::Origin;

	using Registry		= RegistryT<Args>;
	using Requests		= RequestsT<Args::COMPO_REGIONS>;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct Lock {
		HFSM_INLINE Lock(FullControlT& control_);
		HFSM_INLINE ~Lock();

		FullControlT* const control;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE FullControlT(Context& context,
							 RNG& rng,
							 Registry& registry,
							 PlanData& planData,
							 Requests& requests,
							 Logger* const logger)
		: PlanControl{context, rng, registry, planData, logger}
		, _requests{requests}
	{}

	template <typename TState>
	Status updatePlan(TState& headState, const Status subStatus);

	template <typename TState>
	Status buildPlanStatus();

public:
	using PlanControl::stateId;
	using PlanControl::regionId;

	using PlanControl::_;
	using PlanControl::context;

	using PlanControl::isActive;
	using PlanControl::isResumable;
	using PlanControl::isScheduled;

	using PlanControl::plan;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void changeTo (const StateID id);
	HFSM_INLINE void restart  (const StateID id);
	HFSM_INLINE void resume	  (const StateID id);
	HFSM_INLINE void utilize  (const StateID id);
	HFSM_INLINE void randomize(const StateID id);
	HFSM_INLINE void schedule (const StateID id);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Clang trips over 'stateId<>()', so give it a hint it comes from ControlT<>

	template <typename T>
	HFSM_INLINE void changeTo ()							{ changeTo (PlanControl::template stateId<T>());	}

	template <typename T>
	HFSM_INLINE void restart  ()							{ restart  (PlanControl::template stateId<T>());	}

	template <typename T>
	HFSM_INLINE void resume   ()							{ resume   (PlanControl::template stateId<T>());	}

	template <typename T>
	HFSM_INLINE void utilize  ()							{ utilize  (PlanControl::template stateId<T>());	}

	template <typename T>
	HFSM_INLINE void randomize()							{ randomize(PlanControl::template stateId<T>());	}

	template <typename T>
	HFSM_INLINE void schedule ()							{ schedule (PlanControl::template stateId<T>());	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void succeed();
	HFSM_INLINE void fail();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

protected:
	using PlanControl::_planData;
	using PlanControl::_regionId;
	HFSM_IF_LOGGER(using PlanControl::_logger);

	using PlanControl::_originId;
	using PlanControl::_regionIndex;
	using PlanControl::_regionSize;
	using PlanControl::_status;

	Requests& _requests;
	bool _locked = false;
};

//------------------------------------------------------------------------------

template <typename TArgs>
class GuardControlT final
	: public FullControlT<TArgs>
{
	template <typename, typename, typename>
	friend struct S_;

	template <typename, typename>
	friend class R_;

	using Args			= TArgs;
	using FullControl	= FullControlT<Args>;

	using typename FullControl::Context;
	using typename FullControl::RNG;
	using typename FullControl::Logger;
	using typename FullControl::StateList;
	using typename FullControl::RegionList;
	using typename FullControl::PlanData;

	using typename FullControl::Registry;

protected:
	using Requests		= RequestsT<Args::COMPO_REGIONS>;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:
	HFSM_INLINE GuardControlT(Context& context,
							  RNG& rng,
							  Registry& registry,
							  PlanData& planData,
							  Requests& requests,
							  const Requests& pendingChanges,
							  Logger* const logger)
		: FullControl{context, rng, registry, planData, requests, logger}
		, _pending{pendingChanges}
	{}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:
	using FullControl::stateId;
	using FullControl::regionId;

	using FullControl::_;
	using FullControl::context;

	using FullControl::isActive;
	using FullControl::isResumable;
	using FullControl::isScheduled;

	using FullControl::plan;

	using FullControl::changeTo;
	using FullControl::restart;
	using FullControl::resume;
	using FullControl::utilize;
	using FullControl::randomize;
	using FullControl::schedule;
	using FullControl::succeed;
	using FullControl::fail;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE bool isPendingChange(const StateID id) const	{ return _registry.isPendingChange(id);	}
	HFSM_INLINE bool isPendingEnter	(const StateID id) const	{ return _registry.isPendingEnter (id);	}
	HFSM_INLINE bool isPendingExit	(const StateID id) const	{ return _registry.isPendingExit  (id);	}

	template <typename TState>
	HFSM_INLINE bool isPendingChange()		{ return isPendingChange(FullControl::template stateId<TState>());	}

	template <typename TState>
	HFSM_INLINE bool isPendingEnter()		{ return isPendingEnter (FullControl::template stateId<TState>());	}

	template <typename TState>
	HFSM_INLINE bool isPendingExit()		{ return isPendingExit  (FullControl::template stateId<TState>());	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void cancelPendingTransitions();

	HFSM_INLINE const Requests& pendingTransitions() const		{ return _pending;								}

private:
	HFSM_IF_LOGGER(using FullControl::_logger);

	using FullControl::_registry;
	using FullControl::_originId;

	bool _cancelled = false;
	const Requests& _pending;
};

////////////////////////////////////////////////////////////////////////////////

}
}

namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

template <typename TArgs>
ControlT<TArgs>::Region::Region(ControlT& control_,
								const RegionID id)
	: control{control_}
	, prevId{control._regionId}
{
	control.setRegion(id);
}

//------------------------------------------------------------------------------

template <typename TArgs>
ControlT<TArgs>::Region::~Region() {
	control.resetRegion(prevId);
}

////////////////////////////////////////////////////////////////////////////////

template <typename TArgs>
void
ControlT<TArgs>::setRegion(const RegionID id) {
	HFSM_ASSERT(_regionId <= id && id < RegionList::SIZE);

	_regionId = id;
}

//------------------------------------------------------------------------------

template <typename TArgs>
void
ControlT<TArgs>::resetRegion(const RegionID id) { //-V524
	HFSM_ASSERT(id <= _regionId && _regionId < RegionList::SIZE);

	_regionId = id;
}

////////////////////////////////////////////////////////////////////////////////

template <typename TArgs>
PlanControlT<TArgs>::Origin::Origin(PlanControlT& control_,
									const StateID id)
	: control{control_}
	, prevId{control._originId}
{
	control.setOrigin(id);
}

//------------------------------------------------------------------------------

template <typename TArgs>
PlanControlT<TArgs>::Origin::~Origin() {
	control.resetOrigin(prevId);
}

////////////////////////////////////////////////////////////////////////////////

template <typename TArgs>
PlanControlT<TArgs>::Region::Region(PlanControlT& control_,
									const RegionID id,
									const StateID index,
									const LongIndex size)
	: control{control_}
	, prevId{control._regionId}
	, prevIndex{control._regionIndex}
	, prevSize{control._regionSize}
{
	control.setRegion(id, index, size);
}

//------------------------------------------------------------------------------

template <typename TArgs>
PlanControlT<TArgs>::Region::~Region() {
	control.resetRegion(prevId, prevIndex, prevSize);

	control._status.clear();
}

////////////////////////////////////////////////////////////////////////////////

template <typename TArgs>
void
PlanControlT<TArgs>::setOrigin(const StateID id) {
	HFSM_ASSERT(_regionId + _regionSize <= StateList::SIZE);
	HFSM_ASSERT(_originId <= id && id < StateList::SIZE);

	_originId = id;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TArgs>
void
PlanControlT<TArgs>::resetOrigin(const StateID id) { //-V524
	HFSM_ASSERT(_regionId + _regionSize <= StateList::SIZE);
	HFSM_ASSERT(id <= _originId && _originId < StateList::SIZE);

	_originId = id;
}

//------------------------------------------------------------------------------

template <typename TArgs>
void
PlanControlT<TArgs>::setRegion(const RegionID id,
							   const StateID index,
							   const LongIndex size)
{
	HFSM_ASSERT(_regionId <= id && id <  RegionList::SIZE);
	HFSM_ASSERT(_regionIndex <= index && index + size <= _regionIndex + _regionSize);

	_regionId	 = id;
	_regionIndex = index;
	_regionSize	 = size;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TArgs>
void
PlanControlT<TArgs>::resetRegion(const RegionID id, //-V524
								 const StateID index,
								 const LongIndex size)
{
	HFSM_ASSERT(id <= _regionId && _regionId < RegionList::SIZE);
	HFSM_ASSERT(index <= _regionIndex && _regionIndex + _regionSize <= index + size);

	_regionId	 = id;
	_regionIndex = index;
	_regionSize	 = size;
}

////////////////////////////////////////////////////////////////////////////////

template <typename TArgs>
FullControlT<TArgs>::Lock::Lock(FullControlT& control_)
	: control(!control_._locked ? &control_ : nullptr)
{
	if (control)
		control->_locked = true;
}

//------------------------------------------------------------------------------

template <typename TArgs>
FullControlT<TArgs>::Lock::~Lock() {
	if (control)
		control->_locked = false;
}

////////////////////////////////////////////////////////////////////////////////

template <typename TArgs>
template <typename TState>
Status
FullControlT<TArgs>::updatePlan(TState& headState,
								const Status subStatus)
{
	using State = TState;
	static constexpr StateID STATE_ID = State::STATE_ID;

	HFSM_ASSERT(subStatus);

	if (subStatus.result == Status::FAILURE) {
		_status.result = Status::FAILURE;
		headState.wrapPlanFailed(*this);

		if (Plan p = plan(_regionId))
			p.clear();

		return buildPlanStatus<State>();
	} else if (subStatus.result == Status::SUCCESS) {
		if (Plan p = plan(_regionId)) {
			for (auto it = p.first(); it; ++it) {
				if (isActive(it->origin) &&
					_planData.tasksSuccesses.get(it->origin))
				{
					Origin origin{*this, STATE_ID};

					changeTo(it->destination);

					it.remove();
				} else
					break;
			}

			return Status{};
		} else {
			_status.result = Status::SUCCESS;
			headState.wrapPlanSucceeded(*this);

			return buildPlanStatus<State>();
		}
	} else
		return Status{};
}

//------------------------------------------------------------------------------

template <typename TArgs>
template <typename TState>
Status
FullControlT<TArgs>::buildPlanStatus() {
	using State = TState;
	static constexpr StateID STATE_ID = State::STATE_ID;

	switch (_status.result) {
	case Status::NONE:
		HFSM_BREAK();
		break;

	case Status::SUCCESS:
		_planData.tasksSuccesses.template set<STATE_ID>();

		HFSM_LOG_PLAN_STATUS(context(), _regionId, StatusEvent::SUCCEEDED);
		break;

	case Status::FAILURE:
		_planData.tasksFailures.template set<STATE_ID>();

		HFSM_LOG_PLAN_STATUS(context(), _regionId, StatusEvent::FAILED);
		break;

	default:
		HFSM_BREAK();
	}

	return {_status.result};
}

//------------------------------------------------------------------------------

template <typename TArgs>
void
FullControlT<TArgs>::changeTo(const StateID stateId) {
	if (!_locked) {
		_requests.append(Request{Request::Type::CHANGE, stateId});

		if (_regionIndex + _regionSize <= stateId || stateId < _regionIndex)
			_status.outerTransition = true;

		HFSM_LOG_TRANSITION(context(), _originId, TransitionType::CHANGE, stateId);
	}
}

//------------------------------------------------------------------------------

template <typename TArgs>
void
FullControlT<TArgs>::restart(const StateID stateId) {
	if (!_locked) {
		_requests.append(Request{Request::Type::RESTART, stateId});

		if (_regionIndex + _regionSize <= stateId || stateId < _regionIndex)
			_status.outerTransition = true;

		HFSM_LOG_TRANSITION(context(), _originId, TransitionType::RESTART, stateId);
	}
}

//------------------------------------------------------------------------------

template <typename TArgs>
void
FullControlT<TArgs>::resume(const StateID stateId) {
	if (!_locked) {
		_requests.append(Request{Request::Type::RESUME, stateId});

		if (_regionIndex + _regionSize <= stateId || stateId < _regionIndex)
			_status.outerTransition = true;

		HFSM_LOG_TRANSITION(context(), _originId, TransitionType::RESUME, stateId);
	}
}

//------------------------------------------------------------------------------

template <typename TArgs>
void
FullControlT<TArgs>::utilize(const StateID stateId) {
	if (!_locked) {
		_requests.append(Request{Request::Type::UTILIZE, stateId});

		if (_regionIndex + _regionSize <= stateId || stateId < _regionIndex)
			_status.outerTransition = true;

		HFSM_LOG_TRANSITION(context(), _originId, TransitionType::UTILIZE, stateId);
	}
}

//------------------------------------------------------------------------------

template <typename TArgs>
void
FullControlT<TArgs>::randomize(const StateID stateId) {
	if (!_locked) {
		_requests.append(Request{Request::Type::RANDOMIZE, stateId});

		if (_regionIndex + _regionSize <= stateId || stateId < _regionIndex)
			_status.outerTransition = true;

		HFSM_LOG_TRANSITION(context(), _originId, TransitionType::RANDOMIZE, stateId);
	}
}

//------------------------------------------------------------------------------

template <typename TArgs>
void
FullControlT<TArgs>::schedule(const StateID stateId) {
	_requests.append(Request{Request::Type::SCHEDULE, stateId});

	HFSM_LOG_TRANSITION(context(), _originId, TransitionType::SCHEDULE, stateId);
}

//------------------------------------------------------------------------------

template <typename TArgs>
void
FullControlT<TArgs>::succeed() {
	_status.result = Status::SUCCESS;

	_planData.tasksSuccesses.set(_originId);

	// TODO: promote taskSuccess all the way up for all regions without plans
	if (_regionId < RegionList::SIZE && !_planData.planExists.get(_regionId)) {
		HFSM_ASSERT(_regionIndex < StateList::SIZE);

		_planData.tasksSuccesses.set(_regionIndex);
	}

	HFSM_LOG_TASK_STATUS(context(), _regionId, _originId, StatusEvent::SUCCEEDED);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TArgs>
void
FullControlT<TArgs>::fail() {
	_status.result = Status::FAILURE;

	_planData.tasksFailures.set(_originId);

	// TODO: promote taskFailure all the way up for all regions without plans
	if (_regionId < RegionList::SIZE && !_planData.planExists.get(_regionId)) {
		HFSM_ASSERT(_regionIndex < StateList::SIZE);

		_planData.tasksFailures.set(_regionIndex);
	}

	HFSM_LOG_TASK_STATUS(context(), _regionId, _originId, StatusEvent::FAILED);
}

////////////////////////////////////////////////////////////////////////////////

template <typename TArgs>
void
GuardControlT<TArgs>::cancelPendingTransitions() {
	_cancelled = true;

	HFSM_LOG_CANCELLED_PENDING(context(), _originId);
}

////////////////////////////////////////////////////////////////////////////////

}
}

namespace hfsm2 {

//------------------------------------------------------------------------------

#ifdef HFSM_ENABLE_STRUCTURE_REPORT

struct StructureEntry {
	bool isActive;
	const wchar_t* prefix;
	const char* name;
};

#endif

//------------------------------------------------------------------------------

namespace detail {

////////////////////////////////////////////////////////////////////////////////

#ifdef HFSM_ENABLE_STRUCTURE_REPORT

#pragma pack(push, 1)

struct alignas(alignof(void*)) StructureStateInfo {
	enum RegionType : ShortIndex {
		COMPOSITE,
		ORTHOGONAL,
	};

	StructureStateInfo() = default;

	HFSM_INLINE StructureStateInfo(const LongIndex parent_,
								   const RegionType region_,
								   const ShortIndex depth_,
								   const char* const name_)
		: name{name_}
		, parent{parent_}
		, region{region_}
		, depth{depth_}
	{}

	const char* name;
	LongIndex parent;
	RegionType region;
	ShortIndex depth;
};

#pragma pack(pop)

#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef HFSM_ENABLE_TRANSITION_HISTORY

TransitionType
HFSM_INLINE convert(const Request::Type type) {
	switch (type) {
		case Request::CHANGE:
			return TransitionType::CHANGE;

		case Request::RESTART:
			return TransitionType::RESTART;

		case Request::RESUME:
			return TransitionType::RESUME;

		case Request::UTILIZE:
			return TransitionType::UTILIZE;

		case Request::RANDOMIZE:
			return TransitionType::RANDOMIZE;

		case Request::SCHEDULE:
			return TransitionType::SCHEDULE;

		default:
			HFSM_BREAK();
			return TransitionType::CHANGE;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Request::Type
HFSM_INLINE convert(const TransitionType type) {
	switch (type) {
	case TransitionType::CHANGE:
		return Request::CHANGE;

	case TransitionType::RESTART:
		return Request::RESTART;

	case TransitionType::RESUME:
		return Request::RESUME;

	case TransitionType::UTILIZE:
		return Request::UTILIZE;

	case TransitionType::RANDOMIZE:
		return Request::RANDOMIZE;

	case TransitionType::SCHEDULE:
		return Request::SCHEDULE;

	default:
		HFSM_BREAK();
		return Request::CHANGE;
	}
}

#endif

}

//------------------------------------------------------------------------------

#ifdef HFSM_ENABLE_TRANSITION_HISTORY

#pragma pack(push, 1)

struct alignas(4) Transition {
	HFSM_INLINE Transition() = default;

	HFSM_INLINE Transition(const detail::Request request,
						   const Method method_)
		: stateId{request.stateId}
		, method{method_}
		, transitionType{detail::convert(request.type)}
	{
		HFSM_ASSERT(method_ < Method::COUNT);
	}

	HFSM_INLINE Transition(const StateID stateId_,
						   const Method method_,
						   const TransitionType transitionType_)
		: stateId{stateId_}
		, method{method_}
		, transitionType{transitionType_}
	{
		HFSM_ASSERT(method_ < Method::COUNT);
	}

	detail::Request request() const		{ return detail::Request{detail::convert(transitionType), stateId};	}

	StateID stateId = INVALID_STATE_ID;
	Method method;
	TransitionType transitionType;
};

#pragma pack(pop)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

bool operator == (const Transition& l, const Transition& r) {
	return l.stateId		== r.stateId
		&& l.method			== r.method
		&& l.transitionType	== r.transitionType;
}

#endif

////////////////////////////////////////////////////////////////////////////////

}


namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

template <typename TArgs>
class InjectionT {
	template <typename...>
	friend struct B_;

protected:
	using Context		= typename TArgs::Context;
	using Rank			= typename TArgs::Rank;
	using Utility		= typename TArgs::Utility;
	using StateList		= typename TArgs::StateList;
	using RegionList	= typename TArgs::RegionList;

	using Control		= ControlT<TArgs>;

	using PlanControl	= PlanControlT<TArgs>;
	using Plan			= PlanT<TArgs>;

	using FullControl	= FullControlT <TArgs>;
	using GuardControl	= GuardControlT<TArgs>;

public:
	HFSM_INLINE void preEntryGuard(Context&)		{}

	HFSM_INLINE void preEnter	  (Context&)		{}
	HFSM_INLINE void preReenter	  (Context&)		{}

	HFSM_INLINE void preUpdate	  (Context&)		{}

	template <typename TEvent>
	HFSM_INLINE void preReact	  (const TEvent&,
								   Context&)		{}

	HFSM_INLINE void preExitGuard (Context&)		{}

	HFSM_INLINE void postExit	  (Context&)		{}

	template <typename T>
	static constexpr StateID  stateId()		{ return			StateList ::template index<T>();	}

	template <typename T>
	static constexpr RegionID regionId()	{ return (RegionID) RegionList::template index<T>();	}
};

//------------------------------------------------------------------------------

template <typename...>
struct B_;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TFirst, typename... TRest>
struct B_<TFirst, TRest...>
	: TFirst
	, B_<TRest...>
{
	using typename TFirst::Context;
	using typename TFirst::Rank;
	using typename TFirst::Utility;
	using typename TFirst::StateList;
	using typename TFirst::RegionList;

	using typename TFirst::Control;
	using typename TFirst::PlanControl;
	using typename TFirst::Plan;
	using typename TFirst::FullControl;
	using typename TFirst::GuardControl;

	using TFirst::stateId;
	using TFirst::regionId;

	HFSM_INLINE void widePreEntryGuard(Context& context);

	HFSM_INLINE void widePreEnter	  (Context& context);
	HFSM_INLINE void widePreReenter	  (Context& context);

	HFSM_INLINE void widePreUpdate	  (Context& context);

	template <typename TEvent>
	HFSM_INLINE void widePreReact	  (const TEvent& event,
									   Context& context);

	HFSM_INLINE void widePreExitGuard (Context& context);

	HFSM_INLINE void widePostExit	  (Context& context);
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TFirst>
struct B_<TFirst>
	: TFirst
{
	using typename TFirst::Context;
	using typename TFirst::Rank;
	using typename TFirst::Utility;
	using typename TFirst::StateList;
	using typename TFirst::RegionList;

	using typename TFirst::Control;
	using typename TFirst::PlanControl;
	using typename TFirst::Plan;
	using typename TFirst::FullControl;
	using typename TFirst::GuardControl;

	using TFirst::stateId;
	using TFirst::regionId;

	HFSM_INLINE Rank	rank			 (const Control&)			{ return Rank	   {0};	}

	HFSM_INLINE Utility utility			 (const Control&)			{ return Utility{1.0f};	}

	HFSM_INLINE void	entryGuard		 (GuardControl&)			{}

	HFSM_INLINE void	enter			 (PlanControl&)				{}
	HFSM_INLINE void	reenter			 (PlanControl&)				{}

	HFSM_INLINE void	update			 (FullControl&)				{}

	template <typename TEvent>
	HFSM_INLINE void	react			 (const TEvent&,
										  FullControl&)				{}

	HFSM_INLINE void	exitGuard		 (GuardControl&)			{}

	HFSM_INLINE void	exit			 (PlanControl&)				{}

	HFSM_INLINE void	planSucceeded	 (FullControl& control)		{ control.succeed();	}
	HFSM_INLINE void	planFailed		 (FullControl& control)		{ control.fail();		}

	HFSM_INLINE void	widePreEntryGuard(Context& context);

	HFSM_INLINE void	widePreEnter	 (Context& context);
	HFSM_INLINE void	widePreReenter   (Context& context);

	HFSM_INLINE void	widePreUpdate	 (Context& context);

	template <typename TEvent>
	HFSM_INLINE void	widePreReact	 (const TEvent& event,
					 					  Context& context);

	HFSM_INLINE void	widePreExitGuard (Context& context);

	HFSM_INLINE void	widePostExit	 (Context& context);
};

//------------------------------------------------------------------------------

template <typename TArgs>
using EmptyT = B_<InjectionT<TArgs>>;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct Dynamic_ {};

template <typename... TI>
struct DB_
	: Dynamic_
	, B_<TI...>
{};

template <typename TArgs>
using DynamicEmptyT = DB_<InjectionT<TArgs>>;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct Static_ {};

template <typename... TI>
struct SB_
	: Static_
	, B_<TI...>
{};

template <typename TArgs>
using StaticEmptyT = SB_<InjectionT<TArgs>>;

////////////////////////////////////////////////////////////////////////////////

}
}

namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

template <typename TF, typename... TR>
void
B_<TF, TR...>::widePreEntryGuard(Context& context) {
	TF::preEntryGuard(context);
	B_<TR...>::widePreEntryGuard(context);
}

//------------------------------------------------------------------------------

template <typename TF, typename... TR>
void
B_<TF, TR...>::widePreEnter(Context& context) {
	TF::preEnter(context);
	B_<TR...>::widePreEnter(context);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TF, typename... TR>
void
B_<TF, TR...>::widePreReenter(Context& context) {
	TF::preReenter(context);
	B_<TR...>::widePreReenter(context);
}

//------------------------------------------------------------------------------

template <typename TF, typename... TR>
void
B_<TF, TR...>::widePreUpdate(Context& context) {
	TF::preUpdate(context);
	B_<TR...>::widePreUpdate(context);
}

//------------------------------------------------------------------------------

template <typename TF, typename... TR>
template <typename TEvent>
void
B_<TF, TR...>::widePreReact(const TEvent& event,
							Context& context)
{
	TF::preReact(event, context);
	B_<TR...>::widePreReact(event, context);
}

//------------------------------------------------------------------------------

template <typename TF, typename... TR>
void
B_<TF, TR...>::widePreExitGuard(Context& context) {
	TF::preExitGuard(context);
	B_<TR...>::widePreExitGuard(context);
}

//------------------------------------------------------------------------------

template <typename TF, typename... TR>
void
B_<TF, TR...>::widePostExit(Context& context) {
	TF::postExit(context);
	B_<TR...>::widePostExit(context);
}

////////////////////////////////////////////////////////////////////////////////

template <typename TF>
void
B_<TF>::widePreEntryGuard(Context& context) {
	TF::preEntryGuard(context);
}

//------------------------------------------------------------------------------

template <typename TF>
void
B_<TF>::widePreEnter(Context& context) {
	TF::preEnter(context);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TF>
void
B_<TF>::widePreReenter(Context& context) {
	TF::preReenter(context);
}

//------------------------------------------------------------------------------

template <typename TF>
void
B_<TF>::widePreUpdate(Context& context) {
	TF::preUpdate(context);
}

//------------------------------------------------------------------------------

template <typename TF>
template <typename TEvent>
void
B_<TF>::widePreReact(const TEvent& event,
					 Context& context)
{
	TF::preReact(event, context);
}

//------------------------------------------------------------------------------

template <typename TF>
void
B_<TF>::widePreExitGuard(Context& context) {
	TF::preExitGuard(context);
}

//------------------------------------------------------------------------------

template <typename TF>
void
B_<TF>::widePostExit(Context& context) {
	TF::postExit(context);
}

////////////////////////////////////////////////////////////////////////////////

}
}

namespace hfsm2 {

//------------------------------------------------------------------------------

template <typename>
struct Guard {
	template <typename TArgs>
	static void execute(hfsm2::detail::GuardControlT<TArgs>&) {}
};

//------------------------------------------------------------------------------

namespace detail {

//------------------------------------------------------------------------------

HFSM_IF_DEBUG(struct None {});

////////////////////////////////////////////////////////////////////////////////

template <typename T, typename TArgs>
struct DBox final {
	static constexpr bool isBare()								{ return false;					}

	union {
		T t_;
	};

	HFSM_INLINE  DBox() {}
	HFSM_INLINE ~DBox() {}

	HFSM_INLINE void guard(GuardControlT<TArgs>& control)		{ Guard<T>::execute(control);	}

	HFSM_INLINE void construct();
	HFSM_INLINE void destruct();

	HFSM_INLINE		  T& get()							{ HFSM_ASSERT(initialized_); return t_;	}
	HFSM_INLINE const T& get() const					{ HFSM_ASSERT(initialized_); return t_;	}

	HFSM_IF_ASSERT(bool initialized_ = false);

	HFSM_IF_DEBUG(const std::type_index TYPE = typeid(T));
};

////////////////////////////////////////////////////////////////////////////////

template <typename T, typename TArgs>
struct SBox final {
	static constexpr bool isBare()	{ return std::is_base_of<T, StaticEmptyT<TArgs>>::value;	}

	T t_;

	HFSM_INLINE void guard(GuardControlT<TArgs>& control);

	HFSM_INLINE void construct()																{}
	HFSM_INLINE void destruct()																	{}

	HFSM_INLINE		  T& get()									{ return t_;					}
	HFSM_INLINE const T& get() const							{ return t_;					}

	HFSM_IF_DEBUG(const std::type_index TYPE = isBare() ? typeid(None) : typeid(T));
};

////////////////////////////////////////////////////////////////////////////////

template <typename T, typename TArgs>
struct BoxifyT final {
	using Type = typename std::conditional<
					 std::is_base_of<Dynamic_, T>::value,
					 DBox<T, TArgs>,
					 SBox<T, TArgs>
				 >::type;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename T, typename TArgs>
using Boxify = typename BoxifyT<T, TArgs>::Type;

//------------------------------------------------------------------------------

}
}

namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

template <typename T, typename TA>
void
DBox<T, TA>::construct() {
	HFSM_ASSERT(!initialized_);

	new(&t_) T{};

	HFSM_IF_ASSERT(initialized_ = true);
}

//------------------------------------------------------------------------------

template <typename T, typename TA>
void
DBox<T, TA>::destruct() {
	HFSM_ASSERT(initialized_);

	t_.~T();

	HFSM_IF_ASSERT(initialized_ = false);
}

////////////////////////////////////////////////////////////////////////////////

template <typename T, typename TA>
void
SBox<T, TA>::guard(GuardControlT<TA>& control) {
	t_.widePreEntryGuard(control.context());
	t_.		  entryGuard(control);
}

////////////////////////////////////////////////////////////////////////////////

}
}
namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

template <typename TIndices,
		  typename TArgs,
		  typename THead>
struct S_ final {
	static constexpr auto STATE_ID	 = TIndices::STATE_ID;

	using Context		= typename TArgs::Context;
	using Rank			= typename TArgs::Rank;
	using Utility		= typename TArgs::Utility;
	using UP			= typename TArgs::UP;
	using Logger		= typename TArgs::Logger;

	using Control		= ControlT<TArgs>;
	using Registry		= RegistryT<TArgs>;
	using StateParents	= typename Registry::StateParents;

	using PlanControl	= PlanControlT<TArgs>;
	using ScopedOrigin	= typename PlanControl::Origin;

	using FullControl	= FullControlT <TArgs>;
	using GuardControl	= GuardControlT<TArgs>;

	using Head			= THead;
	using HeadBox		= Boxify<Head, TArgs>;

	//----------------------------------------------------------------------

#ifdef HFSM_EXPLICIT_MEMBER_SPECIALIZATION
#ifdef __clang__
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wnull-dereference"
#endif

	template <typename T>
	struct Accessor {
		HFSM_INLINE static		 T&	   get(		 S_&  )			{ HFSM_BREAK(); return *reinterpret_cast<T*>(0);		}
		HFSM_INLINE static const T&	   get(const S_&  )			{ HFSM_BREAK(); return *reinterpret_cast<T*>(0);		}
	};

#ifdef __clang__
	#pragma clang diagnostic pop
#endif

	template <>
	struct Accessor<Head> {
		HFSM_INLINE static		 Head& get(		 S_& s)			{ return s._headBox.get();								}
		HFSM_INLINE static const Head& get(const S_& s)			{ return s._headBox.get();								}
	};

	template <typename T>
	HFSM_INLINE		  T& access()								{ return Accessor<T>::get(*this);						}

	template <typename T>
	HFSM_INLINE const T& access() const							{ return Accessor<T>::get(*this);						}
#endif

	//----------------------------------------------------------------------

	HFSM_INLINE Parent	stateParent			 (Control& control)	{ return control._registry.stateParents[STATE_ID];		}

	HFSM_INLINE void	deepRegister		 (Registry& registry, const Parent parent);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE bool	deepForwardEntryGuard(GuardControl&)					{ return false;	}
	HFSM_INLINE bool	deepEntryGuard		 (GuardControl&	control);

	HFSM_INLINE void	deepConstruct		 (PlanControl&  control);

	HFSM_INLINE void	deepEnter			 (PlanControl&	control);
	HFSM_INLINE void	deepReenter			 (PlanControl&	control);

	HFSM_INLINE Status	deepUpdate			 (FullControl&	control);

	template <typename TEvent>
	HFSM_INLINE Status	deepReact			 (FullControl&	control, const TEvent& event);

	HFSM_INLINE bool	deepForwardExitGuard (GuardControl&)					{ return false; }
	HFSM_INLINE bool	deepExitGuard		 (GuardControl&	control);

	HFSM_INLINE void	deepExit			 (PlanControl&	control);

	HFSM_INLINE void	deepDestruct		 (PlanControl&  control);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void	wrapPlanSucceeded	 (FullControl&	control);
	HFSM_INLINE void	wrapPlanFailed		 (FullControl&	control);
	HFSM_INLINE Rank	wrapRank			 (Control& control);
	HFSM_INLINE Utility	wrapUtility			 (Control& control);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void	deepForwardActive	 (Control&, const Request::Type)					{}
	HFSM_INLINE void	deepForwardRequest	 (Control&, const Request::Type)					{}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void	deepRequestChange	 (Control&)											{}
	HFSM_INLINE void	deepRequestRemain	 (Registry&)										{}
	HFSM_INLINE void	deepRequestRestart	 (Registry&)										{}
	HFSM_INLINE void	deepRequestResume	 (Registry&)										{}
	HFSM_INLINE void	deepRequestUtilize	 (Control&)											{}
	HFSM_INLINE void	deepRequestRandomize (Control&)											{}

	HFSM_INLINE UP		deepReportChange	 (Control& control);
	HFSM_INLINE UP		deepReportUtilize	 (Control& control);
	HFSM_INLINE Rank	deepReportRank		 (Control& control);
	HFSM_INLINE Utility	deepReportRandomize	 (Control& control);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void	deepChangeToRequested(Control&)											{}

	//------------------------------------------------------------------------------

#if defined _DEBUG || defined HFSM_ENABLE_STRUCTURE_REPORT || defined HFSM_ENABLE_LOG_INTERFACE

	static constexpr LongIndex NAME_COUNT = HeadBox::isBare() ? 0 : 1;

#endif

	//----------------------------------------------------------------------

#ifdef HFSM_ENABLE_STRUCTURE_REPORT
	using StructureStateInfos = typename TArgs::StructureStateInfos;
	using RegionType		  = typename StructureStateInfo::RegionType;

	static const char* name();

	void deepGetNames(const LongIndex parent,
					  const RegionType region,
					  const ShortIndex depth,
					  StructureStateInfos& stateInfos) const;
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef HFSM_ENABLE_SERIALIZATION
	using WriteStream	= typename TArgs::WriteStream;
	using ReadStream	= typename TArgs::ReadStream;

	HFSM_INLINE void	deepSaveActive	 (const Registry&, WriteStream&) const					{}
	HFSM_INLINE void	deepSaveResumable(const Registry&, WriteStream&) const					{}

	HFSM_INLINE void	deepLoadRequested(		Registry&, ReadStream& ) const					{}
	HFSM_INLINE void	deepLoadResumable(		Registry&, ReadStream& ) const					{}
#endif

	//----------------------------------------------------------------------

#ifdef HFSM_ENABLE_LOG_INTERFACE

	template <typename>
	struct Traits_;

	template <typename TR_, typename TH_, typename... TAs_>
	struct Traits_<TR_(TH_::*)(TAs_...)> {
		using Host = TH_;
	};

	template <typename TM_>
	using Host_			= typename Traits_<TM_>::Host;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	using Empty			= EmptyT<TArgs>;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	template <typename TMethodType>
	typename std::enable_if<
				 std::is_same<
					 Host_<TMethodType>,
					 Empty
				 >::value
			 >::type
	log(Logger&,
		Context&,
		const Method) const
	{}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	template <typename TMethodType>
	typename std::enable_if<
				 !std::is_same<
					 Host_<TMethodType>,
					 Empty
				 >::value
			 >::type
	log(Logger& logger,
		Context& context,
		const Method method) const
	{
		logger.recordMethod(context, STATE_ID, method);
	}

#endif

	//----------------------------------------------------------------------

	// TODO: account for boxing
	//
	// if you see..
	// VS	 - error C2079: 'hfsm2::detail::S_<...>::_head' uses undefined struct 'Blah'
	// Clang - error : field has incomplete type 'hfsm2::detail::S_<...>::Head' (aka 'Blah')
	//
	// .. add definition for the state 'Blah'
	HeadBox _headBox;
};

////////////////////////////////////////////////////////////////////////////////

}
}

namespace hfsm2 {
namespace detail {

//------------------------------------------------------------------------------

template <StateID NS, typename TA, typename TH>
struct RegisterT {
	using StateParents	= StaticArray<Parent, TA::STATE_COUNT>;
	using StateList		= typename TA::StateList;

	static constexpr StateID STATE_ID = NS;

	static HFSM_INLINE
	void
	execute(StateParents& stateParents,
			const Parent parent)
	{
		static constexpr auto HEAD_ID = StateList::template index<TH>();
		StaticAssertEquality<STATE_ID, HEAD_ID>();

		stateParents[STATE_ID] = parent;
	}
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <StateID NS, typename TA>
struct RegisterT<NS, TA, StaticEmptyT<TA>> {
	using StateParents	= StaticArray<Parent, TA::STATE_COUNT>;

	static HFSM_INLINE
	void
	execute(StateParents&, const Parent) {}
};

////////////////////////////////////////////////////////////////////////////////

template <typename TN_, typename TA, typename TH>
void
S_<TN_, TA, TH>::deepRegister(Registry& registry,
							  const Parent parent)
{
	using Register = RegisterT<STATE_ID, TA, Head>;
	Register::execute(registry.stateParents, parent);
}

//------------------------------------------------------------------------------

template <typename TN_, typename TA, typename TH>
bool
S_<TN_, TA, TH>::deepEntryGuard(GuardControl& control) {
	HFSM_LOG_STATE_METHOD(&Head::entryGuard,
						  control.context(),
						  Method::ENTRY_GUARD);

	ScopedOrigin origin{control, STATE_ID};

	const bool cancelledBefore = control._cancelled;

	_headBox.guard(control);

	return !cancelledBefore && control._cancelled;
}

//------------------------------------------------------------------------------

template <typename TN_, typename TA, typename TH>
void
S_<TN_, TA, TH>::deepConstruct(PlanControl& HFSM_IF_LOGGER(control)) {
	HFSM_ASSERT(!control._planData.tasksSuccesses.template get<STATE_ID>());
	HFSM_ASSERT(!control._planData.tasksFailures .template get<STATE_ID>());

	HFSM_LOG_STATE_METHOD(&Head::enter,
						  control.context(),
						  Method::CONSTRUCT);

	_headBox.construct();
}

//------------------------------------------------------------------------------

template <typename TN_, typename TA, typename TH>
void
S_<TN_, TA, TH>::deepEnter(PlanControl& control) {
	HFSM_LOG_STATE_METHOD(&Head::enter,
						  control.context(),
						  Method::ENTER);

	ScopedOrigin origin{control, STATE_ID};

	_headBox.get().widePreEnter(control.context());
	_headBox.get().		  enter(control);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN_, typename TA, typename TH>
void
S_<TN_, TA, TH>::deepReenter(PlanControl& control) {
	HFSM_ASSERT(!control._planData.tasksSuccesses.template get<STATE_ID>());
	HFSM_ASSERT(!control._planData.tasksFailures .template get<STATE_ID>());

	HFSM_LOG_STATE_METHOD(&Head::reenter,
						  control.context(),
						  Method::REENTER);

	ScopedOrigin origin{control, STATE_ID};

	_headBox.destruct ();
	_headBox.construct();

	_headBox.get().widePreReenter(control.context());
	_headBox.get().		  reenter(control);
}

//------------------------------------------------------------------------------

template <typename TN_, typename TA, typename TH>
Status
S_<TN_, TA, TH>::deepUpdate(FullControl& control) {
	HFSM_LOG_STATE_METHOD(&Head::update,
						  control.context(),
						  Method::UPDATE);

	ScopedOrigin origin{control, STATE_ID};

	_headBox.get().widePreUpdate(control.context());
	_headBox.get().		  update(control);

	return control._status;
}

//------------------------------------------------------------------------------

template <typename TN_, typename TA, typename TH>
template <typename TEvent>
Status
S_<TN_, TA, TH>::deepReact(FullControl& control,
						   const TEvent& event)
{
	auto reaction = static_cast<void(Head::*)(const TEvent&, FullControl&)>(&Head::react);
	HFSM_LOG_STATE_METHOD(reaction,
						  control.context(),
						  Method::REACT);

	ScopedOrigin origin{control, STATE_ID};

	_headBox.get().widePreReact(event, control.context());
	(_headBox.get().*reaction) (event, control);				//_headBox.get().react(event, control);

	return control._status;
}

//------------------------------------------------------------------------------

template <typename TN_, typename TA, typename TH>
bool
S_<TN_, TA, TH>::deepExitGuard(GuardControl& control) {
	HFSM_LOG_STATE_METHOD(&Head::exitGuard,
						  control.context(),
						  Method::EXIT_GUARD);

	ScopedOrigin origin{control, STATE_ID};

	const bool cancelledBefore = control._cancelled;

	_headBox.get().widePreExitGuard(control.context());
	_headBox.get().		  exitGuard(control);

	return !cancelledBefore && control._cancelled;
}

//------------------------------------------------------------------------------

template <typename TN_, typename TA, typename TH>
void
S_<TN_, TA, TH>::deepExit(PlanControl& control) {
	HFSM_LOG_STATE_METHOD(&Head::exit,
						  control.context(),
						  Method::EXIT);

	ScopedOrigin origin{control, STATE_ID};

	// if you see..
	// VS	 - error C2039:  'exit': is not a member of 'Blah'
	// Clang - error : no member named 'exit' in 'Blah'
	//
	// .. inherit state 'Blah' from hfsm2::Machine::Instance::State
	_headBox.get().		   exit(control);
	_headBox.get().widePostExit(control.context());
}

//------------------------------------------------------------------------------

template <typename TN_, typename TA, typename TH>
void
S_<TN_, TA, TH>::deepDestruct(PlanControl& control) {
	HFSM_LOG_STATE_METHOD(&Head::exit,
						  control.context(),
						  Method::DESTRUCT);

	_headBox.destruct();

	control._planData.tasksSuccesses.template reset<STATE_ID>();
	control._planData.tasksFailures .template reset<STATE_ID>();
}

//------------------------------------------------------------------------------

template <typename TN_, typename TA, typename TH>
void
S_<TN_, TA, TH>::wrapPlanSucceeded(FullControl& control) {
	HFSM_LOG_STATE_METHOD(&Head::planSucceeded,
						  control.context(),
						  Method::PLAN_SUCCEEDED);

	ScopedOrigin origin{control, STATE_ID};

	_headBox.get().planSucceeded(control);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN_, typename TA, typename TH>
void
S_<TN_, TA, TH>::wrapPlanFailed(FullControl& control) {
	HFSM_LOG_STATE_METHOD(&Head::planFailed,
						  control.context(),
						  Method::PLAN_FAILED);

	ScopedOrigin origin{control, STATE_ID};

	_headBox.get().planFailed(control);
}

//------------------------------------------------------------------------------

template <typename TN_, typename TA, typename TH>
typename S_<TN_, TA, TH>::Rank
S_<TN_, TA, TH>::wrapRank(Control& control) {
	HFSM_LOG_STATE_METHOD(&Head::rank,
						  control.context(),
						  Method::RANK);

	return _headBox.get().rank(static_cast<const Control&>(control));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN_, typename TA, typename TH>
typename S_<TN_, TA, TH>::Utility
S_<TN_, TA, TH>::wrapUtility(Control& control) {
	HFSM_LOG_STATE_METHOD(&Head::utility,
						  control.context(),
						  Method::UTILITY);

	return _headBox.get().utility(static_cast<const Control&>(control));
}

//------------------------------------------------------------------------------

template <typename TN_, typename TA, typename TH>
typename S_<TN_, TA, TH>::UP
S_<TN_, TA, TH>::deepReportChange(Control& control) {
	const Utility utility = wrapUtility(control);

	const Parent parent = stateParent(control);

	return {utility, parent.prong};
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN_, typename TA, typename TH>
typename S_<TN_, TA, TH>::UP
S_<TN_, TA, TH>::deepReportUtilize(Control& control) {
	const Utility utility = wrapUtility(control);
	const Parent  parent  = stateParent(control);

	return {utility, parent.prong};
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN_, typename TA, typename TH>
typename S_<TN_, TA, TH>::Rank
S_<TN_, TA, TH>::deepReportRank(Control& control) {
	return wrapRank(control);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN_, typename TA, typename TH>
typename S_<TN_, TA, TH>::Utility
S_<TN_, TA, TH>::deepReportRandomize(Control& control) {
	return wrapUtility(control);
}

//------------------------------------------------------------------------------

#ifdef HFSM_ENABLE_STRUCTURE_REPORT

template <typename TN_, typename TA, typename TH>
const char*
S_<TN_, TA, TH>::name() {
	if (HeadBox::isBare())
		return "";
	else {
		const std::type_index type = typeid(Head);
		const char* const raw = type.name();

	#if defined(_MSC_VER)

		auto first =
			raw[0] == 's' ? 7 : // Struct
			raw[0] == 'c' ? 6 : // Class
							0;

		for (auto c = first; raw[c]; ++c)
			if (raw[c] == ':')
				first = c + 1;

		return raw + first;

	#elif defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)

		return raw;

	#else

		return raw;

	#endif
	}
}

//------------------------------------------------------------------------------

template <typename TN_, typename TA, typename TH>
void
S_<TN_, TA, TH>::deepGetNames(const LongIndex parent,
							  const RegionType region,
							  const ShortIndex depth,
							  StructureStateInfos& _stateInfos) const
{
	_stateInfos.append(StructureStateInfo{parent, region, depth, name()});
}

#endif

///////////////////////////////////////////////////////////////////////////////

}
}

namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

template <typename>
struct SI_;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <Strategy, typename, typename...>
struct CI_;

template <typename...>
struct CSI_;

template <typename TInitial, typename... TRemaining>
struct CSI_<TInitial, TRemaining...>;

template <typename TInitial>
struct CSI_<TInitial>;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename, typename...>
struct OI_;

template <typename...>
struct OSI_;

template <typename TInitial, typename... TRemaining>
struct OSI_<TInitial, TRemaining...>;

template <typename TInitial>
struct OSI_<TInitial>;

//------------------------------------------------------------------------------

template <typename...>
struct WrapInfoT;

template <typename TH>
struct WrapInfoT<	 TH> {
	using Type = SI_<TH>;
};

template <Strategy SG, typename TH, typename... TS>
struct WrapInfoT<	 CI_<SG, TH, TS...>> {
	using Type =	 CI_<SG, TH, TS...>;
};

template <typename... TS>
struct WrapInfoT<	 OI_<TS...>> {
	using Type =	 OI_<TS...>;
};

template <typename... TS>
using WrapInfo = typename WrapInfoT<TS...>::Type;

//------------------------------------------------------------------------------

template <typename THead>
struct SI_ final {
	using Head				= THead;
	using StateList			= ITL_<Head>;
	using RegionList		= ITL_<>;

	static constexpr ShortIndex WIDTH			= 1;
	static constexpr LongIndex  REVERSE_DEPTH	= 1;
	static constexpr ShortIndex COMPO_REGIONS	= 0;
	static constexpr LongIndex  COMPO_PRONGS	= 0;
	static constexpr ShortIndex ORTHO_REGIONS	= 0;
	static constexpr ShortIndex ORTHO_UNITS		= 0;

	static constexpr LongIndex  ACTIVE_BITS		= 0;
	static constexpr LongIndex  RESUMABLE_BITS	= 0;

	static constexpr LongIndex  STATE_COUNT		= StateList::SIZE;
	static constexpr ShortIndex REGION_COUNT	= RegionList::SIZE;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TInitial, typename... TRemaining>
struct CSI_<TInitial, TRemaining...> {
	using Initial			= WrapInfo<TInitial>;
	using Remaining			= CSI_<TRemaining...>;
	using StateList			= Merge<typename Initial::StateList,  typename Remaining::StateList>;
	using RegionList		= Merge<typename Initial::RegionList, typename Remaining::RegionList>;

	static constexpr LongIndex  REVERSE_DEPTH	= Max<Initial::REVERSE_DEPTH,	Remaining::REVERSE_DEPTH>::VALUE;
	static constexpr ShortIndex COMPO_REGIONS	=	  Initial::COMPO_REGIONS  + Remaining::COMPO_REGIONS;
	static constexpr LongIndex  COMPO_PRONGS	=	  Initial::COMPO_PRONGS   + Remaining::COMPO_PRONGS;
	static constexpr ShortIndex ORTHO_REGIONS	=	  Initial::ORTHO_REGIONS  + Remaining::ORTHO_REGIONS;
	static constexpr ShortIndex ORTHO_UNITS		=	  Initial::ORTHO_UNITS    + Remaining::ORTHO_UNITS;

	static constexpr LongIndex  ACTIVE_BITS		= Max<Initial::ACTIVE_BITS,		Remaining::ACTIVE_BITS>::VALUE;
	static constexpr LongIndex  RESUMABLE_BITS	=	  Initial::RESUMABLE_BITS + Remaining::RESUMABLE_BITS;

	static constexpr LongIndex  STATE_COUNT		= StateList::SIZE;
	static constexpr ShortIndex REGION_COUNT	= RegionList::SIZE;
};

template <typename TInitial>
struct CSI_<TInitial> {
	using Initial			= WrapInfo<TInitial>;
	using StateList			= typename Initial::StateList;
	using RegionList		= typename Initial::RegionList;

	static constexpr LongIndex  REVERSE_DEPTH	= Initial::REVERSE_DEPTH;
	static constexpr ShortIndex COMPO_REGIONS	= Initial::COMPO_REGIONS;
	static constexpr LongIndex  COMPO_PRONGS	= Initial::COMPO_PRONGS;
	static constexpr ShortIndex ORTHO_REGIONS	= Initial::ORTHO_REGIONS;
	static constexpr ShortIndex ORTHO_UNITS		= Initial::ORTHO_UNITS;

	static constexpr LongIndex  ACTIVE_BITS		= Initial::ACTIVE_BITS;
	static constexpr LongIndex  RESUMABLE_BITS	= Initial::RESUMABLE_BITS;

	static constexpr LongIndex  STATE_COUNT		= StateList::SIZE;
	static constexpr ShortIndex REGION_COUNT	= RegionList::SIZE;
};

template <Strategy TStrategy, typename THead, typename... TSubStates>
struct CI_ final {
	static constexpr Strategy STRATEGY = TStrategy;

	using Head				= THead;
	using HeadInfo			= SI_<Head>;
	using SubStates			= CSI_<TSubStates...>;
	using StateList			= Merge<typename HeadInfo::StateList, typename SubStates::StateList>;
	using RegionList		= Merge<typename HeadInfo::StateList, typename SubStates::RegionList>;

	static constexpr ShortIndex	WIDTH			= sizeof...(TSubStates);
	static constexpr LongIndex	REVERSE_DEPTH	= SubStates::REVERSE_DEPTH + 1;
	static constexpr ShortIndex	COMPO_REGIONS	= SubStates::COMPO_REGIONS + 1;
	static constexpr LongIndex	COMPO_PRONGS	= SubStates::COMPO_PRONGS + WIDTH;
	static constexpr ShortIndex	ORTHO_REGIONS	= SubStates::ORTHO_REGIONS;
	static constexpr ShortIndex	ORTHO_UNITS		= SubStates::ORTHO_UNITS;

	static constexpr LongIndex	WIDTH_BITS		= bitWidth(WIDTH);
	static constexpr LongIndex  ACTIVE_BITS		= SubStates::ACTIVE_BITS	+ WIDTH_BITS;
	static constexpr LongIndex  RESUMABLE_BITS	= SubStates::RESUMABLE_BITS + WIDTH_BITS + 1;

	static constexpr LongIndex	STATE_COUNT		= StateList::SIZE;
	static constexpr ShortIndex	REGION_COUNT	= RegionList::SIZE;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TInitial, typename... TRemaining>
struct OSI_<TInitial, TRemaining...> {
	using Initial			= WrapInfo<TInitial>;
	using Remaining			= OSI_<TRemaining...>;
	using StateList			= Merge<typename Initial::StateList,  typename Remaining::StateList>;
	using RegionList		= Merge<typename Initial::RegionList, typename Remaining::RegionList>;

	static constexpr LongIndex  REVERSE_DEPTH	= Max<Initial::REVERSE_DEPTH,	Remaining::REVERSE_DEPTH>::VALUE;
	static constexpr ShortIndex COMPO_REGIONS	=	  Initial::COMPO_REGIONS  + Remaining::COMPO_REGIONS;
	static constexpr LongIndex  COMPO_PRONGS	=	  Initial::COMPO_PRONGS   + Remaining::COMPO_PRONGS;
	static constexpr ShortIndex ORTHO_REGIONS	=	  Initial::ORTHO_REGIONS  + Remaining::ORTHO_REGIONS;
	static constexpr ShortIndex ORTHO_UNITS		=	  Initial::ORTHO_UNITS    + Remaining::ORTHO_UNITS;

	static constexpr LongIndex  ACTIVE_BITS		=	  Initial::ACTIVE_BITS    + Remaining::ACTIVE_BITS;
	static constexpr LongIndex  RESUMABLE_BITS	=	  Initial::RESUMABLE_BITS + Remaining::RESUMABLE_BITS;
};

template <typename TInitial>
struct OSI_<TInitial> {
	using Initial			= WrapInfo<TInitial>;
	using StateList			= typename Initial::StateList;
	using RegionList		= typename Initial::RegionList;

	static constexpr LongIndex  REVERSE_DEPTH	= Initial::REVERSE_DEPTH;
	static constexpr ShortIndex COMPO_REGIONS	= Initial::COMPO_REGIONS;
	static constexpr LongIndex  COMPO_PRONGS	= Initial::COMPO_PRONGS;
	static constexpr ShortIndex ORTHO_REGIONS	= Initial::ORTHO_REGIONS;
	static constexpr ShortIndex ORTHO_UNITS		= Initial::ORTHO_UNITS;

	static constexpr LongIndex  ACTIVE_BITS		= Initial::ACTIVE_BITS;
	static constexpr LongIndex  RESUMABLE_BITS	= Initial::RESUMABLE_BITS;
};

template <typename THead, typename... TSubStates>
struct OI_ final {
	using Head				= THead;
	using HeadInfo			= SI_<Head>;
	using SubStates			= OSI_<TSubStates...>;
	using StateList			= Merge<typename HeadInfo::StateList, typename SubStates::StateList>;
	using RegionList		= Merge<typename HeadInfo::StateList, typename SubStates::RegionList>;

	static constexpr ShortIndex WIDTH			= sizeof...(TSubStates);
	static constexpr LongIndex  REVERSE_DEPTH	= SubStates::REVERSE_DEPTH + 1;
	static constexpr ShortIndex COMPO_REGIONS	= SubStates::COMPO_REGIONS;
	static constexpr LongIndex  COMPO_PRONGS	= SubStates::COMPO_PRONGS;
	static constexpr ShortIndex ORTHO_REGIONS	= SubStates::ORTHO_REGIONS + 1;
	static constexpr ShortIndex ORTHO_UNITS		= SubStates::ORTHO_UNITS + (WIDTH + 7) / 8;

	static constexpr LongIndex  ACTIVE_BITS		= SubStates::ACTIVE_BITS;
	static constexpr LongIndex  RESUMABLE_BITS	= SubStates::RESUMABLE_BITS;

	static constexpr LongIndex  STATE_COUNT		= StateList::SIZE;
	static constexpr ShortIndex REGION_COUNT	= RegionList::SIZE;
};

////////////////////////////////////////////////////////////////////////////////

template <typename TContext,
		  typename TConfig,
		  typename TStateList,
		  typename TRegionList,
		  LongIndex NCompoCount,
		  LongIndex NOrthoCount,
		  LongIndex NOrthoUnits,
		  LongIndex NSerialBits,
		  LongIndex NTaskCapacity>
struct ArgsT final {
	using Context	 = TContext;

	using Config_	 = TConfig;
	using Rank		 = typename Config_::Rank;
	using Utility	 = typename Config_::Utility;
	using RNG		 = typename Config_::RNG;
	using UP		 = typename Config_::UP;
	using Logger	 = typename Config_::Logger;

	using StateList	 = TStateList;
	using RegionList = TRegionList;

	static constexpr LongIndex  STATE_COUNT	  = StateList::SIZE;
	static constexpr ShortIndex COMPO_REGIONS = NCompoCount;
	static constexpr ShortIndex ORTHO_REGIONS = NOrthoCount;
	static constexpr ShortIndex ORTHO_UNITS	  = NOrthoUnits;
	static constexpr ShortIndex SERIAL_BITS	  = NSerialBits;

	static constexpr LongIndex  TASK_CAPACITY = NTaskCapacity;

#ifdef HFSM_ENABLE_SERIALIZATION
	using SerialBuffer			= StreamBuffer	<SERIAL_BITS>;
	using WriteStream			= BitWriteStream<SERIAL_BITS>;
	using ReadStream			= BitReadStream	<SERIAL_BITS>;
#endif

	HFSM_IF_STRUCTURE(using StructureStateInfos = Array<StructureStateInfo, STATE_COUNT>);
};

//------------------------------------------------------------------------------

template <StateID NStateID,
		  ShortIndex NCompoIndex,
		  ShortIndex NOrthoIndex,
		  ShortIndex NOrthoUnit>
struct I_ {
	static constexpr StateID	STATE_ID	 = NStateID;
	static constexpr ShortIndex COMPO_INDEX	 = NCompoIndex;
	static constexpr ShortIndex ORTHO_INDEX	 = NOrthoIndex;
	static constexpr ShortIndex ORTHO_UNIT	 = NOrthoUnit;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename, typename, typename>
struct S_;

template <typename, typename, Strategy, typename, typename...>
struct C_;

template <typename, typename, Strategy, ShortIndex, typename...>
struct CS_;

template <typename, typename, typename, typename...>
struct O_;

template <typename, typename, ShortIndex, typename...>
struct OS_;

template <typename, typename>
class RW_;

//------------------------------------------------------------------------------

template <typename, typename...>
struct MaterialT;

template <typename TN, typename TA, typename TH>
struct MaterialT   <TN, TA, TH> {
	using Type = S_<TN, TA, TH>;
};

template <typename TN, typename TA, Strategy SG, 			  typename... TS>
struct MaterialT   <TN, TA, CI_<SG, void,             TS...>> {
	using Type = C_<TN, TA,     SG, StaticEmptyT<TA>, TS...>;
};

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
struct MaterialT   <TN, TA, CI_<SG, TH,	TS...>> {
	using Type = C_<TN, TA,     SG, TH,	TS...>;
};

template <typename TN, typename TA,				 typename... TS>
struct MaterialT   <TN, TA, OI_<void,			  TS...>> {
	using Type = O_<TN, TA,     StaticEmptyT<TA>, TS...>;
};

template <typename TN, typename TA, typename TH, typename... TS>
struct MaterialT   <TN, TA, OI_<TH,				  TS...>> {
	using Type = O_<TN, TA,     TH,				  TS...>;
};

template <typename TN, typename... TS>
using Material = typename MaterialT<TN, TS...>::Type;

//------------------------------------------------------------------------------

template <typename TConfig,
		  typename TApex>
struct RF_ final {
	using Config_		= TConfig;
	using Context		= typename Config_::Context;
	using Apex			= TApex;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	static constexpr LongIndex SUBSTITUTION_LIMIT= Config_::SUBSTITUTION_LIMIT;

	static constexpr LongIndex TASK_CAPACITY	 = Config_::TASK_CAPACITY != INVALID_LONG_INDEX ?
													   Config_::TASK_CAPACITY : Apex::COMPO_PRONGS * 2;

	static constexpr ShortIndex COMPO_REGIONS	 = Apex::COMPO_REGIONS;
	static constexpr ShortIndex ORTHO_REGIONS	 = Apex::ORTHO_REGIONS;
	static constexpr ShortIndex ORTHO_UNITS		 = Apex::ORTHO_UNITS;

	static constexpr LongIndex  ACTIVE_BITS		 = Apex::ACTIVE_BITS;
	static constexpr LongIndex  RESUMABLE_BITS	 = Apex::RESUMABLE_BITS;

	using StateList		= typename Apex::StateList;
	using RegionList	= typename Apex::RegionList;

	using Args			= ArgsT<Context,
								Config_,
								StateList,
								RegionList,
								COMPO_REGIONS,
								ORTHO_REGIONS,
								ORTHO_UNITS,
								ACTIVE_BITS + RESUMABLE_BITS,
								TASK_CAPACITY>;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	using Instance		= RW_<Config_, Apex>;

	using Control		= ControlT	   <Args>;
	using FullControl	= FullControlT <Args>;
	using GuardControl	= GuardControlT<Args>;

	using Injection		= InjectionT<Args>;

	//----------------------------------------------------------------------

	using DynamicState	= DynamicEmptyT<Args>;

	template <typename... TInjections>
	using DynamicStateT	= DB_<TInjections...>;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	using StaticState	= StaticEmptyT<Args>;

	template <typename... TInjections>
	using StaticStateT	= SB_<TInjections...>;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	using State			= StaticState;

	template <typename... TInjections>
	using StateT		= StaticStateT<TInjections...>;

	//----------------------------------------------------------------------

	template <typename T>
	static constexpr bool contains() {
		return StateList::template index<T>() != INVALID_LONG_INDEX;
	}

	template <typename T>
	static constexpr StateID  stateId()		{ return			StateList ::template index<T>();	}

	template <typename T>
	static constexpr RegionID regionId()	{ return (RegionID) RegionList::template index<T>();	}
};

////////////////////////////////////////////////////////////////////////////////

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
struct CSubMaterialT;

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
struct CSubMaterialT<TN, TA, SG, NI, ITL_<TS...>> {
	using Type = CS_<TN, TA, SG, NI,	  TS...>;
};

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
using CSubMaterial = typename CSubMaterialT<TN, TA, SG, NI, TS...>::Type;

//------------------------------------------------------------------------------

template <typename...>
struct InfoT;

template <typename TN, typename TA, typename TH>
struct InfoT<S_<TN, TA, TH>> {
	using Type = SI_<	TH>;
};

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
struct InfoT<C_<TN, TA, SG, TH, TS...>> {
	using Type = CI_<	SG, TH, TS...>;
};

template <typename TN, typename TA, typename TH, typename... TS>
struct InfoT<O_<TN, TA, TH, TS...>> {
	using Type = OI_<	TH, TS...>;
};

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
struct InfoT<CS_<TN, TA, SG, NI, TS...>> {
	using Type = CSI_<			 TS...>;
};

////////////////////////////////////////////////////////////////////////////////

}
}
namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

template <typename TIndices,
		  typename TArgs,
		  Strategy TStrategy,
		  ShortIndex NIndex,
		  typename... TStates>
struct CS_ final {
	static_assert(sizeof...(TStates) >= 2, "");

	using Indices		= TIndices;
	static constexpr StateID	INITIAL_ID	= Indices::STATE_ID;
	static constexpr ShortIndex COMPO_INDEX	= Indices::COMPO_INDEX;
	static constexpr ShortIndex ORTHO_INDEX	= Indices::ORTHO_INDEX;
	static constexpr ShortIndex ORTHO_UNIT	= Indices::ORTHO_UNIT;

	static constexpr Strategy	STRATEGY	= TStrategy;

	static constexpr ShortIndex REGION_ID	= COMPO_INDEX + ORTHO_INDEX;
	static constexpr ShortIndex PRONG_INDEX	= NIndex;

	using Args			= TArgs;
	using Rank			= typename Args::Rank;
	using Utility		= typename Args::Utility;
	using UP			= typename Args::UP;
	using StateList		= typename Args::StateList;
	using RegionList	= typename Args::RegionList;

	using Registry		= RegistryT<Args>;
	using StateParents	= typename Registry::StateParents;

	using Control		= ControlT	   <Args>;
	using PlanControl	= PlanControlT <Args>;
	using FullControl	= FullControlT <Args>;
	using GuardControl	= GuardControlT<Args>;

	static constexpr ShortIndex L_PRONG = PRONG_INDEX;

	using LStates		= SplitL<TStates...>;
	using LHalf			= CSubMaterial<I_<INITIAL_ID,
										  COMPO_INDEX,
										  ORTHO_INDEX,
										  ORTHO_UNIT>,
									   Args,
									   STRATEGY,
									   L_PRONG,
									   LStates>;
	using LHalfInfo		= typename InfoT<LHalf>::Type;

	static constexpr ShortIndex R_PRONG = PRONG_INDEX + LStates::SIZE;

	using RStates		= SplitR<TStates...>;
	using RHalf			= CSubMaterial<I_<INITIAL_ID  + LHalfInfo::STATE_COUNT,
										  COMPO_INDEX + LHalfInfo::COMPO_REGIONS,
										  ORTHO_INDEX + LHalfInfo::ORTHO_REGIONS,
										  ORTHO_UNIT  + LHalfInfo::ORTHO_UNITS>,
									   Args,
									   STRATEGY,
									   R_PRONG,
									   RStates>;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef HFSM_EXPLICIT_MEMBER_SPECIALIZATION
	template <typename T>
	HFSM_INLINE		  T& access();

	template <typename T>
	HFSM_INLINE const T& access() const;
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void	wideRegister				  (Registry& registry, const Parent parent);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE bool	wideForwardEntryGuard		  (GuardControl& control,								const ShortIndex prong);
	HFSM_INLINE bool	wideEntryGuard				  (GuardControl& control,								const ShortIndex prong);

	HFSM_INLINE void	wideConstruct				  (PlanControl&  control,								const ShortIndex prong);

	HFSM_INLINE void	wideEnter					  (PlanControl&  control,								const ShortIndex prong);
	HFSM_INLINE void	wideReenter					  (PlanControl&  control,								const ShortIndex prong);

	HFSM_INLINE Status	wideUpdate					  (FullControl&  control,								const ShortIndex prong);

	template <typename TEvent>
	HFSM_INLINE Status	wideReact					  (FullControl&  control, const TEvent& event,			const ShortIndex prong);

	HFSM_INLINE bool	wideForwardExitGuard		  (GuardControl& control,								const ShortIndex prong);
	HFSM_INLINE bool	wideExitGuard				  (GuardControl& control,								const ShortIndex prong);

	HFSM_INLINE void	wideExit					  (PlanControl&  control,								const ShortIndex prong);

	HFSM_INLINE void	wideDestruct				  (PlanControl&  control,								const ShortIndex prong);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void	wideForwardActive			  (Control& control,	 const Request::Type request,	const ShortIndex prong);
	HFSM_INLINE void	wideForwardRequest			  (Control& control,	 const Request::Type request,	const ShortIndex prong);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef HFSM_EXPLICIT_MEMBER_SPECIALIZATION

	template <Strategy = STRATEGY>
	HFSM_INLINE void	wideRequestChange			  (Control& control,									const ShortIndex = INVALID_SHORT_INDEX);

	template <>
	HFSM_INLINE void	wideRequestChange<Composite>  (Control& control,									const ShortIndex)		{ wideRequestChangeComposite(control);			}

	template <>
	HFSM_INLINE	void	wideRequestChange<Resumable>  (Control& control,									const ShortIndex prong)	{ wideRequestChangeResumable(control, prong);	}

#else

	HFSM_INLINE void	wideRequestChange			  (Control& control,									const ShortIndex = INVALID_SHORT_INDEX);

#endif

	HFSM_INLINE void	wideRequestChangeComposite	  (Control& control);
	HFSM_INLINE void	wideRequestChangeResumable	  (Control& control,									const ShortIndex prong);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void	wideRequestRemain			  (Registry& registry);
	HFSM_INLINE void	wideRequestRestart			  (Registry& registry);
	HFSM_INLINE void	wideRequestResume			  (Registry& registry,									const ShortIndex prong);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE UP		wideReportChangeComposite	  (Control& control);
	HFSM_INLINE UP		wideReportChangeResumable	  (Control& control,									const ShortIndex prong);
	HFSM_INLINE UP		wideReportChangeUtilitarian	  (Control& control);
	HFSM_INLINE Utility	wideReportChangeRandom		  (Control& control,	 Utility* const options, const Rank* const ranks, const Rank top);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE UP		wideReportUtilize			  (Control& control);
	HFSM_INLINE Rank	wideReportRank				  (Control& control,	 Rank*	  const ranks);
	HFSM_INLINE Utility	wideReportRandomize			  (Control& control,	 Utility* const options, const Rank* const ranks, const Rank top);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void	wideChangeToRequested		  (PlanControl& control, const ShortIndex prong);

	//----------------------------------------------------------------------

#ifdef HFSM_ENABLE_STRUCTURE_REPORT
	using StructureStateInfos = typename Args::StructureStateInfos;
	using RegionType		  = typename StructureStateInfo::RegionType;

	static constexpr LongIndex NAME_COUNT = LHalf::NAME_COUNT + RHalf::NAME_COUNT;

	void wideGetNames(const LongIndex parent,
					  const RegionType region,
					  const ShortIndex depth,
					  StructureStateInfos& stateInfos) const;
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef HFSM_ENABLE_SERIALIZATION
	using WriteStream	= typename Args::WriteStream;
	using ReadStream	= typename Args::ReadStream;

	HFSM_INLINE void	wideSaveActive	 (const Registry& registry, WriteStream& stream, const ShortIndex prong) const;
	HFSM_INLINE void	wideSaveResumable(const Registry& registry, WriteStream& stream						   ) const;

	HFSM_INLINE void	wideLoadRequested(		Registry& registry, ReadStream&  stream, const ShortIndex prong) const;
	HFSM_INLINE void	wideLoadResumable(		Registry& registry, ReadStream&  stream						   ) const;
#endif

	//----------------------------------------------------------------------

	LHalf lHalf;
	RHalf rHalf;
};

////////////////////////////////////////////////////////////////////////////////

template <typename TIndices,
		  typename TArgs,
		  Strategy TStrategy,
		  ShortIndex NIndex,
		  typename TState>
struct CS_<TIndices, TArgs, TStrategy, NIndex, TState> final {
	using Indices		= TIndices;
	static constexpr StateID	INITIAL_ID	= Indices::STATE_ID;
	static constexpr ShortIndex COMPO_INDEX	= Indices::COMPO_INDEX;
	static constexpr ShortIndex ORTHO_INDEX	= Indices::ORTHO_INDEX;
	static constexpr ShortIndex ORTHO_UNIT	= Indices::ORTHO_UNIT;

	static constexpr Strategy	STRATEGY	= TStrategy;

	static constexpr ShortIndex REGION_ID	= COMPO_INDEX + ORTHO_INDEX;
	static constexpr ShortIndex PRONG_INDEX	= NIndex;

	using Args			= TArgs;
	using Rank			= typename Args::Rank;
	using Utility		= typename Args::Utility;
	using UP			= typename Args::UP;
	using StateList		= typename Args::StateList;
	using RegionList	= typename Args::RegionList;

	using Registry	= RegistryT<Args>;
	using StateParents	= typename Registry::StateParents;

	using Control		= ControlT	   <Args>;
	using PlanControl	= PlanControlT <Args>;
	using FullControl	= FullControlT <Args>;
	using GuardControl	= GuardControlT<Args>;

	using State			= Material<I_<INITIAL_ID,
									  COMPO_INDEX,
									  ORTHO_INDEX,
									  ORTHO_UNIT>,
								   Args,
								   TState>;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef HFSM_EXPLICIT_MEMBER_SPECIALIZATION
	template <typename T>
	HFSM_INLINE		  T& access()					  { return state.template access<T>();	}

	template <typename T>
	HFSM_INLINE const T& access() const				  { return state.template access<T>();	}
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void	wideRegister				  (Registry& registry, const Parent parent);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE bool	wideForwardEntryGuard		  (GuardControl& control,								const ShortIndex prong);
	HFSM_INLINE bool	wideEntryGuard				  (GuardControl& control,								const ShortIndex prong);

	HFSM_INLINE void	wideConstruct				  (PlanControl& control,								const ShortIndex prong);

	HFSM_INLINE void	wideEnter					  (PlanControl& control,								const ShortIndex prong);
	HFSM_INLINE void	wideReenter					  (PlanControl& control,								const ShortIndex prong);

	HFSM_INLINE Status	wideUpdate					  (FullControl& control,								const ShortIndex prong);

	template <typename TEvent>
	HFSM_INLINE Status	wideReact					  (FullControl& control, const TEvent& event,			const ShortIndex prong);

	HFSM_INLINE bool	wideForwardExitGuard		  (GuardControl& control,								const ShortIndex prong);
	HFSM_INLINE bool	wideExitGuard				  (GuardControl& control,								const ShortIndex prong);

	HFSM_INLINE void	wideExit					  (PlanControl& control,								const ShortIndex prong);

	HFSM_INLINE void	wideDestruct				  (PlanControl& control,								const ShortIndex prong);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void	wideForwardActive			  (Control& control,	 const Request::Type request,	const ShortIndex prong);
	HFSM_INLINE void	wideForwardRequest			  (Control& control,	 const Request::Type request,	const ShortIndex prong);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void	wideRequestChangeComposite	  (Control& control);
	HFSM_INLINE void	wideRequestChangeResumable	  (Control& control,	 								const ShortIndex prong);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void	wideRequestRemain			  (Registry& registry);
	HFSM_INLINE void	wideRequestRestart			  (Registry& registry);
	HFSM_INLINE void	wideRequestResume			  (Registry& registry,									const ShortIndex prong);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE UP		wideReportChangeComposite	  (Control& control);
	HFSM_INLINE UP		wideReportChangeResumable	  (Control& control,									const ShortIndex prong);
	HFSM_INLINE UP		wideReportChangeUtilitarian	  (Control& control);
	HFSM_INLINE Utility	wideReportChangeRandom		  (Control& control,	 Utility* const options, const Rank* const ranks, const Rank top);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE UP		wideReportUtilize			  (Control& control);
	HFSM_INLINE Rank	wideReportRank				  (Control& control,	 Rank*	  const ranks);
	HFSM_INLINE Utility	wideReportRandomize			  (Control& control,	 Utility* const options, const Rank* const ranks, const Rank top);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void	wideChangeToRequested		  (PlanControl& control,								const ShortIndex prong);

	//----------------------------------------------------------------------

#ifdef HFSM_ENABLE_STRUCTURE_REPORT
	using StructureStateInfos = typename Args::StructureStateInfos;
	using RegionType		  = typename StructureStateInfo::RegionType;

	static constexpr LongIndex NAME_COUNT = State::NAME_COUNT;

	void wideGetNames(const LongIndex parent,
					  const RegionType region,
					  const ShortIndex depth,
					  StructureStateInfos& stateInfos) const;
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef HFSM_ENABLE_SERIALIZATION
	using WriteStream	= typename Args::WriteStream;
	using ReadStream	= typename Args::ReadStream;

	HFSM_INLINE void	wideSaveActive	 (const Registry& registry, WriteStream& stream, const ShortIndex prong) const;
	HFSM_INLINE void	wideSaveResumable(const Registry& registry, WriteStream& stream						   ) const;

	HFSM_INLINE void	wideLoadRequested(		Registry& registry, ReadStream&  stream, const ShortIndex prong) const;
	HFSM_INLINE void	wideLoadResumable(		Registry& registry, ReadStream&  stream						   ) const;
#endif

	//----------------------------------------------------------------------

	State state;
};

////////////////////////////////////////////////////////////////////////////////

}
}

namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

#ifdef HFSM_EXPLICIT_MEMBER_SPECIALIZATION

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
template <typename T>
T&
CS_<TN, TA, SG, NI, TS...>::access() {
	return LHalfInfo::StateList::template contains<T>() ?
		lHalf.template access<T>() :
		rHalf.template access<T>();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
template <typename T>
const T&
CS_<TN, TA, SG, NI, TS...>::access() const {
	return LHalfInfo::StateList::template contains<T>() ?
		lHalf.template access<T>() :
		rHalf.template access<T>();
}

#endif

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
void
CS_<TN, TA, SG, NI, TS...>::wideRegister(Registry& registry,
										 const Parent parent)
{
	lHalf.wideRegister(registry, Parent{parent.forkId, L_PRONG});
	rHalf.wideRegister(registry, Parent{parent.forkId, R_PRONG});
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
bool
CS_<TN, TA, SG, NI, TS...>::wideForwardEntryGuard(GuardControl& control,
												  const ShortIndex prong)
{
	HFSM_ASSERT(prong != INVALID_SHORT_INDEX);

	if (prong < R_PRONG)
		return lHalf.wideForwardEntryGuard(control, prong);
	else
		return rHalf.wideForwardEntryGuard(control, prong);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
bool
CS_<TN, TA, SG, NI, TS...>::wideEntryGuard(GuardControl& control,
										   const ShortIndex prong)
{
	HFSM_ASSERT(prong != INVALID_SHORT_INDEX);

	if (prong < R_PRONG)
		return lHalf.wideEntryGuard(control, prong);
	else
		return rHalf.wideEntryGuard(control, prong);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
void
CS_<TN, TA, SG, NI, TS...>::wideConstruct(PlanControl& control,
										  const ShortIndex prong)
{
	HFSM_ASSERT(prong != INVALID_SHORT_INDEX);

	if (prong < R_PRONG)
		lHalf.wideConstruct(control, prong);
	else
		rHalf.wideConstruct(control, prong);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
void
CS_<TN, TA, SG, NI, TS...>::wideEnter(PlanControl& control,
									  const ShortIndex prong)
{
	HFSM_ASSERT(prong != INVALID_SHORT_INDEX);

	if (prong < R_PRONG)
		lHalf.wideEnter(control, prong);
	else
		rHalf.wideEnter(control, prong);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
void
CS_<TN, TA, SG, NI, TS...>::wideReenter(PlanControl& control,
										const ShortIndex prong)
{
	HFSM_ASSERT(prong != INVALID_SHORT_INDEX);

	if (prong < R_PRONG)
		lHalf.wideReenter(control, prong);
	else
		rHalf.wideReenter(control, prong);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
Status
CS_<TN, TA, SG, NI, TS...>::wideUpdate(FullControl& control,
									   const ShortIndex prong)
{
	HFSM_ASSERT(prong != INVALID_SHORT_INDEX);

	return prong < R_PRONG ?
		lHalf.wideUpdate(control, prong) :
		rHalf.wideUpdate(control, prong);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
template <typename TEvent>
Status
CS_<TN, TA, SG, NI, TS...>::wideReact(FullControl& control,
									  const TEvent& event,
									  const ShortIndex prong)
{
	HFSM_ASSERT(prong != INVALID_SHORT_INDEX);

	return prong < R_PRONG ?
		lHalf.wideReact(control, event, prong) :
		rHalf.wideReact(control, event, prong);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
bool
CS_<TN, TA, SG, NI, TS...>::wideForwardExitGuard(GuardControl& control,
												 const ShortIndex prong)
{
	HFSM_ASSERT(prong != INVALID_SHORT_INDEX);

	if (prong < R_PRONG)
		return lHalf.wideForwardExitGuard(control, prong);
	else
		return rHalf.wideForwardExitGuard(control, prong);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
bool
CS_<TN, TA, SG, NI, TS...>::wideExitGuard(GuardControl& control,
										  const ShortIndex prong)
{
	HFSM_ASSERT(prong != INVALID_SHORT_INDEX);

	if (prong < R_PRONG)
		return lHalf.wideExitGuard(control, prong);
	else
		return rHalf.wideExitGuard(control, prong);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
void
CS_<TN, TA, SG, NI, TS...>::wideExit(PlanControl& control,
									 const ShortIndex prong)
{
	HFSM_ASSERT(prong != INVALID_SHORT_INDEX);

	if (prong < R_PRONG)
		lHalf.wideExit(control, prong);
	else
		rHalf.wideExit(control, prong);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
void
CS_<TN, TA, SG, NI, TS...>::wideDestruct(PlanControl& control,
										 const ShortIndex prong)
{
	HFSM_ASSERT(prong != INVALID_SHORT_INDEX);

	if (prong < R_PRONG)
		lHalf.wideDestruct(control, prong);
	else
		rHalf.wideDestruct(control, prong);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
void
CS_<TN, TA, SG, NI, TS...>::wideForwardActive(Control& control,
											  const Request::Type request,
											  const ShortIndex prong)
{
	HFSM_ASSERT(prong != INVALID_SHORT_INDEX);

	if (prong < R_PRONG)
		lHalf.wideForwardActive(control, request, prong);
	else
		rHalf.wideForwardActive(control, request, prong);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
void
CS_<TN, TA, SG, NI, TS...>::wideForwardRequest(Control& control,
											   const Request::Type request,
											   const ShortIndex prong)
{
	HFSM_ASSERT(prong != INVALID_SHORT_INDEX);

	if (prong < R_PRONG)
		lHalf.wideForwardRequest(control, request, prong);
	else
		rHalf.wideForwardRequest(control, request, prong);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
void
CS_<TN, TA, SG, NI, TS...>::wideRequestChangeComposite(Control& control) {
	lHalf.wideRequestChangeComposite(control);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
void
CS_<TN, TA, SG, NI, TS...>::wideRequestChangeResumable(Control& control,
													   const ShortIndex prong)
{
	HFSM_ASSERT(prong != INVALID_SHORT_INDEX);

	if (prong < R_PRONG)
		lHalf.wideRequestChangeResumable(control, prong);
	else
		rHalf.wideRequestChangeResumable(control, prong);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
void
CS_<TN, TA, SG, NI, TS...>::wideRequestRemain(Registry& registry) {
	lHalf.wideRequestRemain(registry);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
void
CS_<TN, TA, SG, NI, TS...>::wideRequestRestart(Registry& registry) {
	lHalf.wideRequestRestart(registry);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
void
CS_<TN, TA, SG, NI, TS...>::wideRequestResume(Registry& registry,
											  const ShortIndex prong)
{
	HFSM_ASSERT(prong != INVALID_SHORT_INDEX);

	if (prong < R_PRONG)
		lHalf.wideRequestResume(registry, prong);
	else
		rHalf.wideRequestResume(registry, prong);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
typename TA::UP
CS_<TN, TA, SG, NI, TS...>::wideReportUtilize(Control& control) {
	const UP l = lHalf.wideReportUtilize(control);
	const UP r = rHalf.wideReportUtilize(control);

	return l.utility >= r.utility ?
		l : r;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
typename TA::Rank
CS_<TN, TA, SG, NI, TS...>::wideReportRank(Control& control,
										   Rank* const ranks)
{
	HFSM_ASSERT(ranks);

	const Rank l = lHalf.wideReportRank(control, ranks);
	const Rank r = rHalf.wideReportRank(control, ranks + LStates::SIZE);

	return l > r?
		l : r;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
typename TA::Utility
CS_<TN, TA, SG, NI, TS...>::wideReportRandomize(Control& control,
												Utility* const options,
												const Rank* const ranks,
												const Rank top)
{
	HFSM_ASSERT(options && ranks);

	const Utility l = lHalf.wideReportRandomize(control, options,				  ranks,				 top);
	const Utility r = rHalf.wideReportRandomize(control, options + LStates::SIZE, ranks + LStates::SIZE, top);

	return { l + r };
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
typename TA::UP
CS_<TN, TA, SG, NI, TS...>::wideReportChangeComposite(Control& control) {
	return lHalf.wideReportChangeComposite(control);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
typename TA::UP
CS_<TN, TA, SG, NI, TS...>::wideReportChangeResumable(Control& control,
													  const ShortIndex prong)
{
	HFSM_ASSERT(prong != INVALID_SHORT_INDEX);

	if (prong < R_PRONG)
		return lHalf.wideReportChangeResumable(control, prong);
	else
		return rHalf.wideReportChangeResumable(control, prong);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
typename TA::UP
CS_<TN, TA, SG, NI, TS...>::wideReportChangeUtilitarian(Control& control) {
	const UP l = lHalf.wideReportChangeUtilitarian(control);
	const UP r = rHalf.wideReportChangeUtilitarian(control);

	return l.utility >= r.utility ?
		l : r;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
typename TA::Utility
CS_<TN, TA, SG, NI, TS...>::wideReportChangeRandom(Control& control,
												   Utility* const options,
												   const Rank* const ranks,
												   const Rank top)
{
	HFSM_ASSERT(options && ranks);

	const Utility l = lHalf.wideReportChangeRandom(control, options,				 ranks,					top);
	const Utility r = rHalf.wideReportChangeRandom(control, options + LStates::SIZE, ranks + LStates::SIZE, top);

	return { l + r };
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
void
CS_<TN, TA, SG, NI, TS...>::wideChangeToRequested(PlanControl& control,
												  const ShortIndex prong)
{
	HFSM_ASSERT(prong != INVALID_SHORT_INDEX);

	if (prong < R_PRONG)
		lHalf.wideChangeToRequested(control, prong);
	else
		rHalf.wideChangeToRequested(control, prong);
}

//------------------------------------------------------------------------------

#ifdef HFSM_ENABLE_STRUCTURE_REPORT

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
void
CS_<TN, TA, SG, NI, TS...>::wideGetNames(const LongIndex parent,
										 const RegionType /*region*/,
										 const ShortIndex depth,
										 StructureStateInfos& _stateInfos) const
{
	lHalf.wideGetNames(parent, StructureStateInfo::COMPOSITE, depth, _stateInfos);
	rHalf.wideGetNames(parent, StructureStateInfo::COMPOSITE, depth, _stateInfos);
}

#endif

//------------------------------------------------------------------------------

#ifdef HFSM_ENABLE_SERIALIZATION

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
void
CS_<TN, TA, SG, NI, TS...>::wideSaveActive(const Registry& registry,
										   WriteStream& stream,
										   const ShortIndex prong) const
{
	HFSM_ASSERT(prong != INVALID_SHORT_INDEX);

	if (prong < R_PRONG) {
		lHalf.wideSaveActive   (registry, stream, prong);
		rHalf.wideSaveResumable(registry, stream);
	} else {
		lHalf.wideSaveResumable(registry, stream);
		rHalf.wideSaveActive   (registry, stream, prong);
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
void
CS_<TN, TA, SG, NI, TS...>::wideSaveResumable(const Registry& registry,
											  WriteStream& stream) const
{
	lHalf.wideSaveResumable(registry, stream);
	rHalf.wideSaveResumable(registry, stream);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
void
CS_<TN, TA, SG, NI, TS...>::wideLoadRequested(Registry& registry,
											  ReadStream& stream,
											  const ShortIndex prong) const
{
	HFSM_ASSERT(prong != INVALID_SHORT_INDEX);

	if (prong < R_PRONG) {
		lHalf.wideLoadRequested(registry, stream, prong);
		rHalf.wideLoadResumable(registry, stream);
	} else {
		lHalf.wideLoadResumable(registry, stream);
		rHalf.wideLoadRequested(registry, stream, prong);
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename... TS>
void
CS_<TN, TA, SG, NI, TS...>::wideLoadResumable(Registry& registry,
											  ReadStream& stream) const
{
	lHalf.wideLoadResumable(registry, stream);
	rHalf.wideLoadResumable(registry, stream);
}

#endif

////////////////////////////////////////////////////////////////////////////////

}
}
namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
void
CS_<TN, TA, SG, NI, T>::wideRegister(Registry& registry,
									 const Parent parent)
{
	state.deepRegister(registry, Parent{parent.forkId, PRONG_INDEX});
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
bool
CS_<TN, TA, SG, NI, T>::wideForwardEntryGuard(GuardControl& control,
											  const ShortIndex HFSM_IF_ASSERT(prong))
{
	HFSM_ASSERT(prong == PRONG_INDEX);

	return state.deepForwardEntryGuard(control);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
bool
CS_<TN, TA, SG, NI, T>::wideEntryGuard(GuardControl& control,
									   const ShortIndex HFSM_IF_ASSERT(prong))
{
	HFSM_ASSERT(prong == PRONG_INDEX);

	return state.deepEntryGuard(control);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
void
CS_<TN, TA, SG, NI, T>::wideConstruct(PlanControl& control,
									  const ShortIndex HFSM_IF_ASSERT(prong))
{
	HFSM_ASSERT(prong == PRONG_INDEX);

	state.deepConstruct(control);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
void
CS_<TN, TA, SG, NI, T>::wideEnter(PlanControl& control,
								  const ShortIndex HFSM_IF_ASSERT(prong))
{
	HFSM_ASSERT(prong == PRONG_INDEX);

	state.deepEnter(control);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
void
CS_<TN, TA, SG, NI, T>::wideReenter(PlanControl& control,
									const ShortIndex HFSM_IF_ASSERT(prong))
{
	HFSM_ASSERT(prong == PRONG_INDEX);

	state.deepReenter(control);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
Status
CS_<TN, TA, SG, NI, T>::wideUpdate(FullControl& control,
								   const ShortIndex HFSM_IF_ASSERT(prong))
{
	HFSM_ASSERT(prong == PRONG_INDEX);

	return state.deepUpdate(control);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
template <typename TEvent>
Status
CS_<TN, TA, SG, NI, T>::wideReact(FullControl& control,
								  const TEvent& event,
								  const ShortIndex HFSM_IF_ASSERT(prong))
{
	HFSM_ASSERT(prong == PRONG_INDEX);

	return state.deepReact(control, event);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
bool
CS_<TN, TA, SG, NI, T>::wideForwardExitGuard(GuardControl& control,
											 const ShortIndex HFSM_IF_ASSERT(prong))
{
	HFSM_ASSERT(prong == PRONG_INDEX);

	return state.deepForwardExitGuard(control);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
bool
CS_<TN, TA, SG, NI, T>::wideExitGuard(GuardControl& control,
									  const ShortIndex HFSM_IF_ASSERT(prong))
{
	HFSM_ASSERT(prong == PRONG_INDEX);

	return state.deepExitGuard(control);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
void
CS_<TN, TA, SG, NI, T>::wideExit(PlanControl& control,
								 const ShortIndex HFSM_IF_ASSERT(prong))
{
	HFSM_ASSERT(prong == PRONG_INDEX);

	state.deepExit(control);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
void
CS_<TN, TA, SG, NI, T>::wideDestruct(PlanControl& control,
									 const ShortIndex HFSM_IF_ASSERT(prong))
{
	HFSM_ASSERT(prong == PRONG_INDEX);

	state.deepDestruct(control);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
void
CS_<TN, TA, SG, NI, T>::wideForwardActive(Control& control,
										  const Request::Type request,
										  const ShortIndex HFSM_IF_ASSERT(prong))
{
	HFSM_ASSERT(prong == PRONG_INDEX);

	state.deepForwardActive(control, request);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
void
CS_<TN, TA, SG, NI, T>::wideForwardRequest(Control& control,
										   const Request::Type request,
										   const ShortIndex HFSM_IF_ASSERT(prong))
{
	HFSM_ASSERT(prong == PRONG_INDEX);

	state.deepForwardRequest(control, request);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
void
CS_<TN, TA, SG, NI, T>::wideRequestChangeComposite(Control& control) {
	state.deepRequestChange(control);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
void
CS_<TN, TA, SG, NI, T>::wideRequestChangeResumable(Control& control,
												   const ShortIndex HFSM_IF_ASSERT(prong))
{
	HFSM_ASSERT(prong == PRONG_INDEX);

	state.deepRequestChange(control);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
void
CS_<TN, TA, SG, NI, T>::wideRequestRemain(Registry& registry) {
	state.deepRequestRemain(registry);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
void
CS_<TN, TA, SG, NI, T>::wideRequestRestart(Registry& registry) {
	state.deepRequestRestart(registry);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
void
CS_<TN, TA, SG, NI, T>::wideRequestResume(Registry& registry,
										  const ShortIndex HFSM_IF_ASSERT(prong))
{
	HFSM_ASSERT(prong == PRONG_INDEX);

	state.deepRequestResume(registry);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
typename TA::UP
CS_<TN, TA, SG, NI, T>::wideReportUtilize(Control& control) {
	return state.deepReportUtilize(control);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
typename TA::Rank
CS_<TN, TA, SG, NI, T>::wideReportRank(Control& control,
									   Rank* const ranks)
{
	HFSM_ASSERT(ranks);

	*ranks = state.deepReportRank(control);

	return *ranks;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
typename TA::Utility
CS_<TN, TA, SG, NI, T>::wideReportRandomize(Control& control,
											Utility* const options,
											const Rank* const ranks,
											const Rank top)
{
	HFSM_ASSERT(options && ranks);

	*options = (*ranks == top) ?
		state.deepReportRandomize(control) : Utility{0.0f};

	return *options;
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
typename TA::UP
CS_<TN, TA, SG, NI, T>::wideReportChangeComposite(Control& control) {
	return state.deepReportChange(control);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
typename TA::UP
CS_<TN, TA, SG, NI, T>::wideReportChangeResumable(Control& control,
												  const ShortIndex HFSM_IF_ASSERT(prong))
{
	HFSM_ASSERT(prong == PRONG_INDEX);

	return state.deepReportChange(control);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
typename TA::UP
CS_<TN, TA, SG, NI, T>::wideReportChangeUtilitarian(Control& control) {
	return state.deepReportChange(control);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
typename TA::Utility
CS_<TN, TA, SG, NI, T>::wideReportChangeRandom(Control& control,
											   Utility* const options,
											   const Rank* const ranks,
											   const Rank top)
{
	HFSM_ASSERT(options && ranks);

	*options = (*ranks == top) ?
		state.deepReportChange(control).utility : Utility{0.0f};

	return *options;
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
void
CS_<TN, TA, SG, NI, T>::wideChangeToRequested(PlanControl& control,
											  const ShortIndex HFSM_IF_ASSERT(prong))
{
	HFSM_ASSERT(prong == PRONG_INDEX);

	state.deepChangeToRequested(control);
}

//------------------------------------------------------------------------------

#ifdef HFSM_ENABLE_STRUCTURE_REPORT

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
void
CS_<TN, TA, SG, NI, T>::wideGetNames(const LongIndex parent,
									 const RegionType /*region*/,
									 const ShortIndex depth,
									 StructureStateInfos& _stateInfos) const
{
	state.deepGetNames(parent, StructureStateInfo::COMPOSITE, depth, _stateInfos);
}

#endif

//------------------------------------------------------------------------------

#ifdef HFSM_ENABLE_SERIALIZATION

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
void
CS_<TN, TA, SG, NI, T>::wideSaveActive(const Registry& registry,
									   WriteStream& stream,
									   const ShortIndex HFSM_IF_ASSERT(prong)) const
{
	HFSM_ASSERT(prong == PRONG_INDEX);

	state.deepSaveActive(registry, stream);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
void
CS_<TN, TA, SG, NI, T>::wideSaveResumable(const Registry& registry,
										  WriteStream& stream) const
{
	state.deepSaveResumable(registry, stream);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
void
CS_<TN, TA, SG, NI, T>::wideLoadRequested(Registry& registry,
										  ReadStream& stream,
										  const ShortIndex HFSM_IF_ASSERT(prong)) const
{
	HFSM_ASSERT(prong == PRONG_INDEX);

	state.deepLoadRequested(registry, stream);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, ShortIndex NI, typename T>
void
CS_<TN, TA, SG, NI, T>::wideLoadResumable(Registry& registry,
										  ReadStream& stream) const
{
	state.deepLoadResumable(registry, stream);
}

#endif

////////////////////////////////////////////////////////////////////////////////

}
}
namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

template <typename TIndices,
		  typename TArgs,
		  Strategy TStrategy,
		  typename THead,
		  typename... TSubStates>
struct C_ final {
	using Indices		= TIndices;
	static constexpr StateID	HEAD_ID		= Indices::STATE_ID;
	static constexpr ShortIndex COMPO_INDEX	= Indices::COMPO_INDEX;
	static constexpr ShortIndex ORTHO_INDEX	= Indices::ORTHO_INDEX;
	static constexpr ShortIndex ORTHO_UNIT	= Indices::ORTHO_UNIT;

	static constexpr Strategy	STRATEGY	= TStrategy;

	static constexpr ShortIndex REGION_ID	= COMPO_INDEX + ORTHO_INDEX;
	static constexpr ForkID		COMPO_ID	= COMPO_INDEX + 1;

	using Args			= TArgs;
	using Rank			= typename Args::Rank;
	using Utility		= typename Args::Utility;
	using UP			= typename Args::UP;
	using StateList		= typename Args::StateList;
	using RegionList	= typename Args::RegionList;

	using Registry		= RegistryT<Args>;
	using StateParents	= typename Registry::StateParents;

	using Control		= ControlT<Args>;

	using PlanControl	= PlanControlT<Args>;
	using ScopedOrigin	= typename PlanControl::Origin;
	using ScopedRegion	= typename PlanControl::Region;
	using Plan			= PlanT<Args>;

	using FullControl	= FullControlT<Args>;
	using ControlLock	= typename FullControl::Lock;

	using GuardControl	= GuardControlT<Args>;

	using Head			= THead;

	using HeadState		= S_<Indices, Args, Head>;

	using SubStates		= CS_<I_<HEAD_ID + 1,
								 COMPO_INDEX + 1,
								 ORTHO_INDEX,
								 ORTHO_UNIT>,
							  Args,
							  STRATEGY,
							  0,
							  TSubStates...>;

	using Info			= CI_<STRATEGY, Head, TSubStates...>;
	static constexpr ShortIndex WIDTH		= Info::WIDTH;
	static constexpr ShortIndex WIDTH_BITS	= Info::WIDTH_BITS;
	static constexpr ShortIndex STATE_COUNT	= Info::STATE_COUNT;

	//----------------------------------------------------------------------

#ifdef HFSM_EXPLICIT_MEMBER_SPECIALIZATION
	template <typename T>
	struct Accessor {
		HFSM_INLINE static		 T&	   get(		 C_& c)								{ return c._subStates.template access<T>();		}
		HFSM_INLINE static const T&	   get(const C_& c)								{ return c._subStates.template access<T>();		}
	};

	template <>
	struct Accessor<Head> {
		HFSM_INLINE static		 Head& get(		 C_& c)								{ return c._headState._headBox.get();			}
		HFSM_INLINE static const Head& get(const C_& c)								{ return c._headState._headBox.get();			}
	};

	template <typename T>
	HFSM_INLINE		  T&	access()												{ return Accessor<T>::get(*this);				}

	template <typename T>
	HFSM_INLINE const T&	access() const											{ return Accessor<T>::get(*this);				}
#endif

	//----------------------------------------------------------------------

	HFSM_INLINE		  ShortIndex& compoRequested(	   Registry& registry) const	{ return registry.compoRequested[COMPO_INDEX];	}
	HFSM_INLINE const ShortIndex& compoRequested(const Registry& registry) const	{ return registry.compoRequested[COMPO_INDEX];	}

	HFSM_INLINE		  ShortIndex& compoActive   (	   Registry& registry) const	{ return registry.compoActive	[COMPO_INDEX];	}
	HFSM_INLINE const ShortIndex& compoActive   (const Registry& registry) const	{ return registry.compoActive	[COMPO_INDEX];	}

	HFSM_INLINE		  ShortIndex& compoResumable(	   Registry& registry) const	{ return registry.compoResumable[COMPO_INDEX];	}
	HFSM_INLINE const ShortIndex& compoResumable(const Registry& registry) const	{ return registry.compoResumable[COMPO_INDEX];	}

	HFSM_INLINE		  ShortIndex& compoRequested(	   Control& control) const		{ return compoRequested(control._registry);		}
	HFSM_INLINE const ShortIndex& compoRequested(const Control& control) const		{ return compoRequested(control._registry);		}

	HFSM_INLINE		  ShortIndex& compoActive   (	   Control& control) const		{ return compoActive   (control._registry);		}
	HFSM_INLINE const ShortIndex& compoActive   (const Control& control) const		{ return compoActive   (control._registry);		}

	HFSM_INLINE		  ShortIndex& compoResumable(	   Control& control) const		{ return compoResumable(control._registry);		}
	HFSM_INLINE const ShortIndex& compoResumable(const Control& control) const		{ return compoResumable(control._registry);		}

	HFSM_INLINE		  ShortIndex  resolveRandom (Control& control,
												 const Utility(& options)[Info::WIDTH], const Utility sum,
												 const Rank	  (& ranks)  [Info::WIDTH], const Rank	  top) const;

	HFSM_INLINE bool	compoRemain					  (Control& control)	{ return control._registry.compoRemains.template get<COMPO_INDEX>(); }

	HFSM_INLINE void	deepRegister				  (Registry& registry, const Parent parent);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE bool	deepForwardEntryGuard		  (GuardControl& control);
	HFSM_INLINE bool	deepEntryGuard				  (GuardControl& control);

	HFSM_INLINE void	deepConstruct				  (PlanControl& control);

	HFSM_INLINE void	deepEnter					  (PlanControl&  control);
	HFSM_INLINE void	deepReenter					  (PlanControl&  control);

	HFSM_INLINE Status	deepUpdate					  (FullControl&  control);

	template <typename TEvent>
	HFSM_INLINE Status	deepReact					  (FullControl&  control, const TEvent& event);

	HFSM_INLINE bool	deepForwardExitGuard		  (GuardControl& control);
	HFSM_INLINE bool	deepExitGuard				  (GuardControl& control);

	HFSM_INLINE void	deepExit					  (PlanControl&  control);

	HFSM_INLINE void	deepDestruct				  (PlanControl&  control);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void	deepForwardActive			  (Control& control, const Request::Type request);
	HFSM_INLINE void	deepForwardRequest			  (Control& control, const Request::Type request);

	HFSM_INLINE void	deepRequest					  (Control& control, const Request::Type request);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef HFSM_EXPLICIT_MEMBER_SPECIALIZATION

	template <Strategy = STRATEGY>
	HFSM_INLINE void	deepRequestChange			  (Control& control, const ShortIndex = INVALID_SHORT_INDEX);

	template <>
	HFSM_INLINE void	deepRequestChange<Composite>  (Control& control, const ShortIndex)	{ deepRequestChangeComposite  (control); }

	template <>
	HFSM_INLINE void	deepRequestChange<Resumable>  (Control& control, const ShortIndex)	{ deepRequestChangeResumable  (control); }

	template <>
	HFSM_INLINE void	deepRequestChange<Utilitarian>(Control& control, const ShortIndex)	{ deepRequestChangeUtilitarian(control); }

	template <>
	HFSM_INLINE void	deepRequestChange<RandomUtil> (Control& control, const ShortIndex)	{ deepRequestChangeRandom	  (control); }

#else

	HFSM_INLINE void	deepRequestChange			  (Control& control);

#endif

	HFSM_INLINE void	deepRequestChangeComposite	  (Control& control);
	HFSM_INLINE void	deepRequestChangeResumable	  (Control& control);
	HFSM_INLINE void	deepRequestChangeUtilitarian  (Control& control);
	HFSM_INLINE void	deepRequestChangeRandom		  (Control& control);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void	deepRequestRemain			  (Registry& registry);
	HFSM_INLINE void	deepRequestRestart			  (Registry& registry);
	HFSM_INLINE void	deepRequestResume			  (Registry& registry);
	HFSM_INLINE void	deepRequestUtilize			  (Control& control);
	HFSM_INLINE void	deepRequestRandomize		  (Control& control);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef HFSM_EXPLICIT_MEMBER_SPECIALIZATION

	template <Strategy = STRATEGY>
	HFSM_INLINE UP		deepReportChange			  (Control& control);

	template <>
	HFSM_INLINE UP		deepReportChange<Composite>   (Control& control)	{ return deepReportChangeComposite  (control); }

	template <>
	HFSM_INLINE UP		deepReportChange<Resumable>   (Control& control)	{ return deepReportChangeResumable  (control); }

	template <>
	HFSM_INLINE UP		deepReportChange<Utilitarian> (Control& control)	{ return deepReportChangeUtilitarian(control); }

	template <>
	HFSM_INLINE UP		deepReportChange<RandomUtil>  (Control& control)	{ return deepReportChangeRandom		(control); }

#else

	HFSM_INLINE UP		deepReportChange			  (Control& control);

#endif

	HFSM_INLINE UP		deepReportChangeComposite	  (Control& control);
	HFSM_INLINE UP		deepReportChangeResumable	  (Control& control);
	HFSM_INLINE UP		deepReportChangeUtilitarian   (Control& control);
	HFSM_INLINE UP		deepReportChangeRandom		  (Control& control);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE UP		deepReportUtilize			  (Control& control);
	HFSM_INLINE Rank	deepReportRank				  (Control& control);
	HFSM_INLINE Utility	deepReportRandomize			  (Control& control);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void	deepChangeToRequested		  (PlanControl& control);

	//----------------------------------------------------------------------

#ifdef HFSM_ENABLE_STRUCTURE_REPORT
	using StructureStateInfos = typename Args::StructureStateInfos;
	using RegionType		  = typename StructureStateInfo::RegionType;

	static constexpr LongIndex NAME_COUNT = HeadState::NAME_COUNT + SubStates::NAME_COUNT;

	void deepGetNames(const LongIndex parent,
					  const RegionType region,
					  const ShortIndex depth,
					  StructureStateInfos& stateInfos) const;
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef HFSM_ENABLE_SERIALIZATION
	using WriteStream	= typename Args::WriteStream;
	using ReadStream	= typename Args::ReadStream;

	HFSM_INLINE void	deepSaveActive	 (const Registry& registry, WriteStream& stream) const;
	HFSM_INLINE void	deepSaveResumable(const Registry& registry, WriteStream& stream) const;

	HFSM_INLINE void	deepLoadRequested(		Registry& registry, ReadStream&  stream) const;
	HFSM_INLINE void	deepLoadResumable(		Registry& registry, ReadStream&  stream) const;
#endif

	//----------------------------------------------------------------------

	HeadState _headState;
	SubStates _subStates;

	HFSM_IF_DEBUG(static constexpr Info _info = Info{});
};

////////////////////////////////////////////////////////////////////////////////

}
}

namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
ShortIndex
C_<TN, TA, SG, TH, TS...>::resolveRandom(Control& control,
										 const Utility(& options)[Info::WIDTH],
										 const Utility sum,
										 const Rank(& ranks)[Info::WIDTH],
										 const Rank top) const
{
	const Utility random = control._rng.next();
	HFSM_ASSERT(0.0f <= random && random < 1.0f);

	Utility cursor = random * sum;

	for (ShortIndex i = 0; i < count(ranks); ++i)
		if (ranks[i] == top) {
			HFSM_ASSERT(options[i] >= 0.0f);

			if (cursor >= options[i])
				cursor -= options[i];
			else {
				HFSM_LOG_RANDOM_RESOLUTION(control.context(), HEAD_ID, i, random);

				return i;
			}
		}

	HFSM_BREAK();
	return INVALID_SHORT_INDEX;
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
void
C_<TN, TA, SG, TH, TS...>::deepRegister(Registry& registry,
										const Parent parent)
{
	registry.compoParents[COMPO_INDEX] = parent;

	_headState.deepRegister(registry, parent);
	_subStates.wideRegister(registry, Parent{COMPO_ID});
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
bool
C_<TN, TA, SG, TH, TS...>::deepForwardEntryGuard(GuardControl& control) {
	const ShortIndex active	   = compoActive   (control);
	const ShortIndex requested = compoRequested(control);

	HFSM_ASSERT(active != INVALID_SHORT_INDEX);

	ScopedRegion region{control, REGION_ID, HEAD_ID, STATE_COUNT};

	if (requested == INVALID_SHORT_INDEX)
		return _subStates.wideForwardEntryGuard(control, active);
	else
		return _subStates.wideEntryGuard	   (control, requested);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
bool
C_<TN, TA, SG, TH, TS...>::deepEntryGuard(GuardControl& control) {
	const ShortIndex requested = compoRequested(control);
	HFSM_ASSERT(requested != INVALID_SHORT_INDEX);

	ScopedRegion region{control, REGION_ID, HEAD_ID, STATE_COUNT};

	return _headState.deepEntryGuard(control) ||
		   _subStates.wideEntryGuard(control, requested);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
void
C_<TN, TA, SG, TH, TS...>::deepConstruct(PlanControl& control) {
	ShortIndex& active	  = compoActive   (control);
	ShortIndex& resumable = compoResumable(control);
	ShortIndex& requested = compoRequested(control);

	HFSM_ASSERT(active	  == INVALID_SHORT_INDEX);
	HFSM_ASSERT(requested != INVALID_SHORT_INDEX);

	active	  = requested;

	if (requested == resumable)
		resumable = INVALID_SHORT_INDEX;

	requested = INVALID_SHORT_INDEX;

	_headState.deepConstruct(control);
	_subStates.wideConstruct(control, active);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
void
C_<TN, TA, SG, TH, TS...>::deepEnter(PlanControl& control) {
	const ShortIndex& active = compoActive(control);

	ScopedRegion region{control, REGION_ID, HEAD_ID, STATE_COUNT};

	_headState.deepEnter(control);
	_subStates.wideEnter(control, active);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
void
C_<TN, TA, SG, TH, TS...>::deepReenter(PlanControl& control) {
	ShortIndex& active	  = compoActive   (control);
	ShortIndex& resumable = compoResumable(control);
	ShortIndex& requested = compoRequested(control);

	HFSM_ASSERT(active	  != INVALID_SHORT_INDEX &&
				requested != INVALID_SHORT_INDEX);

	ScopedRegion region{control, REGION_ID, HEAD_ID, STATE_COUNT};

	_headState.deepReenter(control);

	if (active == requested)
		_subStates.wideReenter(control, active);
	else {
		_subStates.wideExit	  (control, active);

		active	  = requested;

		if (requested == resumable)
			resumable = INVALID_SHORT_INDEX;

		_subStates.wideEnter  (control, active);
	}

	requested = INVALID_SHORT_INDEX;
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
Status
C_<TN, TA, SG, TH, TS...>::deepUpdate(FullControl& control) {
	const ShortIndex active = compoActive(control);
	HFSM_ASSERT(active != INVALID_SHORT_INDEX);

	ScopedRegion outer{control, REGION_ID, HEAD_ID, STATE_COUNT};

	if (const Status headStatus = _headState.deepUpdate(control)) {
		ControlLock lock{control};
		_subStates.wideUpdate(control, active);

		return headStatus;
	} else {
		const Status subStatus = _subStates.wideUpdate(control, active);

		if (subStatus.outerTransition)
			return Status{Status::NONE, true};

		ScopedRegion inner{control, REGION_ID, HEAD_ID, STATE_COUNT};

		return subStatus && control._planData.planExists.template get<REGION_ID>() ?
			control.updatePlan(_headState, subStatus) : subStatus;
	}
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
template <typename TEvent>
Status
C_<TN, TA, SG, TH, TS...>::deepReact(FullControl& control,
									 const TEvent& event)
{
	const ShortIndex active = compoActive(control);
	HFSM_ASSERT(active != INVALID_SHORT_INDEX);

	ScopedRegion outer{control, REGION_ID, HEAD_ID, STATE_COUNT};

	if (const Status headStatus = _headState.deepReact(control, event)) {
		ControlLock lock{control};
		_subStates.wideReact(control, event, active);

		return headStatus;
	} else {
		const Status subStatus = _subStates.wideReact(control, event, active);

		if (subStatus.outerTransition)
			return subStatus;

		ScopedRegion inner{control, REGION_ID, HEAD_ID, STATE_COUNT};

		return subStatus && control._planData.planExists.template get<REGION_ID>() ?
			control.updatePlan(_headState, subStatus) : subStatus;
	}
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
bool
C_<TN, TA, SG, TH, TS...>::deepForwardExitGuard(GuardControl& control) {
	const ShortIndex active = compoActive(control);
	HFSM_ASSERT(active != INVALID_SHORT_INDEX);

	ScopedRegion region{control, REGION_ID, HEAD_ID, STATE_COUNT};

	if (compoRequested(control) == INVALID_SHORT_INDEX)
		return _subStates.wideForwardExitGuard(control, active);
	else
		return _subStates.wideExitGuard		  (control, active);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
bool
C_<TN, TA, SG, TH, TS...>::deepExitGuard(GuardControl& control) {
	const ShortIndex active = compoActive(control);
	HFSM_ASSERT(active != INVALID_SHORT_INDEX);

	ScopedRegion region{control, REGION_ID, HEAD_ID, STATE_COUNT};

	return _headState.deepExitGuard(control) ||
		   _subStates.wideExitGuard(control, active);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
void
C_<TN, TA, SG, TH, TS...>::deepExit(PlanControl& control) {
	ShortIndex& active	  = compoActive   (control);
	HFSM_ASSERT(active != INVALID_SHORT_INDEX);

	_subStates.wideExit(control, active);
	_headState.deepExit(control);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
void
C_<TN, TA, SG, TH, TS...>::deepDestruct(PlanControl& control) {
	ShortIndex& active	  = compoActive   (control);
	ShortIndex& resumable = compoResumable(control);

	HFSM_ASSERT(active != INVALID_SHORT_INDEX);

	_subStates.wideDestruct(control, active);
	_headState.deepDestruct(control);

	resumable = active;
	active	  = INVALID_SHORT_INDEX;

	auto plan = control.plan(REGION_ID);
	plan.clear();
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
void
C_<TN, TA, SG, TH, TS...>::deepForwardActive(Control& control,
											 const Request::Type request)
{
	HFSM_ASSERT(control._registry.isActive(HEAD_ID));

	const ShortIndex requested = compoRequested(control);

	if (requested == INVALID_SHORT_INDEX) {
		const ShortIndex active = compoActive(control);

		_subStates.wideForwardActive (control, request, active);
	} else
		_subStates.wideForwardRequest(control, request, requested);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
void
C_<TN, TA, SG, TH, TS...>::deepForwardRequest(Control& control,
											  const Request::Type request)
{
	const ShortIndex requested = compoRequested(control);

	if (requested == INVALID_SHORT_INDEX)
		deepRequest					 (control, request);
	else
		_subStates.wideForwardRequest(control, request, requested);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
void
C_<TN, TA, SG, TH, TS...>::deepRequest(Control& control,
									   const Request::Type request)
{
	switch (request) {
	case Request::REMAIN:
		deepRequestRemain	(control._registry);
		break;

	case Request::CHANGE:
		deepRequestChange	(control);
		break;

	case Request::RESTART:
		deepRequestRestart	(control._registry);
		break;

	case Request::RESUME:
		deepRequestResume	(control._registry);
		break;

	case Request::UTILIZE:
		deepRequestUtilize	(control);
		break;

	case Request::RANDOMIZE:
		deepRequestRandomize(control);
		break;

	default:
		HFSM_BREAK();
	}
}

//------------------------------------------------------------------------------

#ifndef HFSM_EXPLICIT_MEMBER_SPECIALIZATION

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
void
C_<TN, TA, SG, TH, TS...>::deepRequestChange(Control& control)
{
	switch (STRATEGY) {
	case Strategy::Composite:
		deepRequestChangeComposite  (control);
		break;

	case Strategy::Resumable:
		deepRequestChangeResumable  (control);
		break;

	case Strategy::Utilitarian:
		deepRequestChangeUtilitarian(control);
		break;

	case Strategy::RandomUtil:
		deepRequestChangeRandom		(control);
		break;

	default:
		HFSM_BREAK();
	}
}

#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
void
C_<TN, TA, SG, TH, TS...>::deepRequestChangeComposite(Control& control) {
	ShortIndex& requested = compoRequested(control);

	requested = 0;

	_subStates.wideRequestChangeComposite(control);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
void
C_<TN, TA, SG, TH, TS...>::deepRequestChangeResumable(Control& control) {
	const ShortIndex  resumable = compoResumable(control);
		  ShortIndex& requested = compoRequested(control);

	requested = (resumable != INVALID_SHORT_INDEX) ?
		resumable : 0;

	_subStates.wideRequestChangeResumable(control, requested);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
void
C_<TN, TA, SG, TH, TS...>::deepRequestChangeUtilitarian(Control& control) {
	const UP s = _subStates.wideReportChangeUtilitarian(control);
	HFSM_ASSERT(s.prong != INVALID_SHORT_INDEX);

	ShortIndex& requested = compoRequested(control);
	requested = s.prong;

	HFSM_LOG_UTILITY_RESOLUTION(control.context(), HEAD_ID, requested, s.utility);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
void
C_<TN, TA, SG, TH, TS...>::deepRequestChangeRandom(Control& control) {
	Rank ranks[Info::WIDTH];
	Rank top = _subStates.wideReportRank(control, ranks);

	Utility options[Info::WIDTH];
	const UP sum = _subStates.wideReportChangeRandom(control, options, ranks, top);

	ShortIndex& requested = compoRequested(control);
	requested = resolveRandom(control, options, sum.utility, ranks, top);
	HFSM_ASSERT(requested < Info::WIDTH);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
void
C_<TN, TA, SG, TH, TS...>::deepRequestRemain(Registry& registry) {
	const ShortIndex  active	= compoActive   (registry);
		  ShortIndex& requested = compoRequested(registry);

	if (active == INVALID_SHORT_INDEX)
		requested = 0;

	_subStates.wideRequestRemain(registry);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
void
C_<TN, TA, SG, TH, TS...>::deepRequestRestart(Registry& registry) {
	ShortIndex& requested = compoRequested(registry);

	requested = 0;

	_subStates.wideRequestRestart(registry);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
void
C_<TN, TA, SG, TH, TS...>::deepRequestResume(Registry& registry) {
	const ShortIndex  resumable = compoResumable(registry);
		  ShortIndex& requested = compoRequested(registry);

	requested = (resumable != INVALID_SHORT_INDEX) ?
		resumable : 0;

	_subStates.wideRequestResume(registry, requested);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
void
C_<TN, TA, SG, TH, TS...>::deepRequestUtilize(Control& control) {
	const UP s = _subStates.wideReportUtilize(control);

	ShortIndex& requested = compoRequested(control);
	requested = s.prong;

	HFSM_LOG_UTILITY_RESOLUTION(control.context(), HEAD_ID, requested, s.utility);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
void
C_<TN, TA, SG, TH, TS...>::deepRequestRandomize(Control& control) {
	Rank ranks[Info::WIDTH];
	Rank top = _subStates.wideReportRank(control, ranks);

	Utility options[Info::WIDTH];
	const Utility sum = _subStates.wideReportRandomize(control, options, ranks, top);

	ShortIndex& requested = compoRequested(control);
	requested = resolveRandom(control, options, sum, ranks, top);
	HFSM_ASSERT(requested < Info::WIDTH);
}

//------------------------------------------------------------------------------

#ifndef HFSM_EXPLICIT_MEMBER_SPECIALIZATION

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
typename TA::UP
C_<TN, TA, SG, TH, TS...>::deepReportChange(Control& control) {
	switch (STRATEGY) {
	case Strategy::Composite:
		return deepReportChangeComposite  (control);

	case Strategy::Resumable:
		return deepReportChangeResumable  (control);

	case Strategy::Utilitarian:
		return deepReportChangeUtilitarian(control);

	case Strategy::RandomUtil:
		return deepReportChangeRandom	  (control);

	default:
		HFSM_BREAK();
		return {};
	}
}

#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
typename TA::UP
C_<TN, TA, SG, TH, TS...>::deepReportChangeComposite(Control& control) {
	ShortIndex& requested = compoRequested(control);
	requested = 0;

	const UP h = _headState.deepReportChange(control);
	const UP s = _subStates.wideReportChangeComposite(control);

	return {
		h.utility * s.utility,
		h.prong
	};
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
typename TA::UP
C_<TN, TA, SG, TH, TS...>::deepReportChangeResumable(Control& control) {
	const ShortIndex  resumable = compoResumable(control);
		  ShortIndex& requested = compoRequested(control);

	requested = (resumable != INVALID_SHORT_INDEX) ?
		resumable : 0;

	const UP h = _headState.deepReportChange(control);
	const UP s = _subStates.wideReportChangeResumable(control, requested);

	return {
		h.utility * s.utility,
		h.prong
	};
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
typename TA::UP
C_<TN, TA, SG, TH, TS...>::deepReportChangeUtilitarian(Control& control) {
	const UP h = _headState.deepReportChange(control);
	const UP s = _subStates.wideReportChangeUtilitarian(control);

	ShortIndex& requested = compoRequested(control);
	requested = s.prong;

	HFSM_LOG_UTILITY_RESOLUTION(control.context(), HEAD_ID, requested, s.utility);

	return {
		h.utility * s.utility,
		h.prong
	};
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
typename TA::UP
C_<TN, TA, SG, TH, TS...>::deepReportChangeRandom(Control& control) {
	const UP h = _headState.deepReportChange(control);

	Rank ranks[Info::WIDTH];
	Rank top = _subStates.wideReportRank(control, ranks);

	Utility options[Info::WIDTH];
	const UP sum = _subStates.wideReportChangeRandom(control, options, ranks, top);

	ShortIndex& requested = compoRequested(control);
	requested = resolveRandom(control, options, sum.utility, ranks, top);
	HFSM_ASSERT(requested < Info::WIDTH);

	return {
		h.utility * options[requested],
		h.prong
	};
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
typename TA::UP
C_<TN, TA, SG, TH, TS...>::deepReportUtilize(Control& control) {
	const UP h = _headState.deepReportUtilize(control);
	const UP s = _subStates.wideReportUtilize(control);

	ShortIndex& requested = compoRequested(control);
	requested = s.prong;

	HFSM_LOG_UTILITY_RESOLUTION(control.context(), HEAD_ID, requested, s.utility);

	return {
		h.utility * s.utility,
		h.prong
	};
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
typename TA::Rank
C_<TN, TA, SG, TH, TS...>::deepReportRank(Control& control) {
	return _headState.wrapRank(control);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
typename TA::Utility
C_<TN, TA, SG, TH, TS...>::deepReportRandomize(Control& control) {
	const Utility h = _headState.wrapUtility(control);

	Rank ranks[Info::WIDTH];
	Rank top = _subStates.wideReportRank(control, ranks);

	Utility options[Info::WIDTH];
	const Utility sum = _subStates.wideReportRandomize(control, options, ranks, top);

	ShortIndex& requested = compoRequested(control);
	requested = resolveRandom(control, options, sum, ranks, top);
	HFSM_ASSERT(requested < Info::WIDTH);

	return h * options[requested];
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
void
C_<TN, TA, SG, TH, TS...>::deepChangeToRequested(PlanControl& control) {
	ShortIndex& active	  = compoActive	  (control);
	ShortIndex& resumable = compoResumable(control);
	ShortIndex& requested = compoRequested(control);

	HFSM_ASSERT(active != INVALID_SHORT_INDEX);

	if (requested == INVALID_SHORT_INDEX)
		_subStates.wideChangeToRequested(control, active);
	else if (requested != active) {
		_subStates.wideExit		(control, active);
		_subStates.wideDestruct	(control, active);

		resumable = active;
		active	  = requested;
		requested = INVALID_SHORT_INDEX;

		_subStates.wideConstruct(control, active);
		_subStates.wideEnter	(control, active);
	} else if (compoRemain(control)) {
		_subStates.wideExit		(control, active);
		_subStates.wideDestruct	(control, active);

		requested = INVALID_SHORT_INDEX;

		_subStates.wideConstruct(control, active);
		_subStates.wideEnter	(control, active);
	} else {
		requested = INVALID_SHORT_INDEX;

		// no reconstruction on reenter() by design

		_subStates.wideReenter	(control, active);
	}
}

//------------------------------------------------------------------------------

#ifdef HFSM_ENABLE_STRUCTURE_REPORT

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
void
C_<TN, TA, SG, TH, TS...>::deepGetNames(const LongIndex parent,
										const RegionType region,
										const ShortIndex depth,
										StructureStateInfos& _stateInfos) const
{
	_headState.deepGetNames(parent,					 region,						depth,	   _stateInfos);
	_subStates.wideGetNames(_stateInfos.count() - 1, StructureStateInfo::COMPOSITE, depth + 1, _stateInfos);
}

#endif

//------------------------------------------------------------------------------

#ifdef HFSM_ENABLE_SERIALIZATION

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
void
C_<TN, TA, SG, TH, TS...>::deepSaveActive(const Registry& registry,
										  WriteStream& stream) const
{
	const ShortIndex active	   = compoActive   (registry);
	const ShortIndex resumable = compoResumable(registry);

	stream.template write<WIDTH_BITS>(active);

	if (resumable != INVALID_SHORT_INDEX) {
		stream.template write<1>(1);
		stream.template write<WIDTH_BITS>(resumable);
	} else
		stream.template write<1>(0);

	_subStates.wideSaveActive(registry,stream, active);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
void
C_<TN, TA, SG, TH, TS...>::deepSaveResumable(const Registry& registry,
											 WriteStream& stream) const
{
	const ShortIndex resumable = compoResumable(registry);

	if (resumable != INVALID_SHORT_INDEX) {
		stream.template write<1>(1);
		stream.template write<WIDTH_BITS>(resumable);
	} else
		stream.template write<1>(0);

	_subStates.wideSaveResumable(registry, stream);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
void
C_<TN, TA, SG, TH, TS...>::deepLoadRequested(Registry& registry,
											 ReadStream& stream) const
{
	ShortIndex& resumable = compoResumable(registry);
	ShortIndex& requested = compoRequested(registry);

	requested = stream.template read<WIDTH_BITS>();
	HFSM_ASSERT(requested < WIDTH);

	if (stream.template read<1>()) {
		resumable = stream.template read<WIDTH_BITS>();
		HFSM_ASSERT(resumable < WIDTH);
	} else
		resumable = INVALID_SHORT_INDEX;

	_subStates.wideLoadRequested(registry, stream, requested);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, Strategy SG, typename TH, typename... TS>
void
C_<TN, TA, SG, TH, TS...>::deepLoadResumable(Registry& registry,
											 ReadStream& stream) const
{
	ShortIndex& resumable = compoResumable(registry);

	if (stream.template read<1>()) {
		resumable = stream.template read<WIDTH_BITS>();
		HFSM_ASSERT(resumable < WIDTH);
	} else
		resumable = INVALID_SHORT_INDEX;

	_subStates.wideLoadResumable(registry, stream);
}

#endif

////////////////////////////////////////////////////////////////////////////////

}
}
namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

template <typename, typename, ShortIndex, typename...>
struct OS_;

//------------------------------------------------------------------------------

template <typename TIndices,
		  typename TArgs,
		  ShortIndex NIndex,
		  typename TInitial,
		  typename... TRemaining>
struct OS_<TIndices, TArgs, NIndex, TInitial, TRemaining...> final {
	using Indices		= TIndices;
	static constexpr StateID	INITIAL_ID	= Indices::STATE_ID;
	static constexpr ShortIndex COMPO_INDEX	= Indices::COMPO_INDEX;
	static constexpr ShortIndex ORTHO_INDEX	= Indices::ORTHO_INDEX;
	static constexpr ShortIndex ORTHO_UNIT	= Indices::ORTHO_UNIT;

	static constexpr ShortIndex PRONG_INDEX	= NIndex;

	using Args			= TArgs;
	using Rank			= typename Args::Rank;
	using Utility		= typename Args::Utility;
	using UP			= typename Args::UP;

	using Registry		= RegistryT<Args>;
	using StateParents	= typename Registry::StateParents;
	using OrthoForks	= typename Registry::OrthoForks;
	using ProngBits		= typename OrthoForks::Bits;
	using ProngConstBits= typename OrthoForks::ConstBits;

	using Control		= ControlT	   <Args>;
	using PlanControl	= PlanControlT <Args>;
	using FullControl	= FullControlT <Args>;
	using GuardControl	= GuardControlT<Args>;

	using Initial		= Material<I_<INITIAL_ID,
									  COMPO_INDEX,
									  ORTHO_INDEX,
									  ORTHO_UNIT>,
								   Args,
								   TInitial>;

	using InitialInfo	= WrapInfo<TInitial>;
	using InitialStates	= typename InitialInfo::StateList;

	using Remaining		= OS_<I_<INITIAL_ID  + InitialInfo::STATE_COUNT,
								 COMPO_INDEX + InitialInfo::COMPO_REGIONS,
								 ORTHO_INDEX + InitialInfo::ORTHO_REGIONS,
								 ORTHO_UNIT  + InitialInfo::ORTHO_UNITS>,
							  Args,
							  PRONG_INDEX + 1,
							  TRemaining...>;

	using Info	= OSI_<TInitial, TRemaining...>;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef HFSM_EXPLICIT_MEMBER_SPECIALIZATION
	template <typename T>
	HFSM_INLINE		  T& access();

	template <typename T>
	HFSM_INLINE const T& access() const;
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void	wideRegister		 (Registry& registry, const ForkID forkId);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE bool	wideForwardEntryGuard(GuardControl&	control,							const ProngConstBits prongs);
	HFSM_INLINE bool	wideForwardEntryGuard(GuardControl&	control);
	HFSM_INLINE bool	wideEntryGuard		 (GuardControl&	control);

	HFSM_INLINE void	wideConstruct		 (PlanControl&	control);

	HFSM_INLINE void	wideEnter			 (PlanControl&	control);
	HFSM_INLINE void	wideReenter			 (PlanControl&	control);

	HFSM_INLINE Status	wideUpdate			 (FullControl&	control);

	template <typename TEvent>
	HFSM_INLINE Status	wideReact			 (FullControl&	control, const TEvent& event);

	HFSM_INLINE bool	wideForwardExitGuard (GuardControl&	control,							const ProngConstBits prongs);
	HFSM_INLINE bool	wideForwardExitGuard (GuardControl&	control);
	HFSM_INLINE bool	wideExitGuard		 (GuardControl&	control);

	HFSM_INLINE void	wideExit			 (PlanControl&	control);

	HFSM_INLINE void	wideDestruct		 (PlanControl&  control);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void	wideForwardActive	 (Control& control, const Request::Type request,	const ProngConstBits prongs);
	HFSM_INLINE void	wideForwardRequest	 (Control& control, const Request::Type request,	const ProngConstBits prongs);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void	wideRequestChange	 (Control& control);
	HFSM_INLINE void	wideRequestRemain	 (Registry& registry);
	HFSM_INLINE void	wideRequestRestart	 (Registry& registry);
	HFSM_INLINE void	wideRequestResume	 (Registry& registry);
	HFSM_INLINE void	wideRequestUtilize	 (Control& control);
	HFSM_INLINE void	wideRequestRandomize (Control& control);

	HFSM_INLINE Utility	wideReportChange	 (Control& control);
	HFSM_INLINE Utility	wideReportUtilize	 (Control& control);
	HFSM_INLINE Utility	wideReportRandomize	 (Control& control);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void	wideChangeToRequested(PlanControl& control);

	//----------------------------------------------------------------------

#ifdef HFSM_ENABLE_STRUCTURE_REPORT
	using StructureStateInfos = typename Args::StructureStateInfos;

	static constexpr LongIndex NAME_COUNT	 = Initial::NAME_COUNT  + Remaining::NAME_COUNT;

	void wideGetNames(const LongIndex parent,
					  const ShortIndex depth,
					  StructureStateInfos& stateInfos) const;
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef HFSM_ENABLE_SERIALIZATION
	using WriteStream	= typename Args::WriteStream;
	using ReadStream	= typename Args::ReadStream;

	HFSM_INLINE void	wideSaveActive	 (const Registry& registry, WriteStream& stream) const;
	HFSM_INLINE void	wideSaveResumable(const Registry& registry, WriteStream& stream) const;

	HFSM_INLINE void	wideLoadRequested(		Registry& registry, ReadStream&  stream) const;
	HFSM_INLINE void	wideLoadResumable(		Registry& registry, ReadStream&  stream) const;
#endif

	//----------------------------------------------------------------------

	Initial initial;
	Remaining remaining;
};

////////////////////////////////////////////////////////////////////////////////

template <typename TIndices,
		  typename TArgs,
		  ShortIndex NIndex,
		  typename TInitial>
struct OS_<TIndices, TArgs, NIndex, TInitial> final {
	using Indices		= TIndices;
	static constexpr StateID	INITIAL_ID	= Indices::STATE_ID;
	static constexpr ShortIndex COMPO_INDEX	= Indices::COMPO_INDEX;
	static constexpr ShortIndex ORTHO_INDEX	= Indices::ORTHO_INDEX;
	static constexpr ShortIndex ORTHO_UNIT	= Indices::ORTHO_UNIT;

	static constexpr ShortIndex PRONG_INDEX	= NIndex;

	using Args			= TArgs;
	using Rank			= typename Args::Rank;
	using Utility		= typename Args::Utility;
	using UP			= typename Args::UP;

	using Registry		= RegistryT<Args>;
	using StateParents	= typename Registry::StateParents;
	using OrthoForks	= typename Registry::OrthoForks;
	using ProngBits		= typename OrthoForks::Bits;
	using ProngConstBits= typename OrthoForks::ConstBits;

	using Control		= ControlT	   <Args>;
	using PlanControl	= PlanControlT <Args>;
	using FullControl	= FullControlT <Args>;
	using GuardControl	= GuardControlT<Args>;

	using Initial		= Material<I_<INITIAL_ID,
									  COMPO_INDEX,
									  ORTHO_INDEX,
									  ORTHO_UNIT>,
								   Args,
								   TInitial>;

	using Info	= OSI_<TInitial>;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef HFSM_EXPLICIT_MEMBER_SPECIALIZATION
	template <typename T>
	HFSM_INLINE		  T& access()			 { return initial.template access<T>();	}

	template <typename T>
	HFSM_INLINE const T& access() const		 { return initial.template access<T>();	}
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void	wideRegister		 (Registry& registry, const ForkID forkId);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE bool	wideForwardEntryGuard(GuardControl&	control,							const ProngConstBits prongs);
	HFSM_INLINE bool	wideForwardEntryGuard(GuardControl&	control);
	HFSM_INLINE bool	wideEntryGuard		 (GuardControl&	control);

	HFSM_INLINE void	wideConstruct		 (PlanControl&	control);

	HFSM_INLINE void	wideEnter			 (PlanControl&	control);
	HFSM_INLINE void	wideReenter			 (PlanControl&	control);

	HFSM_INLINE Status	wideUpdate			 (FullControl&	control);

	template <typename TEvent>
	HFSM_INLINE Status	wideReact			 (FullControl&	control, const TEvent& event);

	HFSM_INLINE bool	wideForwardExitGuard (GuardControl&	control,							const ProngConstBits prongs);
	HFSM_INLINE bool	wideForwardExitGuard (GuardControl&	control);
	HFSM_INLINE bool	wideExitGuard		 (GuardControl&	control);

	HFSM_INLINE void	wideExit			 (PlanControl&	control);

	HFSM_INLINE void	wideDestruct		 (PlanControl&  control);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void	wideForwardActive	 (Control& control, const Request::Type request,	const ProngConstBits prongs);
	HFSM_INLINE void	wideForwardRequest	 (Control& control, const Request::Type request,	const ProngConstBits prongs);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void	wideRequestChange	 (Control& control);
	HFSM_INLINE void	wideRequestRemain	 (Registry& registry);
	HFSM_INLINE void	wideRequestRestart	 (Registry& registry);
	HFSM_INLINE void	wideRequestResume	 (Registry& registry);
	HFSM_INLINE void	wideRequestUtilize	 (Control& control);
	HFSM_INLINE void	wideRequestRandomize (Control& control);

	HFSM_INLINE Utility	wideReportChange	 (Control& control);
	HFSM_INLINE Utility	wideReportUtilize	 (Control& control);
	HFSM_INLINE Utility	wideReportRandomize	 (Control& control);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void	wideChangeToRequested(PlanControl& control);

	//----------------------------------------------------------------------

#ifdef HFSM_ENABLE_STRUCTURE_REPORT
	using StructureStateInfos = typename Args::StructureStateInfos;

	static constexpr LongIndex NAME_COUNT	 = Initial::NAME_COUNT;

	void wideGetNames(const LongIndex parent,
					  const ShortIndex depth,
					  StructureStateInfos& stateInfos) const;
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef HFSM_ENABLE_SERIALIZATION
	using WriteStream	= typename Args::WriteStream;
	using ReadStream	= typename Args::ReadStream;

	HFSM_INLINE void	wideSaveActive	 (const Registry& registry, WriteStream& stream) const;
	HFSM_INLINE void	wideSaveResumable(const Registry& registry, WriteStream& stream) const;

	HFSM_INLINE void	wideLoadRequested(		Registry& registry, ReadStream&  stream) const;
	HFSM_INLINE void	wideLoadResumable(		Registry& registry, ReadStream&  stream) const;
#endif

	//----------------------------------------------------------------------

	Initial initial;
};

////////////////////////////////////////////////////////////////////////////////

}
}

namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

#ifdef HFSM_EXPLICIT_MEMBER_SPECIALIZATION

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
template <typename T>
T&
OS_<TN, TA, NI, TI, TR...>::access() {
	return InitialStates::template contains<T>() ?
		initial  .template access<T>() :
		remaining.template access<T>();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
template <typename T>
const T&
OS_<TN, TA, NI, TI, TR...>::access() const {
	return InitialStates::template contains<T>() ?
		initial  .template access<T>() :
		remaining.template access<T>();
}

#endif

//------------------------------------------------------------------------------

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
void
OS_<TN, TA, NI, TI, TR...>::wideRegister(Registry& registry,
										 const ForkID forkId)
{
	initial  .deepRegister(registry, Parent{forkId, PRONG_INDEX});
	remaining.wideRegister(registry, forkId);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
bool
OS_<TN, TA, NI, TI, TR...>::wideForwardEntryGuard(GuardControl& control,
												  const ProngConstBits prongs)
{
	const bool i = prongs.get(PRONG_INDEX) ?
				   initial  .deepForwardEntryGuard(control) : false;

	const bool r = remaining.wideForwardEntryGuard(control, prongs);

	return i || r;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
bool
OS_<TN, TA, NI, TI, TR...>::wideForwardEntryGuard(GuardControl& control) {
	const bool i = initial  .deepForwardEntryGuard(control);
	const bool r = remaining.wideForwardEntryGuard(control);

	return i || r;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
bool
OS_<TN, TA, NI, TI, TR...>::wideEntryGuard(GuardControl& control) {
	const bool i = initial  .deepEntryGuard(control);
	const bool r = remaining.wideEntryGuard(control);

	return i || r;
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
void
OS_<TN, TA, NI, TI, TR...>::wideConstruct(PlanControl& control) {
	initial	 .deepConstruct(control);
	remaining.wideConstruct(control);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
void
OS_<TN, TA, NI, TI, TR...>::wideEnter(PlanControl& control) {
	initial  .deepEnter(control);
	remaining.wideEnter(control);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
void
OS_<TN, TA, NI, TI, TR...>::wideReenter(PlanControl& control) {
	initial  .deepReenter(control);
	remaining.wideReenter(control);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
Status
OS_<TN, TA, NI, TI, TR...>::wideUpdate(FullControl& control) {
	const auto status =	   initial	.deepUpdate(control);
	return combine(status, remaining.wideUpdate(control));
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
template <typename TEvent>
Status
OS_<TN, TA, NI, TI, TR...>::wideReact(FullControl& control,
									  const TEvent& event)
{
	const auto status =	   initial	.deepReact(control, event);
	return combine(status, remaining.wideReact(control, event));
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
bool
OS_<TN, TA, NI, TI, TR...>::wideForwardExitGuard(GuardControl& control,
												 const ProngConstBits prongs)
{
	const bool i = prongs.get(PRONG_INDEX) ?
				   initial  .deepForwardExitGuard(control) : false;

	const bool r = remaining.wideForwardExitGuard(control, prongs);

	return i || r;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
bool
OS_<TN, TA, NI, TI, TR...>::wideForwardExitGuard(GuardControl& control) {
	const bool i = initial  .deepForwardExitGuard(control);
	const bool r = remaining.wideForwardExitGuard(control);

	return i || r;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
bool
OS_<TN, TA, NI, TI, TR...>::wideExitGuard(GuardControl& control) {
	const bool i = initial  .deepExitGuard(control);
	const bool r = remaining.wideExitGuard(control);

	return i || r;
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
void
OS_<TN, TA, NI, TI, TR...>::wideExit(PlanControl& control) {
	initial	 .deepExit(control);
	remaining.wideExit(control);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
void
OS_<TN, TA, NI, TI, TR...>::wideDestruct(PlanControl& control) {
	initial	 .deepDestruct(control);
	remaining.wideDestruct(control);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
void
OS_<TN, TA, NI, TI, TR...>::wideForwardActive(Control& control,
											  const Request::Type request,
											  const ProngConstBits prongs)
{
	const Request::Type local = prongs.get(PRONG_INDEX) ?
		request : Request::REMAIN;

	initial	 .deepForwardActive(control, local);
	remaining.wideForwardActive(control, request, prongs);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
void
OS_<TN, TA, NI, TI, TR...>::wideForwardRequest(Control& control,
											   const Request::Type request,
											   const ProngConstBits prongs)
{
	const Request::Type local = prongs.get(PRONG_INDEX) ?
		request : Request::REMAIN;

	initial	 .deepForwardRequest(control, local);
	remaining.wideForwardRequest(control, request, prongs);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
void
OS_<TN, TA, NI, TI, TR...>::wideRequestChange(Control& control) {
	initial  .deepRequestChange(control);
	remaining.wideRequestChange(control);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
void
OS_<TN, TA, NI, TI, TR...>::wideRequestRemain(Registry& registry) {
	initial	 .deepRequestRemain(registry);
	remaining.wideRequestRemain(registry);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
void
OS_<TN, TA, NI, TI, TR...>::wideRequestRestart(Registry& registry) {
	initial	 .deepRequestRestart(registry);
	remaining.wideRequestRestart(registry);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
void
OS_<TN, TA, NI, TI, TR...>::wideRequestResume(Registry& registry) {
	initial	 .deepRequestResume(registry);
	remaining.wideRequestResume(registry);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
void
OS_<TN, TA, NI, TI, TR...>::wideRequestUtilize(Control& control) {
	initial  .deepRequestUtilize(control);
	remaining.wideRequestUtilize(control);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
void
OS_<TN, TA, NI, TI, TR...>::wideRequestRandomize(Control& control) {
	initial  .deepRequestRandomize(control);
	remaining.wideRequestRandomize(control);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
typename TA::Utility
OS_<TN, TA, NI, TI, TR...>::wideReportChange(Control& control) {
	const UP	  i = initial  .deepReportChange(control);
	const Utility r = remaining.wideReportChange(control);

	return i.utility + r;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
typename TA::Utility
OS_<TN, TA, NI, TI, TR...>::wideReportUtilize(Control& control) {
	const UP	  i = initial  .deepReportUtilize(control);
	const Utility r = remaining.wideReportUtilize(control);

	return i.utility + r;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
typename TA::Utility
OS_<TN, TA, NI, TI, TR...>::wideReportRandomize(Control& control) {
	const Utility i = initial  .deepReportRandomize(control);
	const Utility r = remaining.wideReportRandomize(control);

	return i + r;
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
void
OS_<TN, TA, NI, TI, TR...>::wideChangeToRequested(PlanControl& control) {
	initial	 .deepChangeToRequested(control);
	remaining.wideChangeToRequested(control);
}

//------------------------------------------------------------------------------

#ifdef HFSM_ENABLE_STRUCTURE_REPORT

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
void
OS_<TN, TA, NI, TI, TR...>::wideGetNames(const LongIndex parent,
										 const ShortIndex depth,
										 StructureStateInfos& stateInfos) const
{
	initial	 .deepGetNames(parent, StructureStateInfo::ORTHOGONAL, depth, stateInfos);
	remaining.wideGetNames(parent,								   depth, stateInfos);
}

#endif

//------------------------------------------------------------------------------

#ifdef HFSM_ENABLE_SERIALIZATION

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
void
OS_<TN, TA, NI, TI, TR...>::wideSaveActive(const Registry& registry,
										   WriteStream& stream) const
{
	initial	 .deepSaveActive(registry, stream);
	remaining.wideSaveActive(registry, stream);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
void
OS_<TN, TA, NI, TI, TR...>::wideSaveResumable(const Registry& registry,
											  WriteStream& stream) const
{
	initial	 .deepSaveResumable(registry, stream);
	remaining.wideSaveResumable(registry, stream);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
void
OS_<TN, TA, NI, TI, TR...>::wideLoadRequested(Registry& registry,
											  ReadStream& stream) const
{
	initial	 .deepLoadRequested(registry, stream);
	remaining.wideLoadRequested(registry, stream);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI, typename... TR>
void
OS_<TN, TA, NI, TI, TR...>::wideLoadResumable(Registry& registry,
											  ReadStream& stream) const
{
	initial	 .deepLoadResumable(registry, stream);
	remaining.wideLoadResumable(registry, stream);
}

#endif

////////////////////////////////////////////////////////////////////////////////

}
}
namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

template <typename TN, typename TA, ShortIndex NI, typename TI>
void
OS_<TN, TA, NI, TI>::wideRegister(Registry& registry,
									  const ForkID forkId)
{
	initial.deepRegister(registry, Parent{forkId, PRONG_INDEX});
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, ShortIndex NI, typename TI>
bool
OS_<TN, TA, NI, TI>::wideForwardEntryGuard(GuardControl& control,
										   const ProngConstBits prongs)
{
	return prongs.get(PRONG_INDEX) ?
		initial.deepForwardEntryGuard(control) : false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI>
bool
OS_<TN, TA, NI, TI>::wideForwardEntryGuard(GuardControl& control) {
	return initial.deepForwardEntryGuard(control);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI>
bool
OS_<TN, TA, NI, TI>::wideEntryGuard(GuardControl& control) {
	return initial.deepEntryGuard(control);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, ShortIndex NI, typename TI>
void
OS_<TN, TA, NI, TI>::wideConstruct(PlanControl& control) {
	initial.deepConstruct(control);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, ShortIndex NI, typename TI>
void
OS_<TN, TA, NI, TI>::wideEnter(PlanControl& control) {
	initial.deepEnter(control);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI>
void
OS_<TN, TA, NI, TI>::wideReenter(PlanControl& control) {
	initial.deepReenter(control);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, ShortIndex NI, typename TI>
Status
OS_<TN, TA, NI, TI>::wideUpdate(FullControl& control) {
	return initial.deepUpdate(control);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, ShortIndex NI, typename TI>
template <typename TEvent>
Status
OS_<TN, TA, NI, TI>::wideReact(FullControl& control,
							   const TEvent& event)
{
	return initial.deepReact(control, event);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, ShortIndex NI, typename TI>
bool
OS_<TN, TA, NI, TI>::wideForwardExitGuard(GuardControl& control,
										  const ProngConstBits prongs)
{
	return prongs.get(PRONG_INDEX) ?
		initial.deepForwardExitGuard(control) : false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI>
bool
OS_<TN, TA, NI, TI>::wideForwardExitGuard(GuardControl& control) {
	return initial.deepForwardExitGuard(control);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI>
bool
OS_<TN, TA, NI, TI>::wideExitGuard(GuardControl& control) {
	return initial.deepExitGuard(control);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, ShortIndex NI, typename TI>
void
OS_<TN, TA, NI, TI>::wideExit(PlanControl& control) {
	initial.deepExit(control);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, ShortIndex NI, typename TI>
void
OS_<TN, TA, NI, TI>::wideDestruct(PlanControl& control) {
	initial.deepDestruct(control);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, ShortIndex NI, typename TI>
void
OS_<TN, TA, NI, TI>::wideForwardActive(Control& control,
									   const Request::Type request,
									   const ProngConstBits prongs)
{
	const Request::Type local = prongs.get(PRONG_INDEX) ?
		request : Request::REMAIN;

	initial.deepForwardActive(control, local);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI>
void
OS_<TN, TA, NI, TI>::wideForwardRequest(Control& control,
										const Request::Type request,
										const ProngConstBits prongs)
{
	const Request::Type local = prongs.get(PRONG_INDEX) ?
		request : Request::REMAIN;

	initial.deepForwardRequest(control, local);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, ShortIndex NI, typename TI>
void
OS_<TN, TA, NI, TI>::wideRequestChange(Control& control) {
	initial.deepRequestChange(control);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI>
void
OS_<TN, TA, NI, TI>::wideRequestRemain(Registry& registry) {
	initial.deepRequestRemain(registry);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI>
void
OS_<TN, TA, NI, TI>::wideRequestRestart(Registry& registry) {
	initial.deepRequestRestart(registry);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI>
void
OS_<TN, TA, NI, TI>::wideRequestResume(Registry& registry) {
	initial.deepRequestResume(registry);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI>
void
OS_<TN, TA, NI, TI>::wideRequestUtilize(Control& control) {
	initial.deepRequestUtilize(control);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI>
void
OS_<TN, TA, NI, TI>::wideRequestRandomize(Control& control) {
	initial.deepRequestRandomize(control);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, ShortIndex NI, typename TI>
typename TA::Utility
OS_<TN, TA, NI, TI>::wideReportChange(Control& control) {
	const UP i = initial.deepReportChange(control);

	return i.utility;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI>
typename TA::Utility
OS_<TN, TA, NI, TI>::wideReportUtilize(Control& control) {
	const UP i = initial.deepReportUtilize(control);

	return i.utility;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI>
typename TA::Utility
OS_<TN, TA, NI, TI>::wideReportRandomize(Control& control) {
	const Utility i = initial.deepReportRandomize(control);

	return i;
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, ShortIndex NI, typename TI>
void
OS_<TN, TA, NI, TI>::wideChangeToRequested(PlanControl& control) {
	initial.deepChangeToRequested(control);
}

//------------------------------------------------------------------------------

#ifdef HFSM_ENABLE_STRUCTURE_REPORT

template <typename TN, typename TA, ShortIndex NI, typename TI>
void
OS_<TN, TA, NI, TI>::wideGetNames(const LongIndex parent,
								  const ShortIndex depth,
								  StructureStateInfos& _stateInfos) const
{
	initial.deepGetNames(parent, StructureStateInfo::ORTHOGONAL, depth, _stateInfos);
}

#endif

//------------------------------------------------------------------------------

#ifdef HFSM_ENABLE_SERIALIZATION

template <typename TN, typename TA, ShortIndex NI, typename TI>
void
OS_<TN, TA, NI, TI>::wideSaveActive(const Registry& registry,
									WriteStream& stream) const
{
	initial.deepSaveActive(registry, stream);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI>
void
OS_<TN, TA, NI, TI>::wideSaveResumable(const Registry& registry,
									   WriteStream& stream) const
{
	initial.deepSaveResumable(registry, stream);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI>
void
OS_<TN, TA, NI, TI>::wideLoadRequested(Registry& registry,
									   ReadStream& stream) const
{
	initial.deepLoadRequested(registry, stream);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, ShortIndex NI, typename TI>
void
OS_<TN, TA, NI, TI>::wideLoadResumable(Registry& registry,
									   ReadStream& stream) const
{
	initial.deepLoadResumable(registry, stream);
}

#endif

////////////////////////////////////////////////////////////////////////////////

}
}
namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

template <typename TIndices,
		  typename TArgs,
		  typename THead,
		  typename... TSubStates>
struct O_ final {
	using Indices		= TIndices;
	static constexpr StateID	HEAD_ID		= Indices::STATE_ID;
	static constexpr ShortIndex COMPO_INDEX	= Indices::COMPO_INDEX;
	static constexpr ShortIndex ORTHO_INDEX	= Indices::ORTHO_INDEX;
	static constexpr ShortIndex ORTHO_UNIT	= Indices::ORTHO_UNIT;

	static constexpr ShortIndex REGION_ID	= COMPO_INDEX + ORTHO_INDEX;
	static constexpr ForkID		ORTHO_ID	= (ForkID) -ORTHO_INDEX - 1;

	using Args			= TArgs;
	using Rank			= typename Args::Rank;
	using Utility		= typename Args::Utility;
	using UP			= typename Args::UP;
	using StateList		= typename Args::StateList;
	using RegionList	= typename Args::RegionList;

	using Head			= THead;

	using Info			= OI_<Head, TSubStates...>;
	static constexpr ShortIndex WIDTH		= Info::WIDTH;
	static constexpr ShortIndex REGION_SIZE	= Info::STATE_COUNT;
	static constexpr ShortIndex ORTHO_UNITS	= Info::ORTHO_UNITS;

	using Registry		= RegistryT<Args>;
	using StateParents	= typename Registry::StateParents;
	using OrthoForks	= typename Registry::OrthoForks;
	using ProngBits		= typename OrthoForks::Bits;
	using ProngConstBits= typename OrthoForks::ConstBits;

	using Control		= ControlT<Args>;

	using PlanControl	= PlanControlT<Args>;
	using ScopedOrigin	= typename PlanControl::Origin;
	using ScopedRegion	= typename PlanControl::Region;

	using FullControl	= FullControlT<Args>;
	using ControlLock	= typename FullControl::Lock;

	using GuardControl	= GuardControlT<Args>;

	using HeadState		= S_<Indices, Args, Head>;

	using SubStates		= OS_<I_<HEAD_ID + 1,
								 COMPO_INDEX,
								 ORTHO_INDEX + 1,
								 ORTHO_UNIT + (WIDTH + 7) / 8>,
							  Args,
							  0,
							  TSubStates...>;

	//----------------------------------------------------------------------

#ifdef HFSM_EXPLICIT_MEMBER_SPECIALIZATION
	template <typename T>
	struct Accessor {
		HFSM_INLINE static		 T&	   get(		 O_& o)							{ return o._subStates.template access<T>();	}
		HFSM_INLINE static const T&	   get(const O_& o)							{ return o._subStates.template access<T>();	}
	};

	template <>
	struct Accessor<Head> {
		HFSM_INLINE static		 Head& get(		 O_& o)							{ return o._headState._headBox.get();		}
		HFSM_INLINE static const Head& get(const O_& o)							{ return o._headState._headBox.get();		}
	};

	template <typename T>
	HFSM_INLINE		  T&	access()											{ return Accessor<T>::get(*this);			}

	template <typename T>
	HFSM_INLINE const T&	access() const										{ return Accessor<T>::get(*this);			}
#endif

	//----------------------------------------------------------------------

	HFSM_INLINE ProngBits	   orthoRequested(		Registry& registry)			{ return registry.orthoRequested.template bits<ORTHO_UNIT, WIDTH>();	}
	HFSM_INLINE ProngConstBits orthoRequested(const Registry& registry) const	{ return registry.orthoRequested.template bits<ORTHO_UNIT, WIDTH>();	}

	HFSM_INLINE ProngBits	   orthoRequested(		Control& control)			{ return orthoRequested(control._registry);								}
	HFSM_INLINE ProngConstBits orthoRequested(const Control& control) const		{ return orthoRequested(control._registry);								}

	HFSM_INLINE void	deepRegister		 (Registry& registry, const Parent parent);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE bool	deepForwardEntryGuard(GuardControl&	control);
	HFSM_INLINE bool	deepEntryGuard		 (GuardControl&	control);

	HFSM_INLINE void	deepConstruct		 (PlanControl&	control);

	HFSM_INLINE void	deepEnter			 (PlanControl&	control);
	HFSM_INLINE void	deepReenter			 (PlanControl&	control);

	HFSM_INLINE Status	deepUpdate			 (FullControl&	control);

	template <typename TEvent>
	HFSM_INLINE Status	deepReact			 (FullControl&	control, const TEvent& event);

	HFSM_INLINE bool	deepForwardExitGuard (GuardControl&	control);
	HFSM_INLINE bool	deepExitGuard		 (GuardControl&	control);

	HFSM_INLINE void	deepExit			 (PlanControl&	control);

	HFSM_INLINE void	deepDestruct		 (PlanControl&  control);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void	deepForwardActive	 (Control& control, const Request::Type request);
	HFSM_INLINE void	deepForwardRequest	 (Control& control, const Request::Type request);

	HFSM_INLINE void	deepRequest			 (Control& control, const Request::Type request);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void	deepRequestChange	 (Control& control);
	HFSM_INLINE void	deepRequestRemain	 (Registry& registry);
	HFSM_INLINE void	deepRequestRestart	 (Registry& registry);
	HFSM_INLINE void	deepRequestResume	 (Registry& registry);
	HFSM_INLINE void	deepRequestUtilize	 (Control& control);
	HFSM_INLINE void	deepRequestRandomize (Control& control);

	HFSM_INLINE UP		deepReportChange	 (Control& control);
	HFSM_INLINE UP		deepReportUtilize	 (Control& control);
	HFSM_INLINE Rank	deepReportRank		 (Control& control);
	HFSM_INLINE Utility	deepReportRandomize	 (Control& control);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void	deepChangeToRequested(PlanControl& control);

	//----------------------------------------------------------------------

#ifdef HFSM_ENABLE_STRUCTURE_REPORT
	using StructureStateInfos = typename Args::StructureStateInfos;
	using RegionType		  = typename StructureStateInfo::RegionType;

	static constexpr LongIndex NAME_COUNT	 = HeadState::NAME_COUNT  + SubStates::NAME_COUNT;

	void deepGetNames(const LongIndex parent,
					  const RegionType region,
					  const ShortIndex depth,
					  StructureStateInfos& stateInfos) const;
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef HFSM_ENABLE_SERIALIZATION
	using WriteStream	= typename Args::WriteStream;
	using ReadStream	= typename Args::ReadStream;

	HFSM_INLINE void	deepSaveActive	 (const Registry& registry, WriteStream& stream) const;
	HFSM_INLINE void	deepSaveResumable(const Registry& registry, WriteStream& stream) const;

	HFSM_INLINE void	deepLoadRequested(		Registry& registry, ReadStream&  stream) const;
	HFSM_INLINE void	deepLoadResumable(		Registry& registry, ReadStream&  stream) const;
#endif

	//----------------------------------------------------------------------

	HeadState _headState;
	SubStates _subStates;
};

////////////////////////////////////////////////////////////////////////////////

}
}

namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

template <typename TN, typename TA, typename TH, typename... TS>
void
O_<TN, TA, TH, TS...>::deepRegister(Registry& registry,
										const Parent parent)
{
	registry.orthoParents[ORTHO_INDEX] = parent;
	registry.orthoUnits[ORTHO_INDEX] = Units{ORTHO_UNIT, WIDTH};

	_headState.deepRegister(registry, parent);
	_subStates.wideRegister(registry, ORTHO_ID);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, typename TH, typename... TS>
bool
O_<TN, TA, TH, TS...>::deepForwardEntryGuard(GuardControl& control) {
	const ProngConstBits requested = orthoRequested(static_cast<const GuardControl&>(control));

	ScopedRegion region{control, REGION_ID, HEAD_ID, REGION_SIZE};

	if (requested)
		return _subStates.wideForwardEntryGuard(control, requested);
	else
		return _subStates.wideForwardEntryGuard(control);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, typename TH, typename... TS>
bool
O_<TN, TA, TH, TS...>::deepEntryGuard(GuardControl& control) {
	ScopedRegion region{control, REGION_ID, HEAD_ID, REGION_SIZE};

	return _headState.deepEntryGuard(control) ||
		   _subStates.wideEntryGuard(control);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, typename TH, typename... TS>
void
O_<TN, TA, TH, TS...>::deepConstruct(PlanControl& control) {
	ProngBits requested = orthoRequested(control);
	requested.clear();

	_headState.deepConstruct(control);
	_subStates.wideConstruct(control);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, typename TH, typename... TS>
void
O_<TN, TA, TH, TS...>::deepEnter(PlanControl& control) {
	ScopedRegion region{control, REGION_ID, HEAD_ID, REGION_SIZE};

	_headState.deepEnter(control);
	_subStates.wideEnter(control);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, typename TH, typename... TS>
void
O_<TN, TA, TH, TS...>::deepReenter(PlanControl& control) {
	ProngBits requested = orthoRequested(control);
	requested.clear();

	ScopedRegion region{control, REGION_ID, HEAD_ID, REGION_SIZE};

	_headState.deepReenter(control);
	_subStates.wideReenter(control);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, typename TH, typename... TS>
Status
O_<TN, TA, TH, TS...>::deepUpdate(FullControl& control) {
	ScopedRegion outer{control, REGION_ID, HEAD_ID, REGION_SIZE};

	if (const auto headStatus = _headState.deepUpdate(control)) {
		ControlLock lock{control};
		_subStates.wideUpdate(control);

		return headStatus;
	} else {
		const Status subStatus = _subStates.wideUpdate(control);

		if (subStatus.outerTransition)
			return subStatus;

		ScopedRegion inner{control, REGION_ID, HEAD_ID, REGION_SIZE};

		return subStatus && control._planData.planExists.template get<REGION_ID>() ?
			control.updatePlan(_headState, subStatus) : subStatus;
	}
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, typename TH, typename... TS>
template <typename TEvent>
Status
O_<TN, TA, TH, TS...>::deepReact(FullControl& control,
								 const TEvent& event)
{
	ScopedRegion outer{control, REGION_ID, HEAD_ID, REGION_SIZE};

	if (const auto headStatus = _headState.deepReact(control, event)) {
		ControlLock lock{control};
		_subStates.wideReact(control, event);

		return headStatus;
	} else {
		const Status subStatus = _subStates.wideReact(control, event);

		if (subStatus.outerTransition)
			return subStatus;

		ScopedRegion inner{control, REGION_ID, HEAD_ID, REGION_SIZE};

		return subStatus && control._planData.planExists.template get<REGION_ID>() ?
			control.updatePlan(_headState, subStatus) : subStatus;
	}
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, typename TH, typename... TS>
bool
O_<TN, TA, TH, TS...>::deepForwardExitGuard(GuardControl& control) {
	const ProngConstBits requested = orthoRequested(static_cast<const GuardControl&>(control));

	ScopedRegion region{control, REGION_ID, HEAD_ID, REGION_SIZE};

	if (requested)
		return _subStates.wideForwardExitGuard(control, requested);
	else
		return _subStates.wideForwardExitGuard(control);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, typename TH, typename... TS>
bool
O_<TN, TA, TH, TS...>::deepExitGuard(GuardControl& control) {
	ScopedRegion region{control, REGION_ID, HEAD_ID, REGION_SIZE};

	return _headState.deepExitGuard(control) ||
		   _subStates.wideExitGuard(control);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, typename TH, typename... TS>
void
O_<TN, TA, TH, TS...>::deepExit(PlanControl& control) {
	_subStates.wideExit(control);
	_headState.deepExit(control);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, typename TH, typename... TS>
void
O_<TN, TA, TH, TS...>::deepDestruct(PlanControl& control) {
	_subStates.wideDestruct(control);
	_headState.deepDestruct(control);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, typename TH, typename... TS>
void
O_<TN, TA, TH, TS...>::deepForwardActive(Control& control,
										 const Request::Type request)
{
	HFSM_ASSERT(control._registry.isActive(HEAD_ID));

	const ProngConstBits requested = orthoRequested(static_cast<const Control&>(control));
	HFSM_ASSERT(!!requested);

	_subStates.wideForwardActive(control, request, requested);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, typename TH, typename... TS>
void
O_<TN, TA, TH, TS...>::deepForwardRequest(Control& control,
										  const Request::Type request)
{
	const ProngConstBits requested = orthoRequested(static_cast<const Control&>(control));

	if (requested)
		_subStates.wideForwardRequest(control, request, requested);
	else
		deepRequest					 (control, request);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, typename TH, typename... TS>
void
O_<TN, TA, TH, TS...>::deepRequest(Control& control,
								   const Request::Type request)
{
	switch (request) {
	case Request::REMAIN:
		deepRequestRemain (control._registry);
		break;

	case Request::CHANGE:
		deepRequestChange (control);
		break;

	case Request::RESTART:
		deepRequestRestart(control._registry);
		break;

	case Request::RESUME:
		deepRequestResume (control._registry);
		break;

	case Request::UTILIZE:
		deepRequestUtilize(control);
		break;

	case Request::RANDOMIZE:
		deepRequestRandomize(control);
		break;

	default:
		HFSM_BREAK();
	}
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, typename TH, typename... TS>
void
O_<TN, TA, TH, TS...>::deepRequestChange(Control& control) {
	_subStates.wideRequestChange(control);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, typename TH, typename... TS>
void
O_<TN, TA, TH, TS...>::deepRequestRemain(Registry& registry) {
	_subStates.wideRequestRemain(registry);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, typename TH, typename... TS>
void
O_<TN, TA, TH, TS...>::deepRequestRestart(Registry& registry) {
	_subStates.wideRequestRestart(registry);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, typename TH, typename... TS>
void
O_<TN, TA, TH, TS...>::deepRequestResume(Registry& registry) {
	_subStates.wideRequestResume(registry);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, typename TH, typename... TS>
void
O_<TN, TA, TH, TS...>::deepRequestUtilize(Control& control) {
	_subStates.wideRequestUtilize(control);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, typename TH, typename... TS>
void
O_<TN, TA, TH, TS...>::deepRequestRandomize(Control& control) {
	_subStates.wideRequestRandomize(control);
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, typename TH, typename... TS>
typename TA::UP
O_<TN, TA, TH, TS...>::deepReportChange(Control& control) {
	const UP	  h = _headState.deepReportChange(control);
	const Utility s = _subStates.wideReportChange(control);

	const Utility sub = s / WIDTH;

	HFSM_LOG_UTILITY_RESOLUTION(control.context(), HEAD_ID, INVALID_STATE_ID, sub);

	return {
		h.utility * sub,
		h.prong
	};
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, typename TH, typename... TS>
typename TA::UP
O_<TN, TA, TH, TS...>::deepReportUtilize(Control& control) {
	const UP	  h = _headState.deepReportUtilize(control);
	const Utility s = _subStates.wideReportUtilize(control);

	const Utility sub = s / WIDTH;

	HFSM_LOG_UTILITY_RESOLUTION(control.context(), HEAD_ID, INVALID_STATE_ID, sub);

	return {
		h.utility * sub,
		h.prong
	};
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, typename TH, typename... TS>
typename TA::Rank
O_<TN, TA, TH, TS...>::deepReportRank(Control& control) {
	return _headState.wrapRank(control);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, typename TH, typename... TS>
typename TA::Utility
O_<TN, TA, TH, TS...>::deepReportRandomize(Control& control) {
	const Utility h = _headState.wrapUtility(control);
	const Utility s = _subStates.wideReportRandomize(control);

	const Utility sub = s / WIDTH;

	HFSM_LOG_RANDOM_RESOLUTION(control.context(), HEAD_ID, INVALID_STATE_ID, sub);

	return h * sub;
}

//------------------------------------------------------------------------------

template <typename TN, typename TA, typename TH, typename... TS>
void
O_<TN, TA, TH, TS...>::deepChangeToRequested(PlanControl& control) {
	_subStates.wideChangeToRequested(control);
}

//------------------------------------------------------------------------------

#ifdef HFSM_ENABLE_STRUCTURE_REPORT

template <typename TN, typename TA, typename TH, typename... TS>
void
O_<TN, TA, TH, TS...>::deepGetNames(const LongIndex parent,
									const RegionType region,
									const ShortIndex depth,
									StructureStateInfos& stateInfos) const
{
	_headState.deepGetNames(parent, region,			depth,	   stateInfos);
	_subStates.wideGetNames(stateInfos.count() - 1, depth + 1, stateInfos);
}

#endif

//------------------------------------------------------------------------------

#ifdef HFSM_ENABLE_SERIALIZATION

template <typename TN, typename TA, typename TH, typename... TS>
void
O_<TN, TA, TH, TS...>::deepSaveActive(const Registry& registry,
									  WriteStream& stream) const
{
	_subStates.wideSaveActive(registry, stream);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, typename TH, typename... TS>
void
O_<TN, TA, TH, TS...>::deepSaveResumable(const Registry& registry,
										 WriteStream& stream) const
{
	_subStates.wideSaveResumable(registry, stream);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, typename TH, typename... TS>
void
O_<TN, TA, TH, TS...>::deepLoadRequested(Registry& registry,
										 ReadStream& stream) const
{
	_subStates.wideLoadRequested(registry, stream);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN, typename TA, typename TH, typename... TS>
void
O_<TN, TA, TH, TS...>::deepLoadResumable(Registry& registry,
										 ReadStream& stream) const
{
	_subStates.wideLoadResumable(registry, stream);
}

#endif

////////////////////////////////////////////////////////////////////////////////

}
}
namespace hfsm2 {
namespace detail {

#ifndef HFSM_EXPLICIT_MEMBER_SPECIALIZATION

//------------------------------------------------------------------------------

template <typename...>
struct Accessor;

////////////////////////////////////////////////////////////////////////////////

template <typename T,
		  typename TN,
		  typename TA,
		  Strategy TG,
		  typename TH,
		  typename... TS>
struct Accessor<T,		 C_<TN, TA, TG, TH, TS...>> {
	using Host =		 C_<TN, TA, TG, TH, TS...>;

	HFSM_INLINE		  T& get()			{ return Accessor<T,	   typename Host::SubStates>{host._subStates}.get();	}

	Host& host;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename T,
		  typename TN,
		  typename TA,
		  Strategy TG,
		  typename TH,
		  typename... TS>
struct Accessor<T, const C_<TN, TA, TG, TH, TS...>> {
	using Host =   const C_<TN, TA, TG, TH, TS...>;

	HFSM_INLINE const T& get() const	{ return Accessor<T, const typename Host::SubStates>{host._subStates}.get();	}

	Host& host;
};

//------------------------------------------------------------------------------

template <typename T,
		  typename TN,
		  typename TA,
		  Strategy TG,
		  typename... TS>
struct Accessor<T,		 C_<TN, TA, TG,  T, TS...>> {
	using Host =		 C_<TN, TA, TG,  T, TS...>;

	HFSM_INLINE		  T& get()			{ return host._headState._headBox.get();	}

	Host& host;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename T,
		  typename TN,
		  typename TA,
		  Strategy TG,
		  typename... TS>
struct Accessor<T, const C_<TN, TA, TG,  T, TS...>> {
	using Host =   const C_<TN, TA, TG,  T, TS...>;

	HFSM_INLINE const T& get() const	{ return host._headState._headBox.get();	}

	Host& host;
};

////////////////////////////////////////////////////////////////////////////////

template <typename T,
		  typename TN,
		  typename TA,
		  Strategy TG,
		  ShortIndex NI,
		  typename... TS>
struct Accessor<T,		 CS_<TN, TA, TG, NI, TS...>> {
	using Host =		 CS_<TN, TA, TG, NI, TS...>;

	HFSM_INLINE		  T& get()		 {
		return Host::LHalfInfo::StateList::template contains<T>() ?
			Accessor<T,		  typename Host::LHalf>{host.lHalf}.get() :
			Accessor<T,		  typename Host::RHalf>{host.rHalf}.get();
	}

	Host& host;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename T,
		  typename TN,
		  typename TA,
		  Strategy TG,
		  ShortIndex NI,
		  typename... TS>
struct Accessor<T, const CS_<TN, TA, TG, NI, TS...>> {
	using Host =   const CS_<TN, TA, TG, NI, TS...>;

	HFSM_INLINE const T& get() const {
		return Host::LHalfInfo::StateList::template contains<T>() ?
			Accessor<T, const typename Host::LHalf>{host.lHalf}.get() :
			Accessor<T, const typename Host::RHalf>{host.rHalf}.get();
	}

	Host& host;
};

//------------------------------------------------------------------------------

template <typename T,
		  typename TN,
		  typename TA,
		  Strategy TG,
		  ShortIndex NI,
		  typename TS>
struct Accessor<T,		 CS_<TN, TA, TG, NI, TS>> {
	using Host =		 CS_<TN, TA, TG, NI, TS>;

	HFSM_INLINE		  T& get()			{ return Accessor<T,	   typename Host::State>{host.state}.get();				}

	Host& host;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename T,
		  typename TN,
		  typename TA,
		  Strategy TG,
		  ShortIndex NI,
		  typename TS>
struct Accessor<T, const CS_<TN, TA, TG, NI, TS>> {
	using Host =   const CS_<TN, TA, TG, NI, TS>;

	HFSM_INLINE const T& get() const	{ return Accessor<T, const typename Host::State>{host.state}.get();				}

	Host& host;
};

////////////////////////////////////////////////////////////////////////////////

template <typename T,
		  typename TN,
		  typename TA,
		  typename TH,
		  typename... TS>
struct Accessor<T,		 O_<TN, TA, TH, TS...>> {
	using Host =		 O_<TN, TA, TH, TS...>;

	HFSM_INLINE		  T& get()			{ return Accessor<T,	   typename Host::SubStates>{host._subStates}.get();	}

	Host& host;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename T,
		  typename TN,
		  typename TA,
		  typename TH,
		  typename... TS>
struct Accessor<T, const O_<TN, TA, TH, TS...>> {
	using Host =   const O_<TN, TA, TH, TS...>;

	HFSM_INLINE const T& get() const	{ return Accessor<T, const typename Host::SubStates>{host._subStates}.get();	}

	Host& host;
};

//------------------------------------------------------------------------------

template <typename T,
		  typename TN,
		  typename TA,
		  typename... TS>
struct Accessor<T,		 O_<TN, TA,  T, TS...>> {
	using Host =		 O_<TN, TA,  T, TS...>;

	HFSM_INLINE		  T& get()			{ return host._headState._headBox.get();	}

	Host& host;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename T,
		  typename TN,
		  typename TA,
		  typename... TS>
struct Accessor<T, const O_<TN, TA,  T, TS...>> {
	using Host =   const O_<TN, TA,  T, TS...>;

	HFSM_INLINE const T& get() const	{ return host._headState._headBox.get();	}

	Host& host;
};

////////////////////////////////////////////////////////////////////////////////

template <typename T,
		  typename TN,
		  typename TA,
		  ShortIndex NI,
		  typename... TS>
struct Accessor<T,		 OS_<TN, TA, NI, TS...>> {
	using Host =		 OS_<TN, TA, NI, TS...>;

	HFSM_INLINE		  T& get()		 {
		return Host::InitialStates::template contains<T>() ?
			Accessor<T,		  typename Host::Initial  >{host.initial  }.get() :
			Accessor<T,		  typename Host::Remaining>{host.remaining}.get();
	}

	Host& host;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename T,
		  typename TN,
		  typename TA,
		  ShortIndex NI,
		  typename... TS>
struct Accessor<T, const OS_<TN, TA, NI, TS...>> {
	using Host =   const OS_<TN, TA, NI, TS...>;

	HFSM_INLINE const T& get() const {
		return Host::InitialStates::template contains<T>() ?
			Accessor<T, const typename Host::Initial  >{host.initial  }.get() :
			Accessor<T, const typename Host::Remaining>{host.remaining}.get();
	}

	Host& host;
};

//------------------------------------------------------------------------------

template <typename T,
		  typename TN,
		  typename TA,
		  ShortIndex NI,
		  typename TS>
struct Accessor<T,		 OS_<TN, TA, NI, TS>> {
	using Host =		 OS_<TN, TA, NI, TS>;

	HFSM_INLINE		  T& get()			{ return Accessor<T,	   typename Host::Initial>{host.initial  }.get();	}

	Host& host;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename T,
		  typename TN,
		  typename TA,
		  ShortIndex NI,
		  typename TS>
struct Accessor<T, const OS_<TN, TA, NI, TS>> {
	using Host =   const OS_<TN, TA, NI, TS>;

	HFSM_INLINE const T& get() const	{ return Accessor<T, const typename Host::Initial>{host.initial  }.get();	}

	Host& host;
};

////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////

#ifdef __clang__
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wnull-dereference"
#endif

template <typename T,
		  typename TN,
		  typename TA,
		  typename TH>
struct Accessor<T,		 S_<TN, TA, TH>> {
	using Host =		 S_<TN, TA, TH>;

	HFSM_INLINE		  T& get()			{ HFSM_BREAK(); return *reinterpret_cast<T*>(0);	}

	Host& host;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename T,
		  typename TN,
		  typename TA,
		  typename TH>
struct Accessor<T, const S_<TN, TA, TH>> {
	using Host =   const S_<TN, TA, TH>;

	HFSM_INLINE const T& get() const	{ HFSM_BREAK(); return *reinterpret_cast<T*>(0);	}

	Host& host;
};

#ifdef __clang__
	#pragma clang diagnostic pop
#endif

//------------------------------------------------------------------------------

template <typename T,
		  typename TN,
		  typename TA>
struct Accessor<T,		 S_<TN, TA,  T>> {
	using Host =		 S_<TN, TA,  T>;

	HFSM_INLINE		  T& get()			{ return host._headBox.get();			}

	Host& host;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename T,
		  typename TN,
		  typename TA>
struct Accessor<T, const S_<TN, TA,  T>> {
	using Host =   const S_<TN, TA,  T>;

	HFSM_INLINE const T& get() const	{ return host._headBox.get();			}

	Host& host;
};

////////////////////////////////////////////////////////////////////////////////

#endif

}
}

namespace hfsm2 {

//------------------------------------------------------------------------------

template <typename TC_ = EmptyContext,
		  typename TN_ = char,
		  typename TU_ = float,
		  typename TG_ = ::hfsm2::RandomT<TU_>,
		  LongIndex NS = 4,
		  LongIndex NT = INVALID_LONG_INDEX>
struct ConfigT {
	using Context = TC_;

	using Rank	  = TN_;
	using Utility = TU_;
	using RNG	  = TG_;
	using Logger  = LoggerInterfaceT<Context, Utility>;

	static constexpr LongIndex SUBSTITUTION_LIMIT = NS;
	static constexpr LongIndex TASK_CAPACITY	  = NT;

	template <typename T>
	using ContextT			 = ConfigT<  T, TN_, TU_, TG_, NS, NT>;

	template <typename T>
	using RankT				 = ConfigT<TC_,   T, TU_, TG_, NS, NT>;

	template <typename T>
	using UtilityT			 = ConfigT<TC_, TN_,   T, TG_, NS, NT>;

	template <typename T>
	using RandomT			 = ConfigT<TC_, TN_, TU_,   T, NS, NT>;

	template <LongIndex N>
	using SubstitutionLimitN = ConfigT<TC_, TN_, TU_, TG_,  N, NS>;

	template <LongIndex N>
	using TaskCapacityN		 = ConfigT<TC_, TN_, TU_, TG_, NT,  N>;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct UP {
		HFSM_INLINE UP(const Utility utility_  = Utility{1.0f},
					   const ShortIndex prong_ = INVALID_SHORT_INDEX)
			: utility{utility_}
			, prong{prong_}
		{}

		Utility utility;
		ShortIndex prong;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
};

//------------------------------------------------------------------------------

using Config = ConfigT<>;

namespace detail {

////////////////////////////////////////////////////////////////////////////////

template <typename TConfig>
struct M_ {
	using Config_ = TConfig;

	//----------------------------------------------------------------------

	template <typename THead, typename... TSubStates>
	using Composite			 = CI_<Strategy::Composite,	   THead, TSubStates...>;

	template <				  typename... TSubStates>
	using CompositePeers	 = CI_<Strategy::Composite,    void,  TSubStates...>;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	template <typename THead, typename... TSubStates>
	using Resumable			  = CI_<Strategy::Resumable,   THead, TSubStates...>;

	template <				  typename... TSubStates>
	using ResumablePeers	  = CI_<Strategy::Resumable,   void,  TSubStates...>;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	template <typename THead, typename... TSubStates>
	using Utilitarian		  = CI_<Strategy::Utilitarian, THead, TSubStates...>;

	template <				  typename... TSubStates>
	using UtilitarianPeers	  = CI_<Strategy::Utilitarian, void,  TSubStates...>;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	template <typename THead, typename... TSubStates>
	using Random			  = CI_<Strategy::RandomUtil,  THead, TSubStates...>;

	template <				  typename... TSubStates>
	using RandomPeers		  = CI_<Strategy::RandomUtil,  void,  TSubStates...>;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	template <typename THead, typename... TSubStates>
	using Orthogonal		  = OI_<THead, TSubStates...>;

	template <				  typename... TSubStates>
	using OrthogonalPeers	  = OI_<void,  TSubStates...>;

	//----------------------------------------------------------------------

	template <typename THead, typename... TSubStates>
	using Root				  = RF_<Config_, Composite  <THead, TSubStates...>>;

	template <				  typename... TSubStates>
	using PeerRoot			  = RF_<Config_, CompositePeers  <  TSubStates...>>;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	template <typename THead, typename... TSubStates>
	using ResumableRoot		  = RF_<Config_, Resumable  <THead, TSubStates...>>;

	template <				  typename... TSubStates>
	using ResumablePeerRoot	  = RF_<Config_, ResumablePeers  <  TSubStates...>>;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	template <typename THead, typename... TSubStates>
	using UtilitarianRoot	  = RF_<Config_, Utilitarian<THead, TSubStates...>>;

	template <				  typename... TSubStates>
	using UtilitarianPeerRoot = RF_<Config_, UtilitarianPeers<  TSubStates...>>;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	template <typename THead, typename... TSubStates>
	using RandomRoot		  = RF_<Config_, Random		<THead, TSubStates...>>;

	template <				  typename... TSubStates>
	using RandomPeerRoot	  = RF_<Config_, RandomPeers	 <  TSubStates...>>;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	template <typename THead, typename... TSubStates>
	using OrthogonalRoot	  = RF_<Config_, Orthogonal <THead, TSubStates...>>;

	template <				  typename... TSubStates>
	using OrthogonalPeerRoot  = RF_<Config_, OrthogonalPeers <  TSubStates...>>;

	//----------------------------------------------------------------------
};

////////////////////////////////////////////////////////////////////////////////

}

using Machine = detail::M_<Config>;

template <typename TConfig>
using MachineT = detail::M_<TConfig>;

////////////////////////////////////////////////////////////////////////////////

}

namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

template <typename TConfig,
		  typename TApex>
class R_ {
	using Config_				= TConfig;
	using Context				= typename Config_::Context;
	using Rank					= typename Config_::Rank;
	using Utility				= typename Config_::Utility;
	using RNG					= typename Config_::RNG;
	using Logger				= typename Config_::Logger;

	using Apex					= TApex;
	using ApexInfo				= WrapInfo<Apex>;

	using Info					= RF_<Config_, Apex>;
	using StateList				= typename Info::StateList;
	using RegionList			= typename Info::RegionList;

	static constexpr LongIndex SUBSTITUTION_LIMIT = Info::SUBSTITUTION_LIMIT;
	static constexpr LongIndex TASK_CAPACITY	  = Info::TASK_CAPACITY;

public:
	static constexpr LongIndex  REVERSE_DEPTH	  = ApexInfo::REVERSE_DEPTH;
	static constexpr ShortIndex COMPO_REGIONS	  = ApexInfo::COMPO_REGIONS;
	static constexpr LongIndex  COMPO_PRONGS	  = ApexInfo::COMPO_PRONGS;
	static constexpr ShortIndex ORTHO_REGIONS	  = ApexInfo::ORTHO_REGIONS;
	static constexpr ShortIndex ORTHO_UNITS		  = ApexInfo::ORTHO_UNITS;

	static constexpr LongIndex  ACTIVE_BITS		  = ApexInfo::ACTIVE_BITS;
	static constexpr LongIndex  RESUMABLE_BITS	  = ApexInfo::RESUMABLE_BITS;

	static constexpr LongIndex  STATE_COUNT		  = ApexInfo::STATE_COUNT;
	static constexpr LongIndex  REGION_COUNT	  = ApexInfo::REGION_COUNT;

	static_assert(STATE_COUNT <  (ShortIndex) -1, "Too many states in the hierarchy. Change 'ShortIndex' type.");
	static_assert(STATE_COUNT == (ShortIndex) StateList::SIZE, "STATE_COUNT != StateList::SIZE");

private:
	using Args					= typename Info::Args;

	using Registry				= RegistryT<Args>;
	using CompoForks			= typename Registry::CompoForks;
	using RegistryBackUp		= typename Registry::BackUp;

	using MaterialApex			= Material<I_<0, 0, 0, 0>, Args, Apex>;

	using Control				= ControlT<Args>;

	using PlanControl			= PlanControlT<Args>;
	using PlanData				= PlanDataT   <Args>;

	using FullControl			= FullControlT<Args>;
	using Requests				= typename FullControl::Requests;

	using GuardControl			= GuardControlT<Args>;

#ifdef HFSM_ENABLE_STRUCTURE_REPORT
	static constexpr LongIndex NAME_COUNT	  = MaterialApex::NAME_COUNT;

	using Prefix				= StaticArray<wchar_t, REVERSE_DEPTH * 2 + 2>;
	using Prefixes				= StaticArray<Prefix, STATE_COUNT>;

	using StructureStateInfos	= typename Args::StructureStateInfos;
#endif

#ifdef HFSM_ENABLE_SERIALIZATION
	using WriteStream			= typename Args::WriteStream;
	using ReadStream			= typename Args::ReadStream;
#endif

public:
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef HFSM_ENABLE_STRUCTURE_REPORT
	using Structure				= Array<StructureEntry, NAME_COUNT>;
	using ActivityHistory		= Array<char,			NAME_COUNT>;
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:
	explicit R_(Context& context,
				RNG& rng
				HFSM_IF_LOGGER(, Logger* const logger = nullptr));

	~R_();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	template <typename T>
	static constexpr StateID  stateId()				{ return			StateList ::template index<T>();	}

	template <typename T>
	static constexpr RegionID regionId()			{ return (RegionID) RegionList::template index<T>();	}

	//----------------------------------------------------------------------

#ifdef HFSM_EXPLICIT_MEMBER_SPECIALIZATION

#ifdef _MSC_VER
	#pragma warning(push)
	#pragma warning(disable: 4348) // redefinition of default parameter: parameter 2
#endif

	template <typename T, bool = StateList::template contains<T>()>
	struct Accessor;

#ifdef _MSC_VER
	#pragma warning(pop)
#endif

	template <typename T>
	struct Accessor<T, true> {
		HFSM_INLINE static		 T& get(	  MaterialApex& apex)	{ return apex.template access<T>();		}
		HFSM_INLINE static const T& get(const MaterialApex& apex)	{ return apex.template access<T>();		}
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	// if you see..
	// VS	 - error C2027: use of undefined type 'hfsm2::detail::R_<..>::Accessor<T,false>'
	// Clang - error : implicit instantiation of undefined template 'hfsm2::detail::R_<..>::Accessor<*, false>'
	//
	// .. you're trying to access() a type that is not present in the state machine hierarchy

	template <typename T>
	HFSM_INLINE		  T& access()									{ return Accessor<T>::get(_apex);		}

	template <typename T>
	HFSM_INLINE const T& access() const								{ return Accessor<T>::get(_apex);		}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#else

	template <typename T>
	HFSM_INLINE		  T& access()					{ return Accessor<T,	   MaterialApex>{_apex}.get();	}

	template <typename T>
	HFSM_INLINE const T& access() const				{ return Accessor<T, const MaterialApex>{_apex}.get();	}

#endif

	//----------------------------------------------------------------------

	void update();

	template <typename TEvent>
	HFSM_INLINE void react(const TEvent& event);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE bool isActive   (const StateID stateId) const	{ return _registry.isActive   (stateId);	}
	HFSM_INLINE bool isResumable(const StateID stateId) const	{ return _registry.isResumable(stateId);	}

	HFSM_INLINE bool isScheduled(const StateID stateId) const	{ return isResumable(stateId);				}

	template <typename TState>
	HFSM_INLINE bool isActive   () const						{ return isActive	(stateId<TState>());	}

	template <typename TState>
	HFSM_INLINE bool isResumable() const						{ return isResumable(stateId<TState>());	}

	template <typename TState>
	HFSM_INLINE bool isScheduled() const						{ return isResumable<TState>();				}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	HFSM_INLINE void changeTo (const StateID stateId);
	HFSM_INLINE void restart  (const StateID stateId);
	HFSM_INLINE void resume	  (const StateID stateId);
	HFSM_INLINE void utilize  (const StateID stateId);
	HFSM_INLINE void randomize(const StateID stateId);
	HFSM_INLINE void schedule (const StateID stateId);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	template <typename TState>
	HFSM_INLINE void changeTo ()								{ changeTo (stateId<TState>());				}

	template <typename TState>
	HFSM_INLINE void restart  ()								{ restart  (stateId<TState>());				}

	template <typename TState>
	HFSM_INLINE void resume	  ()								{ resume   (stateId<TState>());				}

	template <typename TState>
	HFSM_INLINE void utilize  ()								{ utilize  (stateId<TState>());				}

	template <typename TState>
	HFSM_INLINE void randomize()								{ randomize(stateId<TState>());				}

	template <typename TState>
	HFSM_INLINE void schedule ()								{ schedule (stateId<TState>());				}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef HFSM_ENABLE_STRUCTURE_REPORT
	const Structure&	   structure()		 const				{ return _structure;						}
	const ActivityHistory& activityHistory() const				{ return _activityHistory;					}
#endif

#ifdef HFSM_ENABLE_TRANSITION_HISTORY
	using TransitionHistory		= Array<Transition, COMPO_REGIONS * 4>;

	const TransitionHistory& transitionHistory() const			{ return _transitionHistory;				}

	void replayTransition (const Transition& transition)		{ replayTransitions(&transition, 1);		}
	void replayTransitions(const Transition* const transitions, const uint64_t count);
#endif

	void reset();

#ifdef HFSM_ENABLE_SERIALIZATION
	// Buffer for serialization
	//  Members:
	//   bitSize - Number of payload bits used
	//   payload - Serialized data
	//  See https://doc.hfsm.dev/user-guide/debugging-and-tools/serialization
	using SerialBuffer			= typename Args::SerialBuffer;

	// Serialize the structural configuration into 'buffer'
	void save(		SerialBuffer& buffer) const;

	// De-serialize the configuration and initialize the instance
	void load(const SerialBuffer& buffer);
#endif

#if defined HFSM_ENABLE_LOG_INTERFACE || defined HFSM_ENABLE_VERBOSE_DEBUG_LOG
	void attachLogger(Logger* const logger)						{ _logger = logger;							}
#endif

private:
	void initialEnter();
	void processTransitions();

	bool applyRequest (Control& control, const Request& request);
	bool applyRequests(Control& control);

#ifdef HFSM_ENABLE_TRANSITION_HISTORY
	bool applyRequests(Control& control,
					   const Transition* const transitions,
					   const uint64_t count);
#endif

	bool cancelledByEntryGuards(const Requests& pendingChanges);
	bool cancelledByGuards(const Requests& pendingChanges);

#ifdef HFSM_ENABLE_STRUCTURE_REPORT
	void getStateNames();
	void udpateActivity();
#endif

	HFSM_IF_TRANSITION_HISTORY(void recordRequestsAs(const Method method));

private:
	Context& _context;
	RNG& _rng;

	Registry _registry;
	PlanData _planData;

	Requests _requests;

	MaterialApex _apex;

#ifdef HFSM_ENABLE_STRUCTURE_REPORT
	Prefixes _prefixes;
	StructureStateInfos _stateInfos;

	Structure _structure;
	ActivityHistory _activityHistory;
#endif

	HFSM_IF_TRANSITION_HISTORY(TransitionHistory _transitionHistory);

	HFSM_IF_LOGGER(Logger* _logger);
};

////////////////////////////////////////////////////////////////////////////////

template <typename TConfig,
		  typename TApex>
class RW_ final
	: public R_<TConfig, TApex>
{
public:
	using R_<TConfig, TApex>::R_;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN_,
		  typename TU_,
		  typename TR_,
		  LongIndex NS,
		  LongIndex NT,
		  typename TApex>
class RW_	   <::hfsm2::ConfigT<::hfsm2::EmptyContext, TN_, TU_, TR_, NS, NT>, TApex> final
	: public R_<::hfsm2::ConfigT<::hfsm2::EmptyContext, TN_, TU_, TR_, NS, NT>, TApex>
	, ::hfsm2::EmptyContext
{
	using Config_	= ::hfsm2::ConfigT<::hfsm2::EmptyContext, TN_, TU_, TR_, NS, NT>;
	using Context	= typename Config_::Context;
	using RNG		= typename Config_::RNG;
	using Logger	= typename Config_::Logger;

	using R			= R_<Config_, TApex>;

public:
	explicit HFSM_INLINE RW_(RNG& rng
							 HFSM_IF_LOGGER(, Logger* const logger = nullptr))
		: R{static_cast<Context&>(*this),
			rng
			HFSM_IF_LOGGER(, logger)}
	{}
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TC_,
		  typename TN_,
		  typename TU_,
		  LongIndex NS,
		  LongIndex NT,
		  typename TApex>
class RW_	   <::hfsm2::ConfigT<TC_, TN_, TU_, ::hfsm2::RandomT<TU_>, NS, NT>, TApex> final
	: public R_<::hfsm2::ConfigT<TC_, TN_, TU_, ::hfsm2::RandomT<TU_>, NS, NT>, TApex>
	, ::hfsm2::RandomT<TU_>
{
	using Config_	= ::hfsm2::ConfigT<TC_, TN_, TU_, ::hfsm2::RandomT<TU_>, NS, NT>;
	using Context	= typename Config_::Context;
	using RNG		= typename Config_::RNG;
	using Logger	= typename Config_::Logger;

	using R			= R_<Config_, TApex>;

public:
	explicit HFSM_INLINE RW_(Context& context
							 HFSM_IF_LOGGER(, Logger* const logger = nullptr))
		: R{context,
			static_cast<RNG&>(*this)
			HFSM_IF_LOGGER(, logger)}
		, RNG{0}
	{}
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TN_,
		  typename TU_,
		  LongIndex NS,
		  LongIndex NT,
		  typename TApex>
class RW_	   <::hfsm2::ConfigT<::hfsm2::EmptyContext, TN_, TU_, ::hfsm2::RandomT<TU_>, NS, NT>, TApex> final
	: public R_<::hfsm2::ConfigT<::hfsm2::EmptyContext, TN_, TU_, ::hfsm2::RandomT<TU_>, NS, NT>, TApex>
	, ::hfsm2::EmptyContext
	, ::hfsm2::RandomT<TU_>
{
	using Config_	= ::hfsm2::ConfigT<::hfsm2::EmptyContext, TN_, TU_, ::hfsm2::RandomT<TU_>, NS, NT>;
	using Context	= typename Config_::Context;
	using RNG		= typename Config_::RNG;
	using Logger	= typename Config_::Logger;

	using R			= R_<Config_, TApex>;

public:
	explicit HFSM_INLINE RW_(HFSM_IF_LOGGER(Logger* const logger = nullptr))
		: R{static_cast<Context&>(*this),
			static_cast<RNG&>(*this)
			HFSM_IF_LOGGER(, logger)}
		, RNG{0}
	{}
};

////////////////////////////////////////////////////////////////////////////////

}
}

namespace hfsm2 {
namespace detail {

////////////////////////////////////////////////////////////////////////////////

template <typename TG, typename TA>
R_<TG, TA>::R_(Context& context,
			   RNG& rng
			   HFSM_IF_LOGGER(, Logger* const logger))
	: _context{context}
	, _rng{rng}
	HFSM_IF_LOGGER(, _logger{logger})
{
	_apex.deepRegister(_registry, Parent{});

	HFSM_IF_STRUCTURE(getStateNames());

	initialEnter();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TG, typename TA>
R_<TG, TA>::~R_() {
	PlanControl control{_context,
						_rng,
						_registry,
						_planData,
						HFSM_LOGGER_OR(_logger, nullptr)};

	_apex.deepExit	  (control);
	_apex.deepDestruct(control);

	HFSM_IF_ASSERT(_planData.verifyPlans());
}

//------------------------------------------------------------------------------

template <typename TG, typename TA>
void
R_<TG, TA>::update() {
	FullControl control(_context,
						_rng,
						_registry,
						_planData,
						_requests,
						HFSM_LOGGER_OR(_logger, nullptr));

	_apex.deepUpdate(control);

	HFSM_IF_ASSERT(_planData.verifyPlans());

	if (_requests.count())
		processTransitions();

	_requests.clear();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TG, typename TA>
template <typename TEvent>
void
R_<TG, TA>::react(const TEvent& event) {
	FullControl control{_context,
						_rng,
						_registry,
						_planData,
						_requests,
						HFSM_LOGGER_OR(_logger, nullptr)};

	_apex.deepReact(control, event);

	HFSM_IF_ASSERT(_planData.verifyPlans());

	if (_requests.count())
		processTransitions();

	_requests.clear();
}

//------------------------------------------------------------------------------

template <typename TG, typename TA>
void
R_<TG, TA>::changeTo(const StateID stateId) {
	_requests.append(Request{Request::Type::CHANGE, stateId});

	HFSM_LOG_TRANSITION(_context, INVALID_STATE_ID, TransitionType::CHANGE, stateId);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TG, typename TA>
void
R_<TG, TA>::restart(const StateID stateId) {
	_requests.append(Request{Request::Type::RESTART, stateId});

	HFSM_LOG_TRANSITION(_context, INVALID_STATE_ID, TransitionType::RESTART, stateId);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TG, typename TA>
void
R_<TG, TA>::resume(const StateID stateId) {
	_requests.append(Request{Request::Type::RESUME, stateId});

	HFSM_LOG_TRANSITION(_context, INVALID_STATE_ID, TransitionType::RESUME, stateId);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TG, typename TA>
void
R_<TG, TA>::utilize(const StateID stateId) {
	_requests.append(Request{Request::Type::UTILIZE, stateId});

	HFSM_LOG_TRANSITION(_context, INVALID_STATE_ID, TransitionType::UTILIZE, stateId);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TG, typename TA>
void
R_<TG, TA>::randomize(const StateID stateId) {
	_requests.append(Request{Request::Type::RANDOMIZE, stateId});

	HFSM_LOG_TRANSITION(_context, INVALID_STATE_ID, TransitionType::RANDOMIZE, stateId);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TG, typename TA>
void
R_<TG, TA>::schedule(const StateID stateId) {
	_requests.append(Request{Request::Type::SCHEDULE, stateId});

	HFSM_LOG_TRANSITION(_context, INVALID_STATE_ID, TransitionType::SCHEDULE, stateId);
}

//------------------------------------------------------------------------------

#ifdef HFSM_ENABLE_TRANSITION_HISTORY

template <typename TG, typename TA>
void
R_<TG, TA>::replayTransitions(const Transition* const transitions,
							  const uint64_t count)
{
	if (HFSM_CHECKED(transitions && count)) {
		HFSM_IF_TRANSITION_HISTORY(_transitionHistory.clear());

		PlanControl control{_context,
							_rng,
							_registry,
							_planData,
							HFSM_LOGGER_OR(_logger, nullptr)};

		if (applyRequests(control, transitions, count)) {
			_apex.deepChangeToRequested(control);

			_registry.clearRequests();

			HFSM_IF_ASSERT(_planData.verifyPlans());
		}

		HFSM_IF_STRUCTURE(udpateActivity());
	}
}

#endif

//------------------------------------------------------------------------------

template <typename TG, typename TA>
void
R_<TG, TA>::reset() {
	PlanControl control{_context,
						_rng,
						_registry,
						_planData,
						HFSM_LOGGER_OR(_logger, nullptr)};

	_apex.deepExit	   (control);
	_apex.deepDestruct (control);

	_registry.reset();

	_apex.deepRequestChange(control);
	_apex.deepConstruct(control);
	_apex.deepEnter	   (control);
}

//------------------------------------------------------------------------------

#ifdef HFSM_ENABLE_SERIALIZATION

template <typename TG, typename TA>
void
R_<TG, TA>::save(SerialBuffer& _buffer) const {
	WriteStream stream{_buffer};

	_apex.deepSaveActive(_registry, stream);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TG, typename TA>
void
R_<TG, TA>::load(const SerialBuffer& buffer) {
	PlanControl control{_context,
						_rng,
						_registry,
						_planData,
						HFSM_LOGGER_OR(_logger, nullptr)};

	_apex.deepExit	   (control);
	_apex.deepDestruct (control);

	ReadStream stream{buffer};
	_apex.deepLoadRequested(_registry, stream);

	_apex.deepConstruct(control);
	_apex.deepEnter	   (control);
}

#endif

//------------------------------------------------------------------------------

template <typename TG, typename TA>
void
R_<TG, TA>::initialEnter() {
	HFSM_ASSERT(_requests.count() == 0);
	HFSM_IF_TRANSITION_HISTORY(HFSM_ASSERT(_transitionHistory.count() == 0));

	RegistryBackUp undo;
	HFSM_IF_TRANSITION_HISTORY(TransitionHistory undoTransitionHistory);

	Requests lastRequests;

	PlanControl control{_context,
						_rng,
						_registry,
						_planData,
						HFSM_LOGGER_OR(_logger, nullptr)};

	_apex.deepRequestChange(control);

	cancelledByEntryGuards(_requests);

	for (LongIndex i = 0;
		 i < SUBSTITUTION_LIMIT && _requests.count();
		 ++i)
	{
		backup(_registry, undo);
		HFSM_IF_TRANSITION_HISTORY(undoTransitionHistory = _transitionHistory);

		if (applyRequests(control)) {
			lastRequests = _requests;
			_requests.clear();

			if (cancelledByEntryGuards(lastRequests)) {
				restore(_registry, undo);
				HFSM_IF_TRANSITION_HISTORY(_transitionHistory = undoTransitionHistory);
			}
		} else
			_requests.clear();
	}
	HFSM_ASSERT(_requests.count() == 0);

	_apex.deepConstruct(control);
	_apex.deepEnter	   (control);

	_registry.clearRequests();

	HFSM_IF_ASSERT(_planData.verifyPlans());

	HFSM_IF_STRUCTURE(udpateActivity());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TG, typename TA>
void
R_<TG, TA>::processTransitions() {
	HFSM_ASSERT(_requests.count());

	HFSM_IF_TRANSITION_HISTORY(_transitionHistory.clear());

	RegistryBackUp undo;
	HFSM_IF_TRANSITION_HISTORY(TransitionHistory undoTransitionHistory);

	Requests lastRequests;

	PlanControl control{_context,
						_rng,
						_registry,
						_planData,
						HFSM_LOGGER_OR(_logger, nullptr)};

	bool changesMade = false;

	for (LongIndex i = 0;
		i < SUBSTITUTION_LIMIT && _requests.count();
		++i)
	{
		backup(_registry, undo);
		HFSM_IF_TRANSITION_HISTORY(undoTransitionHistory = _transitionHistory);

		if (applyRequests(control)) {
			lastRequests = _requests;
			_requests.clear();

			if (cancelledByGuards(lastRequests)) {
				restore(_registry, undo);
				HFSM_IF_TRANSITION_HISTORY(_transitionHistory = undoTransitionHistory);
			} else
				changesMade = true;
		} else
			_requests.clear();
	}

	if (changesMade) {
		_apex.deepChangeToRequested(control);

		_registry.clearRequests();

		HFSM_IF_ASSERT(_planData.verifyPlans());
	}

	HFSM_IF_STRUCTURE(udpateActivity());
}

//------------------------------------------------------------------------------

template <typename TG, typename TA>
bool
R_<TG, TA>::applyRequest(Control& control,
						 const Request& request)
{
	HFSM_IF_TRANSITION_HISTORY(_transitionHistory.append(Transition{request, Method::NONE}));

	switch (request.type) {
	case Request::CHANGE:
	case Request::RESTART:
	case Request::RESUME:
	case Request::UTILIZE:
	case Request::RANDOMIZE:
		if (_registry.requestImmediate(request))
			_apex.deepForwardActive(control, request.type);
		else
			_apex.deepRequest	   (control, request.type);

		return true;

	case Request::SCHEDULE:
		_registry.requestScheduled(request.stateId);

		return false;

	default:
		HFSM_BREAK();

		return false;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TG, typename TA>
bool
R_<TG, TA>::applyRequests(Control& control) {
	bool changesMade = false;

	for (const Request& request : _requests)
		changesMade |= applyRequest(control, request);

	return changesMade;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef HFSM_ENABLE_TRANSITION_HISTORY

template <typename TG, typename TA>
bool
R_<TG, TA>::applyRequests(Control& control,
						  const Transition* const transitions,
						  const uint64_t count)
{
	if (HFSM_CHECKED(transitions && count)) {
		bool changesMade = false;

		for (uint64_t i = 0; i < count; ++i)
			changesMade |= applyRequest(control, transitions[i].request());

		return changesMade;
	} else
		return false;
}

#endif

//------------------------------------------------------------------------------

template <typename TG, typename TA>
bool
R_<TG, TA>::cancelledByEntryGuards(const Requests& pendingRequests) {
	GuardControl guardControl{_context,
							  _rng,
							  _registry,
							  _planData,
							  _requests,
							  pendingRequests,
							  HFSM_LOGGER_OR(_logger, nullptr)};

	if (_apex.deepEntryGuard(guardControl)) {
		HFSM_IF_TRANSITION_HISTORY(recordRequestsAs(Method::ENTRY_GUARD));

		return true;
	} else
		return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TG, typename TA>
bool
R_<TG, TA>::cancelledByGuards(const Requests& pendingRequests) {
	GuardControl guardControl{_context,
							  _rng,
							  _registry,
							  _planData,
							  _requests,
							  pendingRequests,
							  HFSM_LOGGER_OR(_logger, nullptr)};

	if (_apex.deepForwardExitGuard(guardControl)) {
		HFSM_IF_TRANSITION_HISTORY(recordRequestsAs(Method::EXIT_GUARD));

		return true;
	} else if (_apex.deepForwardEntryGuard(guardControl)) {
		HFSM_IF_TRANSITION_HISTORY(recordRequestsAs(Method::ENTRY_GUARD));

		return true;
	} else
		return false;
}

//------------------------------------------------------------------------------

#ifdef HFSM_ENABLE_STRUCTURE_REPORT

template <typename TG, typename TA>
void
R_<TG, TA>::getStateNames() {
	_stateInfos.clear();
	_apex.deepGetNames((LongIndex) -1, StructureStateInfo::COMPOSITE, 0, _stateInfos);

	LongIndex margin = (LongIndex) -1;
	for (LongIndex s = 0; s < _stateInfos.count(); ++s) {
		const auto& state = _stateInfos[s];
		auto& prefix      = _prefixes[s];

		if (margin > state.depth && state.name[0] != '\0')
			margin = state.depth;

		if (state.depth == 0)
			prefix[0] = L'\0';
		else {
			const LongIndex mark = state.depth * 2 - 1;

			prefix[mark + 0] = state.region == StructureStateInfo::COMPOSITE ? L'└' : L'╙';
			prefix[mark + 1] = L' ';
			prefix[mark + 2] = L'\0';

			for (auto d = mark; d > 0; --d)
				prefix[d - 1] = L' ';

			for (auto r = s; r > state.parent; --r) {
				auto& prefixAbove = _prefixes[r - 1];

				switch (prefixAbove[mark]) {
				case L' ':
					prefixAbove[mark] = state.region == StructureStateInfo::COMPOSITE ? L'│' : L'║';
					break;
				case L'└':
					prefixAbove[mark] = L'├';
					break;
				case L'╙':
					prefixAbove[mark] = L'╟';
					break;
				}
			}
		}
	}
	if (margin > 0)
		margin -= 1;

	_structure.clear();
	for (LongIndex s = 0; s < _stateInfos.count(); ++s) {
		const auto& state = _stateInfos[s];
		auto& prefix = _prefixes[s];
		const LongIndex space = state.depth * 2;

		if (state.name[0] != L'\0') {
			_structure.append(StructureEntry{false, &prefix[margin * 2], state.name});
			_activityHistory.append((char) 0);
		} else if (s + 1 < _stateInfos.count()) {
			auto& nextPrefix = _prefixes[s + 1];

			if (s > 0)
				for (LongIndex c = 0; c <= space; ++c)
					nextPrefix[c] = prefix[c];

			const LongIndex mark = space + 1;

			switch (nextPrefix[mark]) {
			case L'├':
				nextPrefix[mark] = state.depth == margin ? L'┌' : L'┬';
				break;
			case L'╟':
				nextPrefix[mark] = state.depth == margin ? L'╓' : L'╥';
				break;
			}
		}
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TG, typename TA>
void
R_<TG, TA>::udpateActivity() {
	for (LongIndex s = 0, i = 0; s < _stateInfos.count(); ++s)
		if (_stateInfos[s].name[0] != L'\0') {
			_structure[i].isActive = isActive(s);

			auto& activity = _activityHistory[i];

			if (_structure[i].isActive) {
				if (activity < 0)
					activity = +1;
				else
					activity = activity < INT8_MAX ? activity + 1 : activity;
			} else {
				if (activity > 0)
					activity = -1;
				else
					activity = activity > INT8_MIN ? activity - 1 : activity;
			}

			++i;
		}
}

#endif

//------------------------------------------------------------------------------

#ifdef HFSM_ENABLE_TRANSITION_HISTORY

template <typename TG, typename TA>
void
R_<TG, TA>::recordRequestsAs(const Method method) {
	for (const auto& request : _requests)
		_transitionHistory.append(Transition{request, method});
}

#endif

////////////////////////////////////////////////////////////////////////////////

}
}

#ifdef _MSC_VER
	#pragma warning(pop)
#endif
#ifdef __clang__
	#pragma clang diagnostic pop
#endif
#if defined(__GNUC__) || defined(__GNUG__)
	#pragma GCC diagnostic pop
#endif

#undef HFSM_INLINE
#undef HFSM_IF_LOGGER
#undef HFSM_LOGGER_OR
#undef HFSM_LOG_STATE_METHOD
#undef HFSM_IF_STRUCTURE
