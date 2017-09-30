#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "rfid_list.h"

void rfid_reader_send_config(void)
{
	/* TODO Send config command */
	return ;
}

static bool rfid_reader_crc_check(uint8_t *buf)
{
	uint8_t crc = buf[1] + buf[2];
	for ( uint8_t i = 3; i < buf[2] + 3; ++i )
	{
		crc += buf[i];
	}
	return crc == buf[i];
}

static void rfid_reader_response_type_0x97(uint8_t *data, uint8_t len)
{
	uint8_t n = len/22;
	for ( uint8_t i = 0; i < n; ++i )
	{
		rfid_list_insert(&data[i*22 + 2], 12);
	}
	return ;
}


/**
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
 * */
void rfid_reader_response_parse(uint8_t *buf)
{
	if ( buf[0] != 0xBB					/* Check HEAD */
		|| !rfid_reader_crc_check(buf)	/* Check CRC */
		|| buf[4 + buf[3]] != 0x0D		/* Check End1 */
		|| buf[5 + buf[3]] != 0x0A)		/* Check End2 */
	{
		/* Message format error */
		return ;
	}

	switch ( buf[1] )
	{
		case 0x97:
			rfid_reader_response_type_0x97(&buf[3], buf[2]);
			break;
		default:
			break;
	}
}

