/**
 * @file config.h
 * MQTT (over TCP)
 * @date 10.08.2022
 * Created on: 4 juillet 2022
 * @author Thibault Sampiemon
 */
#ifndef MAIN_CONFIG_H_
#define MAIN_CONFIG_H_

/*-----------------------------------------------------------------------
 * config file options
 * --------------------------------------------------------------------*/
#define USE_CONFIGURATIONFILE 1 //!< use the config file instead of the default config



/*-----------------------------------------------------------------------
 * MQTT communication options
 * --------------------------------------------------------------------*/
#define TOPIC_POTENTIOMETER "topic/potentiometer"  //!< topic where the embedded module will publish the potentiometer value ex:"topic/potentiometer4"
#define TOPIC_BUTTON "topic/buttons"  //!< topic where the embedded module will publish the buttons value ex:"topic/buttons4"
#define TOPIC_LED "topic/leds"  //!< topic that embedded module will subscribe to receive the LEDs value ex:"topic/leds4"


#define USER "pannel1" //!< username for the connection to the broker
#define PASS "itisnotagoodpasswordbutwhocarehaha1" //!< password for the connection to the broker

/*-----------------------------------------------------------------------
 * PCB options
 * --------------------------------------------------------------------*/
#define CONFIG_WITHOUT_POTENTIOMETER 0
//----------------------------------------------------------
// Includes
//----------------------------------------------------------
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "cJSON.h"
#include <string.h>
#include <stdio.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
//----------------------------------------------------------
// Defines
//----------------------------------------------------------
// UART CONFIG
#define CONFIG_TEST_TXD (1)
#define CONFIG_TEST_RXD (3)
#define CONFIG_TEST_RTS (UART_PIN_NO_CHANGE)
#define CONFIG_TEST_CTS (UART_PIN_NO_CHANGE)

#define CONFIG_UART_PORT_NUM      (0)
#define CONFIG_UART_BAUD_RATE     (115200)
#define CONFIG_TASK_STACK_SIZE    (2048)
// Frame format  [HEADER][PAYLOAD][STOP_CMD] (Header) = [R/W SIZE SIZE]
#define WRITE_COMMAND 0x00
#define READ_COMMAND 0x01
#define STOP_COMMAND 0xF0
#define BYTE_FOR_SIZE 0x02
// Inutile ?
#define BUF_SIZE (1024)
//----------------------------------------------------------
// Prototypes
//----------------------------------------------------------
void NVS_RW_task(void *arg);
void storage_init();

#endif /* MAIN_CONFIG_H_ */
