/*
 * button_module.h
 *
 *  Created on: 21 avr. 2022
 *      Author: PRO
 */

#ifndef MAIN_BUTTON_MODULE_H_
#define MAIN_BUTTON_MODULE_H_

void GPIOIntPC_task(void *arg);
void GPIOwrite_task(void *arg);
void Button_isr_config(void);
void Button_i2c_config(void);
void Button_i2c_delte(void);


#endif /* MAIN_BUTTON_MODULE_H_ */
