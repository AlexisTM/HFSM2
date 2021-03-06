#pragma once

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
