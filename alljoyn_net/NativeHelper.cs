using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Reflection.Emit;
using System.Runtime.InteropServices;
using System.Text;

namespace alljoyn_net
{
    /// <summary>
    /// Classe used to access the C++ function and data
    /// </summary>
    internal class NativeHelper
    {

        public static void BindEvents<T>(Type generic, T obj)
        {
            Type type = typeof(T);
            EventInfo[] eventInfos = type.GetEvents();
            foreach (EventInfo eventInfo in eventInfos)
            {
                object[] attributes = eventInfo.GetCustomAttributes(false);
                foreach (object attribute in attributes)
                {
                    if (attribute is RemoteEventAttribute)
                    {
                        Delegate dEmitted = GenerateEventHandler<T>(eventInfo, generic, obj.GetType());
                        MethodInfo addHandler = eventInfo.GetAddMethod();
                        Object[] addHandlerArgs = { dEmitted };
                        addHandler.Invoke(obj, addHandlerArgs);
                        break;
                    }
                }
            }
        }

        public static void BindMethods<T>(Type generic, T obj)
        {
            Type type = typeof(T);
            MethodInfo[] methodInfos = type.GetMethods();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="eventInfo"></param>
        /// <param name="generic"></param>
        /// <param name="classWithEvent"></param>
        /// <returns></returns>
        public static Delegate GenerateEventHandler<T>(EventInfo eventInfo, Type generic, Type classWithEvent)
        {
            Type tDelegate = eventInfo.EventHandlerType;
            MethodInfo invoke = tDelegate.GetMethod("Invoke");
            Type[] parameterTypes = GetDelegateParameterTypes(tDelegate);

            if (invoke == null)
                throw new ApplicationException("Not a delegate.");
            if (invoke.ReturnType != typeof(void))
                throw new ApplicationException("Delegate cannot have a return type other than void. Alljoyn doesn't support return type of signals.");
            //if (parameterTypes.Length > 2)
            //    throw new ApplicationException("Delegate with more than 1 parameter is not supported for now.");



            DynamicMethod handler =
                new DynamicMethod("",
                      typeof(void),
                      parameterTypes,
                      classWithEvent);


            ILGenerator ilGenerator = handler.GetILGenerator();

            MethodInfo sendMethod = ResolveGenericMethod(generic, typeof(T), "ForwardEvent");

            // Create array of objects
            LocalBuilder arr = ilGenerator.DeclareLocal(typeof(object[]));
            ilGenerator.Emit(OpCodes.Ldc_I4, parameterTypes.Length);
            ilGenerator.Emit(System.Reflection.Emit.OpCodes.Newarr, typeof(System.Object));
            ilGenerator.Emit(System.Reflection.Emit.OpCodes.Stloc, arr);

            for (int i = 0; i < parameterTypes.Length; i++)
            {
                // index the array
                ilGenerator.Emit(System.Reflection.Emit.OpCodes.Ldloc, arr);
                ilGenerator.Emit(OpCodes.Ldc_I4, i);

                // load argument
                ilGenerator.Emit(OpCodes.Ldarg, i);

                // Box it if necessary and assign the reference to the array
                if (parameterTypes[i].IsValueType || parameterTypes[i].IsGenericParameter)
                    ilGenerator.Emit(System.Reflection.Emit.OpCodes.Box, parameterTypes[i]);
                ilGenerator.Emit(System.Reflection.Emit.OpCodes.Stelem_Ref);
            }
            // Get the second argument
            ilGenerator.Emit(System.Reflection.Emit.OpCodes.Ldloc_0);
            ilGenerator.Emit(System.Reflection.Emit.OpCodes.Ldstr, eventInfo.Name);

            ilGenerator.Emit(System.Reflection.Emit.OpCodes.Call, sendMethod);
            ilGenerator.Emit(System.Reflection.Emit.OpCodes.Ret);
            Delegate dEmitted = handler.CreateDelegate(tDelegate);
            return dEmitted;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="d"></param>
        /// <returns></returns>
        public static Type[] GetDelegateParameterTypes(Type d)
        {
            if (d.BaseType != typeof(MulticastDelegate))
                throw new ApplicationException("Not a delegate.");

            MethodInfo invoke = d.GetMethod("Invoke");
            if (invoke == null)
                throw new ApplicationException("Not a delegate.");

            ParameterInfo[] parameters = invoke.GetParameters();
            Type[] typeParameters = new Type[parameters.Length];
            for (int i = 0; i < parameters.Length; i++)
            {
                typeParameters[i] = parameters[i].ParameterType;
            }
            return typeParameters;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="generic"></param>
        /// <param name="type"></param>
        /// <param name="name"></param>
        /// <returns></returns>
        private static MethodInfo ResolveGenericMethod(Type generic, Type type, string name)
        {
            var genericType = generic.MakeGenericType(new[] { type });
            List<Type> allparam = new List<Type>();//parameters.ToList<Type>();
            allparam.Add(typeof(object[]));
            allparam.Add(typeof(string));
            return genericType.GetMethod(name, allparam.ToArray());
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="type"></param>
        /// <param name="callback"></param>
        public static void RegisterMembers(Type type, AddInterfaceMemberCallback callback)
        {
            // Parse methods and generate iface descriptions
            //
            MethodInfo[] mInfos = type.GetMethods();
            foreach (MethodInfo method in mInfos)
            {
                object[] attributes = method.GetCustomAttributes(false);
                foreach (object attribute in attributes)
                {
                    if (attribute is RemoteMethodAttribute)
                    {
                        GenerateDecription(callback, method);
                        break;
                    }
                }
            }

            // Parse events and generate iface descriptions
            //
            EventInfo[] eInfos = type.GetEvents();
            foreach (EventInfo objectEvent in eInfos)
            {
                object[] attributes = objectEvent.GetCustomAttributes(false);
                foreach (object attribute in attributes)
                {
                    if (attribute is RemoteEventAttribute)
                    {
                        GenerateDecription(callback, objectEvent);
                        break;
                    }
                }
            }

        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="callback"></param>
        /// <param name="method"></param>
        private static void GenerateDecription(AddInterfaceMemberCallback callback, MethodInfo method)
        {
            string[] args = new string[4];
            args[0] = method.Name;
            args[1] = "";
            args[2] = "" + GetAllJoynCode(method.ReturnType);
            args[3] = "";
            ParameterInfo[] parameters = method.GetParameters();

            for (int i = 0; i < parameters.Length; i++)
            {
                args[1] += GetAllJoynCode(parameters[i].ParameterType);

                args[3] += (i == 0 ? "" : ",") + parameters[i].Name;

            }
            Console.WriteLine("ifaceDesc.AddMethod(\"" + args[0] + "\",\"" + args[1] + "\",\"" + args[2] + "\",\"" + args[3] + "\")");
            callback(args[0], args[1], args[2], args[3], 1 /* Method*/);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="callback"></param>
        /// <param name="eventInfo"></param>
        private static void GenerateDecription(AddInterfaceMemberCallback callback, EventInfo eventInfo)
        {
            Type tDelegate = eventInfo.EventHandlerType;
            MethodInfo invoke = tDelegate.GetMethod("Invoke");
            if (invoke == null)
                throw new ApplicationException("Not a delegate.");
            if (invoke.ReturnType != typeof(void))
                throw new ApplicationException("Object Event delegate " + eventInfo.Name + " cannot have a return type other than void. Alljoyn doesn't support return type of signals.");


            string[] args = new string[3];
            args[0] = eventInfo.Name;
            args[1] = "";
            args[2] = "";
            ParameterInfo[] parameters = invoke.GetParameters();
            for (int i = 0; i < parameters.Length; i++)
            {
                args[1] += GetAllJoynCode(parameters[i].ParameterType);
                args[2] += (i == 0 ? "" : ",") + parameters[i].Name;
            }
            Console.WriteLine("ifaceDesc.AddSignal(\"" + args[0] + "\",\"" + args[1] + "\",\"" + args[2] + "\")");
            callback(args[0], args[1], null, args[2], 0 /* Signal*/);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="t"></param>
        /// <returns></returns>
        public static string GetAllJoynCode(Type[] t)
        {
            string ret = "";
            for (int i = 0; i < t.Length; i++)
            {
                ret += GetAllJoynCode(t[i]);
            }
            return ret;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="t"></param>
        /// <returns></returns>
        public static string GetAllJoynCode(Type t)
        {

            // Simple data types
            //
            if (t == typeof(byte))
                return "y";
            else if (t == typeof(bool))
                return "b";
            else if (t == typeof(Int16))
                return "n";
            else if (t == typeof(UInt16))
                return "q";
            else if (t == typeof(Int32))
                return "i";
            else if (t == typeof(UInt32))
                return "u";
            else if (t == typeof(Int64))
                return "x";
            else if (t == typeof(UInt64))
                return "t";
            else if (t == typeof(double))
                return "d";
            else if (t == typeof(string))
                return "s";

            //  Arrays
            //
            else if (t == typeof(byte[]))
                return "ay";
            else if (t == typeof(Int16[]))
                return "an";
            else if (t == typeof(UInt16[]))
                return "aq";
            else if (t == typeof(Int32[]))
                return "ai";
            else if (t == typeof(UInt32[]))
                return "au";
            else if (t == typeof(Int64[]))
                return "ax";
            else if (t == typeof(UInt64[]))
                return "at";
            else if (t == typeof(double[]))
                return "ad";
            else if (t == typeof(string[]))
                return "as";

            else
                throw new NonSupportedAllJoynTypeException();
        }

        /// <summary>
        /// Returns the signature of the message
        /// </summary>
        /// <param name="message">The message</param>
        /// <returns></returns>
        public static string MessageSignature(IntPtr message)
        {
            IntPtr stringBuff = Marshal.AllocHGlobal(1024);
            int sSize = MessageGetSignature(message, stringBuff);
            string sign;
            if (sSize > 0)
            {
                stringBuff = Marshal.ReAllocHGlobal(stringBuff, (IntPtr)sSize);
                sign = Marshal.PtrToStringAnsi(stringBuff);
            }
            else
                sign = "";
            Marshal.FreeHGlobal(stringBuff);
            stringBuff = IntPtr.Zero;
            return sign;
        }

        /// <summary>
        /// Creates an array of msgArg 
        /// </summary>
        /// <param name="values">The values used to create the messages</param>
        /// <param name="sig">The Alljoyn type of the values</param>
        /// <returns></returns>
        public static IntPtr[] CreateMessages(object[] values)
        {
            IntPtr[] msgs = new IntPtr[values.Length];
            int i = 0;
            foreach (Object o in values)
            {
                switch (o.GetType().Name)
                {
                    case "Int32":
                        msgs[i] = CreateMsgArgInt((int)values[i]);
                        break;
                    case "Int64":
                        msgs[i] = CreateMsgArgLong((long)values[i]);
                        break;
                    case "Double":
                        msgs[i] = CreateMsgArgDouble((double)values[i]);
                        break;
                    case "String":
                        msgs[i] = CreateMsgArgString((string)values[i]);
                        break;
                    case "Boolean":
                        msgs[i] = CreateMsgArgBool((bool)values[i]);
                        break;
                    case "Int32[]":
                        msgs[i] = CreateMsgArgIntArray((int[])values[i], ((int[])values[i]).Length);
                        break;
                    case "Int64[]":
                        msgs[i] = CreateMsgArgLongArray((long[])values[i], ((long[])values[i]).Length);
                        break;
                    case "Double[]":
                        msgs[i] = CreateMsgArgDoubleArray((double[])values[i], ((double[])values[i]).Length);
                        break;
                    case "Boolean[]":
                        msgs[i] = CreateMsgArgBoolArray((bool[])values[i], ((bool[])values[i]).Length);
                        break;
                    case "Void":
                        break;
                    default:
                        throw new System.NotSupportedException("Type not supported");
                }
                i++;
            }
            return msgs;
        }

        /// <summary>
        /// Extracts the value of the MsgArg
        /// </summary>
        /// <param name="message">The message containing the value</param>
        /// <param name="t">The C# type of the value</param>
        /// <returns></returns>
        public static Object extractValue(IntPtr msg, string type, ref int pos)
        {
            Object val = 0;
            switch (type[pos])
            {
                case 'i':
                    val = MsgArgGetInt(msg);
                    break;
                case 'x':
                    val = MsgArgGetLong(msg);
                    break;
                case 's':
                    IntPtr stringPtr = Marshal.AllocHGlobal(1024);
                    int sSize = MsgArgGetStringPtr(msg, stringPtr);
                    stringPtr = Marshal.ReAllocHGlobal(stringPtr, (IntPtr)sSize);
                    val = Marshal.PtrToStringAnsi(stringPtr);
                    Marshal.FreeHGlobal(stringPtr);
                    break;
                case 'd':
                    val = MsgArgGetDouble(msg);
                    break;
                case 'b':
                    val = MsgArgGetBool(msg);
                    break;
                case 'a':
                    pos++;
                    val = ExtractArray(msg, type, pos);
                    break;
                default:
                    throw new System.NotSupportedException("Type not supported");
            }
            return val;
        }

        /// <summary>
        /// Extracts an array from the message and returns it as an object
        /// </summary>
        /// <param name="msg"></param>
        /// <param name="type"></param>
        /// <param name="pos"></param>
        /// <returns></returns>
        public static Object ExtractArray(IntPtr msg, string type, int pos)
        {
            Object val = 0;
            switch (type[pos])
            {
                case 'i':
                    int[] i = new int[MsgArgGetNumArguments(msg, "ai")];
                    MsgArgGetIntArray(msg, i);
                    return i;
                case 'x':
                    long[] l = new long[MsgArgGetNumArguments(msg, "ax")];
                    MsgArgGetLongArray(msg, l);
                    return l;
                case 's':
                    /*
                    IntPtr stringPtr = Marshal.AllocHGlobal(1024);
                    int sSize = MsgArgGetStringPtr(msg, stringPtr);
                    stringPtr = Marshal.ReAllocHGlobal(stringPtr, (IntPtr)sSize);
                    val = Marshal.PtrToStringAnsi(stringPtr);
                    Marshal.FreeHGlobal(stringPtr);
                    break;
                    */
                    throw new System.NotSupportedException("Type not supported");
                case 'd':
                    double[] d = new double[MsgArgGetNumArguments(msg, "ad")];
                    MsgArgGetDoubleArray(msg, d);
                    return d;
                case 'b':
                    bool[] b = new bool[MsgArgGetNumArguments(msg, "ab")];
                    MsgArgGetBoolArray(msg, b);
                    return b;
                default:
                    throw new System.NotSupportedException("Type not supported");
            }
        }

        /// <summary>
        /// Extracts all the values of the message
        /// </summary>
        /// <param name="output">The extracted values</param>
        /// <param name="message">The message containing the values</param>
        /// <param name="sign">The string containing all the Alljoyn type to extract</param>
        public static void extractValues(Object[] output, IntPtr message, string sign)
        {
            int pos = 0;
            for (int i = 0; pos < sign.Length; i++, pos++)
            {
                output[i] = extractValue(NativeHelper.MessageGetArg(message, i), sign, ref pos);
            }
        }

        /// <summary>
        /// Create a MsgArg to reply to a method call
        /// </summary>
        /// <param name="sig">The type of the reply value</param>
        /// <param name="value">The value to send back</param>
        /// <returns></returns>
        public static IntPtr CreateReply(string sig, Object value)
        {
            switch (sig)
            {
                case "Int32":
                    return CreateMsgArgInt((int)value);
                case "Double":
                    return CreateMsgArgDouble((double)value);
                case "String":
                    return CreateMsgArgString((string)value);
                case "Boolean":
                    return CreateMsgArgBool((bool)value);
                case "Void":
                    return IntPtr.Zero;
                default:
                    throw new System.NotSupportedException("Type not supported");

            }

        }

        #region Server native calls

        /// <summary>
        /// DEPRECATED ?
        /// </summary>
        /// <param name="server"></param>
        /// <param name="arg"></param>
        /// <returns></returns>
        [DllImport("alljoyn_native.dll")]
        internal static extern bool Server_RegisterTypeCallbacks(NativeServer server,
            SendStringArg arg);

        [DllImport("alljoyn_native.dll")]
        internal static extern NativeServer Server_Create(string prefix, string name);

        [DllImport("alljoyn_native.dll")]
        internal static extern void Server_Destroy(NativeServer engine);

        [DllImport("alljoyn_native.dll")]
        internal static extern int Server_Initialize(NativeServer engine);

        [DllImport("alljoyn_native.dll")]
        internal static extern int Server_Start(NativeServer engine);

        [DllImport("alljoyn_native.dll")]
        internal static extern int Server_Stop(NativeServer engine);

        [DllImport("alljoyn_native.dll")]
        internal static extern bool Server_RegisterCallbacks(NativeServer server,
            CreateInterfaceCallback createInterfaceHandler);

        [DllImport("alljoyn_native.dll")]
        internal static extern bool Server_AddInterfaceMember(NativeServer server, string name, string inputSig, string outSig, string argNames, int memberType); // 0 signal, 1 method

        [DllImport("alljoyn_native.dll")]
        internal static extern void Server_SetMethodHandler(NativeServer server, ChatMethod func);


        [DllImport("alljoyn_native.dll")]
        internal static extern void Server_SetSignalHandler(NativeServer server, ChatSignal func);
        [DllImport("alljoyn_native.dll")]
        internal static extern void Client_SetSignalHandler(NativeClient client, ChatSignal func);

        [DllImport("alljoyn_native.dll")]
        internal static extern void Server_RegisterMethodHandler(NativeServer server, string intfName);
        [DllImport("alljoyn_native.dll")]
        internal static extern void Server_RegisterSignalHandler(NativeServer server, string intfName);

        #endregion Server native calls

        #region Client native calls

        [DllImport("alljoyn_native.dll")]
        internal static extern NativeClient Client_Create(string prefix, string name);

        [DllImport("alljoyn_native.dll")]
        internal static extern void Client_Destroy(NativeClient client);

        [DllImport("alljoyn_native.dll")]
        internal static extern int Client_Initialize(NativeClient client);

        [DllImport("alljoyn_native.dll")]
        internal static extern int Client_Start(NativeClient client);

        [DllImport("alljoyn_native.dll")]
        internal static extern int Client_Stop(NativeClient client);

        [DllImport("alljoyn_native.dll")]
        internal static extern bool Client_AddInterfaceMember(NativeClient client, string name, string inputSig, string outSig, string argNames, int memberType); // 0 signal, 1 method

        [DllImport("alljoyn_native.dll")]
        internal static extern bool Client_RegisterCreateInterface(NativeClient client,
            CreateInterfaceCallback createInterfaceHandler);

        [DllImport("alljoyn_native.dll")]
        internal static extern IntPtr Client_CallMethod(NativeClient client, string member, IntPtr[] msgs, int size);

        [DllImport("alljoyn_native.dll")]
        internal static extern void Client_RegisterSignalHandler(NativeClient client, string intfName);

        #endregion Client native calls


        #region Alljoyn wrapper
        [DllImport("alljoyn_native.dll")]
        internal static extern IntPtr MessageGetArg(IntPtr message, int index);

        [DllImport("alljoyn_native.dll")]
        internal static extern int MsgArgGetInt(IntPtr msgArg);

        [DllImport("alljoyn_native.dll")]
        internal static extern long MsgArgGetLong(IntPtr msgArg);

        [DllImport("alljoyn_native.dll")]
        internal static extern int MsgArgGetStringPtr(IntPtr msg, IntPtr buff);

        [DllImport("alljoyn_native.dll")]
        internal static extern double MsgArgGetDouble(IntPtr msgArg);

        [DllImport("alljoyn_native.dll")]
        internal static extern bool MsgArgGetBool(IntPtr msgArg);

        [DllImport("alljoyn_native.dll")]
        internal static extern int MessageGetSignature(IntPtr msg, IntPtr buff);

        [DllImport("alljoyn_native.dll")]
        internal static extern IntPtr CreateMsgArgBool(bool val);

        [DllImport("alljoyn_native.dll")]
        internal static extern IntPtr CreateMsgArgInt(int val);

        [DllImport("alljoyn_native.dll")]
        internal static extern IntPtr CreateMsgArgLong(long val);
        [DllImport("alljoyn_native.dll")]
        internal static extern IntPtr CreateMsgArgDouble(double val);

        [DllImport("alljoyn_native.dll")]
        internal static extern IntPtr CreateMsgArgString(string val);

        [DllImport("alljoyn_native.dll")]
        internal static extern IntPtr CreateMsgArgIntArray(int[] val, int size);

        [DllImport("alljoyn_native.dll")]
        internal static extern IntPtr CreateMsgArgLongArray(long[] val, int size);

        [DllImport("alljoyn_native.dll")]
        internal static extern IntPtr CreateMsgArgDoubleArray(double[] val, int size);

        [DllImport("alljoyn_native.dll")]
        internal static extern IntPtr CreateMsgArgBoolArray(bool[] val, int size);


        [DllImport("alljoyn_native.dll")]
        internal static extern int MsgArgGetNumArguments(IntPtr msg, string type);


        [DllImport("alljoyn_native.dll")]
        internal static extern int MsgArgGetDoubleArray(IntPtr msg, double[] values);

        [DllImport("alljoyn_native.dll")]
        internal static extern int MsgArgGetIntArray(IntPtr msg, int[] values);

        [DllImport("alljoyn_native.dll")]
        internal static extern int MsgArgGetLongArray(IntPtr msg, long[] values);

        [DllImport("alljoyn_native.dll")]
        internal static extern int MsgArgGetBoolArray(IntPtr msg, bool[] values);

        #endregion


        // Native signals
        [DllImport("alljoyn_native.dll")]
        internal static extern int Server_SendSignal(NativeServer server, string intfName, IntPtr[] msgs, int numArgs);//NativeServer server, NativeSignal signal);

        /* DEPRECATED ?
        [DllImport("alljoyn_native.dll")]
        internal static extern NativeSignal NativeSignal_Create(int num);

        [DllImport("alljoyn_native.dll")]
        internal static extern void NativeSignal_AddString(NativeSignal signal, string data, int length, int pos);
        */

    }



    public class NonSupportedAllJoynTypeException : Exception
    {

    }
}
