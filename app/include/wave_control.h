#ifndef SOLDEROTTO_H
#define SOLDEROTTO_H

struct wave_control {
    uint16_t ton;
    uint16_t tperiod;
    uint16_t count;
};

int wave_control_init();

#endif