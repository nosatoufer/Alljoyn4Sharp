using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Reflection.Emit;
using System.Runtime.InteropServices;
using System.Text;

namespace alljoyn_net
{
    struct NativeServer
    {
        public IntPtr engine;
    }


    internal delegate bool AddInterfaceMemberCallback(string name, string inputSig, string outSig, string argNames, int memberType);
    public delegate void CreateInterfaceCallback();
    public delegate void StartInvoke(string member, int numParams);
    public delegate void FinishInvoke();

    public delegate IntPtr ChatMethod(string memberName, IntPtr msg);
    public delegate void ChatSignal(string memberName, IntPtr msg);

    internal delegate int SendStringArg(string arg1);


    [AttributeUsage(AttributeTargets.Method)]
    public class RemoteMethodAttribute : System.Attribute
    {
        public RemoteMethodAttribute()
        {
        }
    }

    [AttributeUsage(AttributeTargets.Event)]
    public class RemoteEventAttribute : System.Attribute
    {
        public RemoteEventAttribute()
        {
        }
    }


    public class Server<T>
    {
        private T _remoteObj;

        private static NativeServer? _nativeServer;

        private ChatMethod _chatMethod;
        private ChatSignal _chatSignal;
        #region Type callbacks

        /// <summary>
        /// Manages the reception of a signal received
        /// </summary>
        /// <param name="memberName">The name of the event received</param>
        /// <param name="msg">The message sent with the event</param>
        public void ManageSignalCall(string memberName, IntPtr msg)
        {

            Console.WriteLine("{0} called", memberName);
            //Type type = typeof(T);
            //EventInfo eInfo = type.GetEvent(memberName);
            //Console.WriteLine("Member = " + memberName);
            //string sign = NativeHelper.MessageSignature(msg);
            //Object[] values = new Object[sign.Length];
            //NativeHelper.extractValues(values, msg, sign);
            //MethodInfo mInfo = eInfo.GetRaiseMethod();
        }

        /// <summary>
        /// Manages the remote call of a method to this process
        /// </summary>
        /// <param name="memberName">The name of the method</param>
        /// <param name="msg">The message set with the method call</param>
        /// <returns>The reply to send back</returns>
        public IntPtr ManageMethodCall(string memberName, IntPtr msg)
        {
            Type type = typeof(T);
            MethodInfo method = type.GetMethod(memberName);

            Console.WriteLine("Member = " + memberName);
            Object[] values = new Object[method.GetParameters().Length];

            string sign = NativeHelper.MessageSignature(msg);
            NativeHelper.extractValues(values, msg, sign);
            Object ret = method.Invoke(_remoteObj, values);
            IntPtr reply = NativeHelper.CreateReply(method.ReturnParameter.ParameterType.Name, ret);
            return reply;
        }

        public void SendEvent(string name, Object[] vars)
        {
            Console.WriteLine("SendEvent {0}", name);
            Type t = typeof(T);
            EventInfo eInfo = t.GetEvent(name);
            if (eInfo != null)
            {
                IntPtr[] msgs = NativeHelper.CreateMessages(vars);
                NativeHelper.Server_SendSignal(_nativeServer.Value, name, msgs, msgs.Length);
            }
        }

        #endregion Type callbacks

        #region Interface creation


        internal bool AddInterfaceMember(string name, string inputSig, string outSig, string argNames, int memberType)
        { // 0 signal, 1 method
            return NativeHelper.Server_AddInterfaceMember(_nativeServer.Value, name, inputSig, outSig, argNames, memberType);
        }

        private void CreateInterface()
        {
            Console.WriteLine("Creating Interface");
            NativeHelper.RegisterMembers(typeof(T), AddInterfaceMember);

        }

        #endregion Interface creation



        #region Constructor and initialization

        /// <summary>
        /// Wrapper for native server
        /// </summary>
        /// <param name="prefix">Prefix</param>
        /// <param name="name">Name</param>
        /// <param name="obj">Net object implementing all the methods and signal</param>
        public Server(string prefix, string name, T obj)
        {

            _nativeServer = NativeHelper.Server_Create(prefix, name);
            if (!_nativeServer.HasValue)
                throw new ApplicationException("Unable to create native server part!");

            //bool b = NativeHelper.Server_RegisterTypeCallbacks(_nativeServer.Value, ReceiveString);

            NativeHelper.Server_RegisterCallbacks(_nativeServer.Value, CreateInterface);

            _remoteObj = obj;

            //NativeHelper.BindEvents<T>(typeof(Server<>), obj);
            Console.WriteLine("Server Created");
        }

        #endregion Constructor and initialization

        #region Call Forwarding

        /// <summary>
        /// DEPRECATED ?
        /// </summary>
        /// <param name="args"></param>
        /// <param name="eventName"></param>
        public static void ForwardEvent(object[] args, string eventName)
        {
            if (!_nativeServer.HasValue) return;
            Console.WriteLine(eventName + "(param = " + args + ")");
            //NativeSignal sig = NativeHelper.NativeSignal_Create(args.Length);
            for (int i = 0; i < args.Length; i++)
            {
                if (args[i].GetType() == typeof(string))
                {
                    //NativeHelper.NativeSignal_AddString(sig, (string)args[i], ((string)args[i]).Length, i);
                }
                else
                {
                    throw new ApplicationException("Type " + args[i].GetType() + " is not yet supported by AllJoyn wrrapper.");
                }
            }
            int stat = 0;// NativeHelper.Server_SendSignal(_nativeServer.Value, sig);
            if (stat != 0)
            {
                Console.Error.WriteLine("Unable to send signal!!!");
            }
        }

        #endregion Call Forwarding


        private void Destroy()
        {
            if (_nativeServer != null)
            {
                NativeHelper.Server_Destroy(_nativeServer.Value);
                _nativeServer = null;
            }
        }

        ~Server()
        {
            Console.WriteLine("Calling destructor.");
            Destroy();
        }

        /// <summary>
        /// Starts the alljoyn services
        /// </summary>
        public void Start()
        {
            if (!_nativeServer.HasValue)
                throw new ApplicationException("Server was not initialized!");
            NativeHelper.Server_Initialize(_nativeServer.Value);
            InitializeHandlers();
            NativeHelper.Server_Start(_nativeServer.Value);

        }

        /// <summary>
        /// Stops the alljoyn services
        /// </summary>
        public void Stop()
        {
            if (!_nativeServer.HasValue)
                throw new ApplicationException("Server was not initialized!");
            NativeHelper.Server_Destroy(_nativeServer.Value);
        }


        /// <summary>
        /// Sets signals and methods handlers and register all the methods and events
        /// </summary>
        private void InitializeHandlers()
        {
            _chatMethod = new ChatMethod(this.ManageMethodCall);
            _chatSignal = new ChatSignal(this.ManageSignalCall);
            NativeHelper.Server_SetMethodHandler(_nativeServer.Value, _chatMethod);
            NativeHelper.Server_SetSignalHandler(_nativeServer.Value, _chatSignal);

            // Methods initialization
            Type type = typeof(T);
            MethodInfo[] methodInfos = type.GetMethods();
            foreach (MethodInfo mInfo in methodInfos)
            {
                Console.WriteLine(mInfo.Name);
                object[] attributes = mInfo.GetCustomAttributes(false);
                foreach (object attribute in attributes)
                {
                    if (attribute is RemoteMethodAttribute)
                    {
                        Console.WriteLine("METHOD NAME = {0}", mInfo.Name);
                        NativeHelper.Server_RegisterMethodHandler(_nativeServer.Value, mInfo.Name);
                        break;
                    }
                }
            }
            // Events initialization
            EventInfo[] eInfos = type.GetEvents();
            foreach (EventInfo eInfo in eInfos)
            {
                Console.WriteLine(eInfo.Name);
                object[] attributes = eInfo.GetCustomAttributes(false);
                foreach (object attribute in attributes)
                {
                    if (attribute is RemoteEventAttribute)
                    {
                        Console.WriteLine("EVENT NAME  = {0}", eInfo.Name);

                        NativeHelper.Server_RegisterSignalHandler(_nativeServer.Value, eInfo.Name);
                        break;
                    }
                }
            }
        }
    }





    #region Extraction of annotations

    #endregion Extraction of annotations
}
