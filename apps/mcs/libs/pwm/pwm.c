#include "pwm.h"

#include <stdio.h>   /* snprintf, perror */
#include <string.h>  /* strlen */
#include <fcntl.h>   /* open, O_WRONLY */
#include <unistd.h>  /* write, close */
#include <errno.h>   /* errno, EINTR */
#include <inttypes.h>
#include <math.h>

static int PWM_WriteAttribute(const pwm_t *pwm, const char *attribute, const char *value){
    if (pwm == NULL || attribute == NULL || value == NULL){
        errno = EINVAL;
        return -1;
    }

    char path[256];
    int length = snprintf(path, sizeof(path), "/sys/class/pwm/pwmchip%u/pwm%u/%s", pwm->chip, pwm->channel, attribute);

    if(length < 0)
        return -1;

    if((size_t)length >= sizeof(path)){
        errno = ENAMETOOLONG;
        return -1;
    }

    int fd = open(path, O_WRONLY);

    if(fd == -1){
        perror(path);
        return -1;
    }

    size_t bytes_to_write = strlen(value);
    ssize_t bytes_written;

    do{
        bytes_written = write(fd, value, bytes_to_write);
    
    } while (bytes_written == -1 && errno == EINTR);

    if (bytes_written == -1){
        int saved_errno = errno;

        perror(path);
        close(fd);

        errno = saved_errno;
        return -1;
    }

    if ((size_t)bytes_written != bytes_to_write){
        close(fd);
        errno = EIO;
        return -1;
    }

    /* 4. Liberar el descriptor. */
    if (close(fd) == -1){
        perror(path);
        return -1;
    }

    return 0;

}

static int PWM_WriteChipAttribute(const pwm_t *pwm, const char *attribute, const char *value){
    if (pwm == NULL || attribute == NULL || value == NULL){
        errno = EINVAL;
        return -1;
    }

    char path[256];
    int length = snprintf(path, sizeof(path), "/sys/class/pwm/pwmchip%u/%s", pwm->chip, attribute);

    if(length < 0)
        return -1;

    if((size_t)length >= sizeof(path)){
        errno = ENAMETOOLONG;
        return -1;
    }

    int fd = open(path, O_WRONLY);

    if(fd == -1){
        perror(path);
        return -1;
    }

    size_t bytes_to_write = strlen(value);
    ssize_t bytes_written;

    do{
        bytes_written = write(fd, value, bytes_to_write);
    
    } while (bytes_written == -1 && errno == EINTR);

    if (bytes_written == -1){
        int saved_errno = errno;

        perror(path);
        close(fd);

        errno = saved_errno;
        return -1;
    }

    if ((size_t)bytes_written != bytes_to_write){
        close(fd);
        errno = EIO;
        return -1;
    }

    /* 4. Liberar el descriptor. */
    if (close(fd) == -1){
        perror(path);
        return -1;
    }

    return 0;

}

int PWM_Init(pwm_t *pwm, unsigned int chip, unsigned int channel, unsigned long period_ns){
    if (pwm == NULL || period_ns == 0){
        errno = EINVAL;
        return -1;
    }

    if (pwm->initialized){
        errno = EBUSY;
        return -1;
    }

    pwm->chip = chip;
    pwm->channel = channel;

    char channel_text[32];
    char period_text[32];

    snprintf(channel_text, sizeof(channel_text), "%u", channel);
    snprintf(period_text, sizeof(period_text), "%lu", period_ns);

    if (PWM_WriteChipAttribute(pwm, "export", channel_text) != 0){
        return -1;
    }

    usleep(100000);

    if (PWM_WriteAttribute(pwm, "enable", "0") != 0 ||
        PWM_WriteAttribute(pwm, "duty_cycle", "0") != 0 ||
        PWM_WriteAttribute(pwm, "period", period_text) != 0 ||
        PWM_WriteAttribute(pwm, "polarity", "normal") != 0) {
        
        int saved_errno = errno;

        /* Exportamos nosotros: intentar deshacer la inicialización. */
        if (PWM_WriteChipAttribute(pwm, "unexport", channel_text) != 0){
            fprintf(stderr, "No se pudo liberar el canal PWM\n");
        }

        errno = saved_errno;
        return -1;
    }

    pwm->period_ns = period_ns;
    pwm->duty_percent = 0.0f;
    pwm->enabled = false;
    pwm->initialized = true;
    pwm->duty_ns = 0;

    return 0;
}

int PWM_Enable(pwm_t *pwm){
    
    if(pwm == NULL || !pwm->initialized){
        errno = EINVAL;
        return -1;
    }

    if (PWM_WriteAttribute(pwm, "enable", "1") != 0){
        return -1;
    }

    pwm->enabled = true;
    return 0;
}

int PWM_Disable(pwm_t *pwm){
    if(pwm == NULL || !pwm->initialized){
        errno = EINVAL;
        return -1;
    }

    if (PWM_WriteAttribute(pwm, "enable", "0") != 0){
        return -1;
    }

    pwm->enabled = false;
    return 0;
}

int PWM_SetDutyPercent(pwm_t *pwm, float duty_percent){
    if (pwm == NULL || duty_percent < 0.0f || duty_percent > 100.0f){
        errno = EINVAL;
        return -1;
    }

    if(!pwm->initialized){
        errno = EINVAL;
        return -1;
    }

    unsigned long duty_ns;

    if(duty_percent == 100.0f){
        duty_ns = pwm->period_ns;
    } else {
        duty_ns = (unsigned long)((double)pwm->period_ns * ((double)duty_percent / 100.0));
    }

    char duty_text[32];

    snprintf(duty_text, sizeof(duty_text),"%" PRIu64, duty_ns);

    if( PWM_WriteAttribute(pwm, "duty_cycle", duty_text) != 0){
        return -1;
    }

    pwm->duty_percent = duty_percent;
    pwm->duty_ns = duty_ns;

    return 0;
}

int PWM_Close(pwm_t *pwm)
{
    if (pwm == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    /* Si ya está cerrado, no hay nada que liberar. */
    if (!pwm->initialized){
        return 0;
    }

    if(PWM_Disable(pwm) != 0){
        return -1;
    }

    pwm->enabled = false;

    /* Después liberar el canal que exportamos en Init. */
    char channel_text[32];

    snprintf(channel_text, sizeof(channel_text), "%u", pwm->channel);

    if (PWM_WriteChipAttribute(pwm, "unexport", channel_text) != 0){
        return -1;
    }

    pwm->initialized = false;

    return 0;
}