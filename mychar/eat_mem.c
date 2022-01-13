#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <sys/ioctl.h>
#include "mychar.h"


#define ERRBUF_SIZE 32
#define _PERROR \
    do { \
        char errbuf[ERRBUF_SIZE] = {0}; \
        sprintf(errbuf, "%s:%d", __FILE__, __LINE__); \
        perror(errbuf); \
    } while(0) \


int main(int argc, char **argv) {
    int fd, cnt;
    unsigned long length = 0;
    assert(argc == 3);

    if ((fd=open(argv[1],O_RDONLY)) == -1) {
        _PERROR;
        return errno;
    }

    length = atoi(argv[2]);
    if ((cnt=ioctl(fd,MYCHAR_IOC_AM,&length)) == -1) {
        _PERROR;
        return errno;
    }

    fprintf(stdout, "%d", cnt);
    return 0;
}
