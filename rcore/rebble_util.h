#pragma once
void write_32(uint8_t *addr, int32_t val);
int32_t read_32(uint8_t *addr);
void delay_ms(uint32_t ms);
uint32_t map_range(uint32_t input, uint32_t input_start, uint32_t input_end, uint32_t output_start, uint32_t output_end);
