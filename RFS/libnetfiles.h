#ifndef LIBNETFILES_H
#define LIBNETFILES_H
//define port number
#define pnum 9633
//define error code
#define INVALID_FILE_MODE -3
//define 3 file modes
#define unrestricted 1
#define exclusive 3
#define transaction 9
//define flag
#define O_RDONLY 2
#define O_WRONLY 4
#define O_RDWR 8
//herror
extern int h_errno;
//functions
int netserverinit(char* hostname, int filemode);
int netopen(const char* pathname, int flags);
ssize_t netread(int fildes, void *buf, size_t nbyte);
ssize_t netwrite(int fildes, void *buf, size_t nbyte);
int netclose(int fd);
#endif