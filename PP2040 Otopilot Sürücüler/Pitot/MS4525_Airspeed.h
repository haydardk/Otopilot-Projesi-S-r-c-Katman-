#ifndef MS4525_AIRSPEED_H
#define MS4525_AIRSPEED_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <cmath>

// Sabitler
#define MS4525D0_I2C_ADDR1 0x28
#define I2C_PORT i2c0
#define BUFFER_SIZE 10
#define PRESSURE_OFFSET 0.0f

class MS4525_Airspeed {
private:
    float kalman_estimate;   // Tahmin edilen deðer
    float kalman_error;      // Tahmin hatasý (baþlangýçta 1 olarak ayarlanabilir)
    float measurement_error; // Ölçüm hatasý (uygun bir baþlangýç deðeri verilebilir)
    float process_noise;     // Süreç gürültüsü (verilere göre ayarlanabilir)
    uint8_t dataPitot[4];
    float speed_buffer[BUFFER_SIZE];
    int buffer_index;

public:
    float temperature;
    float pressure;
    float airspeed;

    // Constructor
    MS4525_Airspeed();

    // Kalman filtresi uygulama
    void PitotKalmanFilter(float measurement);

    // Buffer'ý güncelleme
    void PitotUpdateBuffer(float new_speed);

    // Hýz deðiþimini hesaplama
    float PitotCalculateSpeedChange();

    // Ölçüm baþlatma
    void PitotStartMeasurement();

    // Verileri okuma
    void PitotReadData();

    // Basýnç ve sýcaklýk hesaplama
    void PitotCalculate();

    // I2C ve GPIO baþlatma
    void Pitot_init();
};

#endif // MS4525_AIRSPEED_H

