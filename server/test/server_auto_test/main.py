import os
import subprocess as sub
import signal
import sys
import time

import test_suite
import test_runner
import config

def usage():
	print("{} <path to server app>".format(sys.argv[0]))


class Test (test_suite.TestSuite):
	def __init__(self):
		test_suite.TestSuite.__init__(self, "test", config.server_config, ["../../bin/server.out"], 5)
	def test_hello(self):
		print("<")
		time.sleep(10)
		print(">")

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

