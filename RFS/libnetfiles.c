//include the necessary .h files
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include<arpa/inet.h>
#include<netdb.h>
#include <unistd.h>
#include <linux/limits.h>
#include "libnetfiles.h"

//global variables 
char ip[17];
int fm;
int fg;

//check if the hostname and file mode are valid
//take 2 arguments
//1. the host name as a string
//2. the file mode
//return 0 on success, -1 on error and h_errno will be set
int netserverinit(char* hostname, int filemode){
	//check if the hostname is valid
	struct hostent *he;
	he = gethostbyname(hostname);
	if(he==NULL){
		h_errno = HOST_NOT_FOUND;
		return -1;
	}
	//convert hostname to ip
	struct in_addr **addr_list;
	addr_list = (struct in_addr **) he->h_addr_list;
	//chose the first ip address in the list and store it in global variable ip
    strcpy(ip , inet_ntoa(*addr_list[0]) );
	//check if the file mode is valid
	//if it is valid, store it in global variable fm
	switch(filemode){
		case unrestricted:
			fm = unrestricted;
			return 0;
		case exclusive:
			fm = exclusive;
			return 0;
		case transaction:
			fm = transaction;
			return 0;
		default:
			h_errno = INVALID_FILE_MODE;
			return -1;
	}
	return 0;
}

//connect to server
//no input argument, use gloabl variables ip and pre-defined pnum to connect 
//return a socketfd on success, -1 on error and errno will be set.
int cts(){
	//create the sockect for connection
	int csockfd;
	csockfd = socket(AF_INET, SOCK_STREAM,0);
	//create the struct that hold the server's ip and port number
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(ip);
	server.sin_port = htons(pnum);
	//connect to the server
	int ind0 = connect(csockfd, (struct sockaddr*)&server, sizeof(server));
	//check if the connection is success, if not return -1 and the connect funtion already seted the errno
	if(ind0==-1){
		return -1;
	}
	return csockfd;
}

//open the file on the server
//2 input arguments 1.the file's pathname 2.the flag(r,w,r&w)
//return a netfd on success, return -1 on error and errno will be set
int netopen(const char* pathname, int flags){
	//connect to the server
	int sk = cts();
	if(sk==-1){
		return -1;
	}
	//send the open action code to the server
	//action code
	//1 open
	//2 read
	//3 write
	//4 close
	int ac = 1;
	int ind0 = send(sk, (void*)&ac, sizeof(int),0);
	if(ind0==-1){
		return -1;
	}
	//wait for the server to reply
	int rp;
	int ind1 = recv(sk,(void*)&rp,sizeof(int),0);
	if(ind1==-1){
		return -1;
	}
	//find the size of the path
	int sz = strlen(pathname)+1;
	int ind2 = send(sk,(void*)&sz,sizeof(int),0);
	//send the path name to the server
	int ind3 = send(sk,(void*)pathname,sz*sizeof(char),0);
	//send the combain mode
	fg = flags;
	int cb = fm*fg;
	int ind4 = send(sk, (void*)&cb, sizeof(int),0);
	if(ind3==-1||ind2==-1||ind4==-1){
		return -1;
	}
	int nfd = -1;
	//try to recive the netfd
	int ind5 = recv(sk,(void*)&nfd,sizeof(int),0);
	if(ind5 ==-1){
		return -1;
	}else if(nfd>=0){
		errno = nfd;
		return -1;
	}
	return nfd;
}
//read from the server and save the data into a buffer
//three arguments 1.a netfd 2.a buffer pointer 3.the size that the client want to read
//return the size that has been read on success, -1 on error and errno will be set
ssize_t netread(int fildes, void *buf, size_t nbyte){
	//check if the nfd is valid
	if(fildes>=-1){
		errno = EBADF;
		return -1;
	}
	//try to conenct to the server
	int sk = cts();
	if(sk==-1){
		return -1;
	}
	//send the open action code to the server
	//action code
	//1 open
	//2 read
	//3 write
	//4 close
	int ac = 2;
	int ind0 = send(sk, (void*)&ac, sizeof(int),0);
	if(ind0==-1){
		return -1;
	}
	//wait for the server to reply
	int rp;
	int ind1 = recv(sk,(void*)&rp,sizeof(int),0);
	if(ind1==-1){
		return -1;
	}
	//send the netfd
	int ind2 = send(sk, (void*)&fildes, sizeof(int),0);
	if(ind2==-1){
		return -1;
	}
	//send size want to read
	int ind3 = send(sk, (void*)&nbyte, sizeof(ssize_t),0);
	if(ind3==-1){
		return -1;
	}
	//try to recv if the read is success
	int rs;
	int ind = recv(sk,(void*)&rs,sizeof(int),0);
	if(ind==-1){
		return -1;
	}
	if(rs!=-1){
		errno = rs;
		return -1;
	}
	//try to recv the size
	ssize_t s;
	int ind4 = recv(sk,(void*)&s,sizeof(ssize_t),0);
	if(ind4==-1){
		return -1;
	}else if(s<0){
		s = -1*s;
		errno = s;
		return -1;
	}
	//try to recv the buffer
	int ind5 = recv(sk,buf,s,0);
	if(ind5==-1){
		return -1;
	}
	//all process success, return the size
	return s;
}

//write the data to the file that the client sent
//three arguments 1.a netfd 2.a buffer pointer 3.the size that the client want to write
//return the size that has been written on success, -1 on error and errno will be set
ssize_t netwrite(int fildes, void *buf, size_t nbyte){
	//check if the nfd is valid
	if(fildes>=-1){
		errno = EBADF;
		return -1;
	}
	//try to conenct to the server
	int sk = cts();
	if(sk==-1){
		return -1;
	}
	//send the open action code to the server
	//action code
	//1 open
	//2 read
	//3 write
	//4 close
	int ac = 3;
	int ind0 = send(sk, (void*)&ac, sizeof(int),0);
	if(ind0==-1){
		return -1;
	}
	//wait for the server to reply
	int rp;
	int ind1 = recv(sk,(void*)&rp,sizeof(int),0);
	if(ind1==-1){
		return -1;
	}
	//send the netfd
	int ind2 = send(sk, (void*)&fildes, sizeof(int),0);
	if(ind2==-1){
		return -1;
	}
	//send size want to write
	int ind3 = send(sk, (void*)&nbyte, sizeof(ssize_t),0);
	if(ind3==-1){
		return -1;
	}
	//send the buffer want to write
	int ind4 = send(sk,buf,nbyte,0);
	if(ind4==-1){
		return -1;
	}
	//try to recv if the read is success
	int rs;
	int ind = recv(sk,(void*)&rs,sizeof(int),0);
	if(ind==-1){
		return -1;
	}
	if(rs!=-1){
		errno = rs;
		return -1;
	}
	//try to recv the size
	ssize_t s;
	int ind5 = recv(sk,(void*)&s,sizeof(ssize_t),0);
	if(ind5==-1){
		return -1;
	}else if(s<0){
		printf("A\n");
		s = -1*s;
		errno = s;
		return -1;
	}
	//all process success, return the size
	return s;
}

//close the file on the server
//one argument 1.a netfd
//return 0 on success, -1 on error and the errno is seted
int netclose(int fd){
	//check to see if the netfd is valid
	if(fd>=-1){
		errno = EBADF;
		return -1;
	}
	//connect to the server
	int sk = cts();
	if(sk==-1){
		return -1;
	}
	//send the open action code to the server
	//action code
	//1 open
	//2 read
	//3 write
	//4 close
	int ac = 4;
	int ind0 = send(sk, (void*)&ac, sizeof(int),0);
	if(ind0==-1){
		return -1;
	}
	//wait for the server to reply
	int rp;
	int ind1 = recv(sk,(void*)&rp,sizeof(int),0);
	if(ind1==-1){
		return -1;
	}
	//send the netfd to the server
	int ind2 = send(sk,(void*)&fd,sizeof(int),0);
	if(ind2==-1){
		return -1;
	}
	//wait for the server to reply the result
	int r;
	int ind3 = recv(sk, (void*)&r,sizeof(int),0);
	if(ind3==-1){
		return -1;
	}else if(r!=0){
		errno = EBADF;
		return -1;
	}
	return 0;
}










