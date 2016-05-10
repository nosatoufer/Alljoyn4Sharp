#ifndef SERVER_H_
#define SERVER_H_

#include "AllJoynBase.h"
#include "NativeBusObject.h"
#include "ConnectionListener.h"
#include <cassert>

//static const char* CHAT_SERVICE_NAME = "org.alljoyn.bus.samples"; // USED TO ADVERTISE THE SERVICE

/**
 * Class representing a server Alljoyn
 */
class Server : public AllJoynBase, public ISessionUser
{
public:
	Server(std::string prefix, std::string name);

	~Server();

	int Initialize();
	int Start();
	int Stop();

	int SendSignal(std::string intfName, MsgArg** msgs, int numArgs);
	void SetMethodCall(MethodCall func);
	void SetSignalCall(SignalEvent func);

	QStatus RegisterMethodHandler(const char * intfName);
	QStatus RegisterSignalHandler(const char * intfName);

	void setSessionId(SessionId id);
	void setJoined(bool joined);
	SessionId* getSessionId();
	bool isJoined();

protected:
	void RegisterBusListener();

private:

	std::string mPrefix;
	std::string mName;
	std::string mAdvertisedName;
	bool mJoined;
	SessionPort mPort;
	NativeBusObject * mBusObject;
	SessionId mSessionId;
	ConnectionListener * mConnectionListener;


	QStatus CreateBusObject(void);
	QStatus RegisterBusObject();
	QStatus RequestName();
	QStatus AdvertiseName(TransportMask mask);
	QStatus CreateSession(TransportMask mask);


};

#endif SERVER_H_
