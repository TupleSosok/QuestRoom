#include <util/delay.h>

int current_speed = 7;
bool reverse;
bool on;

void delay()
{
    switch(current_speed){
        case 0: _delay_ms(8192);
        case 1: _delay_ms(4096);
        case 2: _delay_ms(2048);
        case 3: _delay_ms(1024);
        case 4: _delay_ms(512);
        case 5: _delay_ms(256);
        case 6: _delay_ms(64);
        case 7: _delay_ms(32);
    }
}

void clockWiseStep(int times)
{
    for(int i = 0; i < times; ++i)
    {
        PORT_STEPPER_PA ^= (1<<STEPPER_PA);
        PORT_STEPPER_PA &= ~(1<<STEPPER_PA);
        delay();

        PORT_STEPPER_PB ^= (1<<STEPPER_PB);
        PORT_STEPPER_PB &= ~(1<<STEPPER_PB);
        delay();

        PORT_STEPPER_NA ^= (1<<STEPPER_NA);
        PORT_STEPPER_NA &= ~(1<<STEPPER_NA);
        delay();

        PORT_STEPPER_NB ^= (1<<STEPPER_NB);
        PORT_STEPPER_NB &= ~(1<<STEPPER_NB);
        delay();           
    }
}

void counterClockWiseStep(int times)
{  
    for(int i = 0; i < times; ++i)
    {
        PORT_STEPPER_NB ^= (1<<STEPPER_NB);
        PORT_STEPPER_NB &= ~(1<<STEPPER_NB);
        delay();

        PORT_STEPPER_NA ^= (1<<STEPPER_NA);
        PORT_STEPPER_NA &= ~(1<<STEPPER_NA);
        delay();

        PORT_STEPPER_PB ^= (1<<STEPPER_PB);
        PORT_STEPPER_PB &= ~(1<<STEPPER_PB);
        delay();

        PORT_STEPPER_PA ^= (1<<STEPPER_PA);
        PORT_STEPPER_PA &= ~(1<<STEPPER_PA);
        delay();    
    }       
}

void stepperStep(int direction) {
    // Направление вращения
    if (direction > 0) {
        clockWiseStep(direction);
    } 
    else if (direction < 0) {
        counterClockWiseStep(direction * -1);
    }
}
