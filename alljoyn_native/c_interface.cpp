#include "Client.h"
#include "Server.h"

#ifdef WIN32
#define EXPORT __declspec(dllexport) 
#endif

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


#pragma region Server

	EXPORT 
		int __stdcall Server_Initialize(NativeServer server)
	{
		return server.engine->Initialize();
	}

	EXPORT 
		int __stdcall Server_Start(NativeServer server)
	{
		return server.engine->Start();
	};

	EXPORT 
		int __stdcall Server_Stop(NativeServer server)
	{
		return server.engine->Stop();
	};

	EXPORT 
		NativeServer __stdcall Server_Create(const char* prefix, const char* name)
	{
		NativeServer server;
		server.engine = new Server(prefix, name);
		return server;
	}

	EXPORT 
		void __stdcall Server_Destroy(NativeServer server)
	{
		delete server.engine;
	};

	EXPORT 
		bool __stdcall Server_RegisterCallbacks(NativeServer server,
			CreateInterfaceCallback handle) {
		server.engine->RegisterCreateInterface(handle);
		return true;
	}

	EXPORT 
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
	EXPORT 
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
	EXPORT 
		bool __stdcall Server_RegisterTypeCallbacks(NativeServer server, SendStringArg arg) {
		return server.engine->RegisterCallback<SendStringArg>(arg, AllJoynTypeId::ALLJOYN_STRING);
	};
	*/

	EXPORT 
		void __stdcall Server_SetMethodHandler(NativeServer server, MethodCall func)
	{
		server.engine->SetMethodCall(func);
	}

	EXPORT 
		void __stdcall Server_SetSignalHandler(NativeServer server, SignalEvent func)
	{
		server.engine->SetSignalCall(func);
	}

	EXPORT 
		void __stdcall Server_RegisterMethodHandler(NativeServer server, const char * intfName)
	{
		QStatus st = server.engine->RegisterMethodHandler(intfName);
	}

	EXPORT 
		void __stdcall Server_RegisterSignalHandler(NativeServer server, const char * intfName)
	{
		QStatus st = server.engine->RegisterSignalHandler(intfName);
	}

	EXPORT 
		void __stdcall Server_SendSignal(NativeServer server, const char * intfName, MsgArg** msg, int numArgs)
	{
		server.engine->SendSignal(intfName, msg, numArgs);
	}
#pragma endregion Server

#pragma region Client

	EXPORT 
		int __stdcall Client_Start(NativeClient client)
	{
		return client.engine->Start();

	};

	EXPORT 
		int __stdcall Client_Initialize(NativeClient client)
	{
		return client.engine->Initialize();

	};

	EXPORT 
		int __stdcall Client_Stop(NativeClient client)
	{
		return client.engine->Stop();
	};

	EXPORT 
		NativeClient __stdcall Client_Create(const char * prefix, const char * name)
	{
		NativeClient client;
		client.engine = new Client(prefix, name);
		return client;
	}

	EXPORT 
		void __stdcall Client_Destroy(NativeClient client)
	{
		delete client.engine;

	};

	EXPORT 
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

	
	EXPORT 
		bool __stdcall Client_RegisterCreateInterface(NativeClient client,
			CreateInterfaceCallback handle) {
		client.engine->RegisterCreateInterface(handle);
		return true;
	}
	

	EXPORT 
		const Message* __stdcall Client_CallMethod(NativeClient client, const char * member, MsgArg** msgs, int size)
	{
		return client.engine->CallMethod(member, msgs, size);

	}

	EXPORT 
		void __stdcall Client_SetSignalHandler(NativeClient client, SignalEvent func)
	{
		client.engine->SetSignalCall(func);
	}

	EXPORT 
		void __stdcall Client_RegisterSignalHandler(NativeClient client, const char * intfName)
	{
		QStatus st = client.engine->RegisterSignalHandler(intfName);
	}



#pragma endregion Client


#pragma region Alljoyn Wrapper

	/// AllJoyn wrapper for MsgArg and Message
	EXPORT 
		const MsgArg* __stdcall MessageGetArg(Message * message, int index)
	{
		return (*message)->GetArg(index);
	}
	EXPORT 
		int __stdcall MessageGetSignature(Message * msg, char * buff)
	{
		const char * ret = (*msg)->GetSignature();
		if (strlen(ret) != 0)
		{
			strncpy(buff, ret, 1024);
			return strlen(buff) + 1;
		}
		return 0;
	}
#pragma region MsgArgConstructors
	EXPORT 
		const NativeMsgArg __stdcall CreateMsgArgShort(short val)
	{
		NativeMsgArg nMsgArg;
		nMsgArg.msg = new MsgArg("n", val);
		return nMsgArg;
	}
	EXPORT 
		const NativeMsgArg __stdcall CreateMsgArgInt(int val)
	{
		NativeMsgArg nMsgArg;
		nMsgArg.msg = new MsgArg("i", val);
		return nMsgArg;
	}
	EXPORT 
		const NativeMsgArg __stdcall CreateMsgArgLong(long val)
	{
		NativeMsgArg nMsgArg;
		nMsgArg.msg = new MsgArg("x", val);
		return nMsgArg;
	}
	EXPORT 
		const NativeMsgArg __stdcall CreateMsgArgShortUInt(uint16_t val)
	{
		NativeMsgArg nMsgArg;
		nMsgArg.msg = new MsgArg("q", val);
		return nMsgArg;
	}
	EXPORT 
		const NativeMsgArg __stdcall CreateMsgArgUInt(uint32_t val)
	{
		NativeMsgArg nMsgArg;
		nMsgArg.msg = new MsgArg("u", val);
		return nMsgArg;
	}
	EXPORT 
		const NativeMsgArg __stdcall CreateMsgArgLongUInt(uint64_t val)
	{
		NativeMsgArg nMsgArg;
		nMsgArg.msg = new MsgArg("t", val);
		return nMsgArg;
	}

	EXPORT 
		NativeMsgArg __stdcall CreateMsgArgDouble(double val)
	{
		NativeMsgArg nMsgArg;
		nMsgArg.msg = new MsgArg("d", val);
		return nMsgArg;
	}
	EXPORT 
		NativeMsgArg __stdcall CreateMsgArgBool(bool val)
	{
		NativeMsgArg nMsgArg;
		nMsgArg.msg = new MsgArg("b", val);
		return nMsgArg;
	}
	EXPORT 
		NativeMsgArg __stdcall CreateMsgArgString(const char * val)
	{
		NativeMsgArg nMsgArg;
		nMsgArg.msg = new MsgArg("s", val);
		nMsgArg.msg->Stabilize();
		return nMsgArg;
	}

	EXPORT 
		const NativeMsgArg __stdcall CreateMsgArgShortArray(short* val, int numArgs)
	{
		NativeMsgArg nMsgArg;
		nMsgArg.msg = new MsgArg("an", numArgs, val);
		return nMsgArg;
	}
	EXPORT 
		const NativeMsgArg __stdcall CreateMsgArgIntArray(int* val, int numArgs)
	{
		NativeMsgArg nMsgArg;
		nMsgArg.msg = new MsgArg("ai", numArgs, val);
		return nMsgArg;
	}
	EXPORT 
		const NativeMsgArg __stdcall CreateMsgArgLongArray(long long* val, int numArgs)
	{
		NativeMsgArg nMsgArg;
		nMsgArg.msg = new MsgArg("ax", numArgs, val);
		return nMsgArg;
	}
	EXPORT 
		const NativeMsgArg __stdcall CreateMsgArgShortUIntArray(uint16_t* val, int numArgs)
	{
		NativeMsgArg nMsgArg;
		nMsgArg.msg = new MsgArg("aq", numArgs, val);
		return nMsgArg;
	}
	EXPORT 
		const NativeMsgArg __stdcall CreateMsgArgUIntArray(uint32_t* val, int numArgs)
	{
		NativeMsgArg nMsgArg;
		nMsgArg.msg = new MsgArg("au", numArgs, val);
		return nMsgArg;
	}
	EXPORT 
		const NativeMsgArg __stdcall CreateMsgArgLongUIntArray(uint64_t* val, int numArgs)
	{
		NativeMsgArg nMsgArg;
		nMsgArg.msg = new MsgArg("at", numArgs, val);
		return nMsgArg;
	}
	EXPORT 
		const NativeMsgArg __stdcall CreateMsgArgDoubleArray(double* val, int numArgs)
	{
		NativeMsgArg nMsgArg;
		nMsgArg.msg = new MsgArg("ad", numArgs, val);
		return nMsgArg;
	}	
	EXPORT 
		const NativeMsgArg __stdcall CreateMsgArgBoolArray(bool* val, int numArgs)
	{
		NativeMsgArg nMsgArg;
		nMsgArg.msg = new MsgArg("ab", numArgs, val);
		return nMsgArg;
	}
#pragma endregion

#pragma region MsgArgGetters

	EXPORT 
		int __stdcall MsgArgGetStringPtr(const MsgArg* msg, char *buff)
	{
		const char * c = msg->v_string.str;
		strncpy(buff, c, 1024);
		return strlen(buff) + 1;

	}
	EXPORT 
		int __stdcall MsgArgGetShort(MsgArg* mess)
	{
		return mess->v_int16;
	}
	EXPORT 
		int __stdcall MsgArgGetInt(MsgArg* mess)
	{
		return mess->v_int32;
	}	
	EXPORT 
		long long __stdcall MsgArgGetLong(MsgArg* mess)
	{
		return mess->v_int64;
	}
	EXPORT 
		int __stdcall MsgArgGetShortUInt(MsgArg* mess)
	{
		return mess->v_uint16;
	}
	EXPORT 
		int __stdcall MsgArgGetUInt(MsgArg* mess)
	{
		return mess->v_uint32;
	}
	EXPORT 
		long long __stdcall MsgArgGetLongUInt(MsgArg* mess)
	{
		return mess->v_uint64;
	}
	EXPORT 
		double __stdcall MsgArgGetDouble(MsgArg* msg)
	{
		double val;
		msg->Get("d", &val);
		return val;
	}
	EXPORT 
		int __stdcall MsgArgGetBool(MsgArg* msg)
	{
		bool val;
		msg->Get("b", &val);
		return val;
	}

	EXPORT 
		int __stdcall MsgArgGetNumArguments(MsgArg* msg, const char * type)
	{
		size_t numArg = 0;
		void * val;
		msg->Get(type, &numArg, &val);
		return numArg;
	}

	EXPORT 
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
	EXPORT 
		int __stdcall MsgArgGetShortArray(MsgArg* msg, short * values)
	{
		short* msgValues;
		size_t numArg;
		QStatus s = msg->Get("an", &numArg, &msgValues);
		for (size_t i = 0; i < numArg; i++)
			values[i] = msgValues[i];
		return (int)s;
	}
	EXPORT 
		int __stdcall MsgArgGetIntArray(MsgArg* msg, int * values)
	{
		int* msgValues;
		size_t numArg;
		QStatus s = msg->Get("ai", &numArg, &msgValues);
		for (size_t i = 0; i < numArg; i++)
			values[i] = msgValues[i];
		return (int)s;
	}

	EXPORT 
		int __stdcall MsgArgGetLongArray(MsgArg* msg, long long* values)
	{
		long long* msgValues;
		size_t numArg;
		QStatus s = msg->Get("ax", &numArg, &msgValues);
		for (size_t i = 0; i < numArg; i++)
			values[i] = msgValues[i];
		return (int)s;
	}
	EXPORT 
		int __stdcall MsgArgGetShortUIntArray(MsgArg* msg, uint16_t * values)
	{
		uint16_t* msgValues;
		size_t numArg;
		QStatus s = msg->Get("aq", &numArg, &msgValues);
		for (size_t i = 0; i < numArg; i++)
			values[i] = msgValues[i];
		return (int)s;
	}
	EXPORT 
		int __stdcall MsgArgGetUIntArray(MsgArg* msg, uint32_t * values)
	{
		uint32_t* msgValues;
		size_t numArg;
		QStatus s = msg->Get("au", &numArg, &msgValues);
		for (size_t i = 0; i < numArg; i++)
			values[i] = msgValues[i];
		return (int)s;
	}
	EXPORT 
		int __stdcall MsgArgGetLongUIntArray(MsgArg* msg, uint64_t * values)
	{
		uint64_t* msgValues;
		size_t numArg;
		QStatus s = msg->Get("at", &numArg, &msgValues);
		for (size_t i = 0; i < numArg; i++)
			values[i] = msgValues[i];
		return (int)s;
	}
	EXPORT 
		int __stdcall MsgArgGetBoolArray(MsgArg* msg, bool * values)
	{
		bool* msgValues;
		size_t numArg;
		QStatus s = msg->Get("ab", &numArg, &msgValues);
		for (size_t i = 0; i < numArg; i++)
			values[i] = msgValues[i];
		return (int)s;
	}
#pragma endregion



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
