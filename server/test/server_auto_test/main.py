import os
import subprocess as sub
import signal
import sys
import time

def usage():
	print("{} <path to server app>".format(sys.argv[0]))

if __name__ == "__main__":
	if len(sys.argv) == 1:
		usage()
		sys.exit(1)
	else:
		server_exe =  sys.argv[1]
		server_app = sub.Popen([server_exe])
		time.sleep(10)
		os.kill(server_app.pid, signal.SIGINT)
