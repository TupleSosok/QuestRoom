#include <avr/io.h>

// Функция для установки линии данных
void lcd_set_data(unsigned char data) {
    if (data & 1<<4) PORTD |= (1<<LCD_D4); else PORTD &= ~(1<<LCD_D4);
    if (data & 1<<5) PORTD |= (1<<LCD_D5); else PORTD &= ~(1<<LCD_D5);
    if (data & 1<<6) PORTB |= (1<<LCD_D6); else PORTB &= ~(1<<LCD_D6);
    if (data & 1<<7) PORTB |= (1<<LCD_D7); else PORTB &= ~(1<<LCD_D7);
}

// Функция для отправки команды на LCD
void lcd_command(unsigned char cmnd) {
    PORTB &= ~(1<<LCD_RS); // RS = 0 для команды
    lcd_set_data(cmnd);
    PORTD |= (1<<LCD_EN);  // EN = 1 для строба
    _delay_us(1);
    PORTD &= ~(1<<LCD_EN); // EN = 0 для завершения строба
    _delay_us(200);
    lcd_set_data(cmnd << 4);
    PORTD |= (1<<LCD_EN);  // EN = 1 для строба
    _delay_us(1);
    PORTD &= ~(1<<LCD_EN); // EN = 0 для завершения строба
    _delay_ms(2);
}

// Функция для отправки данных на LCD
void lcd_data(unsigned char data) {
    PORTB |= (1<<LCD_RS); // RS = 1 для данных
    lcd_set_data(data);
    PORTD |= (1<<LCD_EN);  // EN = 1 для строба
    _delay_us(1);
    PORTD &= ~(1<<LCD_EN); // EN = 0 для завершения строба
    _delay_us(200);
    lcd_set_data(data << 4);
    PORTD |= (1<<LCD_EN);  // EN = 1 для строба
    _delay_us(1);
    PORTD &= ~(1<<LCD_EN); // EN = 0 для завершения строба
    _delay_ms(2);
}

// Инициализация LCD
void lcd_init(void) {
    DDRB = 0xFF; // Установить порт B как выходной
    DDRD = 0xFF; // Установить порт D как выходной
    _delay_ms(20); // Ожидание 20 мс
    lcd_command(0x02); // 4-битный режим
    lcd_command(0x28); // 2 строки, 5x7 матрица
    lcd_command(0x0C); // Включить дисплей, курсор выкл.
    lcd_command(0x06); // Инкремент адресация
    lcd_command(0x01); // Очистить дисплей
    _delay_ms(2);
}

// Функция для вывода строки на LCD
void lcd_string(const char *str) {
    while (*str) {
        lcd_data(*str++);
    }
}

void lcd_char(char str) {
    lcd_data(str);
}

void lcd_set_cursor(unsigned char col, unsigned char row) {
    unsigned char address;

    // Вычисление адреса в зависимости от строки
    switch (row) {
        case 0: address = col; break;
        case 1: address = 0x40 + col; break;
        default: address = col; break;
    }

    // Установка адреса DDRAM
    lcd_command(0x80 | address);
}

void lcd_clear(void) {
    lcd_command(0x01); // Команда очистки экрана
    _delay_ms(2);      // Задержка для выполнения команды
}