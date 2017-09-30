rfid: src/rfid_reader.c src/rfid_reader.h src/rfid_list.c src/rfid_list.h src/communicate.c
	gcc -o rfid src/rfid_list.c src/rfid_reader.c src/communicate.c -luv
clean:
	rm rfid
