/* MQTT (over TCP)

*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdbool.h>
#include "mqtt_module.h"
#include "adc_module.h"
#include "button_module.h"



/*Tasks parameters*/
#define GPIOIntPC_TASK_PRIO      4
#define POTENTIOMETER_TASK_PRIO      3
#define GPIOWRITE_TASK_PRIO      2
#define MQTT_TASK_PRIO      1
//TODO create communication queue between mqtt an button and adc
//QueueHandle_t xmqtt_to_button_Queue = NULL;
//QueueHandle_t xbutton_to_mqtt_Queue = NULL;




void app_main(void)
{
    //If change of micropros to a dualcore in the futur Allow other core to finish initialization
    vTaskDelay(pdMS_TO_TICKS(100));





    potentiometer_config();
    Button_isr_config();
    Button_i2c_config();
	MQTT_init();

    //Task creation
    xTaskCreatePinnedToCore(MQTT_Task, "MQTT_Task", 2048, NULL, MQTT_TASK_PRIO, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(GPIOIntPC_task, "GPIOIntPC_task", 2048, NULL, GPIOIntPC_TASK_PRIO, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(GPIOwrite_task, "GPIOwrite_task", 2048, NULL, GPIOWRITE_TASK_PRIO, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(potentiometer_task, "potar_task", 2048, NULL, POTENTIOMETER_TASK_PRIO, NULL, tskNO_AFFINITY);



        while (1) {
        	vTaskDelay(pdMS_TO_TICKS(1000));
        }
}


/*
 *
 *
 *
 *
 *
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "driver/gpio.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include <stdbool.h>

#define MQTT_TASK_PRIO      4


#define	PIN_PHY_POWER	GPIO_NUM_12

#define CONECTED_FLAG 		0b00000001
#define DISCONECTED_FLAG 	0b00000010
#define PUBLISHED_FLAG 		0b00000100
#define SUSCRIBED_FLAG 		0b00001000
#define UNSUSCRIBED_FLAG 	0b00010000
#define DATA_FLAG 			0b00100000
#define ERROR_FLAG 			0b01000000
#define OTHER_FLAG 			0b10000000
static const char *TAG = "MQTT_EXAMPLE";


static EventGroupHandle_t xMQTTRecieveEventBits;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}


esp_mqtt_client_handle_t client=NULL;



static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
	      xEventGroupSetBits( xMQTTRecieveEventBits,
	    		  	  	  	  CONECTED_FLAG);
	      xEventGroupClearBits( xMQTTRecieveEventBits,
    		  	  	  	  	  DISCONECTED_FLAG);

        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
	      xEventGroupSetBits( xMQTTRecieveEventBits,
	    		  	  	  	  	  DISCONECTED_FLAG);
	      xEventGroupClearBits( xMQTTRecieveEventBits,
	  	  	  	  	  	  	  	  CONECTED_FLAG);

        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
	      xEventGroupSetBits( xMQTTRecieveEventBits,
	    		  	  	  	  	  SUSCRIBED_FLAG);
	      xEventGroupClearBits( xMQTTRecieveEventBits,
	    		  	  	  	  	  UNSUSCRIBED_FLAG);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
	      xEventGroupSetBits( xMQTTRecieveEventBits,
	    		  	  	  	  UNSUSCRIBED_FLAG);
	      xEventGroupClearBits( xMQTTRecieveEventBits,
	    		  	  	  	  	  SUSCRIBED_FLAG);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
	      xEventGroupSetBits( xMQTTRecieveEventBits,
	    		  	  	  	  PUBLISHED_FLAG);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
	      xEventGroupSetBits( xMQTTRecieveEventBits,
	    		  	  	  	  DATA_FLAG);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
  	      xEventGroupSetBits( xMQTTRecieveEventBits,
  	    		  	  	  	  ERROR_FLAG);
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
	      xEventGroupSetBits( xMQTTRecieveEventBits,
	    		  	  	  	  OTHER_FLAG);
        break;
    }
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
            //.uri = CONFIG_BROKER_URL,
        		.username ="pannel1",
    			.password="itisnotagoodpasswordbutwhocarehaha1",
    			.port=1883,
    			.transport=MQTT_TRANSPORT_OVER_TCP,
    			.host="192.168.5.10"
    };


    client = esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}



void MQTT_Task(void *arg)
{
    int i=0;
    EventBits_t com_flags=0;
	//for an internal sychronisation of the led state to the button actual state
	xEventGroupWaitBits(xMQTTRecieveEventBits, CONECTED_FLAG, pdFALSE, pdFALSE, portMAX_DELAY);
    int msg_id = esp_mqtt_client_subscribe(client, "topic/leds", 1);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
    xEventGroupWaitBits(xMQTTRecieveEventBits, SUSCRIBED_FLAG, pdFALSE, pdFALSE, portMAX_DELAY);
        while (1) {
        	com_flags=xEventGroupWaitBits(xMQTTRecieveEventBits, CONECTED_FLAG, pdFALSE, pdFALSE, pdMS_TO_TICKS(20));
        	if((com_flags & CONECTED_FLAG) == CONECTED_FLAG){
            	i++;
            	i=i%10;
            	char str[5]="     ";
            	sprintf(str, "%d", i);
        		esp_mqtt_client_publish(client, "topic/buttons", str, 0, 1, 0);
        		ESP_LOGI(TAG, "publisch data");
        		xEventGroupWaitBits(xMQTTRecieveEventBits, PUBLISHED_FLAG, pdTRUE, pdFALSE, portMAX_DELAY);
        	}

        	vTaskDelay(pdMS_TO_TICKS(5000));
        }


}

void MQTT_init()
{
gpio_pad_select_gpio(PIN_PHY_POWER);
gpio_set_direction(PIN_PHY_POWER,GPIO_MODE_OUTPUT);
gpio_set_level(PIN_PHY_POWER, 1);
vTaskDelay(pdMS_TO_TICKS(10));

ESP_LOGI(TAG, "[APP] Startup..");
ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

esp_log_level_set("*", ESP_LOG_INFO);
esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);


xMQTTRecieveEventBits = xEventGroupCreate();


ESP_ERROR_CHECK(nvs_flash_init());
ESP_ERROR_CHECK(esp_netif_init());
ESP_ERROR_CHECK(esp_event_loop_create_default());



ESP_ERROR_CHECK(example_connect());


mqtt_app_start();
}
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 * int msg_id;
msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
break;
*/

/* msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
       ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);*/
