// **************************** SOURCE FILE **************************************************** / 
#include <stdint.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

static const uint8_t CS_PIN = 17;  // Pin numarasýný projene göre güncelle
static const spi_inst_t* SPI_PORT = spi0;

ICM20948::ICM20948(){
	
}

void ICM20948::sel_user_bank(userbank ub) {
        uint8_t data = ub;
        uint8_t reg = REG_BANK_SEL;
        gpio_put(CS_PIN, 0);
        spi_write_blocking(SPI_PORT, &reg, 1);
        spi_write_blocking(SPI_PORT, &data, 1);
        gpio_put(CS_PIN, 1);
}

void ICM20948::icm_write_reg(userbank ub, uint8_t reg, uint8_t data) {
        sel_user_bank(ub);
        gpio_put(CS_PIN, 0);
        spi_write_blocking(SPI_PORT, &reg, 1);
        spi_write_blocking(SPI_PORT, &data, 1);
        gpio_put(CS_PIN, 1);
}

void ICM20948::icm_read_reg(userbank ub, uint8_t address, uint8_t* data) {
        uint8_t temp_data = 0x80 | address;
        gpio_put(CS_PIN, 0);
        spi_write_blocking(SPI_PORT, &temp_data, 1);
        spi_read_blocking(SPI_PORT, 0, data, 1);
        gpio_put(CS_PIN, 1);
}

void ICM20948::ak09916_write_reg(uint8_t reg, uint8_t data) {
        icm_write_reg(_b3, I2C_SLV0_ADDR, AK099916_ADDRESS);
        icm_write_reg(_b3, I2C_SLV0_REG, reg);
        icm_write_reg(_b3, I2C_SLV0_DO, data);
        icm_write_reg(_b3, I2C_SLV0_CTRL, 0x80 | 0x01);
        sleep_ms(50);
}

void ICM20948::ak09916_read_reg(uint8_t onset_reg, uint8_t len) {
        icm_write_reg(_b3, I2C_SLV0_ADDR, 0x80 | AK099916_ADDRESS);
        icm_write_reg(_b3, I2C_SLV0_REG, onset_reg);
        icm_write_reg(_b3, I2C_SLV0_CTRL, 0x80 | len);
        sleep_ms(50);
}

void ICM20948::ak09916_init() {
        uint8_t temp_data;
        icm_read_reg(_b0, USER_CTRL, &temp_data);
        temp_data |= 0x02;
        icm_write_reg(_b0, USER_CTRL, temp_data);
        sleep_ms(100);

        icm_read_reg(_b0, USER_CTRL, &temp_data);
        temp_data |= 0x20;
        icm_write_reg(_b0, USER_CTRL, temp_data);
        sleep_ms(10);

        temp_data = 0x07;
        icm_write_reg(_b3, I2C_MST_CTRL, temp_data);
        sleep_ms(10);

        temp_data = 0x40;
        icm_write_reg(_b0, LP_CONFIG, temp_data);
        sleep_ms(10);

        temp_data = 0x03;
        icm_write_reg(_b3, I2C_MST_ODR_CONFIG, temp_data);
        sleep_ms(10);

        ak09916_write_reg(MAG_CNTL3, 0x01);
        sleep_ms(100);
        ak09916_write_reg(MAG_CNTL2, 0x08);
}

void ICM20948::gyro_remove_bias() {
        int16_t x_gyro_bias, y_gyro_bias, z_gyro_bias;
        icm_data biasdata;
        int32_t x_bias = 0, y_bias = 0, z_bias = 0;

        for (int i = 0; i < 100; i++) {
            icm_read_data(&biasdata);
            x_bias += (int32_t)biasdata.x_gyro;
            y_bias += (int32_t)biasdata.y_gyro;
            z_bias += (int32_t)biasdata.z_gyro;
            sleep_ms(1);
        }

        x_gyro_bias = (int16_t)(x_bias / 100);
        y_gyro_bias = (int16_t)(y_bias / 100);
        z_gyro_bias = (int16_t)(z_bias / 100);

        printf("GYRO BIAS : %d %d %d\n", x_gyro_bias, y_gyro_bias, z_gyro_bias);
        sleep_ms(10);
}

void ICM20948::icm_read_data(icm_data* data) {
        uint8_t data_rx[22];
        uint8_t temp_data = 0x80 | ACCEL_XOUT_H;
        gpio_put(CS_PIN, 0);
        spi_write_blocking(SPI_PORT, &temp_data, 1);
        spi_read_blocking(SPI_PORT, 0, data_rx, 22);
        gpio_put(CS_PIN, 1);

        data->x_accel = ((int16_t)data_rx[0] << 8) | (int16_t)data_rx[1];
        data->y_accel = ((int16_t)data_rx[2] << 8) | (int16_t)data_rx[3];
        data->z_accel = ((int16_t)data_rx[4] << 8) | (int16_t)data_rx[5];

        data->x_gyro = (((int16_t)data_rx[6] << 8) | (int16_t)data_rx[7]) - XG_OFFSET;
        data->y_gyro = (((int16_t)data_rx[8] << 8) | (int16_t)data_rx[9]) - YG_OFFSET;
        data->z_gyro = (((int16_t)data_rx[10] << 8) | (int16_t)data_rx[11]) - ZG_OFFSET;

        data->x_mag = (((int16_t)data_rx[15] << 8) | (int16_t)data_rx[14]) - XM_OFFSET;
        data->y_mag = (((int16_t)data_rx[17] << 8) | (int16_t)data_rx[16]) - YM_OFFSET;
        data->z_mag = (((int16_t)data_rx[19] << 8) | (int16_t)data_rx[18]) + ZM_OFFSET;
}

void ICM20948::icm20948_spi_init() {
        gpio_init(CS_PIN);
        gpio_set_dir(CS_PIN, GPIO_OUT);

        spi_init(SPI_PORT, 5000 * 1000); // 5 MHz 
        spi_set_format(SPI_PORT, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);

        gpio_set_function(SPI_MOSI_PIN, GPIO_FUNC_SPI);
        gpio_set_function(SPI_SCK_PIN, GPIO_FUNC_SPI);
        gpio_set_function(SPI_MISO_PIN, GPIO_FUNC_SPI);
}
    
void ICM20948::icm_init() {
        uint8_t temp_data;
        icm_write_reg(_b0, PWR_MGMT_1, 0xC1);
        sleep_ms(1000);
        icm_write_reg(_b0, PWR_MGMT_1, 0x01);

        icm_write_reg(_b2, ODR_ALIGN_EN, 0x01);
        icm_write_reg(_b2, GYRO_SMPLRT_DIV, 0x00);
        icm_write_reg(_b2, GYRO_CONFIG_1, ((GYRO_RANGE << 1) | 0x01));
        icm_write_reg(_b2, ACCEL_SMPLRT_DIV1, 0x00);
        icm_write_reg(_b2, ACCEL_SMPLRT_DIV2, 0x00);
        icm_write_reg(_b2, ACCEL_CONFIG, ((ACCEL_RANGE << 1) | 0x01));

        icm_read_reg(_b0, USER_CTRL, &temp_data);
        temp_data |= 0x10;
        icm_write_reg(_b2, USER_CTRL, temp_data);

        sleep_ms(100);
        ak09916_init();
        ak09916_read_reg(MAG_DATA_ONSET, 8);

        sel_user_bank(_b0);
    }
}


