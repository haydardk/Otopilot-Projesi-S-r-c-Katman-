#pragma once

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"

#define READ_BIT 0x80

class BME280 {

  private:
    int32_t t_fine;
    uint16_t dig_T1;
    int16_t dig_T2, dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
    uint8_t dig_H1, dig_H3;
    int8_t dig_H6;
    int16_t dig_H2, dig_H4, dig_H5;

    i2c_inst_t *i2c;
    uint8_t i2c_address;

    void write_register(uint8_t reg, uint8_t data);
    void read_registers(uint8_t reg, uint8_t *buf, uint16_t len);
    void bme280_read_raw(int32_t *humidity, int32_t *pressure, int32_t *temperature);
    void read_compensation_parameters();
    int32_t compensate_temp(int32_t adc_T);
    uint32_t compensate_pressure(int32_t adc_P);
    uint32_t compensate_humidity(int32_t adc_H);

  public:
    BME280(i2c_inst_t *i2c, uint8_t i2c_address);
    void bme280_read(int32_t *humidity, 
                     int32_t *pressure, 
                     int32_t *temperature);

  };