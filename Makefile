PREFIX =/usr/local/bin

rfid: src/opt.c
	gcc -o rfid src/opt.c -luv
install:
	install -d $(PREFIX)
	install -m 0755 rfid $(PREFIX)
	install -m 0755 src/rfidcollect.py $(PREFIX)
clean:
	rm rfid
