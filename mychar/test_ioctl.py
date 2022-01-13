from fcntl import ioctl
import sys
from sys import byteorder
from ctypes import c_long, sizeof


param_len = sizeof(c_long)
MYCHAR_IOC_RESET = 0x5A00
MYCHAR_IOC_QUERY = 0x80085A01
MYCHAR_IOC_SET = 0x40085A02
MYCHAR_IOC_QNS = 0xC0085A03
MYCHAR_IOC_QUERY2 = 0x80085A05
MYCHAR_IOC_CLS = 0x5A06
MYCHAR_IOC_CNS = 0x40085A07
MYCHAR_IOC_READ = 0x80485A04


f = open(sys.argv[1])
assert ioctl(f, MYCHAR_IOC_QUERY2) == 1024
ioctl(f, MYCHAR_IOC_RESET)
assert ioctl(f, MYCHAR_IOC_QUERY2) == 4096
a = bytearray((param_len))
ioctl(f, MYCHAR_IOC_QUERY, a)
assert int.from_bytes(a, byteorder) == 4096
a = bytearray(int.to_bytes(1234,param_len,byteorder))
ioctl(f, MYCHAR_IOC_SET, a)
assert ioctl(f, MYCHAR_IOC_QUERY2) == 1234
a = bytearray(int.to_bytes(2345,param_len,byteorder))
ioctl(f, MYCHAR_IOC_QNS, a)
assert int.from_bytes(a, byteorder) == 1234
assert ioctl(f, MYCHAR_IOC_QUERY2) == 2345
ioctl(f, MYCHAR_IOC_CLS)
assert ioctl(f, MYCHAR_IOC_QUERY2) == 0
a = bytearray(int.to_bytes(1234,param_len,byteorder))
ioctl(f, MYCHAR_IOC_SET, a)
assert ioctl(f, MYCHAR_IOC_QUERY2) == 1234
a = bytearray(int.to_bytes(2345,param_len,byteorder))
ioctl(f, MYCHAR_IOC_CNS, a)
assert ioctl(f, MYCHAR_IOC_QUERY2) == 2345
a = bytearray(int.to_bytes(1024,param_len,byteorder))
ioctl(f, MYCHAR_IOC_SET, a)
assert ioctl(f, MYCHAR_IOC_QUERY2) == 1024 


import os
os.system('echo "123412341234" > '+sys.argv[1])
a = int.to_bytes(4, 8, byteorder)
b = bytearray(a + bytes((64)))
print(ioctl(f, MYCHAR_IOC_READ, b))
print(b)
