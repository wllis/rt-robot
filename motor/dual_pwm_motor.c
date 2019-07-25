#include "dual_pwm_motor.h"

#define DBG_SECTION_NAME "dual_pwm_motor"
#define DBG_LEVEL DBG_LOG
#include <rtdbg.h>

typedef struct dual_pwm_motor *dual_pwm_motor_t;

struct dual_pwm_motor
{
    struct motor mot;
    struct rt_device_pwm *pwm1_dev;
    int pwm1_channel;
    struct rt_device_pwm *pwm2_dev;
    int pwm2_channel;
};

static rt_err_t dual_pwm_motor_enable(motor_t mot);
static rt_err_t dual_pwm_motor_disable(motor_t mot);
static rt_err_t dual_pwm_motor_set_speed(motor_t mot, rt_int16_t thousands);

motor_t dual_pwm_motor_creat(char *pwm1, int pwm1_channel, char *pwm2, int pwm2_channel)
{
    dual_pwm_motor_t new_motor = (dual_pwm_motor_t)rt_malloc(sizeof(struct dual_pwm_motor));
    if (new_motor == RT_NULL)
    {
        LOG_E("Falied to allocate memory for motor");
        return RT_NULL;
    }

    new_motor->pwm1_dev = (struct rt_device_pwm*)rt_device_find(pwm1);
    if (new_motor->pwm1_dev == RT_NULL)
    {
        rt_free(new_motor);
        LOG_E("Falied to find device on %s", pwm1);
        return RT_NULL;
    }
    new_motor->pwm2_dev = (struct rt_device_pwm*)rt_device_find(pwm2);
    if (new_motor->pwm2_dev == RT_NULL)
    {
        rt_free(new_motor);
        LOG_E("Falied to find device on %s", pwm2);
        return RT_NULL;
    }
    new_motor->pwm1_channel = pwm1_channel;
    new_motor->pwm2_channel = pwm2_channel;
    new_motor->mot.enable = dual_pwm_motor_enable;
    new_motor->mot.disable = dual_pwm_motor_disable;
    new_motor->mot.set_speed = dual_pwm_motor_set_speed;

    return &new_motor->mot;
}

static rt_err_t dual_pwm_motor_enable(motor_t mot)
{
    RT_ASSERT(mot != RT_NULL);

    dual_pwm_motor_t mot_sub = (dual_pwm_motor_t)mot;

    rt_pwm_enable(mot_sub->pwm1_dev, mot_sub->pwm1_channel);
    rt_pwm_enable(mot_sub->pwm2_dev, mot_sub->pwm2_channel);

    return RT_EOK;
}

static rt_err_t dual_pwm_motor_disable(motor_t mot)
{
    RT_ASSERT(mot != RT_NULL);

    dual_pwm_motor_t mot_sub = (dual_pwm_motor_t)mot;
    
    rt_pwm_disable(mot_sub->pwm1_dev, mot_sub->pwm1_channel);
    rt_pwm_disable(mot_sub->pwm2_dev, mot_sub->pwm2_channel);

    return RT_EOK;
}

static rt_err_t dual_pwm_motor_set_speed(motor_t mot, rt_int16_t thousands)
{
    RT_ASSERT(mot != RT_NULL);

    dual_pwm_motor_t mot_sub = (dual_pwm_motor_t)mot;

    if (thousands == 0)
    {
        rt_pwm_set(mot_sub->pwm1_dev, mot_sub->pwm1_channel, MOTOR_PWM_PERIOD, 0);
        rt_pwm_set(mot_sub->pwm2_dev, mot_sub->pwm2_channel, MOTOR_PWM_PERIOD, 0);
    }
    else if (thousands > 0)
    {
        rt_uint32_t pluse = MOTOR_PWM_PERIOD * thousands / 1000;
        if (pluse > MOTOR_PWM_PERIOD)
        {
            pluse = MOTOR_PWM_PERIOD;
        }
        rt_pwm_set(mot_sub->pwm1_dev, mot_sub->pwm1_channel, MOTOR_PWM_PERIOD, pluse);
        rt_pwm_set(mot_sub->pwm2_dev, mot_sub->pwm2_channel, MOTOR_PWM_PERIOD, 0);
    }
    else
    {
        rt_uint32_t pluse = MOTOR_PWM_PERIOD * -thousands / 1000;
        if (pluse > MOTOR_PWM_PERIOD)
        {
            pluse = MOTOR_PWM_PERIOD;
        }
        rt_pwm_set(mot_sub->pwm1_dev, mot_sub->pwm1_channel, MOTOR_PWM_PERIOD, 0);
        rt_pwm_set(mot_sub->pwm2_dev, mot_sub->pwm2_channel, MOTOR_PWM_PERIOD, pluse);
    }

    return RT_EOK;
}
