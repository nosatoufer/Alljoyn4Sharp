#ifndef CLIENT_H_
#define CLIENT_H_
#define NDEBUF

#include "AllJoynBase.h"
#include "ConnectionListener.h"
#include "NativeBusObject.h"

/**
* Class representing a client Alljoyn
*/
class Client : public AllJoynBase, public ISessionUser
{

public:
	Client(std::string prefix, std::string name);
	~Client();

	int Initialize();
	int Start();
	int Stop();
	//int Connect(std::string);
	//int Disconnect();
	void SetJoinComplete(bool joined);

	const Message* CallMethod(std::string member, MsgArg** msgs, int size);
	void SetSignalCall(SignalEvent func);
	QStatus RegisterSignalHandler(std::string memberName);

	void setSessionId(SessionId id);
	void setJoined(bool joined);
	SessionId* getSessionId();
	bool isJoined();

protected:
	void RegisterBusListener();
	QStatus CreateBusObject();
	QStatus RegisterBusObject();
private:

	NativeBusObject * mBusObject;
	qcc::String mJoinName;
	bool mJoinComplete = false;
	ConnectionListener * mConnectionListener;



	QStatus Client::FindAdvertisedName(void);
	QStatus Client::WaitForJoinSessionCompletion(void);

	std::string mPrefix;
	std::string mName;
	std::string mAdvertisedName;
	SessionPort mPort;
	SessionId mSessionId;

};
#endif CLIENT_H_