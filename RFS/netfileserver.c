//library required for the server side
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include<pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <linux/limits.h>

//define the prot number
#define pnum 9633
//define different file mode
#define ur 2
#define uw 4
#define uwr 8
#define er 6
#define ew 12
#define ewr 24
#define tr 18
#define tw 36
#define twr 72
//a struct that holding an inusing fd
typedef struct fdsruct{
	char* pt;
	int fd;
	int mode;
	int stat;
}fds;
//a struct array that hold all the inusing fd
fds* fda;
//array pointer, array counter and array size.
int aptr;
int acount;
int asize;
//expand the array that hold all the in using files
//no arugument needed 
//no return
void expand(){
	asize = asize*2;
	fds* tmp = (fds*)realloc(fda,asize*sizeof(fds));
	if(tmp!=NULL){
		fda = tmp;
	}
}
//optimize the datastruct that hold the in using files
//no input argument needed
//return the next slot avaliable for the struct to insert
int opt(){
	if(aptr==(acount-1)&&fda[aptr+1].stat!=1){
		aptr++;
	}else{
		int i;
		for(i=0;i<acount;i++){
			if(fda[i].stat==0){
				aptr=i;
			}
		}
	}
	if(acount == (asize-3)){
		expand();
	}
	return aptr;
}
//check if the file can be open in the mode that the client expect
//2 arguments 1.pathname 2.mode
//return 0 on success, -1 on error, errno is NOT setted
int checkmode(char* pn, int m){
	/*if(aptr == 0){
		return 0;
	}*/
	//if two struct have the same file path and have conflict mode, return -1;
	//the logic and definition of conflict will be explained in readme
	int i;
	int ind0 = 0;
	int ind1 = 0;
	switch(m){
		case ur:
			for(i=0; i<acount;i++){
				if(fda[i].stat==0){
					continue;
				}
				int sm = fda[i].mode;
				if((strcmp(pn, fda[i].pt)==0)&&(sm>=18)){
					return -1;
				}
			}
			return 0;
		case uw:
		case uwr:
			for(i=0; i<acount;i++){
				if(fda[i].stat==0){
					continue;
				}
				int sm = fda[i].mode;
				if((strcmp(pn, fda[i].pt)==0)&&(sm>=12)){
					return -1;
				}
				if(strcmp(pn, fda[i].pt)==0){
					if(sm==4||sm==8){
						ind0 = ind0+1;
					}else if(sm==6){
						ind1 = 1;
					}
				}
			}
			if(ind1==1&&ind0>0){
				return -1;
			}
			return 0;
		case er:
			for(i=0; i<acount;i++){
				if(fda[i].stat==0){
					continue;
				}
				int sm = fda[i].mode;
				if((strcmp(pn, fda[i].pt)==0)&&(sm>=18)){
					return -1;
				}
				if(strcmp(pn, fda[i].pt)==0){
					if(sm==4||sm==8){
						ind0++;
					}
				}
			}
			if(ind0>1){
				return -1;
			}
			return 0;
		case ew:
		case ewr:
			for(i=0; i<acount;i++){
				if(fda[i].stat==0){
					continue;
				}
				int sm = fda[i].mode;
				if((strcmp(pn, fda[i].pt)==0)&&((sm>=12)||(sm==4)||(sm==8))){
					return -1;
				}
			}
			return 0;
		case tr:
		case tw:
		case twr:
			for(i=0; i<acount;i++){
				if(fda[i].stat==0){
					continue;
				}
				printf("%s\n",pn);
				printf("%s\n",fda[i].pt);
				printf("%i\n",strcmp(pn, fda[i].pt));
				if(strcmp(pn, fda[i].pt)==0){
					return -1;
				}
			}
			return 0;
		default:
			return -1;
	}
}

//check if the path is a file
//one argument 1.the path name
//return 0 if the path is a name, -1 if not and the errno are set
int checkfile(char* pathname){
	DIR* pdir;
	int err;
	pdir = opendir(pathname);
	//check if it is a file, a path or an invalid path
	if(pdir==NULL){
		err = errno;
		//invalid file or path name
		if(err==20){
			//it is a file
			return 0;	
		}else{
			return -1;
		}
	}else{
		//it is a dir, set errno
		errno = EISDIR;
		return -1;
	}
	closedir(pdir);
	return 0;
}
//decode which mode the clinet want to open the file
//one argument 1.the combined file open mode
//return a file open mode
int modedecode(int m){
	switch(m){
		case 2:
		case 6:
		case 18:
			return O_RDONLY;
		case 4:
		case 12:
		case 36:
			return O_WRONLY;
		case 8:
		case 24:
		case 72:
			return O_RDWR;
		default:
			return -1;
	}
}
//encode the mapfd
//1 argument 1.the maped fd
//return the netfd
int fdencode(int mfd){
	return -2*(mfd+1);
}
//decode the netfd
//1 argument 1.the netfd
//return the mapfd
int fddecode(int nfd){
	return (nfd/-2)-1;
}
//try to open a file on the server side
//1 arguments 1.the client socketfd
//return a int. Every thing will be sent back to the client 
int sopen(int sfd){
	//tell the client to send the path and the mode
	int a = 0;
	send(sfd, (void*)&a,sizeof(int),0);
	//recv the size of the datastruct
	int sz;
	recv(sfd, (void*)&sz,sizeof(int),0);
	//recv the path
	char* path = (char*)malloc(sz*sizeof(char));
	recv(sfd, (void*)path, sz*sizeof(char),0);
	//recv the mode
	int m;
	recv(sfd,(void*)&m, sizeof(int),0);
	int ind0 = checkfile(path);	
	if(ind0==-1){
		send(sfd, (void*)&errno,sizeof(int),0);
		return -1;
	}
	//if there is no using file, then no need to check if the mode is ok
	//if there are files that is in using check to see if the mode that the client request is ok
	if(acount!=0){
		int ind2 = checkmode(path,m);
		if(ind2==-1){
			errno = EACCES;
			send(sfd, (void*)&errno,sizeof(int),0);
			return -1;
		}
	}
	//the mode is ok and the path is valid.
	//open the file and put it into the in using file datastructure
	int om = modedecode(m);
	int ffd = open(path,om);
	if(ffd==-1){
		send(sfd, (void*)&errno,sizeof(int),0);
		return -1;
	}
	//find a spot avaliable for the strcut
	int p;
	if(acount==0){
		p = 0;
	}else{
		p = opt();
	}
	//check if the spot in the array is a empty struct or nothing
	if(fda[p].stat==0){
		//it is a used empty struct
		fda[p].pt = path;
		fda[p].fd = ffd;
		fda[p].mode = m;
		fda[p].stat = 1;
	}else{
		//there is nothing in this spot
		//construct a struct for the file
		fds* x = (fds*)malloc(sizeof(fds));
		x->pt = path;
		x->fd = ffd;
		x->mode = m;
		x->stat = 1;
		fda[p] = *x;
	}
	++acount;
	//encode the netfiledescripter
	int b = fdencode(p);
	//send back the netfd
	send(sfd, (void*)&b,sizeof(int),0);
	return 0;
}

//read data to the buffer and send back the buffer and the amount this function read
//one argument 1.the netfd
//return 0 on success, -1 on error and all errors will be send back to the client
int sread(sfd){
	//tell the client to send the fd he/she wants to write
	int a = 0;
	send(sfd, (void*)&a,sizeof(int),0);
	//try to recv the nfd and the buffer size
	int nfd;
	ssize_t s;
	recv(sfd, (void*)&nfd,sizeof(int),0);
	recv(sfd, (void*)&s,sizeof(ssize_t),0);
	//first decode the nfd to a maped fd
	int mfd = fddecode(nfd);
	//chekc if the mfd is valid
	if(mfd>=asize){
		int f = -1*EBADF;
		send(sfd, (void*)&f,sizeof(int),0);
		return -1;
	}
	//map the maped fd to find the fd stored in the struct
	fds* t = (fds*)malloc(sizeof(fds)); 
	t = &(fda[mfd]);
	//check if this file has already been closed
	if(t->stat==0){
		int ff = -1*EBADF;
		send(sfd, (void*)&ff,sizeof(int),0);
		return -1;
	}
	//check if the fd has the permission to read or write
	int m = t->mode;
	m = modedecode(m);
	if(m==O_WRONLY){
		int err = EACCES;
		send(sfd,(void*)&err,sizeof(int),0);
		return -1;
	}else{
		int rs = -1;
		send(sfd,(void*)&rs,sizeof(int),0);
	}
	//pre erro check is done, read from the file
	void* b = (void*)malloc(s);
	int fd = t->fd;
	ssize_t rt = read(fd,b,s);
	//check if the read is success
	if(rt==-1){
		rt = -1*errno;
		send(sfd,(void*)&rt, sizeof(ssize_t),0);
		return -1;
	}
	//send back the buffer and the size
	send(sfd,(void*)&rt, sizeof(ssize_t),0);
	send(sfd,b, rt,0);
	free(b);
	b = NULL;
	return 0;
}

//write data from the buffer to the file and send back the amount this function write
//one argument 1.the netfd
//return 0 on success, -1 on error and all errors will be send back to the client
int swrite(sfd){
	//tell the client to send the fd he/she wants to write
	int a = 0;
	send(sfd, (void*)&a,sizeof(int),0);
	//try to recv the nfd and the buffer size
	int nfd;
	ssize_t s;
	recv(sfd, (void*)&nfd,sizeof(int),0);
	recv(sfd, (void*)&s,sizeof(ssize_t),0);
	//get ready to recv the buffer
	void* bf = (void*)malloc(s);
	recv(sfd,bf,s,0);
	//first decode the nfd to a maped fd
	int mfd = fddecode(nfd);
	//chekc if the mfd is valid
	if(mfd>=asize){
		int f = -1*EBADF;
		send(sfd, (void*)&f,sizeof(int),0);
		return -1;
	}
	//map the maped fd to find the fd stored in the struct
	fds* t = (fds*)malloc(sizeof(fds)); 
	t = &(fda[mfd]);
	//check if this file has already been closed
	if(t->stat==0){
		int ff = -1*EBADF;
		send(sfd, (void*)&ff,sizeof(int),0);
		return -1;
	}
	//check if the fd has the permission to read or write
	int m = t->mode;
	m = modedecode(m);
	if(m==O_RDONLY){
		int err = EACCES;
		send(sfd,(void*)&err,sizeof(int),0);
		return -1;
	}else{
		int rs = -1;
		send(sfd,(void*)&rs,sizeof(int),0);
	}
	//pre erro check is done, read from the file
	int fd = t->fd;
	ssize_t rt = write(fd,bf,s);
	//check if the read is success
	if(rt==-1){
		rt = -1*errno;
		send(sfd,(void*)&rt, sizeof(ssize_t),0);
		return -1;
	}
	//sed back the size
	send(sfd,(void*)&rt, sizeof(ssize_t),0);
	free(bf);
	bf = NULL;
	return 0;
}

//close the in using file
//one argument 1. a netfd
//return 0 on success, if fail, any errno will be senmt to the client
int sclose(int sfd){
	//tell the client to send the fd he/she wants to close
	int a = 0;
	send(sfd, (void*)&a,sizeof(int),0);
	//try to recv the nfd
	int nfd;
	recv(sfd, (void*)&nfd,sizeof(int),0);
	//first decode the nfd to a maped fd
	int mfd = fddecode(nfd);
	//chekc if the mfd is valid
	if(mfd>=asize){
		int f = -1;
		send(sfd, (void*)&f,sizeof(int),0);
		return -1;
	}
	//map the maped fd to find the fd stored in the struct
	fds* t = (fds*)malloc(sizeof(fds)); 
	t = &(fda[mfd]);
	//check if this file has already been closed
	if(t->stat==0){
		int ff = -1;
		send(sfd, (void*)&ff,sizeof(int),0);
		return -1;
	}
	//take this fd off the in using struct
	free(t->pt);
	t->pt = NULL;
	t->stat = 0;
	t->mode = 0;
	close(t->fd);
	acount--;
	//reply the client that the file is closed
	int s = 0;
	send(sfd,(void*)&s, sizeof(int),0);
	return 0;
}


//process that execute for the client thread
//one argument 1.the sockfd pointer
//renturn a void*
void *process(void* sockptr){
	//convert the void ptr to the socket fd
	int csfd = *(int*)sockptr;
	//check what action dose the client want to do
	//acn is a action code
	//action code
	//1 open
	//2 read
	//3 write
	//4 close
	int ac;
	int ind0;
	//try to recv action code
	ind0 = recv(csfd, (void*)&ac, sizeof(int),0);
	//check if the recv is success
	//yes, let corresponding function handle it
	//no, send back the errno and stop
	if(ind0==-1){
		send(csfd, (void*)&errno,sizeof(int),0);
	}else{
		if(ac==1){
			sopen(csfd);
		}else if(ac==2){
			sread(csfd);
		}else if(ac==3){
			swrite(csfd);
		}else if(ac==4){
			sclose(csfd);
		}
	}
	//close the socket and terminate the thread
	close(csfd);
	pthread_exit(NULL);
	return 0;
}

//the main function
int main(int argc, char** argv){
	//allocate memory for the fda array
	fda = (fds*)malloc(10*sizeof(fds));
	//initialize pointer counter and size
	aptr = 0;
	acount = 0;
	asize = 10;
	//create the server socket
	int ssocketfd;
	ssocketfd = socket(AF_INET, SOCK_STREAM,0);
	//check if the socket is opend successfully
	if(ssocketfd==-1){
		printf("ERROR:%s\n", strerror(errno));
		return -1;
	}else{
		printf("Socket created\n");
	}
	//create the sockaddr_in struct for binding
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(pnum);
	//bind the ip address to the ssocketfd
	int ind0 = bind(ssocketfd, (struct sockaddr*)&server, sizeof(server));
	//check if bind is success
	if(ind0==-1){
		printf("ERROR:%s\n", strerror(errno));
		return -1;
	}else{
		printf("Successfully binded\n");
	}
	//let the socket to listen(the max of the listening queue is set to 12)
	int ind1 = listen(ssocketfd,12);
	//check if socket is set to listen
	if(ind1==-1){
		printf("ERROR:%s\n", strerror(errno));
		return -1;
	}else{
		printf("Listening on port %i\n", pnum);
	}
	//Waiting client connections
	printf("Waiting for client connections...\n");
	while(1){
		//create socket for the client
		int csocketfd;
		struct sockaddr_in client;
		//accept client connections
		int s = sizeof(client);
		csocketfd = accept(ssocketfd, (struct sockaddr*)&client, (socklen_t*)&s);
		//check if the acception is successful
		if(csocketfd==-1){
			printf("ERROR:%s\n", strerror(errno));
		}else{
			printf("Accepted a client\n");
		}
		//create a thread for client
		pthread_t td;
		int* ptrcsocket = &csocketfd;
		int ind2 = pthread_create(&td, NULL, process, (void*)ptrcsocket);
		if(ind2!=0){
			printf("ERROR:%s\n", strerror(ind2));
			return -1;
		}else{
			printf("Client thread created\n");
		}
	}
	return 0;
}