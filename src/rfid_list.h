#ifndef RFID_LIST_H
#define RFID_LIST_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>

void rfid_list_set_empty(void);
void rfid_list_insert(uint8_t *buf, uint8_t len);
void rfid_list_string_update(void);

extern char rfid_list_string[4 * 7 * 2][12 * 2 + 1];

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RFID_LIST_H */

