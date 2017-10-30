RFID READER SERVER AND CLIENT

#Installation

        make && [sudo] make install
        
#Installing Requirements and Dependencies

        make, gcc, python(2.7), libuv1 1.8.0
        
#Instructions for use

        server : rfid

        client : rfidcollect.py
        
        server protocol： tcp

        server bindport: 6000
		
	The server needs to be running all the time. if you want get rfid data run rfidcollect.py, the result as fallows:
	
	["e2-00-32-9c-19-37-87-b1-12-03-5e-1e","e3-00-32-96-19-31-87-c1-02-03-5e-1e".....,"e2-00-32-4b-19-a7-87-b1-12-03-5e-1e"]

	or empty array: []
	
	you will wait 2 seconds to get the data after run rfidcollect.py, and the data is fresh every time.
	
----------------------------------
