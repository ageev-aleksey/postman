import test_suite
import time
import sys
import traceback


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

    def add(self, ts):
        if isinstance(ts, test_suite.ServerTestSuite):
            self.tests.append(ts)
        else:
            raise TypeError("error type test_suite.TestSuite")

    def run(self):
        for test in self.tests:
            print("-==TestSuite: " + test.test_name + "==-")
            sys.stdout.flush()
            test.before()
            time.sleep(2)
            for name, test_case in test.test_cases.items():
                print(" -- " + name + ": ")
                sys.stdout.flush()
                is_passed = False
                try:
                    test_case()
                    is_passed = True
                except Exception as exp:
                    print("       Fail: " + str(exp))
                    sys.stdout.flush()
                    traceback.print_exc()
                if is_passed:
                    print("       Passed")
                    sys.stdout.flush()
            test.after()


class ParallelTestRunner(TestRunner):
    def __init__(self, max_threads):
        TestRunner.__init__(self)
        self.max_threads = max_threads

    def run(self):
        pass
