# README #
## What does it do ?
Alljoyn4Sharp is a C++/C# module to use the AllJoyn framework. It allows multiples remotes application to communicate through event and method calls. 

## Setting it up
See the wiki pages

## How to use it ?

* Interface  
Create an interface for all the methods / event you will use between the client and the server.
* NetObject  
Create the class implementing all the methods of the interface.
* Creation of the server/client  
1. Create the server or client using the NativeHelper  
1. Assign the method you want to be called to the event you want to listen to
* Method / event call  
All the methods call and event are used by giving the name of the method/event and an array of object containing the parameters.
  
