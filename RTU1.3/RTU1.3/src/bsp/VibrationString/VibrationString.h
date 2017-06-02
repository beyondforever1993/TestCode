#ifndef  VIBRATINGWIRE_H
#define  VIBRATINGWIRE_H

#define _USING_PWM_NO	1
#define PWM_VMT_PORT	2
#define PWM_VMT_PIN		0
#define PWM_VMT_CHANNEL	1
#define PWM_CAP0_PIN	6
#define _USING_PWM_NO	1
#define PRESCALEVALUE	1


#define _MEASURE_TIM			(LPC_TIM2)
#define _MEASURE_TIM_INTR		(TIMER2_IRQn)

#define BRD_PWM_POWER_5V_PORT		(0)//(1)
#define BRD_PWM_POWER_5V_PIN		(19)//(13)
#define BRD_PWM_POWER_5V_MASK		(1 << BRD_PWM_POWER_5V_PIN)

#define BRD_PWM_POWER_12V_PORT		(0)//(1)
#define BRD_PWM_POWER_12V_PIN		(18)//(13)
#define BRD_PWM_POWER_12V_MASK		(1 << BRD_PWM_POWER_12V_PIN)

#define BRD_PWM_SHUTD_PORT			(1)//(1)
#define BRD_PWM_SHUTD_PIN		(12)//(13)
#define BRD_PWM_SHUTD_MASK		(1 << BRD_PWM_SHUTD_PIN)

#define FRQ_LOW		400
#define FRQ_HIGH	3800
#define FRQ_STEP	600

struct DATA_MEASURE{
	uint32_t CaptureVal;
	uint32_t CapMatch_cnt;
	uint32_t ScanMatch_cnt;
};

typedef struct{
	uint32_t 	type;
	uint32_t 	frq;
	uint8_t		frq_changed;
	uint32_t 	freq_Value;  //检测到的外部频率值	
	char 		sname[21];
}pwm_para_t;

extern pwm_para_t pwm_para;

void pwm_task(void);
void Pwm_init(void);
void Stri_measure(void);
void pwm_timer_handler(void);



#endif
