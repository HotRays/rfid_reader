PREFIX =/usr/local/bin

rfid: opt.c
	gcc -o rfid opt.c -luv
install:
	install -d $(PREFIX)
	install -m 0755 rfid $(PREFIX)
	install -m 0755 rfidcollect.py $(PREFIX)
clean:
	rm rfid
