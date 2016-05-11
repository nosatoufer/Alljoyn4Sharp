#include "Client.h"
#include "Server.h"

extern "C"
{
	struct  NativeServer
	{
		Server * engine;
	};

	struct  NativeClient
	{
		Client * engine;
	};

	struct NativeMsgArg
	{
		MsgArg * msg;
	};

	void NativeSignal_Destroy(NativeSignal & signal);

#pragma region Server

	__declspec(dllexport)
		int __stdcall Server_Initialize(NativeServer server)
	{
		return server.engine->Initialize();
	}

	__declspec(dllexport)
		int __stdcall Server_Start(NativeServer server)
	{
		return server.engine->Start();
	};

	__declspec(dllexport)
		int __stdcall Server_Stop(NativeServer server)
	{
		return server.engine->Stop();
	};

	__declspec(dllexport)
		NativeServer __stdcall Server_Create(const char* prefix, const char* name)
	{
		NativeServer server;
		server.engine = new Server(prefix, name);
		return server;
	}

	__declspec(dllexport)
		void __stdcall Server_Destroy(NativeServer server)
	{
		delete server.engine;
	};

	__declspec(dllexport)
		bool __stdcall Server_RegisterCallbacks(NativeServer server,
			CreateInterfaceCallback handle) {
		server.engine->RegisterCreateInterface(handle);
		return true;
	}

	__declspec(dllexport)
		bool __stdcall Server_AddInterfaceMember(
			NativeServer server,
			const char* name,
			const char* inputSig,
			const char* outSig,
			const char* argNames,
			int memberType) // 0 signal, 1 method
	{
		return server.engine->AddInterfaceMember(name, inputSig, outSig, argNames, memberType);
	}
	/* OLD
	__declspec(dllexport)
		int __stdcall Server_SendSignal(
			NativeServer server, NativeSignal signal) {
		int ret = server.engine->Send(signal);
		NativeSignal_Destroy(signal);
		return ret;
	}
	*/

	//
	// Type dependant register functions for registering callbacks from managed code
	//

	/**
	 * DEPRECATED ?
	 */
	/*
	__declspec(dllexport)
		bool __stdcall Server_RegisterTypeCallbacks(NativeServer server, SendStringArg arg) {
		return server.engine->RegisterCallback<SendStringArg>(arg, AllJoynTypeId::ALLJOYN_STRING);
	};
	*/

	__declspec(dllexport)
		void __stdcall Server_SetMethodHandler(NativeServer server, MethodCall func)
	{
		server.engine->SetMethodCall(func);
	}

	__declspec(dllexport)
		void __stdcall Server_SetSignalHandler(NativeServer server, SignalEvent func)
	{
		server.engine->SetSignalCall(func);
	}

	__declspec(dllexport)
		void __stdcall Server_RegisterMethodHandler(NativeServer server, const char * intfName)
	{
		QStatus st = server.engine->RegisterMethodHandler(intfName);
	}

	__declspec(dllexport)
		void __stdcall Server_RegisterSignalHandler(NativeServer server, const char * intfName)
	{
		QStatus st = server.engine->RegisterSignalHandler(intfName);
	}

	__declspec(dllexport)
		void __stdcall Server_SendSignal(NativeServer server, const char * intfName, MsgArg** msg, int numArgs)
	{
		server.engine->SendSignal(intfName, msg, numArgs);
	}
#pragma endregion Server

#pragma region Client

	__declspec(dllexport)
		int __stdcall Client_Start(NativeClient client)
	{
		return client.engine->Start();

	};

	__declspec(dllexport)
		int __stdcall Client_Initialize(NativeClient client)
	{
		return client.engine->Initialize();

	};

	__declspec(dllexport)
		int __stdcall Client_Stop(NativeClient client)
	{
		return client.engine->Stop();
	};

	__declspec(dllexport)
		NativeClient __stdcall Client_Create(const char * prefix, const char * name)
	{
		NativeClient client;
		client.engine = new Client(prefix, name);
		return client;
	}

	__declspec(dllexport)
		void __stdcall Client_Destroy(NativeClient client)
	{
		delete client.engine;

	};

	__declspec(dllexport)
		bool __stdcall Client_AddInterfaceMember(
			NativeClient client,
			const char* name,
			const char* inputSig,
			const char* outSig,
			const char* argNames,
			int memberType) // 0 signal, 1 method
	{
		return client.engine->AddInterfaceMember(name, inputSig, outSig, argNames, memberType);
	}

	
	__declspec(dllexport)
		bool __stdcall Client_RegisterCreateInterface(NativeClient client,
			CreateInterfaceCallback handle) {
		client.engine->RegisterCreateInterface(handle);
		return true;
	}
	

	__declspec(dllexport)
		const Message* __stdcall Client_CallMethod(NativeClient client, const char * member, MsgArg** msgs, int size)
	{
		return client.engine->CallMethod(member, msgs, size);

	}

	__declspec(dllexport)
		void __stdcall Client_SetSignalHandler(NativeClient client, SignalEvent func)
	{
		client.engine->SetSignalCall(func);
	}

	__declspec(dllexport)
		void __stdcall Client_RegisterSignalHandler(NativeClient client, const char * intfName)
	{
		QStatus st = client.engine->RegisterSignalHandler(intfName);
	}



#pragma endregion Client


#pragma region Alljoyn Wrapper
	/// AllJoyn wrapper for MsgArg and Message
	__declspec(dllexport)
		const MsgArg* __stdcall MessageGetArg(Message * message, int index)
	{
		return (*message)->GetArg(index);
	}

	__declspec(dllexport)
		const NativeMsgArg __stdcall CreateMsgArgInt(int val)
	{
		NativeMsgArg nMsgArg;
		nMsgArg.msg = new MsgArg("i", val);
		return nMsgArg;
	}
	__declspec(dllexport)
		const NativeMsgArg __stdcall CreateMsgArgLong(long val)
	{
		NativeMsgArg nMsgArg;
		nMsgArg.msg = new MsgArg("x", val);
		return nMsgArg;
	}
	__declspec(dllexport)
		NativeMsgArg __stdcall CreateMsgArgDouble(double val)
	{
		NativeMsgArg nMsgArg;
		nMsgArg.msg = new MsgArg("d", val);
		return nMsgArg;
	}
	__declspec(dllexport)
		NativeMsgArg __stdcall CreateMsgArgBool(bool val)
	{
		NativeMsgArg nMsgArg;
		nMsgArg.msg = new MsgArg("b", val);
		return nMsgArg;
	}
	__declspec(dllexport)
		NativeMsgArg __stdcall CreateMsgArgString(const char * val)
	{
		NativeMsgArg nMsgArg;
		nMsgArg.msg = new MsgArg("s", val);
		nMsgArg.msg->Stabilize();
		return nMsgArg;
	}

	__declspec(dllexport)
		const NativeMsgArg __stdcall CreateMsgArgIntArray(int* val, int numArgs)
	{
		NativeMsgArg nMsgArg;
		nMsgArg.msg = new MsgArg("ai", numArgs, val);
		return nMsgArg;
	}	__declspec(dllexport)
		const NativeMsgArg __stdcall CreateMsgArgLongArray(long long* val, int numArgs)
	{
		NativeMsgArg nMsgArg;
		nMsgArg.msg = new MsgArg("ax", numArgs, val);
		return nMsgArg;
	}	__declspec(dllexport)
		const NativeMsgArg __stdcall CreateMsgArgDoubleArray(double* val, int numArgs)
	{
		NativeMsgArg nMsgArg;
		nMsgArg.msg = new MsgArg("ad", numArgs, val);
		return nMsgArg;
	}	__declspec(dllexport)
		const NativeMsgArg __stdcall CreateMsgArgBoolArray(bool* val, int numArgs)
	{
		NativeMsgArg nMsgArg;
		nMsgArg.msg = new MsgArg("ab", numArgs, val);
		return nMsgArg;
	}


	__declspec(dllexport)
		int __stdcall MsgArgGetStringPtr(const MsgArg* msg, char *buff)
	{
		const char * c = msg->v_string.str;
		strncpy_s(buff, 1024, c, strlen(c) + 1);
		return strlen(buff) + 1;

	}

	__declspec(dllexport)
		int __stdcall MsgArgGetInt(MsgArg* mess)
	{
		return mess->v_int32;
	}	
	__declspec(dllexport)
		long long __stdcall MsgArgGetLong(MsgArg* mess)
	{
		return mess->v_int64;
	}
	__declspec(dllexport)
		double __stdcall MsgArgGetDouble(MsgArg* msg)
	{
		double val;
		msg->Get("d", &val);
		return val;
	}
	__declspec(dllexport)
		int __stdcall MsgArgGetBool(MsgArg* msg)
	{
		bool val;
		msg->Get("b", &val);
		return val;
	}

	__declspec(dllexport)
		int __stdcall MsgArgGetNumArguments(MsgArg* msg, const char * type)
	{
		size_t numArg = 0;
		void * val;
		msg->Get(type, &numArg, &val);
		return numArg;
	}

	__declspec(dllexport)
		int __stdcall MsgArgGetDoubleArray(MsgArg* msg, double * values)
	{
		double* msgValues;
		size_t numArg;
		QStatus s = msg->Get("ad", &numArg, &msgValues);
		for (size_t i = 0; i < numArg; i++)
		{
			values[i] = msgValues[i];
		}
		return (int)s;
	}

	__declspec(dllexport)
		int __stdcall MsgArgGetIntArray(MsgArg* msg, int * values)
	{
		int* msgValues;
		size_t numArg;
		QStatus s = msg->Get("ai", &numArg, &msgValues);
		for (size_t i = 0; i < numArg; i++)
			values[i] = msgValues[i];
		return (int)s;
	}

	__declspec(dllexport)
		int __stdcall MsgArgGetLongArray(MsgArg* msg, long long* values)
	{
		long long* msgValues;
		size_t numArg;
		QStatus s = msg->Get("ax", &numArg, &msgValues);
		for (size_t i = 0; i < numArg; i++)
			values[i] = msgValues[i];
		return (int)s;
	}

	__declspec(dllexport)
		int __stdcall MsgArgGetBoolArray(MsgArg* msg, bool * values)
	{
		bool* msgValues;
		size_t numArg;
		QStatus s = msg->Get("ab", &numArg, &msgValues);
		for (size_t i = 0; i < numArg; i++)
			values[i] = msgValues[i];
		return (int)s;
	}

	__declspec(dllexport)
		int __stdcall MessageGetSignature(Message * msg, char * buff)
	{
		const char * ret = (*msg)->GetSignature();
		if (strlen(ret) != 0)
		{
			strcpy_s(buff, 1024, ret);
			return strlen(buff)+1;
		}
		return 0;
	}

#pragma endregion AllJoyn Wrapper

	//
	// List of types that ARE SUPPORTED so far.
	//
	//ALLJOYN_STRING = 's',    ///< AllJoyn UTF-8 NULL terminated string basic type
	//
	// List of types that ARE NOT SUPPORTED so far.
	//
	//ALLJOYN_ARRAY = 'a',    ///< AllJoyn array container type
	//ALLJOYN_BOOLEAN = 'b',    ///< AllJoyn boolean basic type, @c 0 is @c FALSE and @c 1 is @c TRUE - Everything else is invalid
	//ALLJOYN_DOUBLE = 'd',    ///< AllJoyn IEEE 754 double basic type
	//ALLJOYN_DICT_ENTRY = 'e',    ///< AllJoyn dictionary or map container type - an array of key-value pairs
	//ALLJOYN_SIGNATURE = 'g',    ///< AllJoyn signature basic type
	//ALLJOYN_HANDLE = 'h',    ///< AllJoyn socket handle basic type
	//ALLJOYN_INT32 = 'i',    ///< AllJoyn 32-bit signed integer basic type
	//ALLJOYN_INT16 = 'n',    ///< AllJoyn 16-bit signed integer basic type
	//ALLJOYN_OBJECT_PATH = 'o',    ///< AllJoyn Name of an AllJoyn object instance basic type
	//ALLJOYN_UINT16 = 'q',    ///< AllJoyn 16-bit unsigned integer basic type
	//ALLJOYN_STRUCT = 'r',    ///< AllJoyn struct container type
	//ALLJOYN_UINT64 = 't',    ///< AllJoyn 64-bit unsigned integer basic type
	//ALLJOYN_UINT32 = 'u',    ///< AllJoyn 32-bit unsigned integer basic type
	//ALLJOYN_VARIANT = 'v',    ///< AllJoyn variant container type
	//ALLJOYN_INT64 = 'x',    ///< AllJoyn 64-bit signed integer basic type
	//ALLJOYN_BYTE = 'y',    ///< AllJoyn 8-bit unsigned integer basic type
	//ALLJOYN_STRUCT_OPEN = '(', /**< Never actually used as a typeId: specified as ALLJOYN_STRUCT */
	//ALLJOYN_STRUCT_CLOSE = ')', /**< Never actually used as a typeId: specified as ALLJOYN_STRUCT */
	//ALLJOYN_DICT_ENTRY_OPEN = '{', /**< Never actually used as a typeId: specified as ALLJOYN_DICT_ENTRY */
	//ALLJOYN_DICT_ENTRY_CLOSE = '}', /**< Never actually used as a typeId: specified as ALLJOYN_DICT_ENTRY */
	//ALLJOYN_BOOLEAN_ARRAY = ('b' << 8) | 'a',   ///< AllJoyn array of booleans
	//ALLJOYN_DOUBLE_ARRAY = ('d' << 8) | 'a',   ///< AllJoyn array of IEEE 754 doubles
	//ALLJOYN_INT32_ARRAY = ('i' << 8) | 'a',   ///< AllJoyn array of 32-bit signed integers
	//ALLJOYN_INT16_ARRAY = ('n' << 8) | 'a',   ///< AllJoyn array of 16-bit signed integers
	//ALLJOYN_UINT16_ARRAY = ('q' << 8) | 'a',   ///< AllJoyn array of 16-bit unsigned integers
	//ALLJOYN_UINT64_ARRAY = ('t' << 8) | 'a',   ///< AllJoyn array of 64-bit unsigned integers
	//ALLJOYN_UINT32_ARRAY = ('u' << 8) | 'a',   ///< AllJoyn array of 32-bit unsigned integers
	//ALLJOYN_INT64_ARRAY = ('x' << 8) | 'a',   ///< AllJoyn array of 64-bit signed integers
	//ALLJOYN_BYTE_ARRAY = ('y' << 8) | 'a',   ///< AllJoyn array of 8-bit unsigned integers
}
