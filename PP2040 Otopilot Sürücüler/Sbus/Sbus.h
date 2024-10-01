#ifndef SBUS_H_
#define SBUS_H_

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "hardware/dma.h"




class SBUS {
public:

    static int dma_channel;
    static uint8_t _sbusData[25];
    static uint16_t _channels[16];
    
    SBUS(uart_inst_t* uart_port, uint baudrate);
    void readChannels(uint16_t* channels);
    void begin();
    void update();
    

private:
    uart_inst_t* _uart;
    uint _baudrate;
    const uint uart_tx_pin = 4;  // GPIO numarası 4
    const uint uart_rx_pin = 5;  // GPIO numarası 5
    
  
    void setupDMA();
    static void dmaHandler();

   
};

#endif // SBUS_H_
