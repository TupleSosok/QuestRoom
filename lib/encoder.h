#include <util/delay.h>
#include <avr/io.h>

uint8_t readEncoder(uint8_t num) 
{ 
    uint8_t val=0; 
    bool encoderBit1;
    bool encoderBit2;
    switch (num)
    {
    case 1:
        encoderBit1 = !bit_is_clear(PIN_ENCODER_1_DT, ENCODER_1_DT);
        encoderBit2 = !bit_is_clear(PIN_ENCODER_1_CLK, ENCODER_1_CLK);
        break;
    case 2:
        encoderBit1 = !bit_is_clear(PIN_ENCODER_2_DT, ENCODER_2_DT);
        encoderBit2 = !bit_is_clear(PIN_ENCODER_2_CLK, ENCODER_2_CLK);
        break;
    default:
        break;
    }

    if(encoderBit1) val |= (1<<1); 
    if(encoderBit2) val |= (1<<0); 
    return val; 
}

int8_t getEncoderDirection(uint8_t encoder){
    int8_t direction = 0;
    uint8_t current_pos = readEncoder(encoder);
    uint8_t last_pos = current_pos;
    //чтобы прокрутки не было
    _delay_ms(1);

    current_pos = readEncoder(encoder);

    if(current_pos != last_pos) 
    { 
        if((last_pos==3 && current_pos==1) || (last_pos==0 && current_pos==2)) 
            direction = 1;
        if((last_pos==2 && current_pos==0) || (last_pos==1 && current_pos==3)) 
            direction = -1;
    } 
    return direction;
}