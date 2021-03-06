// HFSM2 (hierarchical state machine for games and interactive applications)
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

#include "detail/shared/utility.hpp"
#include "detail/shared/iterator.hpp"
#include "detail/shared/array.hpp"
#include "detail/shared/bit_array.hpp"
#include "detail/shared/bit_stream.hpp"
#include "detail/shared/list.hpp"
#include "detail/shared/random.hpp"
#include "detail/shared/type_list.hpp"

#include "detail/debug/shared.hpp"
#include "detail/debug/logger_interface.hpp"

#include "detail/root/plan_data.hpp"
#include "detail/root/plan.hpp"
#include "detail/root/registry.hpp"
#include "detail/root/control.hpp"
#include "detail/debug/structure_report.hpp"

#include "detail/structure/injections.hpp"
#include "detail/structure/state_box.hpp"
#include "detail/structure/state.hpp"
#include "detail/structure/forward.hpp"
#include "detail/structure/composite_sub.hpp"
#include "detail/structure/composite.hpp"
#include "detail/structure/orthogonal_sub.hpp"
#include "detail/structure/orthogonal.hpp"
#include "detail/root/state_access.hpp"

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

#include "detail/root.hpp"

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
