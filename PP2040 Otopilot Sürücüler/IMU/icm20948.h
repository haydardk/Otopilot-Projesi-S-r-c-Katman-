#include "pico/stdlib.h"
#include "stdio.h"
#include "hardware/spi.h"
#include "stdint.h"
#include "time.h"
#include "math.h"

/*
    Clock Polarity High - Phase 2 - Baud 5.0 MB/s
*/

//************************** HEADER FILE ******************************** */
#define SPI_PORT spi0
#define CS_PIN 17
#define SPI_SCK_PIN  18
#define SPI_MOSI_PIN 19 // TX 19
#define SPI_MISO_PIN 16 // RX 16

#define ACCEL_RANGE _accel_4g
#define GYRO_RANGE _gyro_1000dps
#define M_PI		3.14159265358979323846

typedef struct
{
    int16_t x_accel;
    int16_t y_accel;
    int16_t z_accel;
    int16_t x_gyro;
    int16_t y_gyro;
    int16_t z_gyro;
    int16_t x_mag;
    int16_t y_mag;
    int16_t z_mag;
}icm_data;

typedef enum{
    _gyro_250dps,
    _gyro_500dps,
    _gyro_1000dps,
    _gyro_2000dps,
} gyro_range;

typedef enum{
    _accel_2g,
    _accel_4g,
    _accel_8g,
    _accel_16g,
}accel_range;

typedef enum{
    _b0 = 0,
    _b1 = 1<<4,
    _b2 = 2<<4,
    _b3 = 3<<4
}userbank;

#define REG_BANK_SEL 0x7F
#define ODR_ALIGN_EN       0x09
#define WHO_AM_I           0x00
#define PWR_MGMT_1         0x06
#define USER_CTRL          0x03
#define GYRO_CONFIG_1      0x01
#define GYRO_CONFIG_2      0x02
#define GYRO_SMPLRT_DIV 0x00

#define ACCEL_SMPLRT_DIV1 0x10
#define ACCEL_SMPLRT_DIV2 0x10
#define ACCEL_CONFIG 0x14

#define GYRO_XOUT_H    0x33
#define GYRO_XOUT_L    0x34
#define GYRO_YOUT_H    0x35
#define GYRO_YOUT_L    0x36
#define GYRO_ZOUT_H    0x37
#define GYRO_ZOUT_L    0x38

#define ACCEL_XOUT_H   0x2D
#define ACCEL_XOUT_L   0x2E
#define ACCEL_YOUT_H   0x2F
#define ACCEL_YOUT_L   0x30
#define ACCEL_ZOUT_H   0x31
#define ACCEL_ZOUT_L   0x32

#define I2C_SLV0_ADDR   0x03
#define I2C_SLV0_REG    0x04
#define I2C_SLV0_CTRL   0x05
#define AK09916_ADDR    0x0C // Adres
#define AK09916_XOUT_L  0x11 // X ekseni düşük byte veri register adresi
#define AK09916_CNTL2   0x31 // Konfigürasyon register adresi
#define AK09916_ST1     0x10 // Durum register adresi

#define I2C_MST_CTRL 0x01
#define LP_CONFIG 0x05
#define I2C_MST_ODR_CONFIG 0x00
#define I2C_SLV0_DO 0x06
#define AK099916_ADDRESS   0x0C  // Magnetometre adresi (AUX I2C)
#define MAG_WIA             0x00  // Who am I register for magnetometre
#define MAG_CNTL2           0x31
#define MAG_CNTL3           0x32
#define MAG_DATA_ONSET      0x11
#define MAG_DATA_XOUT_L     0x11  // Magnetometre X ekseni veri düşük bayt
#define MAG_DATA_XOUT_H     0x12  // Magnetometre X ekseni veri yüksek bayt
#define MAG_DATA_YOUT_L     0x13  // Magnetometre Y ekseni veri düşük bayt
#define MAG_DATA_YOUT_H     0x14  // Magnetometre Y ekseni veri yüksek bayt
#define MAG_DATA_ZOUT_L     0x15  // Magnetometre Z ekseni veri düşük bayt
#define MAG_DATA_ZOUT_H     0x16  // Magnetometre Z ekseni veri yüksek bayt

#define XG_OFFS_USRH 0x03  // BIAS OFFSET REGISTERLARI
#define XG_OFFS_USRL 0x04
#define YG_OFFS_USRH 0x05
#define YG_OFFS_USRL 0x06
#define ZG_OFFS_USRH 0x07
#define ZG_OFFS_USRL 0x08

#define XG_OFFSET 16400// GYRO BIAS : 16406 -2 62
#define YG_OFFSET 0
#define ZG_OFFSET 60
#define DEG_TO_RAD 0.01745329252
#define RAD_TO_DEG 57.3
#define SAMPLE_MS 1000

#define XM_OFFSET 20 // MINUS
#define YM_OFFSET 40 // MINUS
#define ZM_OFFSET 167

#define ACCEL_SCALE 0.000066 // 4g / 65535 (±4g = 2^15 - 1)
#define GYRO_SCALE 0.030518 // 2000 dps / 32768 (±2000 dps = 2^15 - 1)
#define MAGNET_SCALE 0.15 // 1 uT/LSB (Bu örnek bir ölçekleme faktörü, sensör datasheet'indeki değere göre ayarlanmalıdır)

class ICM20948 {
public:
    ICM20948();

    void icm20948_spi_init();
    void icm_init();
    void icm_read_data(icm_data* data);
    void gyro_remove_bias();

private:
    void sel_user_bank(userbank ub);
    void icm_write_reg(userbank ub, uint8_t reg, uint8_t data);
    void icm_read_reg(userbank ub, uint8_t address, uint8_t* data);
    void ak09916_write_reg(uint8_t reg, uint8_t data);
    void ak09916_read_reg(uint8_t onset_reg, uint8_t len);
    void ak09916_init();

    static const uint8_t CS_PIN = 17;
    static const spi_inst_t* SPI_PORT = spi0;
};

