import sys
import os
import time
import random

maildir_path = "./maildir"
mailboxes = ["user@postman.local", "people@yandex.ru", "client@gmail.com"]
other_servers = ".OTHER_SERVERS"
selv_server = "postman.local"

mail_body = ""



def make_root_dir():
	try:
		os.mkdir(maildir_path)
	except:
		pass

	try:
		os.mkdir(os.path.join(maildir_path, other_servers))
	except:
		pass

def name_generate(sender):
	timestamp = str(int(time.time()))
	pid = str(os.getpid())
	rand = str(int(random.random() * 99999))
	return timestamp + sender + pid + rand 

def send_mail(sender, reciver):
	user, server = reciver.split("@", 1)
	message_path = ""
	if server == selv_server:
		message_path = os.path.join(maildir_path, user)
	else:
		message_path = os.path.join(maildir_path, other_servers, server, user)
	try:
		os.makedirs(message_path)
	except:
		pass
	filename = name_generate(sender)
	message_path = os.path.join(message_path, filename)
	f = open(message_path, "w")
	f.write(mail_body)

if __name__ == "__main__":
	make_root_dir()
	f = open('mail.txt', 'r')
	for line in f:
		mail_body = mail_body + line

	for mb_sender in mailboxes:
		for md_reciver in mailboxes:
			send_mail(mb_sender, md_reciver)
