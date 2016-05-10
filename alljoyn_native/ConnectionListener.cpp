#include "ConnectionListener.h"



ConnectionListener::ConnectionListener(std::string prefix, ajn::BusAttachment * bus, SessionPort port, ISessionUser * sessionUser) :
	mPrefix(prefix),
	mBus(bus),
	mPort(port),
	mSessionUser(sessionUser)
{
}


ConnectionListener::~ConnectionListener()
{
}

void ConnectionListener::FoundAdvertisedName(const char* name, TransportMask transport, const char* namePrefix)
{

		printf("FoundAdvertisedName(name='%s', transport = 0x%x, prefix='%s')\n", name, transport, namePrefix);

		const char* convName = name + strlen(mPrefix.c_str());
		printf("Discovered chat conversation: \"%s\"\n", convName);

		/* Join the conversation */
		/* Since we are in a callback we must enable concurrent callbacks before calling a synchronous method. */
		mBus->EnableConcurrentCallbacks();
		SessionOpts opts(SessionOpts::TRAFFIC_MESSAGES, true, SessionOpts::PROXIMITY_ANY, TRANSPORT_ANY);
		QStatus status = mBus->JoinSession(name, mPort, this, *(mSessionUser->getSessionId()), opts);
		if (ER_OK == status) {
			printf("Joined conversation \"%s\"\n", convName);
			mSessionUser->setJoined(true);

		}
		else {
			printf("JoinSession failed (status=%s)\n", QCC_StatusText(status));
		}
		if (!mSessionUser->isJoined())
		{
		uint32_t timeout = 20;
		status = mBus->SetLinkTimeout(*(mSessionUser->getSessionId()), timeout);
		if (ER_OK == status) {
			printf("Set link timeout to %d\n", timeout);
		}
		else {
			printf("Set link timeout failed\n");
		}
	}
}
void ConnectionListener::LostAdvertisedName(const char* name, TransportMask transport, const char* namePrefix)
{
	QCC_UNUSED(namePrefix);
	printf("Got LostAdvertisedName for %s from transport 0x%x\n", name, transport);
}
void ConnectionListener::NameOwnerChanged(const char* busName, const char* previousOwner, const char* newOwner)
{
	printf("NameOwnerChanged: name=%s, oldOwner=%s, newOwner=%s\n", busName, previousOwner ? previousOwner : "<none>",
		newOwner ? newOwner : "<none>");
}
bool ConnectionListener::AcceptSessionJoiner(SessionPort sessionPort, const char* joiner, const SessionOpts& opts)
{
	if (sessionPort != mPort) {
		printf("Rejecting join attempt on non-chat session port %d\n", sessionPort);
		return false;
	}

	printf("Accepting join session request from %s (opts.proximity=%x, opts.traffic=%x, opts.transports=%x)\n",
		joiner, opts.proximity, opts.traffic, opts.transports);
	return true;
}

void ConnectionListener::SessionJoined(SessionPort sessionPort, SessionId id, const char* joiner)
{
	QCC_UNUSED(sessionPort);
	mSessionUser->setSessionId(id);
	printf("SessionJoined with %s (id=%d)\n", joiner, id);
	mBus->EnableConcurrentCallbacks();
	uint32_t timeout = 20;
	QStatus status = mBus->SetLinkTimeout(*(mSessionUser->getSessionId()), timeout);
	if (ER_OK == status) {
		printf("Set link timeout to %d\n", timeout);
	}
	else {
		printf("Set link timeout failed\n");
	}
}