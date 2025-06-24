#include "BinaryStarFsm.h"

#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

BinaryStarFsm::BinaryStarFsm( StateName initState ) : mCurrentState{ initState } {
	init();
}

void BinaryStarFsm::init() {
	createStates();
	connectStates();
}

void BinaryStarFsm::connectStates() {
	connect( StateName::Primary, StateName::Active, EventName::PeerBackup );
	connect( StateName::Primary, StateName::Active, EventName::ClientRequest );
	connect( StateName::Primary, StateName::Passive, EventName::PeerActive );
	connect( StateName::Passive, StateName::Unknown, EventName::PeerPassive );
	connect( StateName::Passive, StateName::Active, EventName::PeerBackup );
	connect( StateName::Passive, StateName::Active, EventName::PeerPrimary );
	connect( StateName::Passive, StateName::Active, EventName::ClientRequest );
	connect( StateName::Active, StateName::Unknown, EventName::PeerActive );
	connect( StateName::Active, StateName::Active, EventName::ClientRequest );
	connect( StateName::Backup, StateName::Passive, EventName::PeerActive );
}

void BinaryStarFsm::createStates() {
	for ( const auto st : { StateName::Unknown, StateName::Primary, StateName::Backup, StateName::Active, StateName::Passive } ) {
		State state;
		state.name = st;
		mStates[ st ] = state;
	}
}

void BinaryStarFsm::connect( StateName src, StateName dest, EventName evt ) {
	auto &state = mStates.at( src );
	state.other[ evt ] = &mStates.at( dest );
	state.act[ evt ] = [ this, src, dest, evt ] { logTransition( src, dest, evt ); };
}

void BinaryStarFsm::trigger( EventName evt ) {
	if ( !mStates.contains( mCurrentState ) ) {
		return;
	}

	auto &src = mStates.at( mCurrentState );
	if ( !src.other.contains( evt ) || !src.act.contains( evt ) ) {
		return;
	}

	mCurrentState = src.other.at( evt )->name;

	src.act.at( evt )();
}

void BinaryStarFsm::logTransition( StateName src, StateName dest, EventName evt ) {
	spdlog::info( "Event: {}, transition from {} -> {}",
	              magic_enum::enum_name( evt ),
	              magic_enum::enum_name( src ),
	              magic_enum::enum_name( dest ) );
}

StateName BinaryStarFsm::getState() {
	return mCurrentState;
}

void BinaryStarFsm::set( StateName src, EventName evt, std::function<void( void )> callback ) {
	if ( !mStates.contains( src ) ) {
		spdlog::error( "Invalid {} state does not exist!", magic_enum::enum_name( src ) );
		return;
	}

	auto &srcState = mStates.at( src );

	if ( !srcState.other.contains( evt ) || !srcState.act.contains( evt ) ) {
		spdlog::error( "Invalid {} event does not exist!", magic_enum::enum_name( evt ) );
		return;
	}
	srcState.act.at( evt ) = std::move( callback );
}