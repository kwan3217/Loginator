#define PWM_BASE_ADDR 0xE0014000

rw0(pwm,PWMIR,PWM_BASE_ADDR+0x00);
rw0(pwm,PWMTCR,PWM_BASE_ADDR+0x04);
rw0(pwm,PWMTC,PWM_BASE_ADDR+0x08);
rw0(pwm,PWMPR,PWM_BASE_ADDR+0x0C);
rw0(pwm,PWMPC,PWM_BASE_ADDR+0x10);
rw0(pwm,PWMMCR,PWM_BASE_ADDR+0x14);
//We are trying to match the registers of a normal Timer, but a PWM timer has
//more channels than a normal timer. We write these extra channels in a hole in
//the map of a timer. Channel 3 is at 0x18+4*3=0x24, but channel 4 falls at
//(0x18+0x18)+4*4=0x40. This is a difference of 0x18 from the 0x28 we would
//otherwise, equivalent to 6 channels.
rw1(pwm,PWMMR,7,PWM_BASE_ADDR+0x18+4*(i+((i>3)?6:0)));
rw0(pwm,PWMEMR,PWM_BASE_ADDR+0x3C);
rw0(pwm,PWMPCR,PWM_BASE_ADDR+0x4C);
rw0(pwm,PWMLER,PWM_BASE_ADDR+0x50);

#undef PWM_BASE_ADDR