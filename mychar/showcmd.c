#include <stdio.h>
#include "mychar.h"


int main(void) {
    printf("MYCHAR_IOC_RESET:0x%X\n", MYCHAR_IOC_RESET);
    printf("MYCHAR_IOC_QUERY:0x%X\n", (unsigned int)MYCHAR_IOC_QUERY);
    printf("MYCHAR_IOC_SET:0x%X\n", (unsigned int)MYCHAR_IOC_SET);
    printf("MYCHAR_IOC_QNS:0x%X\n", (unsigned int)MYCHAR_IOC_QNS);
    printf("MYCHAR_IOC_READ:0x%X\n", (unsigned int)MYCHAR_IOC_READ);
    printf("MYCHAR_IOC_QUERY2:0x%X\n", (unsigned int)MYCHAR_IOC_QUERY2);
    printf("MYCHAR_IOC_CLS:0x%X\n", (unsigned int)MYCHAR_IOC_CLS);
    printf("MYCHAR_IOC_CNS:0x%X\n", (unsigned int)MYCHAR_IOC_CNS);
    printf("MYCHAR_IOC_AM:0x%X\n", (unsigned int)MYCHAR_IOC_AM);
    printf("MYCHAR_IOC_FALL:0x%X\n", (unsigned int)MYCHAR_IOC_FALL);
    return 0;
}
