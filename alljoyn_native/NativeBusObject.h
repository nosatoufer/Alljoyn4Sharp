#pragma once

#include "alljoyn\BusObject.h"
#include "AllJoynBase.h"

using namespace ajn;

/**
 * Class managing the sending and reception of method calls and signals.
 */
class NativeBusObject : public BusObject
{
private:
	BusAttachment & mBus;
	ProxyBusObject * mProxy;
	MethodCall mMethodCall;
	SignalEvent mSignalCall;
	const InterfaceDescription* mIntf;
	SessionId * mSessionId;

public:
	NativeBusObject(BusAttachment& bus, std::string advName, std::string path, SessionId * sessionId, std::string intfName);

	~NativeBusObject();


	//Sends a Signal to the session
	QStatus SendSignal(std::string intfName, MsgArg* msgs[], int size);
	//Calls a method
	const Message* CallMethod(std::string member, MsgArg * msgs[], int size);

	//Sets the function that will be called on method received
	void SetMethodCall(MethodCall func);
	//Sets the function that will be called on method received
	void SetEventCall(SignalEvent func);

	//Adds the interface member to the list of registrer methods
	QStatus RegisterMethodHandler(std::string intfName);
	//Adds the interface member to the list of registrer signals
	QStatus RegisterEventHandler(std::string intfName);

	//Manages the received signals, will call the mSignalCall if not null
	void SignalHandler(const InterfaceDescription::Member* member, const char* srcPath, Message& msg);
	//Manages the received method calls, will call the mMethodCall if not null
	void MethodHandler(const InterfaceDescription::Member* member, Message& msg);
};

