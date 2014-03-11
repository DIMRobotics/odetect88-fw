#ifndef ODETECT_SONAR_H
#define ODETECT_SONAR_H

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <arch/antares.h>

#include <lib/cerebellum/gpio.h>
#include <lib/printk.h>

void sonar_start();
uint8_t sonar_is_ready();
uint16_t *sonar_get();
uint8_t sonar_scale(uint16_t x);
        
#endif
