#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/socket.h>

#include <uv.h>

#define ARFI_NUM 128
#define ARFI_CHA 36
#define SERVER_PORT 6000
#define SERVER_ADDR "0.0.0.0"
#define SIZE 1024

typedef struct {
  uv_write_t req;
  uv_buf_t buf;
} write_req_t;

struct {
  unsigned char *base[ARFI_NUM];
  uint32_t cnt;
  bool lock; 
} recv_list;

void alloc_cb(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
	buf->base = (unsigned char *)malloc(SIZE);
	buf->len  = SIZE;
}

void uv_buf_free(uv_buf_t *buf)
{
	if ( buf && buf->base && buf->len )
	{
		free(buf->base);
	}
}

static void after_write(uv_write_t* req, int status) {
  write_req_t* wr;

  /* Free the read/write buffer and the request */
  wr = (write_req_t*) req;
  //free(wr->buf.base);
  free(wr);

  if (status == 0)
    return;

  fprintf(stderr,
          "uv_write error: %s - %s\n",
          uv_err_name(status),
          uv_strerror(status));
}

void rfid_tcp_read_cb(uv_stream_t *client, ssize_t n, const uv_buf_t *buf)
{
	write_req_t *wr;

	if ( UV_EOF == n )
	{
		/**
		 * EOF reached, this situation will never happen
		 * while the rfid reader at loop read mode.
		 * */
		uv_read_stop(client);
		free(buf->base);
		return;
	}
	else if ( n < 0 )
	{
		/* Some error happened */
		uv_read_stop(client);
		free(buf->base);
		return;
	}
	else if ( 0 == n )
	{
		/*not error, nothing read */
		return;
	}
	/*
	printf("*************0x");
	while(n--){
		printf("%02x ", (unsigned char )buf->base[i]);
		i++;
	}
	printf("*************\n");
	*/
	
	/*
	* the UHF RFID READER communication protocol
	* ------------------------------------------------
	* | Head | Type | Len | Data | CRC | End1 | End2 |
	* ------------------------------------------------
	* |   1  |   1  |  1  |  N   |  1  |   1  |   1  |
	* ------------------------------------------------
	* Head: Always 0xBB
	* Type: Different request get different response
	* Len : The length of Data
	* Data: Specify by different type
	* CRC : The sum of three field: Type, Len and Data
	* End1: Always 0x0D
	* End2: Always 0x0A
	*/

	/*
	*
	* the RFID READER cyclic response inquiring label's ID 
	* data format: 0xbb 0x97 0x16 ... 0x0d 0x0a
	* 
	* TYPE: 0x97 
	* PC: 4-5
	* EPC: 6-17
	* RSSI: 18-19
	* NATID: 20
	*/

	if ((unsigned char )buf->base[0] == 0xbb && (unsigned char )buf->base[1] == 0x97 && (unsigned char )buf->base[2] == 0x16){
		/*
		unsigned char bf[64];
		sprintf(bf, "%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x", (unsigned char )buf->base[5], (unsigned char )buf->base[6], (unsigned char )buf->base[7],
			(unsigned char )buf->base[8], (unsigned char )buf->base[9], (unsigned char )buf->base[10], (unsigned char )buf->base[11], (unsigned char )buf->base[12],
			(unsigned char )buf->base[13], (unsigned char )buf->base[14], (unsigned char )buf->base[15], (unsigned char )buf->base[16]);
		printf("BF: %s ; len: %d \n", bf, (int)strlen(bf));
		if (!recv_list.lock) {
			printf("===========not lock");
		}
		printf("=====cnt: %d \n", recv_list.cnt);
		*/
		int p = recv_list.cnt;
		int est_flag = 0, i = 0;

		if (!recv_list.cnt && !recv_list.lock){
			memcpy(recv_list.base[0], (unsigned char *)buf->base + 5, 12);
			recv_list.cnt ++;

		} else	if (recv_list.cnt && !recv_list.lock) {
			while(p--){
				if (!memcmp(recv_list.base[i++], (unsigned char *)buf->base + 5, 12)){
					est_flag = 1;
					break;
				}
			}
			if (!est_flag && recv_list.cnt < ARFI_NUM){
				memcpy(recv_list.base[recv_list.cnt], (unsigned char *)buf->base + 5, 12);
				recv_list.cnt ++;
			}
		}
	
		free(buf->base);
		return; 
	}

	/*
	* network get ready, and start scanning
	* cyclic inquire label's ID.  data format: 0xbb 0x17 0x02 0x00 0x00 0x19 0x0d 0x0a
	* network is ready.  data format: 0xbb 0x3a 0x00 0x3a 0x0d 0x0a
	*/
	if ((unsigned char )buf->base[0] == 0xbb && (unsigned char )buf->base[1] == 0x3a) {
		static unsigned char base[10] = {0xbb, 0x17, 0x02, 0x00, 0x00, 0x19, 0x0d, 0x0a};

		wr = (write_req_t*) malloc(sizeof( *wr));
		assert(wr != NULL);
		wr->buf = uv_buf_init(base, 8);

		if (uv_write(&wr->req, client, &wr->buf, 1, after_write)) {
			printf("uv_write failed \n");
		}

		free(buf->base);
		return;
	}

	/*
	* tcp client will send twice to get scanning result
	* client get rfid information 
	* first prepare data format
	* 0xcc 0xa4 0xa0
	*/	
	if ((unsigned char )buf->base[0] == 0xcc && (unsigned char )buf->base[1] == 0xa4 && (unsigned char )buf->base[2] == 0xa0) {
		recv_list.lock = false;
		wr = (write_req_t*) malloc(sizeof( *wr));
		assert(wr != NULL);
		wr->buf = uv_buf_init("success", 7);

		//printf("prepared data: %d - %s \n", (int )wr->buf.len, wr->buf.base);
		if (uv_write(&wr->req, client, &wr->buf, 1, after_write)) {
			printf("uv_write failed \n");
		}

		free(buf->base);
		return;
		
	}

	/*
	* client get rfid information
	* then get data format
	* 0xcc 0x14 0xa0
	*/
	if ((unsigned char )buf->base[0] == 0xcc && (unsigned char )buf->base[1] == 0x14 && (unsigned char )buf->base[2] == 0xa0) {
		unsigned char res[SIZE*4];
		
		int i = 0, c; 
		recv_list.lock = true;
		int p = recv_list.cnt;

		c = sprintf(res, "[");

		while (p--) {
			c += sprintf(res + c, "\"%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\"", recv_list.base[i][0], recv_list.base[i][1],
					recv_list.base[i][2], recv_list.base[i][3], recv_list.base[i][4], recv_list.base[i][5], recv_list.base[i][6],
					recv_list.base[i][7], recv_list.base[i][8], recv_list.base[i][9], recv_list.base[i][10], recv_list.base[i][11]);
			if(p){
				c += sprintf(res + c, ",");
			}
			i++;	
		}

		c += sprintf(res + c, "]");

		int n = ARFI_NUM, j = 0;
		while(n--){
			memset(recv_list.base[j++], 0, ARFI_CHA);
		}
		//printf("send to client cnt: %d  %s \n", recv_list.cnt, res);	
		recv_list.cnt = 0;
		
		wr = (write_req_t*) malloc(sizeof( *wr));
		assert(wr != NULL);
		wr->buf = uv_buf_init(res, strlen(res));

		if (uv_write(&wr->req, client, &wr->buf, 1, after_write)) {
			printf("uv_write failed \n");
		}
		
	}
	free(buf->base);
}

void rfid_tcp_connection_cb(uv_stream_t *server, int status)
{
	assert( 0 == status );
	
	uv_tcp_t *rfid_tcp_client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
	assert( 0 == uv_tcp_init(server->loop, rfid_tcp_client) );
	
	assert( 0 == uv_accept(server, (uv_stream_t*)rfid_tcp_client) );
	assert( 0 == uv_read_start((uv_stream_t*)rfid_tcp_client, alloc_cb, rfid_tcp_read_cb) );
}

int main(int argc, char *argv[])
{
	struct sockaddr_in	rfid_tcp_server_addr;
	uv_tcp_t		rfid_tcp_server;
	recv_list.cnt = 0;
	recv_list.lock = true;
	int n = ARFI_NUM, i = 0;
	while(n--){
		recv_list.base[i++] =(unsigned char *)malloc(ARFI_CHA);
	}	
	assert( 0 == uv_ip4_addr(SERVER_ADDR, SERVER_PORT, &rfid_tcp_server_addr));

	assert( 0 == uv_tcp_init(uv_default_loop(), &rfid_tcp_server) );
	assert( 0 == uv_tcp_bind(&rfid_tcp_server, (struct sockaddr*) &rfid_tcp_server_addr, 0) );
	assert( 0 == uv_listen((uv_stream_t *)&rfid_tcp_server, 5, rfid_tcp_connection_cb) );

	return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

