﻿// HFSM (hierarchical state machine for games and interactive applications)
// Created by Andrew Gresyk
//
// Attachable logger example:

// Full output:
//
//	---------- ctor: ---------
//
//	enter()
//	From::enter()
//
//	--------- update: --------
//
//	update()
//	From::update()
//
//	--------- react: ---------
//
//	react()
//	From::react()
//	changeTo<To>()
//	To::guard()
//	From::exit()
//	To::enter()
//
//	-- external transition: --
//
//	changeTo<From>()
//
//	--------- detach: --------
//
//
//	---------- dtor: ---------
//
//	To::exit()
//	exit()
//
//	---------- done! ---------

// enable logger functionality
#define HFSM_ENABLE_LOG_INTERFACE
#include <hfsm/machine.hpp>

#include <iostream>

//------------------------------------------------------------------------------

// data shared between FSM states and outside code
struct Context {};

// convenience typedef
using M = hfsm::Machine<Context>;

#define S(s) struct s

using FSM = M::Root<S(Top),
				S(From),
				S(To)
			>;

#undef S

//------------------------------------------------------------------------------

static_assert(FSM::contains<Top>(),	 "");
static_assert(FSM::contains<From>(), "");
static_assert(FSM::contains<To>(),	 "");

static_assert(FSM::stateId<Top>()  == 0, "");
static_assert(FSM::stateId<From>() == 1, "");
static_assert(FSM::stateId<To>()   == 2, "");

////////////////////////////////////////////////////////////////////////////////

struct Logger
	: hfsm::LoggerInterface
{
	void recordMethod(const hfsm::StateID /*origin*/,
					  const Method method) override
	{
		std::cout << /*hfsm::stateName(origin) << "::" <<*/ hfsm::methodName(method) << "()\n";
	}

	void recordTransition(const Transition transition,
						  const hfsm::StateID /*target*/) override
	{
		std::cout << hfsm::transitionName(transition) << "<" << /*hfsm::stateName(target) << ">()"*/ "\n";
	}
};

////////////////////////////////////////////////////////////////////////////////

// top-level state in the hierarchy
struct Top
	: FSM::Base // necessary boilerplate!
{
	// all state methods:
	void guard(TransitionControl&)					{}	// not going to be called in this example
	void enter(Control&)							{}
	void update(TransitionControl&)					{}
	template <typename TEvent>
	void react(const TEvent&, TransitionControl&)	{}
	void exit(Control&)								{}
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// initial state
struct From
	: FSM::Base
{
	// all state methods:
	void guard(TransitionControl&)					{}	// not going to be called in this example
	void enter(Control&)							{}
	void update(TransitionControl&)					{}
	template <typename TEvent>
	void react(const TEvent&, TransitionControl& control)	{ control.changeTo<To>(); }
	void exit(Control&)								{}
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// transition target state
struct To
	: FSM::Base
{
	// all state methods:
	void guard(TransitionControl&)					{}
	void enter(Control&)							{}
	void update(TransitionControl&)					{}
	template <typename TEvent>
	void react(const TEvent&, TransitionControl&)	{}	// not going to be called in this example
	void exit(Control&)								{}
};

////////////////////////////////////////////////////////////////////////////////

int main() {
	{
		// shared data storage instance
		Context context;

		// logger
		Logger logger;

		std::cout << "\n---------- ctor: ---------\n\n";

		// state machine instance - all initial states are activated
		FSM::Instance machine(context, &logger);

		// output:
		//	enter()
		//	From::enter()

		std::cout << "\n--------- update: --------\n\n";

		// first update
		machine.update();

		// output:
		//	update()
		//	From::update()

		std::cout << "\n--------- react: ---------\n\n";

		machine.react(1);

		// output:
		//	react()
		//	From::react()
		//	changeTo<To>()
		//	To::guard()
		//	From::exit()
		//	To::enter()

		std::cout << "\n-- external transition: --\n\n";

		machine.changeTo<From>();

		// output:
		//	changeTo<From>()

		std::cout << "\n--------- detach: --------\n\n";

		// detach logger and update again
		machine.attachLogger(nullptr);
		machine.update();

		// no output, since logger is detached

		std::cout << "\n---------- dtor: ---------\n\n";

		// re-attach logger for destruction log
		machine.attachLogger(&logger);

		// state machine instance gets destroyed
	}

	// output:
	//	To::exit()
	//	exit()

	std::cout << "\n---------- done! ---------\n\n";

	return 0;
}

////////////////////////////////////////////////////////////////////////////////