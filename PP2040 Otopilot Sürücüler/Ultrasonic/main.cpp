/*
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "stdio.h"

#define TRIG_PIN 9
#define ECHO_PIN 14

int timeout = 26100; // (800cm / 0.0343(cm/us) ) ~ 23382 mikrosaniye

void setupUltrasonicPins(int trig, int echo)
{
    gpio_init(trig);
    gpio_init(echo);
    gpio_set_dir(trig, GPIO_OUT);
    gpio_set_dir(echo, GPIO_IN);
}

uint64_t getPulse(int trig, int echo)
{
    gpio_put(trig, 1);
    sleep_us(10);
    gpio_put(trig, 0);

    uint64_t width = 0;

    while (gpio_get(echo) == 0) tight_loop_contents();
    absolute_time_t startTime = get_absolute_time();
    while (gpio_get(echo) == 1) 
    {
        width++;
        sleep_us(1);
        if (width > timeout) return 0;
    }
    absolute_time_t endTime = get_absolute_time();
    
    return absolute_time_diff_us(startTime, endTime);
}

uint64_t getCm(int trig, int echo)
{
    uint64_t pulseLength = getPulse(trig, echo);
    return pulseLength/29/2;
}

int main() {
    stdio_init_all();
    // GPIO pinlerini ayarla
    setupUltrasonicPins(TRIG_PIN , ECHO_PIN);

    // Sonsuz döngüde mesafe ölçme
    sleep_ms(5000);
    while (1) {
        uint64_t distance = getCm(TRIG_PIN,ECHO_PIN);
        printf("%llu\n",distance);
    }
} 
*/
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "stdio.h"
#include "Lib/ultrasonic.h"

Ultrasonic ultrasonicsensor;

int main() {
    stdio_init_all();

    // Initialize GPIO
    ultrasonicsensor.ultrasonic_init();

    int16_t distance;

    sleep_ms(5000);

    while (1) {
        distance = ultrasonicsensor.ultrasonic_measure();
        printf("%d\n",distance);
    }
}


