# README #

### How do I get set up? ###

Add environment variables to find alljoyn lib :

* ALLJOYN_SDK_COMMON == <PathToAlljoyn>\alljoyn\common\inc
* ALLJOYN_SDK_HOME == <PathToAlljoyn>\alljoyn
* ALLJOYN_SDK_INC == <PathToAlljoyn>\alljoyn\build\win7\x86\debug\dist\cpp\inc

Change the platform to Win32 on Visual studio  

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
  