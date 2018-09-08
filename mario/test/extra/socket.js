
b = new Bytes(100);

fd = Socket.socket(Socket.TCP);
Socket.bind(fd, "", 8000);
Socket.listen(fd, 10);

while(true) {
	cfd = Socket.accept(fd);
	if(cfd < 0)
		break;

	while(true) {
		res = Socket.read(cfd, b, 100);
		if(res <= 0)
			break;
		Socket.write(cfd, b, res);
	}

	Socket.close(cfd);
}

Socket.close(fd);
