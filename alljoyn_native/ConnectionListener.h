#pragma once

#include "ISessionUser.h"
#include <alljoyn\BusListener.h>
#include <alljoyn\SessionListener.h>
#include <alljoyn\SessionPortListener.h>
#include <alljoyn\BusAttachment.h>

using namespace ajn;

class ConnectionListener : public BusListener, public SessionListener, public SessionPortListener
{
private:
	std::string mPrefix;
	ajn::BusAttachment * mBus;
	SessionPort mPort;
	ISessionUser * mSessionUser;
public:

	ConnectionListener(std::string prefix, ajn::BusAttachment * bus, SessionPort port, ISessionUser * mSessionUser);
	~ConnectionListener();
	void FoundAdvertisedName(const char * name, TransportMask transport, const char * namePrefix);
	void LostAdvertisedName(const char * name, TransportMask transport, const char * namePrefix);
	void NameOwnerChanged(const char * busName, const char * previousOwner, const char * newOwner);
	bool AcceptSessionJoiner(SessionPort sessionPort, const char * joiner, const SessionOpts & opts);
	void SessionJoined(SessionPort sessionPort, SessionId id, const char * joiner);

};
