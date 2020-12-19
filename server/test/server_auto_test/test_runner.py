import test_suite
import time
import re

class TestRunner:
	def __init__(self):
		self.tests = []
	
	def add(self, ts):
		if isinstance(ts, test_suite.TestSuite):
			self.tests.append(ts)
		else:
			raise TypeError("error type test_suite.TestSuite")
	
	def run(self):
		for test in self.tests:
			test.before()
			time.sleep(2)
			for att in dir(test):
				if re.fullmatch("test_.*", att):
					f = getattr(test, att);
					if callable(f):
						f()
			test.after()