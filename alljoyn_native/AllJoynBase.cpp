#include "AllJoynBase.h"

//static volatile sig_atomic_t s_interrupt = false;
//
//static void CDECL_CALL SigIntHandler(int sig)
//{
//	QCC_UNUSED(sig);
//	s_interrupt = true;
//}

volatile sig_atomic_t AllJoynBase::mInterrupt = false;

void CDECL_CALL AllJoynBase::SigIntHandler(int sig)
{
	QCC_UNUSED(sig);
	AllJoynBase::mInterrupt = true;
}


AllJoynBase::AllJoynBase() {
	mBus = NULL;
	mObjectInterface = NULL;
}

QStatus AllJoynBase::CreateInterface()
{
	/* Create org.alljoyn.bus.samples.chat interface */
	mObjectInterface = NULL;
	QStatus status = mBus->CreateInterface(CHAT_SERVICE_INTERFACE_NAME, mObjectInterface);
	if (ER_OK == status) {
		mCreateInterfaceManaged();
		mObjectInterface->Activate();
		return status;
	}
	else {
		printf("Failed to create interface \"%s\" (%s)\n", CHAT_SERVICE_INTERFACE_NAME, QCC_StatusText(status));
	}

	return status;
}
/*
void AllJoynBase::Invoke(const InterfaceDescription::Member* member, Message& msg) {
	const MsgArg * pArgs;
	size_t numArgs = 0;
	msg->GetArgs(numArgs, pArgs);
	mStartInvokeManaged(member->name.c_str(), numArgs);
	for (size_t i = 0; i < numArgs; i++) {

		switch (pArgs[i].typeId) {
		case AllJoynTypeId::ALLJOYN_STRING:
			((SendStringArg)handlers[AllJoynTypeId::ALLJOYN_STRING])(pArgs[i].v_string.str);
			break;
		default:
			// Throw an error
			break;
		}
	}
	mFinishInvokeManaged();
}
*/

QStatus AllJoynBase::StartMessageBus(void)
{
	QStatus status = mBus->Start();

	if (ER_OK == status) {
		printf("BusAttachment started.\n");
	}
	else {
		printf("Start of BusAttachment failed (%s).\n", QCC_StatusText(status));
	}

	return status;
}


bool AllJoynBase::AddInterfaceMember(
	const char* name,
	const char* inputSig,
	const char* outSig,
	const char* argNames,
	int memberType) // 0 signal, 1 method
{
	std::string args = argNames;
	std::string input;
	if (!args.empty())
		args += ",";
	if (outSig != "")
		args += "reply";

	printf("Interface name = %s.\n", mObjectInterface->GetName());
	switch (memberType) {
	case 0:
		printf("Adding Signal %s \n", name);
		return mObjectInterface->AddSignal(name, inputSig, argNames, 0) == ER_OK;
	case 1:
		input = inputSig;

		printf("Adding method \"%s\" \"%s\" \"%s\" \"%s\" %d \n", name, input.c_str(), outSig, args.c_str(), 0);
		return mObjectInterface->AddMethod(name, input.c_str(), outSig, args.c_str(), 0) == ER_OK;

	default:
		return false;
	}
}

/** Connect, report the result to stdout, and return the status code. */
QStatus AllJoynBase::ConnectBusAttachment(void)
{
	QStatus status = mBus->Connect();

	if (ER_OK == status) {
		printf("Connect to '%s' succeeded.\n", mBus->GetConnectSpec().c_str());
	}
	else {
		printf("Failed to connect to '%s' (%s).\n", mBus->GetConnectSpec().c_str(), QCC_StatusText(status));
	}

	return status;
}

QStatus AllJoynBase::Initialize()
{
	QStatus status;
	if ((status = AllJoynInit()) != ER_OK) {
		return status;
	}
#ifdef ROUTER
	if ((status = AllJoynRouterInit()) != ER_OK) {
		AllJoynShutdown();
		return status;
	}
#endif

	/* Install SIGINT handler. */
	signal(SIGINT, AllJoynBase::SigIntHandler);

	status = ER_OK;

	/* Create message bus */
	mBus = new BusAttachment("chat", true);

	if (mBus) {
		if (ER_OK == status) {
			status = CreateInterface();
		}

		if (ER_OK == status) {

			RegisterBusListener();
		}

		if (ER_OK == status) {
			status = StartMessageBus();
		}

		if (ER_OK == status) {
			CreateBusObject();
		}
		return status;
	}
	else {
		status = ER_OUT_OF_MEMORY;
	}
	return status;
}
QStatus AllJoynBase::Start()
{
	QStatus status;
	status = RegisterBusObject();

	if (ER_OK == status) {
		status = ConnectBusAttachment();
	}

	return status;
}

QStatus AllJoynBase::Stop() {
	printf("Alljoyn stop in\n");

	if (mBus) {
		delete mBus;
		mBus = NULL;
	}

#ifdef ROUTER
	QStatus status;
	status = AllJoynRouterShutdown();
#endif
	return AllJoynShutdown();
}