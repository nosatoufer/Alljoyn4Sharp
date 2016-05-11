#include "client.h"



QStatus Client::RegisterBusObject(void)
{
	/* Create the bus object that will be used to send and receive signals */

	QStatus status = mBus->RegisterBusObject(*mBusObject);

	if (ER_OK == status) {
		printf("RegisterBusObject succeeded.\n");
	}
	else {
		printf("RegisterBusObject failed (%s).\n", QCC_StatusText(status));
	}

	return status;
}
void Client::SetSignalCall(SignalEvent func)
{
	mBusObject->SetEventCall(func);
}

QStatus Client::RegisterSignalHandler(std::string memberName)
{
	return mBusObject->RegisterEventHandler(memberName);
}

void Client::setSessionId(SessionId id)
{
	mSessionId = id;
}

void Client::setJoined(bool joined)
{
	mJoinComplete = joined;
}

SessionId* Client::getSessionId()
{
	return &mSessionId;
}

bool Client::isJoined()
{
	return mJoinComplete;
}

void Client::RegisterBusListener()
{
	mConnectionListener = new ConnectionListener(mPrefix, mBus, mPort,  this);
	mBus->RegisterBusListener(*mConnectionListener);
}

QStatus Client::CreateBusObject()
{
	if (mBusObject = new NativeBusObject(*mBus, mAdvertisedName, CHAT_SERVICE_OBJECT_PATH, &mSessionId, CHAT_SERVICE_INTERFACE_NAME))
		return QStatus::ER_OK;
	else
		return QStatus::ER_FAIL;
}

/** Begin discovery on the well-known name of the service to be called, report the result to
stdout, and return the result status. */
QStatus Client::FindAdvertisedName(void)
{
	/* Begin discovery on the well-known name of the service to be called */
	QStatus status = mBus->FindAdvertisedName(mAdvertisedName.c_str()); // 1

	if (status == ER_OK) {
		printf("org.alljoyn.Bus.FindAdvertisedName ('%s') succeeded.\n", mAdvertisedName.c_str());
	}
	else {
		printf("org.alljoyn.Bus.FindAdvertisedName ('%s') failed (%s).\n", mAdvertisedName.c_str(), QCC_StatusText(status));
	}
	return status;
}

/** Wait for join session to complete, report the event to stdout, and return the result status. */
QStatus Client::WaitForJoinSessionCompletion(void)
{
	unsigned int count = 0;

	while (!mJoinComplete && !AllJoynBase::mInterrupt) {
		if (0 == (count++ % 100)) {
			printf("Waited %u seconds for JoinSession completion.\n", count / 100);
		}

#ifdef _WIN32
		Sleep(10);
#else
		usleep(10 * 1000);
#endif
	}

	return (mJoinComplete && !AllJoynBase::mInterrupt) ? ER_OK : ER_ALLJOYN_JOINSESSION_REPLY_CONNECT_FAILED;
}



Client::Client(std::string prefix, std::string name) : AllJoynBase(), mName(name), mPrefix(prefix)
{
	mAdvertisedName = prefix + name;
	mBusObject = NULL;
	mPort = 27;
	mJoinComplete = false;

}

Client::~Client()
{
	mBus->UnregisterBusListener(*mConnectionListener);
	mBus->UnregisterBusObject(*mBusObject);
	AllJoynBase::Stop();
	delete mConnectionListener;
	//delete mBusObject;
}

int Client::Initialize()
{
	return AllJoynBase::Initialize();
}

int Client::Start()
{
	int ret = AllJoynBase::Start();
	if (ret != 0) {
		return ret;
	}

	QStatus status = ER_OK;
	if (mBus) {
		if (ER_OK == status) {
			status = FindAdvertisedName();
		}

		if (ER_OK == status) {
			status = WaitForJoinSessionCompletion();
		}
	}
	else {
		status = ER_OUT_OF_MEMORY;
	}

	return (int)status;
}


int Client::Stop() {
	return (int)AllJoynBase::Stop();

}

void Client::SetJoinComplete(bool joined)
{
	mJoinComplete = joined;
}

const Message * Client::CallMethod(std::string member, MsgArg ** msgs, int size)
{
	return mBusObject->CallMethod(member, msgs, size);

}
