import sys
import subprocess
from fcntl import ioctl


MYCHAR_IOC_FALL = 0x5A09


if sys.argv[2] == 'freeall':
    with open(sys.argv[1]) as f:
        ioctl(f, MYCHAR_IOC_FALL)
    sys.exit(0)


left = int(sys.argv[2])
eat = 0
piece = 1024*1024*100
while left > eat:
    cmd = f'./eat_mem {sys.argv[1]} {piece}'
    proc = subprocess.run(cmd.split(),
                          stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT)
    if proc.returncode != 0 and piece <= 4096:
        print('eat: %dM' % (eat/1024/1024))
        print('last eat_mem return code: ', proc.returncode)
        sys.exit(proc.returncode)
    elif proc.returncode != 0:
        piece //= 2
        print('try piece: %dB, %fM' % (piece,piece/1024/1024))
    else:
        eat += piece


print('eat: %dM' % (eat/1024/1024))
