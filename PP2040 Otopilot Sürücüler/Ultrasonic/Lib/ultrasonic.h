#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "stdio.h"

#define TRIG_PIN 14
#define ECHO_PIN 15
#define MAX_DISTANCE 200 // Santimetre

class Ultrasonic{
    public: 
        Ultrasonic();
        void ultrasonic_init (void);
        int16_t ultrasonic_measure(void);
        static void static_echo_callback(uint gpio, uint32_t events);
    private: 
        void ultrasonic_trig(void);
        void echo_callback(uint gpio, uint32_t events);

        static Ultrasonic* instance;

        volatile uint32_t start_time = 0;
        volatile uint32_t end_time = 0;
        volatile bool echo_received = false;
};