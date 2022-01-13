import multiprocessing


s1 = '012345678901234\n'
s2 = 'abcdefghijklmno\n'
s3 = 'cs.pynote.net..\n'


def write_string(string):
    with open('mychar-3','wb') as f:
        a = 0
        while True:
            f.write(string.encode())
            f.flush()
            a += 1
            if (a == 63):
                a = 0
                f.seek(0)


def read_string():
    while True:
        with open('mychar-3') as f:
            lines = f.readlines()
            for line in lines:
                if not line in (s1, s2, s3):
                    print('read err')


th1 = multiprocessing.Process(target=write_string,
                       args=('012345678901234\n',), daemon=True)
th2 = multiprocessing.Process(target=write_string,
                       args=('abcdefghijklmno\n',), daemon=True)
th3 = multiprocessing.Process(target=write_string,
                       args=('cs.pynote.net..\n',), daemon=True)

th4 = multiprocessing.Process(target=read_string, args=(), daemon=True)

th1.start()
th2.start()
th3.start()
th4.start()
th1.join()
th2.join()
th3.join()
th4.join()

        
