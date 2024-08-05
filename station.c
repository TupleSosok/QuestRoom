#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "lib/encoder.h"
#include "lib/eeprom.h"
#include "lib/UART.h"
#include "lib/stepper.h"

int total_percent = 0;
int minHZ = 70;
int maxHZ = 120;
int steps = 100;
int needHZ = 115;

bool tumblerFrsflag = false;
bool tumblerSndflag = false;
bool tumblerTrdflag = false;
bool mobileConnected = false;

void init();
void makeNoise();
int generateRandomInt();
unsigned char* generateRandomKey();
void enterKey();
void checkMobileConnection();
void waitFstTumbler();
void setSettings();
void waitSndTumbler();
void packetTransfer();

void init()
{
    srand(time(TCNT0));

    // STEPPER
    DDR_STEPPER_PA |= (1<<STEPPER_PA); // выход
    PORT_STEPPER_PA &= ~(1<<STEPPER_PA); // выключен

    DDR_STEPPER_PB |= (1<<STEPPER_PB); // выход
    PORT_STEPPER_PB &= ~(1<<STEPPER_PB); // выключен

    DDR_STEPPER_NA |= (1<<STEPPER_NA); // выход
    PORT_STEPPER_NA &= ~(1<<STEPPER_NA); // выключен

    DDR_STEPPER_NB |= (1<<STEPPER_NB); // выход
    PORT_STEPPER_NB &= ~(1<<STEPPER_NB); // выключен

    // ENCODER 1
    DDR_ENCODER_1_DT &= ~(1<<ENCODER_1_DT); // вход
    PORT_ENCODER_1_DT |= (1<<ENCODER_1_DT); // выключен
    DDR_ENCODER_1_CLK &= ~(1<<ENCODER_1_CLK); // вход
    PORT_ENCODER_1_CLK |= (1<<ENCODER_1_CLK); // выключен

    // ENCODER 2
    DDR_ENCODER_2_DT &= ~(1<<ENCODER_2_DT); // вход
    PORT_ENCODER_2_DT |= (1<<ENCODER_2_DT); // выключен
    DDR_ENCODER_2_CLK &= ~(1<<ENCODER_2_CLK); // вход
    PORT_ENCODER_2_CLK |= (1<<ENCODER_2_CLK); // выключен

    // SPEAKER
    DDR_SPEAKER |= (1<<SPEAKER); // выход
    PORT_SPEAKER &= ~(1<<SPEAKER); // выключен

    // TUMBLER 1
    DDR_TUMBLER_1 &= ~(1<<TUMBLER_1); // вход
    PORT_TUMBLER_1 |= (1<<TUMBLER_1); // выключен

    // TUMBLER 2
    DDR_TUMBLER_2 &= ~(1<<TUMBLER_2); // вход
    PORT_TUMBLER_2 |= (1<<TUMBLER_2); // выключен

    // TUMBLER 3
    DDR_TUMBLER_3 &= ~(1<<TUMBLER_3); // вход
    PORT_TUMBLER_3 |= (1<<TUMBLER_3); // выключен

    // LED 1 R
    DDR_LED_1_R |= (1<<LED_1_R); // выход
    PORT_LED_1_R &= ~(1<<LED_1_R); // выключен

    // LED 1 Y
    DDR_LED_1_Y |= (1<<LED_1_Y); // выход
    PORT_LED_1_Y &= ~(1<<LED_1_Y); // выключен

    // LED 1 G
    DDR_LED_1_G |= (1<<LED_1_G); // выход
    PORT_LED_1_G &= ~(1<<LED_1_G); // выключен

    // LED 2 R
    DDR_LED_2_R |= (1<<LED_2_R); // выход
    PORT_LED_2_R &= ~(1<<LED_2_R); // выключен

    // LED 2 Y
    DDR_LED_2_Y |= (1<<LED_2_Y); // выход
    PORT_LED_2_Y &= ~(1<<LED_2_Y); // выключен

    // LED 2 G
    DDR_LED_2_G |= (1<<LED_2_G); // выход
    PORT_LED_2_G &= ~(1<<LED_2_G); // выключен

    // LED 3 R
    DDR_LED_3_R |= (1<<LED_3_R); // выход
    PORT_LED_3_R &= ~(1<<LED_3_R); // выключен

    // LED 3 Y
    DDR_LED_3_Y |= (1<<LED_3_Y); // выход
    PORT_LED_3_Y &= ~(1<<LED_3_Y); // выключен

    // LED 3 G
    DDR_LED_3_G |= (1<<LED_3_G); // выход
    PORT_LED_3_G &= ~(1<<LED_3_G); // выключен

    // UART
    UBRRL=round(F_CPU/(16*9600-1.0)); //103
	UCSRB = (1<<RXEN) | (1<<TXEN);
	UCSRC = (1<<URSEL) | (1<<UCSZ0) | (1<<UCSZ1);
    
    sei(); // Enable global interrupts
}

void makeNoise(){
    PORT_SPEAKER |= (1<<SPEAKER);
    _delay_ms(3);
    PORT_SPEAKER &= ~(1<<SPEAKER);
}

int generateRandomInt(int min, int max)
{
    int angle = rand()%max + min;
    return angle;
}

unsigned char* generateRandomKey()
{
    int8_t num = EEPROMRead(1);
    int8_t newNum = num + 1;
    EEPROMWrite(1, newNum);
    srand(newNum);
    
    static unsigned char number_str[5];
    for (int i = 0; i < 4; i++)
    {
        number_str[i] = '0' + rand()%10;
    }
    number_str[4] = '\0';
    return number_str;
}

//Надо +1 к первой цифры, если 9 тогда это было 0
void enterKey() {
    unsigned char* key = generateRandomKey();
    unsigned char* visibleCode = (unsigned char*)malloc(sizeof(unsigned char) * (strlen(key) + 1));;
    strcpy(visibleCode, key);

    // //Перетасовка кода и того что показывается
    unsigned char firstNum = visibleCode[0];
    //48 код нуля
    if(firstNum == '0') 
        firstNum = '9';
    else 
        --firstNum;
    visibleCode[0] = firstNum;

    unsigned char response;
    bool received_responce = false;
    while (!received_responce) {
        uartTransmitString(visibleCode);
        response = uartReceive();
        if (response == 'C')
        {
            bool received_accept = false;
            while (!received_accept)
            {
                char* responce_password = uartReceiveNumber();
                if (strcmp(responce_password, key) == 0)
                {
                    uartTransmit('D');
                    received_accept = true;
                    received_responce = true;
                    PORT_LED_1_Y &= ~(1<<LED_1_Y);
                    PORT_LED_1_G |= (1<<LED_1_G);
                }
                else
                {
                    uartTransmit('E');
                    makeNoise();
                }
            }
        }
    }
}

void checkMobileConnection() {
    unsigned char request = 'A';
    unsigned char response;
    bool received_responce = false;
    while (!received_responce) {
        uartTransmit(request);
        _delay_ms(100);

        response = uartReceive();
        if (response == 'B') {
            received_responce = true;
        }
    }
    PORT_LED_1_R &= ~(1<<LED_1_R);
    PORT_LED_1_Y |= (1<<LED_1_Y);
    enterKey();
}

void waitFstTumbler() {
    while (true)
    {
        if (!(PIN_TUMBLER_1 & (1 << TUMBLER_1)))
        {
            tumblerFrsflag = true;
            checkMobileConnection();
            break;
        }
    }
}

void setSettings() {
    int8_t position1 = 10, position2 = minHZ;
    while ((PIN_TUMBLER_3 & (1 << TUMBLER_3)))
    {
        int8_t direction1 = getEncoderDirection(1);
        if(direction1 > 0){
            position1 += 1;
            stepperStep(1); 
        }
        if(direction1 < 0){
            position1 -= 1;
            stepperStep(-1); 
        }

        if(direction1 != 0){
            if(position1 > 100) position1 = 0;
            if(position1 < 0) position1 = 100;
        }
        
    
        int8_t direction2 = getEncoderDirection(2);
        if(direction2 > 0){
            position2 += 1;
        }
        if(direction2 < 0){
            position2 -= 1;
        }
    
        if(direction2 != 0){
            if(position2 > maxHZ) position2 = maxHZ;
            if(position2 < minHZ) position2 = minHZ;
        }

        if(direction1 != 0 || direction2 != 0){
            uartTransmit((char)position1);
            uartTransmit((char)position2);

            int hzProcent = 50 - abs(needHZ - position2);
            int positionProcent = position1 / 2;
            total_percent = positionProcent + hzProcent;

            if(total_percent > 85)
                PORT_LED_2_Y |= (1<<LED_2_Y);
            else
                PORT_LED_2_Y &= ~(1<<LED_2_Y);

            if(total_percent > 97)
                PORT_LED_2_G |= (1<<LED_2_G);
            else
                PORT_LED_2_G &= ~(1<<LED_2_G);
        }
    }
    uartTransmit((char)255);
    uartTransmit((char)255);
    PORT_LED_2_Y &= ~(1<<LED_2_Y);
    PORT_LED_2_R &= ~(1<<LED_2_R);
}

void waitSndTumbler() {
    while (true)
    {
        if (!(PIN_TUMBLER_2 & (1 << TUMBLER_2)))
        {
            tumblerSndflag = true;
            setSettings();
            break;
        }
    }
}

void packetTransfer(){
    PORT_LED_3_Y |= (1<<LED_3_Y);
    int mistakes = 0;
    for(int i = 0; i < 100 && mistakes < 3; ++i){
        _delay_ms(200);
        int chance = rand() % 100;
        if(chance < total_percent){
            uartTransmit((char)i);
            mistakes = 0;
        }
        else{
            PORT_LED_3_Y &= ~(1<<LED_3_Y);
            _delay_ms(10);
            PORT_LED_3_Y |= (1<<LED_3_Y);
            ++mistakes;
            makeNoise();
        }
    }
    if(mistakes < 3) {
        PORT_LED_3_G |= (1<<LED_3_G);
        uartTransmit((char)255);
    }
    else{
        PORT_LED_3_Y &= ~(1<<LED_3_Y);
        uartTransmit((char)254);
    }
    PORT_LED_3_Y &= ~(1<<LED_3_Y);
    PORT_LED_3_R &= ~(1<<LED_3_R);
}

int main()
{
    init();
    PORT_LED_1_R |= (1<<LED_1_R);
    PORT_LED_2_R |= (1<<LED_2_R);
    PORT_LED_3_R |= (1<<LED_3_R);


    waitFstTumbler();

    waitSndTumbler();
    
    packetTransfer();
}