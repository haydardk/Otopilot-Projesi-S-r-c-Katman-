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

// Constructor
GPS::GPS() {
    // Initialize member variables if needed
}

// Initialize UART and GPIO settings
void GPS::init() {
    stdio_init_all();
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
}

// Parse GNRMC sentence
void GPS::parse_GNRMC(uint8_t *sentence) {
    char lat[11], lon[12];
    sscanf(reinterpret_cast<char*>(sentence), "$GNRMC,%9[^,],%c,%10[^,],%c,%11[^,],%c,%f,,%6[^,]",
           g_time, &g_status, lat, &g_ns, lon, &g_ew, &g_speed, g_date);

    int lat_deg = static_cast<int>(atof(lat) / 100);
    float lat_min = atof(lat) - (lat_deg * 100);
    g_latitude = lat_deg + (lat_min / 60.0);

    int lon_deg = static_cast<int>(atof(lon) / 100);
    float lon_min = atof(lon) - (lon_deg * 100);
    g_longitude = lon_deg + (lon_min / 60.0);

    if (g_ns == 'S') g_latitude = -g_latitude;
    if (g_ew == 'W') g_longitude = -g_longitude;

    g_speed = g_speed * 1.852;
}

// Parse GNGGA sentence
void GPS::parse_GNGGA(uint8_t *sentence) {
    char lat[11], lon[12];
    sscanf(reinterpret_cast<char*>(sentence), "$GNGGA,%9[^,],%10[^,],%c,%11[^,],%c,%d,%d,%f,%f",
           g_time, lat, &g_ns, lon, &g_ew, &g_fix_quality, &g_num_satellites, &g_hdop, &g_altitude);

    int lat_deg = static_cast<int>(atof(lat) / 100);
    float lat_min = atof(lat) - (lat_deg * 100);
    g_latitude = lat_deg + (lat_min / 60.0);

    int lon_deg = static_cast<int>(atof(lon) / 100);
    float lon_min = atof(lon) - (lon_deg * 100);
    g_longitude = lon_deg + (lon_min / 60.0);

    if (g_ns == 'S') g_latitude = -g_latitude;
    if (g_ew == 'W') g_longitude = -g_longitude;
}

// Parse GPS data
void GPS::parse_GPS_data(uint8_t *buffer) {
    uint8_t *line = strtok(reinterpret_cast<char*>(buffer), "\n");
    while (line != NULL) {
        if (strncmp(reinterpret_cast<char*>(line), "$GNRMC", 6) == 0) {
            parse_GNRMC(line);
        } else if (strncmp(reinterpret_cast<char*>(line), "$GNGGA", 6) == 0) {
            parse_GNGGA(line);
        }
        line = strtok(NULL, "\n");
    }
}

// Convert GPS time to long
long GPS::convert_gps_time_to_long(const char* time_str) {
    char hours[3] = {time_str[0], time_str[1], '\0'};
    char minutes[3] = {time_str[2], time_str[3], '\0'};
    char seconds[3] = {time_str[4], time_str[5], '\0'};

    long time_in_seconds = atol(hours) * 3600 + atol(minutes) * 60 + atol(seconds);
    return time_in_seconds;
}

// Update GPS data
void GPS::update() {
    uart_read_blocking(UART_ID, GPS_Buffer, BUFFER_SIZE);
    parse_GPS_data(GPS_Buffer);

    long gps_time_long = convert_gps_time_to_long(g_time);
}

// Getters
float GPS::get_latitude() const { return g_latitude; }
float GPS::get_longitude() const { return g_longitude; }
float GPS::get_speed() const { return g_speed; }
float GPS::get_altitude() const { return g_altitude; }
int GPS::get_fix_quality() const { return g_fix_quality; }
int GPS::get_num_satellites() const { return g_num_satellites; }

