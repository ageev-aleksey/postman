import subprocess
import config
import os
import signal
import re



class ServerTestSuite:
	def __init__(self, test_name, config, application, timeout, server_manual=False):
		self.test_name = test_name
		self.application = application
		self.cfg = config
		self.cfg_name = "./app_config_test.cfg"
		self.application.append("--config")
		self.application.append(self.cfg_name)
		self.timeout = timeout
		self.is_manual = server_manual
		self.tests = {}
		for att in dir(self):
			if re.fullmatch("test_.*", att) and callable(getattr(self, att)):
				self.tests[att] = getattr(self, att)

	@property
	def test_cases(self):
		return self.tests

	@property
	def config(self):
		return self.cfg

	def before(self):
		print("before")
		config.to_file(self.cfg, self.cfg_name)
		print(self.application)
		if not self.is_manual:
			self.app = subprocess.Popen(self.application)
		return True

	def after(self):
		print("after")
		status = False
		if not self.is_manual:
			self.app.send_signal(signal.SIGTERM)
			try:
				ret = self.app.wait(self.timeout)
				status = (ret == 0)
			except subprocess.TimeoutExpired as exp:
				print(exp)
				self.app.kill()
				status = False
		os.remove(self.cfg_name)
		return status
