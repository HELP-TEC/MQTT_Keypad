/**
 * @file main.c
 * MQTT (over TCP)
 * @date 6.03.2022
 * Created on: 6 mai 2022
 * @author Thibault Sampiemon
 */
#include "adc_module.h"
#include "button_module.h"
#include "config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqtt_module.h"
#include "storage_module.h"
#include <stdbool.h>
#if USE_CONFIGURATIONFILE != 1
#define CONFIG_WITHOUT_POTENTIOMETER 0 //!< config must be =1 if the pcb has no potentiometer connected
#endif
/*Tasks parameters*/
#define NVS_RW_TASK_PRIO 5    //!< the priority of the task that configure the storage with the json file
#define GPIOIntPC_TASK_PRIO 4 //!< the priority of the task that interpret the values from the gpio
#define POTENTIOMETER_TASK_PRIO                                                                                        \
    3 //!< the priority of the task that sample the value coming from the potentiometer with the adc
#define GPIOWRITE_TASK_PRIO 2 //!< the priority of the task that update the LEDs
#define MQTT_TASK_PRIO 1      //!< the priority of the task that manage the communication with the broker mqtt
/**
 * @fn void app_main(void)
 *
 * @brief main function
 *
 *  the main only start all the FreeRTOS tasks after the initialization of each modules (one task for each modules)
 *
 */
void app_main(void)
{
    // If change of micropros to a dualcore in the futur Allow other core to finish initialization
    vTaskDelay(pdMS_TO_TICKS(100));
    // config peripherals
    uart_init_config();
    storage_init();
    ethernet_init();
    potentiometer_config();
    Button_isr_config();
    Button_i2c_config();
    MQTT_init();
    // Task creation
    xTaskCreatePinnedToCore(MQTT_Task, "MQTT_Task", 2048, NULL, MQTT_TASK_PRIO, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(GPIOIntPC_task, "GPIOIntPC_task", 2048, NULL, GPIOIntPC_TASK_PRIO, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(GPIOwrite_task, "GPIOwrite_task", 2048, NULL, GPIOWRITE_TASK_PRIO, NULL, tskNO_AFFINITY);
#if CONFIG_WITHOUT_POTENTIOMETER != 1
    xTaskCreatePinnedToCore(potentiometer_task, "potar_task", 2048, NULL, POTENTIOMETER_TASK_PRIO, NULL,
                            tskNO_AFFINITY);
#endif
    xTaskCreatePinnedToCore(NVS_RW_task, "NVS_RW_task", 2048, NULL, NVS_RW_TASK_PRIO, NULL, tskNO_AFFINITY);
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
