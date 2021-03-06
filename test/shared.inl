////////////////////////////////////////////////////////////////////////////////

template <typename TContext>
void
LoggerT<TContext>::recordMethod(Context& /*context*/,
								const StateID origin,
								const Method method)
{
	REQUIRE(hfsm2::methodName(method));

	switch (method) {
		case Method::RANK:
			history.emplace_back(origin, Event::RANK);
			break;
		case Method::UTILITY:
			history.emplace_back(origin, Event::UTILITY);
			break;
		case Method::ENTRY_GUARD:
			history.emplace_back(origin, Event::ENTRY_GUARD);
			break;
		case Method::CONSTRUCT:
			history.emplace_back(origin, Event::CONSTRUCT);
			break;
		case Method::ENTER:
			history.emplace_back(origin, Event::ENTER);
			break;
		case Method::REENTER:
			history.emplace_back(origin, Event::REENTER);
			break;
		case Method::UPDATE:
			history.emplace_back(origin, Event::UPDATE);
			break;
		case Method::REACT:
			history.emplace_back(origin, Event::REACT);
			break;
		case Method::EXIT_GUARD:
			history.emplace_back(origin, Event::EXIT_GUARD);
			break;
		case Method::EXIT:
			history.emplace_back(origin, Event::EXIT);
			break;
		case Method::DESTRUCT:
			history.emplace_back(origin, Event::DESTRUCT);
			break;
		case Method::PLAN_SUCCEEDED:
			history.emplace_back(origin, Event::PLAN_SUCCEEDED);
			break;
		case Method::PLAN_FAILED:
			history.emplace_back(origin, Event::PLAN_FAILED);
			break;
		default:
			HFSM_BREAK();
	}
}

//------------------------------------------------------------------------------

template <typename TContext>
void
LoggerT<TContext>::recordTransition(Context& /*context*/,
									const StateID origin,
									const TransitionType transitionType,
									const StateID target)
{
	REQUIRE(hfsm2::transitionName(transitionType));

	switch (transitionType) {
		case TransitionType::CHANGE:
			history.emplace_back(origin, Event::CHANGE,    target);
			break;

		case TransitionType::RESTART:
			history.emplace_back(origin, Event::RESTART,   target);
			break;

		case TransitionType::RESUME:
			history.emplace_back(origin, Event::RESUME,    target);
			break;

		case TransitionType::UTILIZE:
			history.emplace_back(origin, Event::UTILIZE,   target);
			break;

		case TransitionType::RANDOMIZE:
			history.emplace_back(origin, Event::RANDOMIZE, target);
			break;

		case TransitionType::SCHEDULE:
			history.emplace_back(origin, Event::SCHEDULE,  target);
			break;

		default:
			HFSM_BREAK();
	}
}

//------------------------------------------------------------------------------

template <typename TContext>
void
LoggerT<TContext>::recordTaskStatus(Context& /*context*/,
									const RegionID region,
									const StateID origin,
									const StatusEvent event)
{
	switch (event) {
		case StatusEvent::SUCCEEDED:
			history.emplace_back(region, Event::TASK_SUCCESS, origin);
			break;
		case StatusEvent::FAILED:
			history.emplace_back(region, Event::TASK_FAILURE, origin);
			break;
		default:
			HFSM_BREAK();
	}
}

//------------------------------------------------------------------------------

template <typename TContext>
void
LoggerT<TContext>::recordPlanStatus(Context& /*context*/,
									const RegionID region,
									const StatusEvent event)
{
	switch (event) {
		case StatusEvent::SUCCEEDED:
			history.emplace_back(region, Event::PLAN_SUCCESS, hfsm2::INVALID_STATE_ID);
			break;
		case StatusEvent::FAILED:
			history.emplace_back(region, Event::PLAN_FAILURE, hfsm2::INVALID_STATE_ID);
			break;
		default:
			HFSM_BREAK();
	}
}

//------------------------------------------------------------------------------

template <typename TContext>
void
LoggerT<TContext>::recordUtilityResolution(Context& /*context*/,
										   const StateID head,
										   const StateID prong,
										   const Utilty utilty)
{
	history.emplace_back(head, Event::UTILITY_RESOLUTION, prong, utilty);
}

//------------------------------------------------------------------------------

template <typename TContext>
void
LoggerT<TContext>::recordRandomResolution(Context& /*context*/,
										  const StateID head,
										  const StateID prong,
										  const Utilty utilty)
{
	history.emplace_back(head, Event::RANDOM_RESOLUTION, prong, utilty);
}

//------------------------------------------------------------------------------

template <typename TContext>
void
LoggerT<TContext>::recordCancelledPending(Context& /*context*/,
										  const StateID origin)
{
	history.emplace_back(origin, Event::CANCEL_PENDING);
}

//------------------------------------------------------------------------------

template <typename TContext>
void
LoggerT<TContext>::assertSequence(const Events& reference) {
	const auto count = std::max(history.size(), reference.size());

	for (unsigned i = 0; i < count; ++i) {
		REQUIRE(i < history.size()); //-V521
		REQUIRE(i < reference.size()); //-V521

		if (i < history.size() &&
			i < reference.size())
		{
			REQUIRE(history[i].type	  == reference[i].type); //-V521
			REQUIRE(history[i].origin == reference[i].origin); //-V521
			REQUIRE(history[i].target == reference[i].target); //-V521
		}
	}

	history.clear();
}

//------------------------------------------------------------------------------

template <typename TMachine>
void assertActive(TMachine& machine,
				  const Types& all,
				  const Types& toCheck)
{
	for (const auto& type : all) {
		if (std::find(toCheck.begin(), toCheck.end(), type) != toCheck.end())
			REQUIRE( machine.isActive(type)); //-V521
		else
			REQUIRE(!machine.isActive(type)); //-V521
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename TMachine>
void assertResumable(TMachine& machine,
					 const Types& all,
					 const Types& toCheck)
{
	for (const auto& type : all) {
		if (std::find(toCheck.begin(), toCheck.end(), type) != toCheck.end())
			REQUIRE( machine.isResumable(type)); //-V521
		else
			REQUIRE(!machine.isResumable(type)); //-V521
	}
}

////////////////////////////////////////////////////////////////////////////////
