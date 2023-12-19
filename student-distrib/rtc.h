#ifndef _X_RTC_H
#define _X_RTC_H

#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "tests.h"

void rtc_init();
int32_t rtc_open (const uint8_t* filename);
int32_t rtc_close(int32_t fd);
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_write (int32_t fd, const void* buf, int32_t nbytes);
int32_t rtc_change_frequency(int buf);

#endif
