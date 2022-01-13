#ifndef MYCHAR_H
#define MYCHAR_H
#include <uapi/asm-generic/ioctl.h>

#define MYCHAR_IOC_READ_LEN 64

struct ioc_read {
    size_t skip;
    char content[MYCHAR_IOC_READ_LEN];
};

#define MYCHAR_IOC_MAGIC  'Z'  // do not use T
#define MYCHAR_IOC_RESET  _IO(MYCHAR_IOC_MAGIC, 0)
#define MYCHAR_IOC_QUERY  _IOR(MYCHAR_IOC_MAGIC, 1, unsigned long)
#define MYCHAR_IOC_SET    _IOW(MYCHAR_IOC_MAGIC, 2, unsigned long)
#define MYCHAR_IOC_QNS    _IOWR(MYCHAR_IOC_MAGIC, 3, unsigned long)
#define MYCHAR_IOC_READ   _IOR(MYCHAR_IOC_MAGIC, 4, struct ioc_read)
#define MYCHAR_IOC_QUERY2 _IOR(MYCHAR_IOC_MAGIC, 5, unsigned long)
#define MYCHAR_IOC_CLS    _IO(MYCHAR_IOC_MAGIC, 6)
#define MYCHAR_IOC_CNS    _IOW(MYCHAR_IOC_MAGIC, 7, unsigned long)
#define MYCHAR_IOC_AM     _IOW(MYCHAR_IOC_MAGIC, 8, unsigned long)
#define MYCHAR_IOC_FALL   _IO(MYCHAR_IOC_MAGIC, 9)
#define MYCHAR_IOC_MAXNR  9

#endif
