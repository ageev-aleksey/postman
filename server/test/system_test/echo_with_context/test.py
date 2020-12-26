import socket
import time
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM);
s.connect(("127.0.0.1", 8080));
s.send(b"Hello, World!\r\n");
s.send(b"FromPython\r\n");
#time.sleep(2);
s.send(b"End Message\r\n.\r\nThis is new message");
res = s.recv(150);
print(res);
#time.sleep(1);
s.send(b" which send in two steps with sleep\r\n.\r\n")
res = s.recv(150);
print(res);
