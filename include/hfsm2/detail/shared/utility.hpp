#pragma once

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

	#define HFSM_LOG_TRANSITION(CONTEXT, ORIGIN, TRANSITION, DESTINATION)		\
		if (_logger)															\
			_logger->recordTransition(CONTEXT, ORIGIN, TRANSITION, DESTINATION)

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

	#define HFSM_LOG_TRANSITION(CONTEXT, ORIGIN, TRANSITION, DESTINATION)
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

#if defined _MSC_VER || defined __clang_major__ && __clang_major__ >= 7
	#define HFSM_EXPLICIT_MEMBER_SPECIALIZATION									1
#else
	#define HFSM_EXPLICIT_MEMBER_SPECIALIZATION									0
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

HFSM_IF_DEBUG(struct None {});

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

template <int N1_, int N2_>
struct Min {
	static constexpr auto VALUE = N1_ < N2_ ? N1_ : N2_;
};

//------------------------------------------------------------------------------

template <int N1_, int N2_>
struct Max {
	static constexpr auto VALUE = N1_ > N2_ ? N1_ : N2_;
};

//------------------------------------------------------------------------------

template <unsigned NCapacity>
struct UnsignedIndex {
	static constexpr LongIndex CAPACITY = NCapacity;

	using Type = typename std::conditional<CAPACITY <= UINT8_MAX,  uint8_t,
				 typename std::conditional<CAPACITY <= UINT16_MAX, uint16_t,
				 typename std::conditional<CAPACITY <= UINT32_MAX, uint32_t,
																   uint64_t>::type>::type>::type;

	static_assert(CAPACITY <= UINT64_MAX, "STATIC ASSERT");
};

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
