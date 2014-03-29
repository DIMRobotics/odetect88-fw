#ifndef INCLUDE_ODETECT_ADC_H
#define INCLUDE_ODETECT_ADC_H

#include <stdint.h>

void adc_start(void);
uint8_t adc_is_ready(void);
uint8_t *adc_get(void);
uint8_t adc_scale_hd(uint8_t x);
uint8_t adc_scale_ld(uint8_t x);
uint8_t adc_scale_md(uint8_t x);

#endif
