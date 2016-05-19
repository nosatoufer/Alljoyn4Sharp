#ifndef ALLJOYNBASE_H_
#define ALLJOYNBASE_H_

#include <alljoyn/AllJoynStd.h>
#include <alljoyn/BusAttachment.h>
#include <alljoyn/BusObject.h>
#include <alljoyn/DBusStd.h>
#include <alljoyn/Init.h>
#include <alljoyn/InterfaceDescription.h>
#include <alljoyn/ProxyBusObject.h>
#include <qcc/Log.h>
#include <qcc/String.h>
#include <signal.h>
#include <iostream>
#include <string>
#include <map>

#ifndef WIN32
#define __stdcall
#endif

/* constants. */
static const char* CHAT_SERVICE_INTERFACE_NAME = "org.alljoyn.bus.samples.chat";
static const char* CHAT_SERVICE_OBJECT_PATH = "/chatService";

typedef void(__stdcall * CreateInterfaceCallback)();

/*
typedef int(__stdcall * SendIntArg)(int);
typedef int(__stdcall * SendDoubleArg)(double);
typedef int(__stdcall * SendStringArg)(const char * arg);
*/


typedef ajn::MsgArg*(__stdcall * MethodCall)(const char * member, void * msg);
typedef void(__stdcall * SignalEvent)(const char * member, void* msg);

using namespace ajn;

/* OLD 
struct  NativeSignal
{
	int numargs;
	MsgArg * args;
	void ** data;
};
*/
struct Connection {
	SessionId session;
};

/**
* Class defining common method for client and server
*/
class AllJoynBase
{
public:
	void RegisterCreateInterface(CreateInterfaceCallback handle) { mCreateInterfaceManaged = handle; }
	bool AddInterfaceMember(
		const char* name,
		const char* inputSig,
		const char* outSig,
		const char* argNames,
		int memberType); // 0 signal, 1 method

	/**
	 * DEPRECATED ?
	 */
	template<typename T> bool RegisterCallback(T callback, AllJoynTypeId type) {
		handlers.insert(std::pair<AllJoynTypeId, void *>(type, (void *)callback));
		return false;
	}

	//void Invoke(const InterfaceDescription::Member* member, Message& msg);

protected:

	AllJoynBase();
	QStatus Initialize();
	QStatus Start();
	QStatus Stop();

	virtual void RegisterBusListener() = 0;
	virtual QStatus CreateBusObject() = 0;
	virtual QStatus RegisterBusObject() = 0;

	ajn::BusAttachment* mBus;
	Connection mConnection;
	static volatile sig_atomic_t mInterrupt;
	static void CDECL_CALL SigIntHandler(int sig);
private:
	QStatus CreateInterface();
	QStatus StartMessageBus();

	QStatus ConnectBusAttachment(void);

	// Argument handles:
	CreateInterfaceCallback mCreateInterfaceManaged;
	std::map<AllJoynTypeId, void *> handlers;

	InterfaceDescription * mObjectInterface;
};

#endif ALLJOYNBASE_H_