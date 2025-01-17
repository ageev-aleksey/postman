import test_suite
import time
import sys
import traceback
from multiprocessing import Pool


class AssertException(Exception):
    def __init__(self, what):
        self._what = what

    @property
    def what(self):
        return self._what

    def __str__(self):
        return "AssertException{" + self.what + "}"




def assert_equal(actual: object, expected: object) -> object:
    if actual != expected:
        raise AssertException(f"equal error: expected -> {expected}; actual -> {actual}")


class TestRunner:
    def __init__(self):
        self.tests = []
        self.passed = []
        self.failed = []

    def add(self, ts):
        if isinstance(ts, test_suite.ServerTestSuite):
            self.tests.append(ts)
        else:
            raise TypeError("error type test_suite.TestSuite")

    @property
    def passed_tests(self):
        return self.passed

    @property
    def failed_tests(self):
        return self.failed

    @property
    def have_failed(self):
        return (len(self.failed) != 0)

    def run(self):
        for test in self.tests:
            print("-==TestSuite: " + test.test_name + "==-")
            sys.stdout.flush()
            test.before()
            time.sleep(2)
            for name, test_case in test.test_cases.items():
                print("-------- " + name.upper() + " --------")
                sys.stdout.flush()
                is_passed = False
                try:
                    test_case()
                    is_passed = True
                except Exception as exp:
                    print("       Fail: " + str(exp))
                    sys.stdout.flush()
                    traceback.print_exc()
                    self.failed.append(name)
                if is_passed:
                    print("       Passed")
                    sys.stdout.flush()
                    self.passed.append(name)
            test.after()


# class ParallelTestRunner(TestRunner):
#     def __init__(self, max_process):
#         TestRunner.__init__(self)
#         self.max_process = max_process
#         self.proc_pool = Pool(self.max_process)
#
#     def run(self):
#         for test in self.tests:
#             print("-==ParallelTestSuite: " + test.test_name + "==-")
#             sys.stdout.flush()
#             test.before()
#             time.sleep(2)
#             test_cases = list(test.test_cases.keys())
#             self.proc_pool.map(executor, [test])
#             test.after()
#
# def executor(self, arg):
#     print(type(arg))

