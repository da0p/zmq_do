#ifndef BINARY_STAR_FSM_H_
#define BINARY_STAR_FSM_H_
#include <cstdint>
#include <functional>
#include <unordered_map>

enum class StateName : uint8_t {
	Unknown = 0, // initial state, or invalid state
	Primary = 1, // primary, waiting for peer to connect
	Backup = 2,  // backup, waiting for peer to connect
	Active = 3,  // active - accepting connections
	Passive = 4, // passive - not accepting connections
};

enum class EventName {
	Unknown = 0,       // invalid event
	PeerPrimary = 1,   // peer is pending primary
	PeerBackup = 2,    // peer is pending backup
	PeerActive = 3,    // peer is active
	PeerPassive = 4,   // peer is passive
	ClientRequest = 5, // client makes request
};

struct State {
	StateName name;
	std::unordered_map<EventName, State *> other;
	std::unordered_map<EventName, std::function<void( void )>> act;
};

class BinaryStarFsm {
  public:
	BinaryStarFsm( StateName initState );

	void trigger( EventName event );

	[[nodiscard]] StateName getState();

	void set( StateName src, EventName evt, std::function<void( void )> callback );

  private:
	void init();

	void createStates();

	void connectStates();

	void connect( StateName src, StateName dest, EventName evt );

	void logTransition( StateName src, StateName dest, EventName evt );

	StateName mCurrentState{ StateName::Unknown };
	std::unordered_map<StateName, State> mStates;
};
#endif