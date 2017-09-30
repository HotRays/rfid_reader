#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* 4 layers, 7 axle position, 12 byte of EPC length. One byte to indicate valid.*/
static uint8_t rfid_list[4 * 7 * 2][1 + 12];

/* 12 byte of EPC length convert to char need 24 byte, last byte will be NULL.*/
char rfid_list_string[4 * 7 * 2][12 * 2 + 1];

static uint8_t rfid_list_get_length(void)
{
	uint8_t i;
	for ( i = 0; i < 4*7*2; ++i )
	{
		if ( 0 == rfid_list[i][0] )
		{
			break;
		}
	}
	return i;
}

void rfid_list_set_empty(void)
{
	uint8_t i;
	for ( i = 0; i < 4*7*2; ++i )
	{
		rfid_list[i][0] = 0;
	}
}

void rfid_list_insert(uint8_t *buf, uint8_t len)
{
	uint8_t i, len;
	len = rfid_list_get_length();

	for ( i = 0; i < len; ++i )
	{
		if ( 0 == memcmp(&rfid_list[i][1], buf, len) )
		{
			/* Duplicate tag */
			break;
		}
	}

	if ( i == len )
	{
		/* Current epc no find in rfid_list, add to list */
		rfid_list[i][0] = 1;
		memcpy(&rfid_list[i][1], buf, len);
	}
}

void rfid_list_string_update(void)
{
	uint8_t i, j, len;
	len = rfid_list_get_length();

	for ( i = 0; i < len; ++i )
	{
		for ( j = 0; j < 12; ++j )
		{
			sprintf(rfid_list_string[i][j*2], "%02X", rfid_list[i][j]);
		}
		rfid_list_string[i][j*2] = '\0';
	}

	for (; i < 4*7*2; ++i)
	{
		rfid_list_string[i][0] = '\0';
	}
}

