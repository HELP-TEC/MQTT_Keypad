
/**
 * @file button_module.h
 * MQTT (over TCP)
 * @date 6.03.2022
 * Created on: 6 mai 2022
 * @author Thibault Sampiemon
 */
#ifndef MAIN_BUTTON_MODULE_H_
#define MAIN_BUTTON_MODULE_H_
void GPIOIntPC_task(void *arg);
void GPIOwrite_task(void *arg);
void Button_isr_config(void);
void Button_i2c_config(void);
void Button_i2c_delte(void);
#endif /* MAIN_BUTTON_MODULE_H_ */
