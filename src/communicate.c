#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/socket.h>

#include <uv.h>

#include "rfid_reader.h"
#include "rfid_list.h"

//#define NDEBUG

void alloc_cb(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
	buf->base = malloc(1024);
	buf->len  = 1024;
}

void uv_buf_free(uv_buf_t *buf)
{
	if ( buf && buf->base && buf->len )
	{
		free(buf->base);
	}
}

void rfid_tcp_read_cb(uv_stream_t *client, ssize_t n, const uv_buf_t *buf)
{
	if ( UV_EOF == n )
	{
		printf("EOF reached!\n");
	}
	else if ( n < 0 )
	{
		printf("n <= 0\n");
		uv_read_stop(client);
		free(buf->base);
		return ;
	}
	else if ( 0 == n )
	{
		buf->base[buf->len] = '\0';
		printf("%s", buf->base);
	}

	rfid_protocol_parse(buf);
}

void rfid_tcp_connection_cb(uv_stream_t *server, int status)
{
	assert( 0 == status );

	uv_tcp_t *rfid_tcp_client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
	assert( 0 == uv_tcp_init(uv_default_loop(), rfid_tcp_client) );

	assert( 0 == uv_accept(server, (uv_stream_t*)rfid_tcp_client) );
	assert( 0 == uv_read_start((uv_stream_t*)rfid_tcp_client, alloc_cb, rfid_tcp_read_cb) );
	rfid_reader_send_config();
}

int main(int argc, char *argv[])
{
	struct sockaddr_in	rfid_tcp_server_addr;
	uv_tcp_t			rfid_tcp_server;

	assert( 0 == uv_ip4_addr("0.0.0.0", 8080, &rfid_tcp_server_addr) );

	assert( 0 == uv_tcp_init(uv_default_loop(), &rfid_tcp_server) );
	assert( 0 == uv_tcp_bind(&rfid_tcp_server, (struct sockaddr *)&rfid_tcp_server_addr, 0) );
	assert( 0 == uv_listen((uv_stream_t *)&rfid_tcp_server, 5, rfid_tcp_connection_cb) );

	return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

