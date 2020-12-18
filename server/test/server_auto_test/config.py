

server_config = {
	"port": 8080,
	"host": "127.0.0.1",
	"domain": "postman.local",
	"maildir_path": "./maildir",
	"worker_threads": 4
}

def convert(el):
	if isinstance(el, str):
		return '"' + el + '"'
	else:
		return str(el)


def to_file(config, filename):
	if not isinstance(config, dict):
		raise TypeError("expected dict")
	buf = "server: {\n"
	for key, value in config.items():
		buf = buf + key + ': ' + convert(value) + '\n'
	buf = buf + "}"
	file = open(filename, "w")
	file.write(buf)
	file.close()
