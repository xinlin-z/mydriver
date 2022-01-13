#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <assert.h>
#include "mychar.h"


#define ERRBUF_SIZE 32
#define _PERROR \
    do { \
        char errbuf[ERRBUF_SIZE] = {0}; \
        sprintf(errbuf, "%s:%d", __FILE__, __LINE__); \
        perror(errbuf); \
    } while(0) \


int main(int argc, char **argv) {
    int fd;
    unsigned long rtv;

    assert(argc == 2);

    if ((fd=open(argv[1],O_RDONLY)) == -1) {
        _PERROR;
        return errno;
    }
    assert(ioctl(fd, MYCHAR_IOC_QUERY2) == 1024);

    ioctl(fd, MYCHAR_IOC_RESET);
    assert(ioctl(fd, MYCHAR_IOC_QUERY2) == 4096);
    if (ioctl(fd, MYCHAR_IOC_QUERY, &rtv) == -1) {
        _PERROR;
        return errno;
    }
    assert(rtv == 4096);

    rtv = 1234;
    if (ioctl(fd, MYCHAR_IOC_SET, &rtv) == -1) {
        _PERROR;
        return errno;
    }
    assert(ioctl(fd, MYCHAR_IOC_QUERY2) == 1234);

    rtv = 2345;
    if (ioctl(fd, MYCHAR_IOC_QNS, &rtv) == -1) {
        _PERROR;
        return errno;
    }
    assert(rtv == 1234);
    assert(ioctl(fd, MYCHAR_IOC_QUERY2) == 2345);

    if (ioctl(fd, MYCHAR_IOC_CLS) == -1) {
        _PERROR;
        return errno;
    }
    assert(ioctl(fd, MYCHAR_IOC_QUERY2) == 0);
    
    rtv = 1234;
    if (ioctl(fd, MYCHAR_IOC_SET, &rtv) == -1) {
        _PERROR;
        return errno;
    }
    assert(ioctl(fd, MYCHAR_IOC_QUERY2) == 1234);

    rtv = 2345;
    if (ioctl(fd, MYCHAR_IOC_CNS, &rtv) == -1) {
        _PERROR;
        return errno;
    }
    assert(ioctl(fd, MYCHAR_IOC_QUERY2) == 2345);

    rtv = 1024;
    if (ioctl(fd, MYCHAR_IOC_SET, &rtv) == -1) {
        _PERROR;
        return errno;
    }
    assert(ioctl(fd, MYCHAR_IOC_QUERY2) == 1024);

    printf("all ioctl success!\n");
    return 0;
}
