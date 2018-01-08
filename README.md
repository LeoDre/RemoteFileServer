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
## Connection and action codes
The server will first create the socket, bind the port number and ip address, listen
on port 9633 and wait for the client connection. Once the client connects to the server,
the server will accept the client connection and create a new worker thread to handle
the operations that the client want to perform. The client will need to first send the
action code to the server. The action code is a code that identified what action the
client want to do. It can be open, read, write and close. Here’s how action codes map
to these four operations.  
netopen() 1  
netread() 2  
netwrite() 3  
netclose() 4  
Based on the action code, the server will do the operations that the client request.  
## Net file descriptor
The netopen(), netread(), netwrite() and netclose() work just like functions
open(), read(), write(), and close(). The difference is that instead of using a file
descriptor, net…() function uses a netfiledescriptor. It is a special file descriptor. Here
is an explanation of how it is implemented and how it is used:  
struct fds:  
char* pt --------------------> The path name of the file  
int fd --------------------> The file descriptor of this file on the server side  
int mode --------------------> The mode this file is opened  
int stat --------------------> indicate whether this closed  
fda is an array of fds  
Once a file is successfully opened, a fds struct will be created to store the
information of this file. The fds will then be stored in the fda. For example, test.txt is
opened, and a fds is created for it and stored in fda[3]. The server will take this index
3 as a maped file descriptor and encoded (3+1) * (-2) and finally get a net file
descriptor -8. And this netfd will be used until the file is closed.
## Optimization of the data structure
-the array starts at 10 and will expend to twice as big if there are less than 3 free
slots.  
-there is a counter and a ptr that is used to monitor the array. Once a file is close,
the struct that stored in the array will not be deleted. Instead, the pt will be freed
and set to NULL and stat will be set to 0 to indicate this is an “empty” struct that
other files can use.  
##  Error handling
Every errno will be sent back to the user so that user can print it out. Except for
the INVAILD_FILE_MODE. This error is not an error in errno or h_errno. This error
has a value of -3 and requests client to handle it by him/herself.
## others
Other than the 4 points (Connection and action codes, Net file descriptor, Optimization of the data structure, Error handling) mentioned above, the program works in a similar way as
open(), read(), write(), and close(). The server will receive the action code and the
data needed and process all these requests and send back the data/error to the client.
More details can be found in the comments.
## For extra credit
**File mode**  
unrestricted 1  
exclusive 3  
transaction 9  
**Flag**  
O_RDONLY 2  
O_WRONLY 4  
O_RDWR 8  
Combined mode = file mode X flag  
**1.Logic behind**  
UR: file can be opened unless the same file has already been opened under
transaction mode  
UW: file can be open unless the same file has already been opened under
transaction mode or exclusive mode and has already been opened with write
flag  
UWR: same as UW  
ER: file can be opened unless the same file has already been opened under
transaction mode or has already been opened with write flag for more than
one time  
EW: file can be opened unless the same file has already been opened under
transaction mode or has already been opened in write flag.  
TR: file can be opened if this file hasn’t been opened already.  
TW: same as TR  
TWR: same as TR  
**2.how to judge if a file can be opened**  
The fda will be traversed to see check on every file to make sure the file that is
going to be opened satisfied the logic list in 1.
Two files will be considered as same if they have the same path name
The combined mode will be checked during each check
##  Contributions
**Team Members**: Ran Sa, Chengguizi Han
