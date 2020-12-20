import os
from enum import Enum

OTHER_SERVERS = ".OTHER_SERVERS"
xPostmanTo = "X-Postman-To"

class InvalidFileStructure(Exception):
	def __init__(self, what):
		self._what = what
	@property
	def what(self):
		return self._what

	def __str__(self):
		return self._what

class MailType(Enum):
	CUR = 'cur'
	NEW = 'new'
	TMP = 'tmp'


class Mail:
	def __init__(self, md, **kwargs):
		"""
		:param md: MailDir
		:param kwargs:
			- user, type, filename: (User) пользователь, которому принадлежит письмо;
									(MailType) тип письма
									(str) имя файла с письмо
			- file_path: (str) полный путь до файла с письмом
		"""
		self._md = md
		self._x_headers = {}
		self._body = ""
		self._user = None
		if "user" in kwargs.keys() and "type" in  kwargs.keys() and "filename" in kwargs.keys():
			self._user = kwargs["user"];
			self._file_path = os.path.join(kwargs["user"].path, kwargs["type"], kwargs["filename"])
		elif "file_path" in kwargs.keys():
			self._file_path = kwargs["file_path"]
		self._parse()

	@property
	def path(self):
		return self._file_path

	def _parse(self):
		f = open(self.path, "r")
		is_header = True
		for line in f.readlines():
			if is_header:
				if line == '\r\n' or line == '\n':
					is_header = False
				else:
					s = line.split(":")
					if len(s) != 2:
						raise InvalidFileStructure(f"invalid x_header in file [{self._file_path}]: " + str(s))
					if s[0] == xPostmanTo:
						self._x_headers[s[0].strip()] = s[1].strip().split(',')
					else:
						self._x_headers[s[0].strip()] = s[1].strip()
			else:
				self._body = self._body + line
		f.close()

	@property
	def x_headers(self):
		return self._x_headers

	@property
	def body(self):
		return self._body


class User:
	def __init__(self, md, user_name):
		self._md = md
		self._name = user_name

	@property
	def path(self):
		return os.path.join(self._md.path, self._name)

	@property
	def mails(self):
		current_dir_n, dirs_n, files_n = next(os.walk(os.path.join(self._md.path, self._name, "new")))
		current_dir_c, dirs_c, files_c = next(os.walk(os.path.join(self._md.path, self._name, "cur")))
		mails_list = []
		for file_name in files_n:
			mails_list.append(Mail(self._md, file_path=os.path.join(current_dir_n, file_name)))
		for file_name in files_c:
			mails_list.append(Mail( self._md, file_path=os.path.join(current_dir_n, file_name)))
		return mails_list


	def __str__(self):
		return "User{" + self._name + "}"



class Maildir:
	def __init__(self, root):
		self._root = root;

	@property
	def users(self):
		current_dir, dirs, files = next(os.walk(self._root))
		users = []
		for d in dirs:
			if d != OTHER_SERVERS:
				users.append(User(self, d))
		return users

	def getUser(self, username):
		current_dir, dirs, files = next(os.walk(self._root))
		for d in dirs:
			if d == username:
				return User(self, d)
		return None

	@property
	def outer_mails(self):
		current_dir, dirs, files = next(os.walk(os.path.join(self._root, ".OTHER_SERVERS", "new")))
		mails = []
		for fn in files:
			mails.append(Mail(self, file_path=os.path.join(current_dir, fn)))
		return mails

	@property
	def path(self):
		return self._root

	def clear(self):
		"""Метод очистки maildir от писем"""
		root, dirs, files = next(os.walk(self._root))
		for d in dirs:
			if d != OTHER_SERVERS:
				user_new_dir, user_new_dirs, user_new_files = next(os.walk(os.path.join(root, d, MailType.NEW.value)))
				user_cur_dir, user_cur_dirs, user_cur_files = next(os.walk(os.path.join(root, d, MailType.CUR.value)))
				for f in user_new_files:
					os.remove(os.path.join(user_new_dir, f))
				for f in user_cur_files:
					os.remove(os.path.join(user_cur_dir, f))
		outer_root, outer_dirs, outer_files = next(os.walk(os.path.join(self._root, OTHER_SERVERS, MailType.NEW.value)))
		for f in outer_files:
			os.remove(os.path.join(outer_root, f));


if __name__ == "__main__":
	root = "./maildir"
	md = Maildir(root)
	md.clear()
	# for m in md.outer_mails:
	# 	print(m.x_headers)
	# 	print(m.body)
	# users = md.users
	# for u in users:
	# 	for m in u.mails:
	# 		print(m.x_headers)
	# 		print(m.body)
