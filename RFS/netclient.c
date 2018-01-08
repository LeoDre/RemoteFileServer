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
#include "libnetfiles.h"

int main(int argc, char** argv){
	//resolve host
	char *hostname = "ls.cs.rutgers.edu";
	int ck  = netserverinit(hostname, unrestricted);
	if(ck==-1){
			printf("ERROR:%s\n", hstrerror(h_errno));
			return -1;
	}
	int c = netopen("./tf/a2/test4.txt", O_RDWR);
	if(c==-1){
		printf("ERROR:%s\n", strerror(errno));
		return -1;
	}else{
		printf("The fd is %i\n", c);
	}
	char* b = (char*)malloc(101);
	ssize_t s= netread(c,(void*)b,100);
	b[100] = '\0';
	if(s==-1){
		printf("ERROR:%s\n", strerror(errno));
		return -1;
	}else{
		printf("%s\n",b);
		printf("Read %zd byte\n",s);
	}
	char* d = (char*)malloc(101);
	ssize_t ss= netread(c,(void*)d,100);
	d[100] = '\0';
	if(ss==-1){
		printf("ERROR:%s\n", strerror(errno));
		return -1;
	}else{
		printf("%s\n",d);
		printf("Read %zd byte\n",ss);
	}
	ssize_t sss;
	char* st = (char*)malloc(24*sizeof(char));
	st = "wish you have a good day";
	sss = netwrite(c, (void*)st, 24*sizeof(char));
	if(sss==-1){
		printf("ERROR:%s\n", strerror(errno));
		return -1;
	}else{
		printf("Write %zd byte\n",sss);
	}
	/*int tst,tsst;
	for(tst = 0;tst<99999;tst++){
		for(tsst = 0;tsst<99999;tsst++){
			tsst++;
			--tsst;
		}
	}
	int ind0 = netclose(c);
	if(ind0==-1){
		printf("ERROR:%s\n", strerror(errno));
		return -1;
	}else{
		printf("The fd is closed\n");
	}*/
	return 0;
}