#include "Sbus.h"
#include <cstdio>
int SBUS::dma_channel = dma_claim_unused_channel(true); // Statik üye değişkenin başlatılması
uint8_t SBUS::_sbusData[25];
uint16_t SBUS::_channels[16];
static uart_inst_t* static_uart; // Statik UART referansı


SBUS::SBUS(uart_inst_t* uart_port, uint baudrate) : _uart(uart_port), _baudrate(baudrate) {
    // DMA kanalını ayarla
    static_uart = _uart; 
    // dma_channel = dma_claim_unused_channel(true);
}

void SBUS::begin() {
    // UART ayarlarını yap
    uart_init(_uart, _baudrate);
    uart_set_format(_uart, 8, 2, UART_PARITY_EVEN);   
    gpio_set_function(uart_tx_pin, GPIO_FUNC_UART);
    gpio_set_function(uart_rx_pin, GPIO_FUNC_UART);
    setupDMA();
}



void SBUS::setupDMA() {
    dma_channel_config cfg = dma_channel_get_default_config(dma_channel);

    // Okuma yapılacak UART RX adresini belirle
    channel_config_set_dreq(&cfg, uart_get_dreq(_uart, false));
    channel_config_set_read_increment(&cfg, false);
    channel_config_set_write_increment(&cfg, false);
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_8);
    
    channel_config_set_ring(&cfg, true, 5);

    // DMA tamamlanma IRQ'sunu ayarla
    dma_channel_set_irq0_enabled(dma_channel, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dmaHandler);
    irq_set_enabled(DMA_IRQ_0, true);

    // DMA kanalını ve transferi başlat
    dma_channel_configure(dma_channel, &cfg,
                          _sbusData,           // Yazılacak adres (SBUS veri dizisi)
                          &uart_get_hw(_uart)->dr, // UART veri kaynağı
                          25,                  // Transfer boyutu
                          false);              // Otomatik olarak başlatma
}

void SBUS::update() {
    
    dma_channel_start(dma_channel);
    
}

// DMA tamamlanma işleyicisi
void SBUS::dmaHandler() {

    // DMA transferini sıfırla ve tekrar başlat
    dma_channel_set_read_addr(dma_channel, &uart_get_hw(static_uart)->dr, false);
    dma_channel_start(dma_channel);
    
    // SBUS verisini oku ve kanalları ayıkla
    _channels[0] = (_sbusData[1] | _sbusData[2] << 8) & 0x07FF;
    _channels[1] = (_sbusData[2] >> 3 | _sbusData[3] << 5) & 0x07FF;
    _channels[2] = (_sbusData[3] >> 6 | _sbusData[4] << 2 | _sbusData[5] << 10) & 0x07FF;
    _channels[3] = (_sbusData[5] >> 1 | _sbusData[6] << 7) & 0x07FF;
    _channels[4] = (_sbusData[6] >> 4 | _sbusData[7] << 4) & 0x07FF;
    _channels[5] = (_sbusData[7] >> 7 | _sbusData[8] << 1 | _sbusData[9] << 9) & 0x07FF;
    _channels[6] = (_sbusData[9] >> 2 | _sbusData[10] << 6) & 0x07FF;
    _channels[7] = (_sbusData[10] >> 5 | _sbusData[11] << 3) & 0x07FF;
    _channels[8] = (_sbusData[12] | _sbusData[13] << 8) & 0x07FF;
    _channels[9] = (_sbusData[13] >> 3 | _sbusData[14] << 5) & 0x07FF;
    _channels[10] = (_sbusData[14] >> 6 | _sbusData[15] << 2 | _sbusData[16] << 10) & 0x07FF;
    _channels[11] = (_sbusData[16] >> 1 | _sbusData[17] << 7) & 0x07FF;
    _channels[12] = (_sbusData[17] >> 4 | _sbusData[18] << 4) & 0x07FF;
    _channels[13] = (_sbusData[18] >> 7 | _sbusData[19] << 1 | _sbusData[20] << 9) & 0x07FF;
    _channels[14] = (_sbusData[20] >> 2 | _sbusData[21] << 6) & 0x07FF;
    _channels[15] = (_sbusData[21] >> 5 | _sbusData[22] << 3) & 0x07FF;
}




void SBUS::readChannels(uint16_t* channels) {
    // for(int i = 0; i < 16; i++) {
    //     channels[i] = _channels[i];
    
    // }
     channels[5] = _channels[5];
     printf("Kanal 1: %d\n", _channels[5]);
    
}
