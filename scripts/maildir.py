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
	rand = str(int(random.random() * 999999999))
	return timestamp + "_" + rand 

def make_x_header(sender, recivers):
	"""
	X-Postman-From: <domain>
	X-Postman-Date: <timestamp>
	X-Postman-To: <domain> [, <domain> [...]]
	"""
	x_header = "X-Postman-From: " + sender + "\r\n"
	x_header = x_header +  "X-Postman-Date: " + str(time.time()) + "\r\n"
	x_header = x_header +  "X-Postman-To: "
	for i in range(0, len(recivers)-1):
		x_header = x_header + recivers[i] + ","
	x_header = x_header + recivers[-1] + "\r\n"
	return x_header

def send_outer_mail(sender, recivers):
	"""
	sender - email отправителя
	recivers - спиосок email'ов получателей
	"""
	message_path = ""
	mail = mail_body
	message_path = os.path.join(maildir_path, other_servers, "new")
	mail = make_x_header(sender, recivers) + "\r\n" + mail
	try:
		os.makedirs(message_path)
	except:
		pass
	filename = name_generate(sender)
	message_path = os.path.join(message_path, filename)
	f = open(message_path, "w")
	f.write(mail)
	f.close()

def send_local_mail(sender, recivers):
	"""
	sender - email отправителя
	recivers - спиосок email'ов получателей
	"""
	for reciver in recivers:
		user, server = reciver.split("@", 2)
		mail = mail_body
		message_path = os.path.join(maildir_path, user, "new")
		try:
			os.makedirs(message_path)
		except:
			pass

		try:
			os.makedirs(os.path.join(maildir_path, user, "tmp"))
		except:
			pass

		try:
			os.makedirs(os.path.join(maildir_path, user, "cur"))
		except:
			pass

		filename = name_generate(sender)
		message_path = os.path.join(message_path, filename)
		f = open(message_path, "w")
		f.write(mail)
		f.close()



if __name__ == "__main__":
	make_root_dir()
	f = open('mail.txt', 'r')
	for line in f:
		mail_body = mail_body + line

	outer_mailboxes = []
	inner_mailboxes = []

	for m in mailboxes:
		user, server = m.split("@", 2)
		if server == selv_server:
			inner_mailboxes.append(m)
		else:
			outer_mailboxes.append(m)

	print(outer_mailboxes)
	print(inner_mailboxes)

	for mb_sender in mailboxes:
		send_outer_mail(mb_sender, outer_mailboxes)

	for mb_sender in mailboxes:
		send_local_mail(mb_sender, inner_mailboxes)

