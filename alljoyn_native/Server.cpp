
#include "Server.h"

using namespace ajn;

Server::Server(std::string prefix, std::string name) : AllJoynBase(), mName(name), mPrefix(prefix)
{
	mAdvertisedName = prefix + name;
	mBusObject = NULL;
	mPort = 27;
}


Server::~Server()
{
	printf("1\n");
	if (mConnectionListener)
	{
		mBus->UnregisterBusListener(*mConnectionListener);
		delete mConnectionListener;
	}
	printf("2\n");

	if (mBusObject)
		mBus->UnregisterBusObject(*mBusObject);
	printf("3\n");

	AllJoynBase::Stop();
	printf("4\n");

	printf("5\n");

}

QStatus Server::CreateBusObject(void)
{
	if (mBusObject = new NativeBusObject(*mBus, mAdvertisedName, CHAT_SERVICE_OBJECT_PATH, &mSessionId, CHAT_SERVICE_INTERFACE_NAME))
		return QStatus::ER_OK;
	else
		return QStatus::ER_FAIL;
}

QStatus Server::RegisterBusObject(void)
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



/** Request the service name, report the result to stdout, and return the status code. */
QStatus Server::RequestName(void)
{
	QStatus status = mBus->RequestName(mAdvertisedName.c_str(), DBUS_NAME_FLAG_DO_NOT_QUEUE);

	if (ER_OK == status) {
		printf("RequestName('%s') succeeded.\n", mAdvertisedName.c_str());
	}
	else {
		printf("RequestName('%s') failed (status=%s).\n", mAdvertisedName.c_str(), QCC_StatusText(status));
	}

	return status;
}

/** Create the session, report the result to stdout, and return the status code. */
QStatus Server::CreateSession(TransportMask mask)
{
	SessionOpts opts(SessionOpts::TRAFFIC_MESSAGES, true, SessionOpts::PROXIMITY_ANY, mask);
	QStatus status = mBus->BindSessionPort(mPort, opts, *mConnectionListener);

	if (ER_OK == status) {
		printf("BindSessionPort succeeded.\n");
	}
	else {
		printf("BindSessionPort failed (%s).\n", QCC_StatusText(status));
	}
	return status;
}

/** Advertise the service name, report the result to stdout, and return the status code. */
QStatus Server::AdvertiseName(TransportMask mask)
{
	QStatus status = mBus->AdvertiseName(mAdvertisedName.c_str(), mask);

	if (ER_OK == status) {
		printf("Advertisement of the service name '%s' succeeded.\n", mAdvertisedName.c_str());
	}
	else {
		printf("Failed to advertise name '%s' (%s).\n", mAdvertisedName.c_str(), QCC_StatusText(status));
	}
	return status;
}


int Server::SendSignal(std::string intfName, MsgArg ** msgs, int numArgs)
{
	QStatus status = (mBusObject->SendSignal(intfName, msgs, numArgs));
	return(int)status;
}

void Server::SetMethodCall(MethodCall func)
{
	mBusObject->SetMethodCall(func);
}

void Server::SetSignalCall(SignalEvent func)
{
	mBusObject->SetEventCall(func);
}


QStatus Server::RegisterMethodHandler(const char * intfName)
{
	if (mBusObject)
		return mBusObject->RegisterMethodHandler(intfName);
	else
		return QStatus::ER_FAIL;

}

QStatus Server::RegisterSignalHandler(const char * intfName)
{
	if (mBusObject)
		return mBusObject->RegisterEventHandler(intfName);
	else
		return QStatus::ER_FAIL;
}

void Server::setSessionId(SessionId id)
{
	mSessionId = id;
}

void Server::setJoined(bool joined)
{
	mJoined = joined;
}

SessionId* Server::getSessionId()
{
	return &mSessionId;
}

bool Server::isJoined()
{
	return mJoined;
}


void Server::RegisterBusListener() {
	//if (mConnectionListener)
	//	delete mConnectionListener;
	mConnectionListener = new ConnectionListener(mPrefix, mBus, mPort, this);
	mBus->RegisterBusListener(*mConnectionListener);
}



int Server::Initialize()
{
	return AllJoynBase::Initialize();
}

int Server::Start()
{
	int ret = AllJoynBase::Start();
	if (ret != 0) {
		return ret;
	}

	QStatus status = ER_OK;

	if (mBus) {
		if (ER_OK == status) {
			status = RequestName();
		}

		const TransportMask SERVICE_TRANSPORT_TYPE = TRANSPORT_ANY;

		if (ER_OK == status) {
			status = CreateSession(SERVICE_TRANSPORT_TYPE);
		}

		if (ER_OK == status) {
			status = AdvertiseName(SERVICE_TRANSPORT_TYPE);
		}
	}
	else {
		status = ER_OUT_OF_MEMORY;
	}
	return (int)status;
}


int Server::Stop() {
	AllJoynBase::Stop();
	delete mBusObject;
	//	delete mConnectionListener;
	delete mBusObject;
	return 0;//(int)status;
}
