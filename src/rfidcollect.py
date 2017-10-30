#!/usr/bin/python

import socket
import time

HOST = 'localhost'
PORT = 6000
recv = chr(0xcc)+chr(0x14)+chr(0xa0)
pre = chr(0xcc)+chr(0xa4)+chr(0xa0)

clisock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#clisock.settimeout(3)
clisock.connect((HOST, PORT))

clisock.send(pre)
data = clisock.recv(16)
if data == 'success':
	time.sleep(2)
	clisock.send(recv)
	source = clisock.recv(1024*4)
	print source
else:
	print 'unknown error'

clisock.close
	
