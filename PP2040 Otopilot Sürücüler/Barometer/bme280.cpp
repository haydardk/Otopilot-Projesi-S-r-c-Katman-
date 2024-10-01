

#include "bme280.h"

/*

   GPIO 16 (pin 21) MISO/spi0_rx-> SDO/SDO on bme280 board
   GPIO 17 (pin 22) Chip select -> CSB/!CS on bme280 board
   GPIO 18 (pin 24) SCK/spi0_sclk -> SCL/SCK on bme280 board
   GPIO 19 (pin 25) MOSI/spi0_tx -> SDA/SDI on bme280 board
   3.3v (pin 36) -> VCC on bme280 board
   GND (pin 38)  -> GND on bme280 board

*/

BME280::BME280(i2c_inst_t *i2c, uint8_t i2c_address) {

    int32_t humidity, pressure, temperature;

    uint8_t osrsT   = 1;         //Temperature oversampling x 1
    uint8_t osrsP   = 1;         //Pressure oversampling x 1
    uint8_t osrsH   = 1;         //Humidity oversampling x 1
    uint8_t mode    = 3;         //Normal mode
    uint8_t tSb     = 5;         //Tstandby 1000ms
    uint8_t filter  = 0;         //Filter off
    uint8_t spi3wEn = 0;         //3-wire SPI Disable

    uint8_t ctrlMeasReg = (osrsT << 5) | (osrsP << 2) | mode;
    uint8_t configReg   = (tSb << 5) | (filter << 2) | spi3wEn;
    uint8_t ctrlHumReg  = osrsH;

    this->i2c = i2c;
    this->i2c_address = i2c_address;

    //Read compensation parameters
    read_compensation_parameters();

    write_register(0xF2, ctrlHumReg); 
    write_register(0xF4, ctrlMeasReg);
    write_register(0xF5, configReg);
}


int32_t BME280::compensate_temp(int32_t adc_T) {
    int32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((int32_t) this->dig_T1 << 1))) * ((int32_t) this->dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t) this->dig_T1)) * ((adc_T >> 4) - ((int32_t) this->dig_T1))) >> 12) * ((int32_t) this->dig_T3))
            >> 14;

    this->t_fine = var1 + var2;
    T = (this->t_fine * 5 + 128) >> 8;
    return T;
}

uint32_t BME280::compensate_pressure(int32_t adc_P) {
    int32_t var1, var2;
    uint32_t p;
    var1 = (((int32_t) this->t_fine) >> 1) - (int32_t) 64000;
    var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t) this->dig_P6);
    var2 = var2 + ((var1 * ((int32_t) this->dig_P5)) << 1);
    var2 = (var2 >> 2) + (((int32_t) this->dig_P4) << 16);
    var1 = (((this->dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((int32_t) this->dig_P2) * var1) >> 1)) >> 18;
    var1 = ((((32768 + var1)) * ((int32_t) this->dig_P1)) >> 15);
    if (var1 == 0)
        return 0;

    p = (((uint32_t) (((int32_t) 1048576) - adc_P) - (var2 >> 12))) * 3125;
    if (p < 0x80000000)
        p = (p << 1) / ((uint32_t) var1);
    else
        p = (p / (uint32_t) var1) * 2;

    var1 = (((int32_t) this->dig_P9) * ((int32_t) (((p >> 3) * (p >> 3)) >> 13))) >> 12;
    var2 = (((int32_t) (p >> 2)) * ((int32_t) this->dig_P8)) >> 13;
    p = (uint32_t) ((int32_t) p + ((var1 + var2 + this->dig_P7) >> 4));

    return p;
}

uint32_t BME280::compensate_humidity(int32_t adc_H) {
    int32_t v_x1_u32r;
    v_x1_u32r = (this->t_fine - ((int32_t) 76800));
    v_x1_u32r = (((((adc_H << 14) - (((int32_t) this->dig_H4) << 20) - (((int32_t) this->dig_H5) * v_x1_u32r)) +
                   ((int32_t) 16384)) >> 15) * (((((((v_x1_u32r * ((int32_t) this->dig_H6)) >> 10) * (((v_x1_u32r *
                                                                                                  ((int32_t) this->dig_H3))
            >> 11) + ((int32_t) 32768))) >> 10) + ((int32_t) 2097152)) *
                                                 ((int32_t) this->dig_H2) + 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t) this->dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);

    return (uint32_t) (v_x1_u32r >> 12);
}


void BME280::write_register(uint8_t reg, uint8_t data) {
    uint8_t buf[2];
    //buf[0] = reg & 0x7f;  // remove read bit as this is a write
    buf[0] = reg;
    buf[1] = data;
   	i2c_write_blocking(this->i2c,this->i2c_address,buf,2,true);
}

void BME280::read_registers(uint8_t reg, uint8_t *buf, uint16_t len) {
    // For this particular device, we send the device the register we want to read
    // first, then subsequently read from the device. The register is auto incrementing
    // so we don't need to keep sending the register we want, just the first.
    reg |= READ_BIT;
  	i2c_write_blocking(this->i2c,this->i2c_address,&reg,1,true);
  	i2c_read_blocking(this->i2c,this->i2c_address,buf,len,false);
}

/* This function reads the manufacturing assigned compensation parameters from the device */
void BME280::read_compensation_parameters() {
    uint8_t buffer[26];

    read_registers(0x88, buffer, 24);

    this->dig_T1 = buffer[0] | (buffer[1] << 8);
    this->dig_T2 = buffer[2] | (buffer[3] << 8);
    this->dig_T3 = buffer[4] | (buffer[5] << 8);

    this->dig_P1 = buffer[6] | (buffer[7] << 8);
    this->dig_P2 = buffer[8] | (buffer[9] << 8);
    this->dig_P3 = buffer[10] | (buffer[11] << 8);
    this->dig_P4 = buffer[12] | (buffer[13] << 8);
    this->dig_P5 = buffer[14] | (buffer[15] << 8);
    this->dig_P6 = buffer[16] | (buffer[17] << 8);
    this->dig_P7 = buffer[18] | (buffer[19] << 8);
    this->dig_P8 = buffer[20] | (buffer[21] << 8);
    this->dig_P9 = buffer[22] | (buffer[23] << 8);

    this->dig_H1 = buffer[25];

    read_registers(0xE1, buffer, 8);

    this->dig_H2 = buffer[0] | (buffer[1] << 8);
    this->dig_H3 = (int8_t) buffer[2];
    this->dig_H4 = buffer[3] << 4 | (buffer[4] & 0xf);
    this->dig_H5 = (buffer[5] >> 4) | (buffer[6] << 4);
    this->dig_H6 = (int8_t) buffer[7];
}

void BME280::bme280_read_raw(int32_t *humidity, int32_t *pressure, int32_t *temperature) {
    uint8_t buffer[8];

    read_registers(0xF7, buffer, 8);
    *pressure = ((uint32_t) buffer[0] << 12) | ((uint32_t) buffer[1] << 4) | (buffer[2] >> 4);
    *temperature = ((uint32_t) buffer[3] << 12) | ((uint32_t) buffer[4] << 4) | (buffer[5] >> 4);
    *humidity = (uint32_t) buffer[6] << 8 | buffer[7];
}


void BME280::bme280_read(int32_t *humidity, int32_t *pressure, int32_t *temperature) {
    int32_t _humidity;
    int32_t _pressure;
    int32_t _temperature;

    bme280_read_raw(&_humidity, &_pressure, &_temperature);
    // These are the raw numbers from the chip, so we need to run through the
    // compensations to get human understandable numbers
    *pressure = compensate_pressure(_pressure);
    *temperature = compensate_temp(_temperature);
    *humidity = compensate_humidity(_humidity);
}