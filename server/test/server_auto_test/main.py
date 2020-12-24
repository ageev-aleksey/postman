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
import maildir

IS_MANUAL = False
SERVER_PATH = "../../bin/server.out"
SERVER_RESPONSE_PATTERN = "([0-9]{3}) (.*)\r\n$"
RUN_APP = ["valgrind", "--leak-check=full", "--show-leak-kinds=all", SERVER_PATH]

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
    s.send(b"ehlo mx.yandex.ru\r\n")
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


def check_x_headers(actual, expected):
    for key in expected:
        if actual[key] != expected[key]:
            raise test_runner.AssertException(f"invalid x_headers: expected -> {expected}; actual -> {actual}")
    return True


def response_check(sock, code_response):
    buf = sock.recv(100)
    response = server_response_parse(buf)
    test_runner.assert_equal(response[0], code_response)


class Test(test_suite.ServerTestSuite):
    def __init__(self):
        test_suite.ServerTestSuite.__init__(self, "test", config.server_config, RUN_APP, 5, IS_MANUAL)

    def connect(self):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((self.config["host"], self.config["port"]))
        buf = s.recv(100)
        response = server_response_parse(buf)
        test_runner.assert_equal(response[0], SMTP_CODE_START_SMTP_SERVICE)
        return s

    def test_empty_mail(self):
        md = maildir.Maildir(self.config["maildir_path"])
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.connect((self.config["host"], self.config["port"]))
            buf = s.recv(100)
            response = server_response_parse(buf)
            test_runner.assert_equal(response[0], SMTP_CODE_START_SMTP_SERVICE)

            smtp_transaction(s, "test@test.server.ru", [f"user@{self.config['domain']}", f"client@{self.config['domain']}",
                                                        "user@other.server", "client@other.server",
                                                        "client@good.server.ru", "foo@good.server.ru"])

            s.send(b".\r\n")
            buf = s.recv(100)
            response = server_response_parse(buf)
            test_runner.assert_equal(response[0], SMTP_CODE_OK)

            s.send(b"quit\r\n")
            buf = s.recv(100)
            response = server_response_parse(buf)
            test_runner.assert_equal(response[0], SMTP_CODE_CLOSE_CONNECTION)

            user = md.getUser("user")
            self.check_one_mail("test@test.server.ru", user, "",
                                {"X-Postman-From": "test@test.server.ru", "X-Postman-To": [f"user@{self.config['domain']}"]})

            user = md.getUser("client")
            self.check_one_mail("test@test.server.ru", user, "",
                                {"X-Postman-From": "test@test.server.ru", "X-Postman-To": [f"client@{self.config['domain']}"]})

            servers = md.servers
            if len(servers) != 2:
                raise AssertionError("Invalid number of outer servers")
            for s in servers:
                if s.domain == "other.server":
                    self.check_one_mail("test@test.server.ru", s, "",
                                        {"X-Postman-From": "test@test.server.ru",
                                         "X-Postman-To": ["user@other.server", "client@other.server"]})

                elif s.domain == "good.server.ru":
                    self.check_one_mail("test@test.server.ru", s, "",
                                        {"X-Postman-From": "test@test.server.ru",
                                         "X-Postman-To": ["client@good.server.ru", "foo@good.server.ru"]})
                else:
                    raise AssertionError(f"Invalid server name [{s.domain}]")

        finally:
            md.clear()

    def test_big_line(self):
        md = maildir.Maildir(self.config["maildir_path"])
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.connect((self.config["host"], self.config["port"]))
            buf = s.recv(100)
            response = server_response_parse(buf)
            test_runner.assert_equal(response[0], SMTP_CODE_START_SMTP_SERVICE)

            smtp_transaction(s, "test@test.server.ru",
                             [f"user@{self.config['domain']}", f"client@{self.config['domain']}",
                              "user@other.server", "client@other.server",
                              "client@good.server.ru", "foo@good.server.ru"])
            mbody = ""
            s.send(("test"*1000 + "\r\n").encode("utf-8"))
            mbody = mbody + "test"*1000 + "\r\n"
            time.sleep(1)
            s.send(("test" * 1000 + "\r\n").encode("utf-8"))
            mbody = mbody + "test" * 1000 + "\r\n"
            time.sleep(1)
            s.send(("test" * 1000 + "\r\n").encode("utf-8"))
            mbody = mbody + "test" * 1000 + "\r\n"
            s.send((("test" * 100 + "\r\n")*100).encode("utf-8"))
            mbody = mbody + ("test" * 100 + "\r\n")*100 + "\r\n"

            s.send(b".\r\n")
            buf = s.recv(100)
            response = server_response_parse(buf)
            test_runner.assert_equal(response[0], SMTP_CODE_OK)
            s.send(b"quit\r\n")
            buf = s.recv(100)
            response = server_response_parse(buf)
            test_runner.assert_equal(response[0], SMTP_CODE_CLOSE_CONNECTION)

            # user = md.getUser("user")
            # self.check_one_mail("test@test.server.ru", user, mbody,
            #                     {"X-Postman-From": "test@test.server.ru",
            #                      "X-Postman-To": [f"user@{self.config['domain']}"]})
            #
            # user = md.getUser("client")
            # self.check_one_mail("test@test.server.ru", user, mbody,
            #                     {"X-Postman-From": "test@test.server.ru",
            #                      "X-Postman-To": [f"client@{self.config['domain']}"]})
            #
            # servers = md.servers
            # if len(servers) != 2:
            #     raise AssertionError("Invalid number of outer servers")
            # for s in servers:
            #     if s.domain == "other.server":
            #         self.check_one_mail("test@test.server.ru", s, mbody,
            #                             {"X-Postman-From": "test@test.server.ru",
            #                              "X-Postman-To": ["user@other.server", "client@other.server"]})
            #
            #     elif s.domain == "good.server.ru":
            #         self.check_one_mail("test@test.server.ru", s, mbody,
            #                             {"X-Postman-From": "test@test.server.ru",
            #                              "X-Postman-To": ["client@good.server.ru", "foo@good.server.ru"]})
            #     else:
            #         raise AssertionError(f"Invalid server name [{s.domain}]")

        finally:
            md.clear()

    # def test_timer_to_auto_disconnect(self):
    #     s = self.connect()
    #     s.sleep(30)

    def check_one_mail(self, SENDER, client, mail_body, x_headers):
        mails = client.mails
        if len(mails) == 1:
            check_x_headers(mails[0].x_headers, x_headers)
            test_runner.assert_equal(mails[0].body, mail_body)
        else:
            raise test_runner.AssertException(
                f"invalid number mail file in other servers path; expected [1]; actual [{len(mails)}];")


    def test_rset(self):
        s = self.connect()
        s.send(b"helo domain.com\r\n")
        s.send(b"mail from: <test@test.ru>\r\n")
        response_check(s, SMTP_CODE_OK)
        s.send(b"rset\r\n")
        response_check(s, SMTP_CODE_OK)
        s.send(b"rcpt to: <test@test.ru>\r\n")
        response_check(s, SMTP_CODE_INVALID_SEQUENCE)

        s.send(b"helo [127.0.0.1]\r\n")
        response_check(s, SMTP_CODE_OK)
        s.send(b"mail from: <test@test.ru>\r\n")
        response_check(s, SMTP_CODE_OK)
        s.send(b"rcpt to: <client@test.ru>\r\n")
        response_check(s, SMTP_CODE_OK)
        s.send(b"rset\r\n")
        response_check(s, SMTP_CODE_OK)
        s.send(b"rcpt to: <client2@test.ru>\r\n")
        response_check(s, SMTP_CODE_INVALID_SEQUENCE)

        s.send(b"helo [127.0.0.1]\r\n")
        response_check(s, SMTP_CODE_OK)
        s.send(b"mail from: <test@test.ru>\r\n")
        response_check(s, SMTP_CODE_OK)
        s.send(b"rcpt to: <client@test.ru>\r\n")
        response_check(s, SMTP_CODE_OK)
        s.send(b"rcpt to: <client2@test.ru>\r\n")
        response_check(s, SMTP_CODE_OK)
        s.send(b"rset\r\n")
        response_check(s, SMTP_CODE_OK)

        s.send(b"helo [127.0.0.1]\r\n")
        response_check(s, SMTP_CODE_OK)
        s.send(b"mail from: <test@test.ru>\r\n")
        response_check(s, SMTP_CODE_OK)
        s.send(b"rcpt to: <client@test.ru>\r\n")
        response_check(s, SMTP_CODE_OK)
        s.send(b"rcpt to: <client2@test.ru>\r\n")
        response_check(s, SMTP_CODE_OK)
        s.send(b"quit\r\n")
        response_check(s, SMTP_CODE_CLOSE_CONNECTION)
        s.close()


    def test_noop(self):
        s = self.connect()
        s.send(b"noop\r\n")
        response_check(s, SMTP_CODE_OK)

        s.send(b"helo [127.0.0.1]\r\n")
        response_check(s, SMTP_CODE_OK)
        s.send(b"noop\r\n")
        response_check(s, SMTP_CODE_OK)
        s.send(b"mail from: <test@test.ru>\r\n")
        response_check(s, SMTP_CODE_OK)
        s.send(b"noop\r\n")
        response_check(s, SMTP_CODE_OK)
        s.send(b"rcpt to: <client@test.ru>\r\n")
        response_check(s, SMTP_CODE_OK)
        s.send(b"noop\r\n")
        response_check(s, SMTP_CODE_OK)
        s.send(b"rcpt to: <client2@test.ru>\r\n")
        response_check(s, SMTP_CODE_OK)
        s.send(b"noop\r\n")
        response_check(s, SMTP_CODE_OK)
        s.send(b"quit\r\n")
        response_check(s, SMTP_CODE_CLOSE_CONNECTION)

    @test_suite.skip
    def test_invalid_sequence(self):
        pass

    @test_suite.skip
    def test_batch_of_commands(self):
        s = self.connect()
        s.send(b"helo: [127.0.0.1]\r\nmail form: <test@test.ru>\r\nrcpt to: <aaa@ya.ru>\r\n")
        buf = s.recv(1000)


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
    print(f"Passed Tests [{len(r.passed_tests)}]:")
    for t in r.passed_tests:
        print("    - " + t)

    print(f"Failed Tests [{len(r.failed_tests)}]:")
    for t in r.failed_tests:
        print("    - " + t)

    if r.have_failed:
        sys.exit(1)
    else:
        sys.exit(0)
