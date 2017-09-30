#ifndef RFID_READER_H
#define RFID_READER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>

void rfid_reader_send_config(void);
void rfid_reader_response_parse(uint8_t *buf);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RFID_READER_H */

