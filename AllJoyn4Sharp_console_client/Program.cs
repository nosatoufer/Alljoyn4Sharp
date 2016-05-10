using alljoyn_net;
using System;
using System.Reflection;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace AllJoyn4Sharp_console_client
{
    public delegate int ChatMethod(string memberName, IntPtr msg);
    public delegate void ChatEvent(string num);
    public delegate void TestEvent();

    public interface IObject
    {
        [RemoteEvent]
        event ChatEvent Chat;

        [RemoteEvent]
        event TestEvent Test;

        [RemoteMethod]
        string cat(string s1, string s2);

        [RemoteMethod]
        int mySum(int a, int b);
    }

    public class NetObject : IObject
    {

        [RemoteEvent]
        public event ChatEvent Chat;

        public void OnChat(string s)
        {
            Console.WriteLine("Machin sent : {0}", s);
        }

        [RemoteEvent]
        public event TestEvent Test;

        public void OnTest()
        {
            Console.WriteLine("OnTest Called !");
        }

        [RemoteMethod]
        public string cat(string s1, string s2)
        {
            string ret = s1 + s2;
            Console.WriteLine("Invoking NetObject.cat(\"" + s1 + "\" , \"" + s2 + "\") = \"" + ret + "\"");
            return ret;
        }

        [RemoteMethod]
        public int mySum(int a, int b)
        {
            return a + b;
        }

        public void FireEvent(string name, Object[] vars)
        {
            var eventInfo = this.GetType().GetField(name,
                                BindingFlags.Instance | BindingFlags.NonPublic);
            if (eventInfo != null)
            {
                var event_member = eventInfo.GetValue(this);
                // Note : If event_member is null, nobody registered to the event, you can't call it.
                if (event_member != null)
                    event_member.GetType().GetMethod("Invoke").Invoke(event_member, vars);
            }
        }

    }

    class Program
    {
        static void Main(string[] args)
        {
            NetObject nObject = new NetObject();
            Client<IObject> c = new Client<IObject>("org.alljoyn.bus.samples.chat.", "bubo", nObject);
            nObject.Test += nObject.OnTest;
            nObject.Chat += nObject.OnChat;
            c.Connect();
            Object[] testE = new Object[1];
            //IObject obj = c.GetProxy();
            Object[] vars = new Object[2];
            vars[0] = 10;
            vars[1] = 12;

            Object[] vars2 = new Object[2];
            vars2[0] = "Test";
            vars2[1] = " passed.";

            try
            {
                c.CallMethod("mySum", vars);
                Console.ReadLine();
                c.CallMethod("cat", vars2);
            }
            catch (ApplicationException aException)
            {
                Console.WriteLine(aException.Message);
            }
            string str = "";
            while (str != "quit")
            {
                if (str != "")
                {
                    Console.WriteLine("I wrote this: " + str);
                    nObject.OnChat(str);
                }
                str = Console.ReadLine();
                vars = new Object[2];
                vars[0] = 123;
                vars[1] = 321;
                c.CallMethod("mySum", vars);

            }
            c.Disconnect();
        }
    }
}
