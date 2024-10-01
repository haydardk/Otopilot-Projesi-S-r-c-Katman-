#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <cmath>

// ******************** MS4525_Airspeed CLASS ***************************

MS4525_Airspeed::MS4525_Airspeed() {
        kalman_estimate = 0.0f;
        kalman_error = 1.0f;
        measurement_error = 1.0f;
        process_noise = 0.01f;
        buffer_index = 0;
        airspeed = 0.0f;
        pressure = 0.0f;
        temperature = 0.0f;
}

void MS4525_Airspeed::PitotKalmanFilter(float measurement) {
        // Kalman kazanc�
        float kalman_gain = kalman_error / (kalman_error + measurement_error);
        
        // Yeni tahmini hesapla
        kalman_estimate = kalman_estimate + kalman_gain * (measurement - kalman_estimate);
        
        // Hata g�ncellemesi
        kalman_error = (1.0f - kalman_gain) * kalman_error + fabs(kalman_estimate) * process_noise;
        
        // Sonu� olarak filtrelenmi� de�eri airspeed'e aktar
        airspeed = kalman_estimate;
}

void MS4525_Airspeed::PitotUpdateBuffer(float new_speed) {
        // Buffer'a yeni h�z� ekle ve eski verileri g�ncelle
        speed_buffer[buffer_index] = new_speed;
        buffer_index = (buffer_index + 1) % BUFFER_SIZE;
}

float MS4525_Airspeed::PitotCalculateSpeedChange() {
        // H�z de�i�imini buffer'daki veriler �zerinden hesapla
        float speed_change = 0.0f;
        for (int i = 1; i < BUFFER_SIZE; i++) {
            speed_change += speed_buffer[i] - speed_buffer[i - 1];
        }
        return speed_change / (BUFFER_SIZE - 1); // Ortalama h�z de�i�imi
}

void MS4525_Airspeed::PitotStartMeasurement() {
        uint8_t cmd = 0; // �l��m ba�latma komutu
        int result = i2c_write_blocking(I2C_PORT, MS4525D0_I2C_ADDR1, &cmd, 1, false);
        if (result == PICO_ERROR_GENERIC) {
            printf("I2C transmission error\n");
        }
}

void MS4525_Airspeed::PitotReadData() {
        int result = i2c_read_blocking(I2C_PORT, MS4525D0_I2C_ADDR1, dataPitot, 4, false);
        if (result == PICO_ERROR_GENERIC) {
            printf("I2C reception error\n");
        }
}

void MS4525_Airspeed::PitotCalculate() {
        uint8_t status = (dataPitot[0] & 0xC0) >> 6;
        int16_t dp_raw, dT_raw;
        if (status == 0 || status == 1) {
            dp_raw = ((dataPitot[0] & 0x3F) << 8) | dataPitot[1];
            dT_raw = (dataPitot[2] << 3) | ((dataPitot[3] & 0xE0) >> 5);
        }

        const float P_max = 1.0f; // psi
        const float P_min = -1.0f; // psi
        const float PSI_to_Pa = 6894.757f;
        float diff_press_PSI = -((dp_raw - 0.1f * 16383) * (P_max - P_min) / (0.8f * 16383) + P_min);
        pressure = diff_press_PSI * PSI_to_Pa;
        pressure = pressure - PRESSURE_OFFSET;
        temperature = ((200.0f * dT_raw) / 2047) - 50;

        // Hava yo�unlu�unu s�cakl��a g�re hesapla (deniz seviyesinde varsay�lan bas�n�)
        float temperature_K = temperature + 273.15f; // S�cakl�k Kelvin cinsinden
        const float standard_pressure = 101325.0f; // Pa
        const float specific_gas_constant = 287.05f; // J/(kg�K)
        float air_density = standard_pressure / (specific_gas_constant * temperature_K);

        // Hava h�z�n� hesapla
        if (pressure > 0) {
            airspeed = sqrt(2 * pressure / air_density);

            // H�z de�i�imini buffer ile hesapla
            float speed_change = PitotCalculateSpeedChange();
            airspeed += speed_change; // Tahmine h�z de�i�imini ekle

            // Buffer'� g�ncelle
            PitotUpdateBuffer(airspeed);

            // Kalman filtresi uygula
            PitotKalmanFilter(airspeed);
        } else {
            airspeed = 0; // Bas�n� fark� negatif veya s�f�rsa h�z s�f�r olmal�
        }
}

void MS4525_Airspeed::Pitot_init() {
        // I2C yap�land�rma ve GPIO pin ayarlar�
        i2c_init(I2C_PORT, 100 * 1000);
        gpio_set_function(2, GPIO_FUNC_I2C);
        gpio_set_function(3, GPIO_FUNC_I2C);
        gpio_pull_up(2);
        gpio_pull_up(3);
    }
};

