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
// Structure
//----------------------------------------------------------
/**
 * @struct typedef struct JsonCOnfig
 *
 * @brief used to communicate button activity from interupt
 * */
typedef struct JsonConfig {
    char * MQTT_username;
    char * password;
    char * ip;
	uint16_t port; /*!< pin number that take the interrupt*/
    char * topic_bp;
    char * topic_del;
    char * topic_pot;
} JsonConfig;
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
#define BUF_SIZE 1024
// Frame format  [HEADER][PAYLOAD][STOP_CMD] (Header) = [R/W SIZE SIZE]
#define WRITE_COMMAND 0x00
#define READ_COMMAND 0x01
// Json objects
#define JSON_STRING 0
#define JSON_INT 1
//----------------------------------------------------------
// Prototypes
//----------------------------------------------------------
void NVS_RW_task(void *arg);
void storage_init(void);
void uart_init_config(void);
void read_json_config(JsonConfig *);
#endif /* MAIN_CONFIG_H_ */
