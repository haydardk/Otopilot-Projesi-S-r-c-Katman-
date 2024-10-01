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
    float kalman_estimate;   // Tahmin edilen de�er
    float kalman_error;      // Tahmin hatas� (ba�lang��ta 1 olarak ayarlanabilir)
    float measurement_error; // �l��m hatas� (uygun bir ba�lang�� de�eri verilebilir)
    float process_noise;     // S�re� g�r�lt�s� (verilere g�re ayarlanabilir)
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

    // Buffer'� g�ncelleme
    void PitotUpdateBuffer(float new_speed);

    // H�z de�i�imini hesaplama
    float PitotCalculateSpeedChange();

    // �l��m ba�latma
    void PitotStartMeasurement();

    // Verileri okuma
    void PitotReadData();

    // Bas�n� ve s�cakl�k hesaplama
    void PitotCalculate();

    // I2C ve GPIO ba�latma
    void Pitot_init();
};

#endif // MS4525_AIRSPEED_H

