#pragma once

#include "alljoyn\Session.h"

class ISessionUser {
public:
	virtual void setSessionId(ajn::SessionId id) = 0;
	virtual void setJoined(bool joined) = 0;
	virtual bool isJoined() = 0;
	virtual ajn::SessionId* getSessionId() = 0;
};