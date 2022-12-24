#ifndef __CTRL_H
#define __CTRL_H

#include "defs.h"

#define CTRL_UP             117
#define CTRL_DOWN           114
#define CTRL_LEFT           107
#define CTRL_RIGHT          116
#define CTRL_UP_LEFT        108
#define CTRL_UP_RIGHT       125
#define CTRL_DOWN_LEFT      105
#define CTRL_DOWN_RIGHT     122

#define CTRL_TAKE_ITEM      112
#define CTRL_ACCEPT         90

#define CTRL_F1                 0x05
#define CTRL_F2                 0x06
#define CTRL_F5                 0x03
#define CTRL_F6                 0x0b

void    ctrl_init();
uint8_t ctrl_pop_key();

#endif