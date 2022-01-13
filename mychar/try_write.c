#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>


#define ERRBUF_SIZE 32
#define _PERROR \
    do { \
        char errbuf[ERRBUF_SIZE] = {0}; \
        sprintf(errbuf, "%s:%d", __FILE__, __LINE__); \
        perror(errbuf); \
    } while(0) \


int main(int argc, char **argv) {
    int i, fd;
    ssize_t cnt;
    char *newline = "\n";

    /* disable the unused warning */
    assert(argc == 3);

    /* open */
    if ((fd=open(argv[1],O_WRONLY)) == -1) {
        _PERROR;
        return errno;
    }

    /* write loop */
    for (i=0; i<10; ++i) { 
        if ((cnt=write(fd,argv[2],strlen(argv[2]))) == -1) {
            _PERROR;
            return errno;
        }
        if ((cnt=write(fd,newline,1)) == -1) {
            _PERROR;
            return errno;
        }
    }

    /* close */
    if (close(fd) == -1) {
        _PERROR;
        return errno;
    }

    return 0;
}
