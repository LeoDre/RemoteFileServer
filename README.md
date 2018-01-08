# RemoteFileServer
##  Structure of this program
-libnetfiles.h  
-libnetfiles.h  
-netfileserver.c  
-netclient.c  
-Makefile  
-tf (test file directory)  
## General Information
  This program includes two parts, the server part and the client library part. A
client program named “netclient” uses the function in the client library “libnetfiles” to
open, read, write and close files on the server remotely. And server will open, read,
write and close files locally and send back the data that the client requests. The
details, input arguments, return values and error handling of each function (no matter
on the server side or client side) are all clearly commented at the start of the function.
##  Contributions
**Team Members**: Ran Sa, Chengguizi Han
