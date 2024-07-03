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
        request = uartReceive();
        //lcdChar(request);

        if (request == 'A') {
            received_request = true;
            uartTransmit(response);
            lcdString("Initialize...");
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
        key = keypadGetKey(row_pins, col_pins);
        if (key)
        {
            if (key != '#' && key != '*')
            {
                if (index < 4)
                {
                    password[index] = key;
                    index++;
                    lcdChar('*');
                }
            }
            else if (key == '#')
            {
                try_count++;
                password[index] = '\0';
                uartTransmitString(password);
                char answer = uartReceive();
                if (answer == 'D')
                {
                    lcdClear();
                    lcdSetCursor(0, 0);
                    lcdString("Correct!");
                    enter_flag = true;
                }
                else if (answer == 'E')
                {
                    lcdClear();
                    lcdSetCursor(0, 0);
                    lcdString("Incorrect!");
                    lcdSetCursor(0,1);
                    lcdString("Popitok: ");
                    lcdChar(((3 - try_count) + '0'));
                    index = 0;
                    memset(password, 0, sizeof(password));
                    _delay_ms(1000);
                    lcdClear();
                    lcdSetCursor(0, 0);
                    lcdString(request);
                    lcdSetCursor(0,1);
                }
            }
            else if (key == '*')
            {
                index = 0;
                memset(password, 0, sizeof(password));
                lcdClear();
                lcdString(request);
                lcdSetCursor(0,1);
            }
        }
        if (try_count > 2)
        {
            lcdClear();
            lcdSetCursor(0, 0);
            lcdString("Popitok nema!");
            enter_flag = true;
        }
    }
}

void receiveKey()
{
    char* request;
    bool received_responce = false;
    while (!received_responce) {
        request = uartReceiveNumber();
        uartTransmit('C');
        lcdClear();
        lcdString(request);
        lcdSetCursor(0,1);
        received_responce = true;
        enterKey(request);
    }
}

int getNumFromUART(){
    return (int) uartReceive();
}

void getSettings()
{
    //0 - 100 проценты заполнения
    //255 - окончание
    lcdClear();
    lcdSetCursor(0,0);
    char procentStr[4] = "";
    char hzStr[4] = "";
    int procent = (int) uartReceive();
    int hz = (int) uartReceive();
    while(procent != 255 && hz != 255){
        lcdClear();
        
        itoa(procent, procentStr, 10);
        itoa(hz,hzStr, 10);

        lcdString(procentStr);
        lcdChar('%');
        lcdChar(' ');
        lcdString("HZ");
        lcdString(hzStr);

        procent = (int) uartReceive();
        hz = (int) uartReceive();
    }
}

//если 3 пакета потерялись то все
bool acceptingPackages(){
    lcdSetCursor(0,0);
    char str[4] = "";
    int num = (int) uartReceive();
    int mistakes = 0;
    int number_of_package = 0;
    while(num != 255 && mistakes < 3){
        if(num == number_of_package){
            ++number_of_package;
            lcdClear();
            itoa(num, str, 10);
            lcdString(str);
            mistakes = 0;
        }
        else{
            lcdClear();
            lcdString("Missing");
            mistakes += num - number_of_package;
            number_of_package = num;
        }
        num = (int) uartReceive();
    }
    if(mistakes >= 3){
        lcdClear();
        lcdString("Errore");
        return false;
    }
    return true;
}

int main()
{
    init();
    lcdInit(); // Инициализация LCD
    bool rerole = true;
    while(rerole){
        keypadInit(row_pins, col_pins); 
        waitForConnection();
        receiveKey();
        getSettings();
        rerole = !acceptingPackages();
    }
    lcdClear();
    lcdString("Good Job");
    return 0;
}
