/*
 * mqtt_module.h
 *
 *  Created on: 6 mai 2022
 *      Author: PRO
 */

#ifndef MAIN_MQTT_MODULE_H_
#define MAIN_MQTT_MODULE_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#define RECEIVE_LED_FLAG 				0b10000000
#define SEND_BUTTON_FLAG 				0b00000001
#define SEND_POTENTIOMETRE_FLAG 		0b00000010


void MQTT_Task(void *arg);
void MQTT_init();

EventGroupHandle_t xEvent_data_COM;

QueueHandle_t xQueue_data_LED_COM;
QueueHandle_t xQueue_data_Button_COM;
QueueHandle_t xQueue_data_Potentimeter_COM;
#endif /* MAIN_MQTT_MODULE_H_ */
