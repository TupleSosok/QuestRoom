#include <avr/io.h>
#include <util/delay.h>


void uartTransmit(unsigned char data) {
    // Wait for empty transmit buffer
    while (!( UCSRA & (1<<UDRE)));
    // Put data into buffer, sends the data
    UDR = data;
}

unsigned char uartReceive(void) {
    // Wait for data to be received
    while (!(UCSRA & (1<<RXC)));
    // Get and return received data from buffer
    return UDR;
}

void uartTransmitString(const char* str) {
    while (*str) {
        uartTransmit(*str++);
    }
}

char* uartReceiveNumber(void) {
    static char buffer[5]; // Статический буфер для хранения строки числа
    for (uint8_t i = 0; i < 4; i++) {
        buffer[i] = uartReceive(); // Получение каждого символа числа
    }
    buffer[4] = '\0'; // Завершение строки

    return buffer; // Возвращение указателя на строку
}