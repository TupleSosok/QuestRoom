#ifndef MATRIX_KEYPAD_H
#define MATRIX_KEYPAD_H

#include <avr/io.h>

// Define the number of rows and columns
#define NUM_ROWS 4
#define NUM_COLS 3

const char keymap[NUM_ROWS][NUM_COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}
};

// Function prototypes
void keypadInit(const uint8_t rows[], const uint8_t cols[])
{
        // Set rows as outputs and initialize to high
    for (uint8_t i = 0; i < NUM_ROWS; i++) {
        uint8_t row_pin = rows[i];
        DDRC |= (1 << row_pin);
        PORTC |= (1 << row_pin);
    }
    
    // Set columns as inputs and enable pull-up resistors
    for (uint8_t i = 0; i < NUM_COLS; i++) {
        uint8_t col_pin = cols[i];
        DDRB &= ~(1 << col_pin);
        PORTB |= (1 << col_pin);
    }
}
char keypadGetKey(const uint8_t rows[], const uint8_t cols[])
{
        for (uint8_t row = 0; row < NUM_ROWS; row++) {
        // Set the current row low
        uint8_t row_pin = rows[row];
        PORTC &= ~(1 << row_pin);
        
        // Read columns
        for (uint8_t col = 0; col < NUM_COLS; col++) {
            uint8_t col_pin = cols[col];
            if (!(PINB & (1 << col_pin))) {
                // Wait for key release
                while (!(PINB & (1 << col_pin)));
                // Set the current row high again
                PORTC |= (1 << row_pin);
                // Return the corresponding key from the keymap
                return keymap[row][col];
            }
        }
        
        // Set the current row high again
        PORTC |= (1 << row_pin);
    }
    
    // No key pressed
    return 0;
}

#endif // MATRIX_KEYPAD_H
