#ifndef __KB_H
#define __KB_H

#include "./defs.h"

typedef void (*key_handler_ptr)(uint8_t sc);

// RESERVED SCAN CODES
#define RSC_RELEASE         0xf0
#define RSC_EXTENDED_SC     0xe0
#define RSC_PAUSE_EXT_SC    0xe1
#define RSC_LSHIFT          0x12
#define RSC_RSHIFT          0x59
#define RSC_CTRL            0x14

void kb_init(key_handler_ptr handler);

void kb_send_cmd(uint8_t cmd);
void kb_lock();
void kb_unlock();

#endif