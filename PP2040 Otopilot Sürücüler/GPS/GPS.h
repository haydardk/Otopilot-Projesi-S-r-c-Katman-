#ifndef GPS_H
#define GPS_H

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

#define UART_ID uart0
#define BAUD_RATE 9600
#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define BUFFER_SIZE 300

class GPS {
private:
    uint8_t GPS_Buffer[BUFFER_SIZE];
    char g_time[10];
    char g_status;
    char g_ns;
    char g_ew;
    char g_date[7];
    float g_latitude;
    float g_longitude;
    float g_speed;
    float g_hdop;
    float g_altitude;
    int g_fix_quality;
    int g_num_satellites;

    void parse_GNRMC(uint8_t *sentence);
    void parse_GNGGA(uint8_t *sentence);
    void parse_GPS_data(uint8_t *buffer);
    long convert_gps_time_to_long(const char* time_str);

public:
    GPS();
    void init();
    void update();
    float get_latitude() const;
    float get_longitude() const;
    float get_speed() const;
    float get_altitude() const;
    int get_fix_quality() const;
    int get_num_satellites() const;
};

#endif // GPS_H



