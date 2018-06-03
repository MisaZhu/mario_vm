
fd = FS.open("Makefile", FS.RDONLY);

b = new Bytes(100);
FS.read(fd, b, 100);

Debug.outln(b.toString());

FS.close(fd);
