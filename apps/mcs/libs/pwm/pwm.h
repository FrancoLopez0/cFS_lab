#ifndef PWM_H
#define PWM_H

#include <stdint.h>
#include <stdbool.h>

#define PWM_DEFAULT_PERIOD_NS 1000000ul

typedef struct{
    unsigned int chip;
    unsigned int channel;

    unsigned long period_ns;
    float duty_percent;
    unsigned long duty_ns;

    bool enabled;
    bool initialized;
} pwm_t;


int PWM_Init(pwm_t *pwm, unsigned int chip, unsigned int channel, unsigned long period_ns);
int PWM_SetDutyPercent(pwm_t *pwm, float duty_percent);
int PWM_Enable(pwm_t *pwm);
int PWM_Disable(pwm_t *pwm);
int PWM_Close(pwm_t *pwm);



#endif