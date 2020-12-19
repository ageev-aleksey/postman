import os
import subprocess as sub
import signal
import sys
import time
import socket

import test_suite
import test_runner
import config

def usage():
	print("{} <path to server app>".format(sys.argv[0]))


class Test (test_suite.TestSuite):
	def __init__(self):
		test_suite.TestSuite.__init__(self, "test", config.server_config, ["../../bin/server.out"], 5)
	def test_hello(self):
		s = socket.socket(socket.AF_INET, socket.SOCK_STREAM);
		s.connect((self.config["host"], self.config["port"]));
		buf = s.recv(100)
		print("recv:", buf)

		s.send(b"ehlo: [127.0.0.1]\r\n")
		buf = s.recv(100)
		print("recv:", buf)

		s.send(b"mail from: <test@test.server.ru>\r\n")
		buf = s.recv(100)
		print("recv:", buf)

		s.send("rcpt to: <client@{}>\r\n".format(self.config["domain"]).encode("utf-8"))
		buf = s.recv(100)
		print("recv:", buf)

		s.send("rcpt to: <client2@{}>\r\n".format(self.config["domain"]).encode("utf-8"))
		buf = s.recv(100)
		print("recv:", buf)

		s.send("rcpt to: <client3@{}>\r\n".format(self.config["domain"]).encode("utf-8"))
		buf = s.recv(100)
		print("recv:", buf)

		s.send("rcpt to: <client4@{}>\r\n".format(self.config["domain"]).encode("utf-8"))
		buf = s.recv(100)
		print("recv:", buf)

		s.send("rcpt to: <client5@{}>\r\n".format(self.config["domain"]).encode("utf-8"))
		buf = s.recv(100)
		print("recv:", buf)

		s.send(b"data\r\n")
		buf = s.recv(100)
		print("recv:", buf)

		s.send(("test message\r\n melcome to the server \r\n"*100).encode("utf-8"))
		s.send(b".\r\n")
		buf = s.recv(100)
		print("recv:", buf)

		s.send(b"quit\r\n")
		buf = s.recv(100)
		print("recv:", buf)

if __name__ == "__main__":
	# if len(sys.argv) == 1:
	# 	usage()
	# 	sys.exit(1)
	# else:
	# 	server_exe =  sys.argv[1]
	# 	server_app = sub.Popen([server_exe])
	# 	time.sleep(10)
	# 	os.kill(server_app.pid, signal.SIGINT)
	r = test_runner.TestRunner()
	r.add(Test())
	r.run()

