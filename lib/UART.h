#include <avr/io.h>
#include <util/delay.h>


void uart_transmit(unsigned char data) {
    // Wait for empty transmit buffer
    while (!( UCSRA & (1<<UDRE)));
    // Put data into buffer, sends the data
    UDR = data;
}

unsigned char uart_receive(void) {
    // Wait for data to be received
    while (!(UCSRA & (1<<RXC)));
    // Get and return received data from buffer
    return UDR;
}

void uart_transmit_string(const char* str) {
    while (*str) {
        uart_transmit(*str++);
    }
}

char* uart_receive_number(void) {
    static char buffer[5]; // Статический буфер для хранения строки числа
    for (uint8_t i = 0; i < 4; i++) {
        buffer[i] = uart_receive(); // Получение каждого символа числа
    }
    buffer[4] = '\0'; // Завершение строки

    return buffer; // Возвращение указателя на строку
}