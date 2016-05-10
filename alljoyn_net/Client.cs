using System;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace alljoyn_net
{
    struct NativeClient
    {
        public IntPtr engine;
    }

    /// <summary>
    /// Represents a client connecting to a server
    /// This class is used to connect, call method and receive signals.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public class Client<T>
    {
        private T _proxyObject;
        private static NativeClient? _nativeClient;
        ChatSignal _eventHandler;


        /// <summary>
        /// Wrapper for the Native client
        /// </summary>
        /// <param name="prefix">Prefix</param>
        /// <param name="name">The name</param>
        /// <param name="proxyObject">The object implementing the interface used for communication</param>
        public Client(string prefix, string name, T proxyObject)
        {
            Console.WriteLine("Creating client");
            _proxyObject = proxyObject;
            _nativeClient = NativeHelper.Client_Create(prefix, name);
            if (!_nativeClient.HasValue)
                throw new ApplicationException("Unable to create native server part!");

            NativeHelper.Client_RegisterCreateInterface(_nativeClient.Value,
                                CreateInterface);
        }

        /// <summary>
        /// Manages the received signal
        /// </summary>
        /// <param name="memberName">The name of the signal</param>
        /// <param name="msg">The message containing all the parameters</param>
        public void ManageSignalCall(string memberName, IntPtr msg)
        {
            Console.WriteLine("{0} called", memberName);

            Type type = typeof(T);
            EventInfo eInfo = type.GetEvent(memberName);
            string signature = NativeHelper.MessageSignature(msg);
            Object[] values = new Object[signature.Length];
            NativeHelper.extractValues(values, msg, signature);
            FireEvent(memberName, values);

        }

        /// <summary>
        /// Fires an event when the corresponding signal is received
        /// </summary>
        /// <param name="name">The name of the event</param>
        /// <param name="vars">The parameters received with the signal</param>
        public void FireEvent(string name, Object[] vars)
        {
            var eventInfo = _proxyObject.GetType().GetField(name,
                                BindingFlags.Instance | BindingFlags.NonPublic);
            if (eventInfo != null)
            {
                var event_member = eventInfo.GetValue(_proxyObject);
                // Note : If event_member is null, nobody registered to the event, you can't call it.
                if (event_member != null)
                    event_member.GetType().GetMethod("Invoke").Invoke(event_member, vars);
            }
        }

        /// <summary>
        /// Calls a method by name
        /// </summary>
        /// <param name="name">The name of the method to call</param>
        /// <param name="vars">All the parameters that will be sent</param>
        public void CallMethod(string name, object[] vars)
        {
            Type type = typeof(T);
            MethodInfo mInfo = type.GetMethod(name);
            if (mInfo != null)
            {
                ParameterInfo[] sig = mInfo.GetParameters();
                if (sig.Length != vars.Length)
                    throw new ApplicationException("Wrong number of arguments");

                IntPtr[] msgs = NativeHelper.CreateMessages(vars, sig);
                IntPtr reply = NativeHelper.Client_CallMethod(_nativeClient.Value, name, msgs, msgs.Length);
                if (reply != IntPtr.Zero)
                {
                    Object replyValue = NativeHelper.extractValue(reply, mInfo.ReturnType);
                    Console.WriteLine("Value received : {0}", replyValue);
                }
                else
                {
                    // TODO : MANAGE NULL REPLY ( ERROR )
                }
            }

        }

        /// <summary>
        /// Manages the reply received when a method is called on the server
        /// </summary>
        /// <param name="name">The name of the method</param>
        /// <param name="reply">The reply received from the method call</param>
        /// <returns></returns>
        private Object ManageReply(string name, IntPtr reply)
        {
            Type type = typeof(T);
            MethodInfo mInfo = type.GetMethod(name);
            return mInfo;//extractValue(reply, mInfo.ReturnType);
        }

        /// <summary>
        /// Tries to 
        /// </summary>
        /// <returns></returns>
        public bool Connect()
        {
            if (!_nativeClient.HasValue)
                throw new ApplicationException("Client was not created!");

            NativeHelper.Client_Initialize(_nativeClient.Value);
            InitializeHandlers();

            NativeHelper.Client_Start(_nativeClient.Value);
            return true;
        }

        public void Disconnect()
        {
            NativeHelper.Client_Destroy(_nativeClient.Value);
        }

        #region Interface creation

        internal bool AddInterfaceMember(string name, string inputSig, string outSig, string argNames, int memberType)
        { // 0 signal, 1 method
            return NativeHelper.Client_AddInterfaceMember(_nativeClient.Value, name, inputSig, outSig, argNames, memberType);
        }

        private void CreateInterface()
        {
            Console.WriteLine("Creating Interface");
            NativeHelper.RegisterMembers(typeof(T), AddInterfaceMember);
        }
        #endregion Interface creation

        /// <summary>
        /// Initialize the handlers used by the C++ dll to handle the signals
        /// </summary>
        private void InitializeHandlers()
        {
            _eventHandler = new ChatSignal(this.ManageSignalCall);
            NativeHelper.Client_SetSignalHandler(_nativeClient.Value, _eventHandler);

            Type type = typeof(T);
            EventInfo[] eInfos = type.GetEvents();
            foreach (EventInfo eInfo in eInfos)
            {
                object[] attributes = eInfo.GetCustomAttributes(false);
                foreach (object attribute in attributes)
                {
                    if (attribute is RemoteEventAttribute)
                    {
                        NativeHelper.Client_RegisterSignalHandler(_nativeClient.Value, eInfo.Name);
                        break;
                    }
                }
            }
        }
    }
}
