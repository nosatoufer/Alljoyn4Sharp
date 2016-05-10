using alljoyn_net;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace AllJoyn4Sharp_console
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
            if (Chat != null)
            {
                Chat(s);
            }
        }

        [RemoteEvent]
        public event TestEvent Test;

        public void OnTest()
        {
            if(Test !=null)
            {
                Test();
            }
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
    }

    class Program
    {
        static void Main(string[] args)
        {
            NetObject obj = new NetObject();
            Server<IObject> s = new Server<IObject>("org.alljoyn.bus.samples.chat.", "bubo", obj);
            s.Start();
            string str = "";
            while (str != "quit")
            {
                if (str != "")
                {
                    Console.WriteLine("I wrote this: " + str);
                    obj.OnChat(str);
                }
                str = Console.ReadLine();
                s.test();
            }
            s.Stop();
        }
    }
}
