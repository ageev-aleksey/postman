import os
import subprocess as sub
import signal
import sys
import time
import socket
import re

import test_suite
import test_runner
import config

IS_MANUAL = False
SERVER_PATH = "../../bin/server.out"
SERVER_RESPONSE_PATTERN = "([0-9]{3}) (.*)\r\n$"

SMTP_CODE_START_SMTP_SERVICE      = 220
SMTP_CODE_CLOSE_CONNECTION        = 221
SMTP_CODE_OK                      = 250
SMTP_CODE_MAIL_INPUT              = 354
SMTP_CODE_ERROR_IN_PROCESSING     = 451
SMTP_CODE_SYNTAX_ERROR            = 500
SMTP_CODE_INVALID_ARGUMENT        = 501
SMTP_CODE_COMMAND_NOT_IMPLEMENTED = 502
SMTP_CODE_INVALID_SEQUENCE        = 503



def usage():
    print("{} <path to server app>".format(sys.argv[0]))


def server_response_parse(string):
    if isinstance(string, bytes):
        string = string.decode("utf-8")
    matcher = re.search(SERVER_RESPONSE_PATTERN, string)
    if matcher:
        g1 = matcher.group(1)
        g2 = matcher.group(2)
        return int(g1), g2


def smtp_transaction(s, sender, recipients):
    s.send(b"ehlo: [127.0.0.1]\r\n");
    buf = s.recv(100)
    response = server_response_parse(buf)
    test_runner.assert_equal(response[0], SMTP_CODE_OK)

    s.send(f"mail from: <{sender}>\r\n".encode("utf-8"))
    buf = s.recv(100)
    response = server_response_parse(buf)
    test_runner.assert_equal(response[0], SMTP_CODE_OK)

    for rcpt in recipients:
        s.send(f"rcpt to: <{rcpt}>\r\n".encode("utf-8"))
        buf = s.recv(100)
        response = server_response_parse(buf)
        test_runner.assert_equal(response[0], SMTP_CODE_OK)

    s.send(b"data\r\n")
    buf = s.recv(100)
    response = server_response_parse(buf)
    test_runner.assert_equal(response[0], SMTP_CODE_MAIL_INPUT)



class Test(test_suite.ServerTestSuite):
    def __init__(self):
        test_suite.ServerTestSuite.__init__(self, "test", config.server_config, [SERVER_PATH], 5, IS_MANUAL)

    def test_empty_mail(self):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((self.config["host"], self.config["port"]))
        buf = s.recv(100)
        response = server_response_parse(buf)
        test_runner.assert_equal(response[0], SMTP_CODE_START_SMTP_SERVICE)

        smtp_transaction(s, "test@test.server.ru", [f"user@{self.config['domain']}", "user@other.server"])

        s.send(b".\r\n")
        buf = s.recv(100)
        response = server_response_parse(buf)
        test_runner.assert_equal(response[0], SMTP_CODE_OK)

        s.send(b"quit\r\n")
        buf = s.recv(100)
        response = server_response_parse(buf)
        test_runner.assert_equal(response[0], SMTP_CODE_CLOSE_CONNECTION)

    def test_big_line(self):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((self.config["host"], self.config["port"]))
        buf = s.recv(100)
        response = server_response_parse(buf)
        test_runner.assert_equal(response[0], SMTP_CODE_START_SMTP_SERVICE)

        smtp_transaction(s, "test@test.server.ru", [f"user@{self.config['domain']}", "user@other.server"])

        s.send(b"H"*5000)
        time.sleep(1)
        s.send(b"H" * 5000)
        time.sleep(1)
        s.send(b"\r\n")

        s.send(b".\r\n")
        buf = s.recv(100)
        response = server_response_parse(buf)
        test_runner.assert_equal(response[0], SMTP_CODE_OK)

        s.send(b"quit\r\n")
        buf = s.recv(100)
        response = server_response_parse(buf)
        test_runner.assert_equal(response[0], SMTP_CODE_CLOSE_CONNECTION)


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
