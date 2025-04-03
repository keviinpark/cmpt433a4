// Manage certain hardware with I2C

#ifndef _I2C_H_
#define _I2C_H_

#include <stdint.h>

// initializes I2C device
int init_i2c_bus(char* bus, int address);
// write to 16 bit i2c register
void write_i2c_reg16(int i2c_file_desc, uint8_t reg_addr, uint16_t value);
// read val from 16 bit i2c register
uint16_t read_i2c_reg16(int i2c_file_desc, uint8_t reg_addr);

// write to 8 bit i2c register
void write_i2c_reg8(int i2c_file_desc, uint8_t reg_addr, uint8_t value);
// read val from 8 bit i2c register
uint8_t read_i2c_reg8(int i2c_file_desc, uint8_t reg_addr);

#endif
