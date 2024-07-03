#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include "lib/lcd.h"
#include "lib/UART.h"
#include "lib/matrix_keypad.h"
#include <string.h>

const uint8_t row_pins[4] = {3, 2, 1, 0};
const uint8_t col_pins[3] = {4, 3, 2};

void init();
void waitForConnection();
void enterKey(char* request);
void receiveKey();
int getNumFromUART();
void getSettings();
bool acceptingPackages();

void init()
{
    // DDR_LCD_SDA |= (1<<LCD_SDA);
    // PORT_LCD_SDA &= ~(1<<LCD_SDA);

    // DDR_LCD_SCL |= (1<<LCD_SCL);
    // PORT_LCD_SCL &= ~(1<<LCD_SCL);

    DDR_KEY_R1 &= ~(1<<KEY_R1);
    PORT_KEY_R1 |= (1<<KEY_R1);

    DDR_KEY_R2 &= ~(1<<KEY_R2);
    PORT_KEY_R2 |= (1<<KEY_R2);

    DDR_KEY_R3 &= ~(1<<KEY_R3);
    PORT_KEY_R3 |= (1<<KEY_R3);

    DDR_KEY_R4 &= ~(1<<KEY_R4);
    PORT_KEY_R4 |= (1<<KEY_R4);

    DDR_KEY_C1 &= ~(1<<KEY_C1);
    PORT_KEY_C1 |= (1<<KEY_C1);

    DDR_KEY_C2 &= ~(1<<KEY_C2);
    PORT_KEY_C2 |= (1<<KEY_C2);

    DDR_KEY_C3 &= ~(1<<KEY_C3);
    PORT_KEY_C3 |= (1<<KEY_C3);

    UBRRL=round(F_CPU/(16*9600-1.0));//103
	UCSRB = (1<<RXEN) | (1<<TXEN);
	UCSRC=(1<<URSEL)|(3<<UCSZ0)|(1<<UCSZ1);

    sei();
}

void waitForConnection()
{
    unsigned char request;
    unsigned char response = 'B';
    bool received_request = false;
    while (!received_request) {
        request = uart_receive();
        //lcd_char(request);

        if (request == 'A') {
            received_request = true;
            uart_transmit(response);
            lcd_string("Initialize...");
        }
    }
}

void enterKey(char* request)
{
    char key;
    int try_count = 0;
    char password[5];
    int index = 0;
    bool enter_flag = false;
    memset(password, 0, sizeof(password));
    while (!enter_flag)
    {
        key = keypad_get_key(row_pins, col_pins);
        if (key)
        {
            if (key != '#' && key != '*')
            {
                if (index < 4)
                {
                    password[index] = key;
                    index++;
                    lcd_char('*');
                }
            }
            else if (key == '#')
            {
                try_count++;
                password[index] = '\0';
                uart_transmit_string(password);
                char answer = uart_receive();
                if (answer == 'D')
                {
                    lcd_clear();
                    lcd_set_cursor(0, 0);
                    lcd_string("Correct!");
                    enter_flag = true;
                }
                else if (answer == 'E')
                {
                    lcd_clear();
                    lcd_set_cursor(0, 0);
                    lcd_string("Incorrect!");
                    lcd_set_cursor(0,1);
                    lcd_string("Popitok: ");
                    lcd_char(((3 - try_count) + '0'));
                    index = 0;
                    memset(password, 0, sizeof(password));
                    _delay_ms(1000);
                    lcd_clear();
                    lcd_set_cursor(0, 0);
                    lcd_string(request);
                    lcd_set_cursor(0,1);
                }
            }
            else if (key == '*')
            {
                index = 0;
                memset(password, 0, sizeof(password));
                lcd_clear();
                lcd_string(request);
                lcd_set_cursor(0,1);
            }
        }
        if (try_count > 2)
        {
            lcd_clear();
            lcd_set_cursor(0, 0);
            lcd_string("Popitok nema!");
            enter_flag = true;
        }
    }
}

void receiveKey()
{
    char* request;
    bool received_responce = false;
    while (!received_responce) {
        request = uart_receive_number();
        uart_transmit('C');
        lcd_clear();
        lcd_string(request);
        lcd_set_cursor(0,1);
        received_responce = true;
        enterKey(request);
    }
}

int getNumFromUART(){
    return (int) uart_receive();
}

void getSettings()
{
    //0 - 100 проценты заполнения
    //255 - окончание
    lcd_clear();
    lcd_set_cursor(0,0);
    char procentStr[4] = "";
    char hzStr[4] = "";
    int procent = (int) uart_receive();
    int hz = (int) uart_receive();
    while(procent != 255 && hz != 255){
        lcd_clear();
        
        itoa(procent, procentStr, 10);
        itoa(hz,hzStr, 10);

        lcd_string(procentStr);
        lcd_char('%');
        lcd_char(' ');
        lcd_string("HZ");
        lcd_string(hzStr);

        procent = (int) uart_receive();
        hz = (int) uart_receive();
    }
}

//если 3 пакета потерялись то все
bool acceptingPackages(){
    lcd_set_cursor(0,0);
    char str[4] = "";
    int num = (int) uart_receive();
    int mistakes = 0;
    int number_of_package = 0;
    while(num != 255 && mistakes < 3){
        if(num == number_of_package){
            ++number_of_package;
            lcd_clear();
            itoa(num, str, 10);
            lcd_string(str);
            mistakes = 0;
        }
        else{
            lcd_clear();
            lcd_string("Missing");
            mistakes += num - number_of_package;
            number_of_package = num;
        }
        num = (int) uart_receive();
    }
    if(mistakes >= 3){
        lcd_clear();
        lcd_string("Errore");
        return false;
    }
    return true;
}

int main()
{
    init();
    lcd_init(); // Инициализация LCD
    bool rerole = true;
    while(rerole){
        keypad_init(row_pins, col_pins); 
        waitForConnection();
        receiveKey();
        getSettings();
        rerole = !acceptingPackages();
    }
    lcd_clear();
    lcd_string("Good Job");
    return 0;
}
