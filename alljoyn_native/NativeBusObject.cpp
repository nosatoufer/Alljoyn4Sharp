#include "NativeBusObject.h"


NativeBusObject::NativeBusObject(BusAttachment & bus, std::string advName, std::string path, SessionId * sessionId, std::string intfName)
	: BusObject(path.c_str()), mSessionId(sessionId), mBus(bus), mProxy(NULL)
{
	mIntf = bus.GetInterface(intfName.c_str());
	AddInterface(*mIntf);

	mProxy = new ProxyBusObject(bus, advName.c_str(), path.c_str(), *mSessionId);
	mProxy->AddInterface(*mIntf);
}

NativeBusObject::~NativeBusObject()
{
	delete mProxy;
}

QStatus NativeBusObject::SendSignal(std::string intfName, MsgArg * msgs[], int size)
{
	uint8_t flags = 0;
	MsgArg * msg = new MsgArg[size];
	for (int i = 0; i < size; i++)
	{
		msg[i] = *msgs[i];
		msg[i].Stabilize();
	}
	QStatus status = Signal(NULL, *mSessionId, *(mIntf->GetMember(intfName.c_str())), msg, size, 0, flags);
	for (int i = 0; i < size; i++)
	{
		delete msgs[i];
	}
	delete[] msg;
	return status;
}

const Message * NativeBusObject::CallMethod(std::string member, MsgArg * msgs[], int size)
{
	QStatus status = ER_OK;
	Message * copiedReply = NULL;
	if (mProxy)
	{
		MsgArg * msg = new MsgArg[size];
		for (int i = 0; i < size; i++)
		{
			msg[i] = *msgs[i];
		}
		ajn::Message reply(mBus);
		if ((status = mProxy->MethodCall(*(mIntf->GetMember(member.c_str())), msg, size, reply, 400)) == ER_OK)
		{		
			copiedReply = new Message(reply);
		}
		delete[] msg;

	}
	printf("else : Status of methodcall = %s\n", QCC_StatusText(status));
	for (int i = 0; i < size; i++)
	{
		delete msgs[i];
	}
	return copiedReply;
}

void NativeBusObject::SetMethodCall(MethodCall func)
{
	mMethodCall = func;
}

void NativeBusObject::SetEventCall(SignalEvent func)
{
	mSignalCall = func;
}

QStatus NativeBusObject::RegisterMethodHandler(std::string intfName)
{
	if (mIntf->GetMember(intfName.c_str()) == NULL)
		return ER_FAIL;
	return AddMethodHandler(mIntf->GetMember(intfName.c_str()),
		static_cast<MessageReceiver::MethodHandler>(&NativeBusObject::MethodHandler));
}

QStatus NativeBusObject::RegisterEventHandler(std::string intfName)
{
	if (mIntf->GetMember(intfName.c_str()) == NULL)
		return ER_FAIL;

	return mBus.RegisterSignalHandler(this,
		static_cast<MessageReceiver::SignalHandler>(&NativeBusObject::SignalHandler),
		mIntf->GetMember(intfName.c_str()),
		NULL);
}

void NativeBusObject::SignalHandler(const InterfaceDescription::Member * member, const char * srcPath, Message & msg)
{
	if (mSignalCall)
		mSignalCall(member->name.c_str(), &msg);
}

void NativeBusObject::MethodHandler(const InterfaceDescription::Member * member, Message & msg)
{
	if (mMethodCall)
	{
		MsgArg * reply = mMethodCall(member->name.c_str(), &msg);
		MethodReply(msg, reply, 1);
		delete reply;
	}
	else
		MethodReply(msg, NULL, 1);
}
