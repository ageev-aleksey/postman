import socket
import sys

NEW_LINE = b"\r\n"
END_PACKAGE = NEW_LINE + b'.' + NEW_LINE
ADDR = ("127.0.0.1", 8080)

CLIENT1_FIRST_PACKAGE = b"client1"
CLIENT1_SECOND_PACKAGE_PART1 = b"00000000"
CLIENT1_SECOND_PACKAGE_PART2 = b"111111111"
CLIENT1_SECOND_PACKAGE_PART3 = b"0000000"

CLIENT2_FIRST_PACKAGE_PART1 = b"client2"
CLIENT2_FIRST_PACKAGE_PART2 = b"222222222"
CLIENT2_SECOND_PACKAGE = 1000*"client2_second_package*"

def create_socket():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(0.5)
    return s

print("Started testing for service two clients")

s1 = create_socket()
s2 = create_socket()

s1.connect(ADDR)
s1.send(CLIENT1_FIRST_PACKAGE + END_PACKAGE + CLIENT1_SECOND_PACKAGE_PART1)
s2.connect(ADDR)
res = s1.recv(len(CLIENT1_FIRST_PACKAGE))
if res != CLIENT1_FIRST_PACKAGE:
    print(" - Error recv first package of socket 1")
    sys.exit(-1)

s1.send(CLIENT1_SECOND_PACKAGE_PART2)
s2.send(CLIENT2_FIRST_PACKAGE_PART1)
s1.send(CLIENT1_SECOND_PACKAGE_PART3 + END_PACKAGE)
s2.send(CLIENT2_FIRST_PACKAGE_PART2 + END_PACKAGE)
res = s1.recv(len(CLIENT1_SECOND_PACKAGE_PART1) + len(CLIENT1_SECOND_PACKAGE_PART2) + len(CLIENT1_SECOND_PACKAGE_PART3))
if res != CLIENT1_SECOND_PACKAGE_PART1 + CLIENT1_SECOND_PACKAGE_PART2 + CLIENT1_SECOND_PACKAGE_PART3:
    print(" - Error recv second package of socket 1")
    sys.exit(-1)
res = s2.recv(len(CLIENT2_FIRST_PACKAGE_PART1) + len(CLIENT2_FIRST_PACKAGE_PART2))
if res != CLIENT2_FIRST_PACKAGE_PART1 + CLIENT2_FIRST_PACKAGE_PART2:
    print(" - Error recv  package of socket 2")
    sys.exit(-1)

s1.close()

s2.send(CLIENT2_SECOND_PACKAGE + END_PACKAGE);
res = s2.recv(len(CLIENT2_SECOND_PACKAGE))
if res != CLIENT2_SECOND_PACKAGE:
    print(" - Error recv  second  of socket 2")
    sys.exit(-1)


print(" - OK")
sys.exit(0)
