#ifndef RESET_BUTTON_H
#define RESET_BUTTON_H

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

void startResetButtonTask(gpio_num_t pin, uint32_t holdTimeMs = 3000);

#ifdef __cplusplus
}
#endif

#endif // RESET_BUTTON_H
