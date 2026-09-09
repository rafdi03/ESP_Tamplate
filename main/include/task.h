/*

* task.h
*
* Created on: 4 Sept 2026
* ```
   Author: Rafdi
  ```

*/

#ifndef MAIN_INCLUDE_TASK_H_
#define MAIN_INCLUDE_TASK_H_

#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef void (*int_callback_t)(void);

void register_int_callback(int_callback_t cb);
void execute_int_callback(void);

void startup_application(void);
void input_init(void);

void job_1ms(void);
void job_5ms(void);

void job_10ms(void);
void job_15ms(void);
void job_20ms(void);

void job_50ms(void);
void job_100ms(void);

void job_200ms(void);
void job_300ms(void);
void job_500ms(void);

void job_1000ms(void);

#endif
