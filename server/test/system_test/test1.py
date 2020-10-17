import socket

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM);
s.connect(("127.0.0.1", 8080));
for i in range(1000000):
	s.send(b"Hello, World!");
	res = s.recv(15);
	print(res);