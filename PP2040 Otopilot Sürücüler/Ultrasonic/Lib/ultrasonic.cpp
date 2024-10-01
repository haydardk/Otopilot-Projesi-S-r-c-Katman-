#include "Lib/ultrasonic.h"

Ultrasonic* Ultrasonic::instance = nullptr; 

Ultrasonic::Ultrasonic() {
    instance = this; // Statik instance'ı ayarla
}

void Ultrasonic::ultrasonic_init(void){
    gpio_init(TRIG_PIN);
    gpio_set_dir(TRIG_PIN, GPIO_OUT);
    gpio_init(ECHO_PIN);
    gpio_set_dir(ECHO_PIN, GPIO_IN);
    gpio_set_irq_enabled_with_callback(ECHO_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &Ultrasonic::static_echo_callback);
}

void Ultrasonic::ultrasonic_trig(void){
    gpio_put(TRIG_PIN, 1);
    sleep_us(10);
    gpio_put(TRIG_PIN, 0);
}

int16_t Ultrasonic::ultrasonic_measure(void){
    ultrasonic_trig();
    while (!echo_received) {
            tight_loop_contents();  // Wait in low-power mode
        }

        uint32_t pulse_duration = end_time - start_time;
        float distance_cm = (pulse_duration / 2.0f) * 0.0343f;

        int16_t dist16t = (int16_t) distance_cm;
        if(dist16t > MAX_DISTANCE){
            dist16t = MAX_DISTANCE;
        }
        echo_received = false;
        return dist16t;
}

void Ultrasonic::static_echo_callback(uint gpio, uint32_t events){
    // Sınıfın örneğini kullanarak non-static fonksiyon çağrısı yapıyoruz.
    instance->echo_callback(gpio, events);
}

// Gerçek callback işlemi
void Ultrasonic::echo_callback(uint gpio, uint32_t events){
    if (gpio == ECHO_PIN) {
        if (gpio_get(ECHO_PIN)) {  // Rising edge (pulse start)
            start_time = time_us_32();
        } else {  // Falling edge (pulse end)
            end_time = time_us_32();
            echo_received = true;
        }
    }
}