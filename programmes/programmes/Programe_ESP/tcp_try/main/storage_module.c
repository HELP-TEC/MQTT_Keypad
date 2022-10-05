/**
 * @file storage_module.c
 * Storage essential data for config via python script
 * @date 28.09.2022
 * Created on: 18 september 2022
 * @author Louis COLIN
 */
#include "config.h"
//#include "storage_module.h"

#define DEBUG 1

/**
 * @fn void NVS_RW_task(void *arg)
 *
 * @brief Freertos task that configure MQTT parameters
 *
 *  this task allow the MQTT configuration from a software through uart
 *
 * @param arg FreeRTOS standard argument of a task
 */
static const char *TAG = "STORAGE TEST";
void NVS_RW_task(void *arg)
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = CONFIG_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    ESP_ERROR_CHECK(uart_driver_install(CONFIG_UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(CONFIG_UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(CONFIG_UART_PORT_NUM, CONFIG_TEST_TXD, CONFIG_TEST_RXD, CONFIG_TEST_RTS, CONFIG_TEST_CTS));

    // Configure a temporary buffer for the incoming data
    uint8_t *data=NULL;
    uint8_t *header = (uint8_t *) malloc(3);
    uint16_t size;
    nvs_handle_t my_handle;
    esp_err_t err;
     cJSON *root, *pnl_cfg_args;
	 root = cJSON_CreateObject();
	//cJSON_Delete(root);
    while (1) {
        // Read data from the UART
        int len =uart_read_bytes(CONFIG_UART_PORT_NUM, header, (3), 20 / portTICK_PERIOD_MS);
        if(len)
        {
            if(header[0]==WRITE_COMMAND)
            {
                size = (header[1]<<8)+(header[2]);
                free(data);
                data= (uint8_t *) malloc(size);
                len = uart_read_bytes(CONFIG_UART_PORT_NUM, data, size, 20 / portTICK_PERIOD_MS);
                nvs_open("nvs", NVS_READWRITE, &my_handle);
                nvs_set_u16(my_handle, "MQTTsize",size);
                nvs_set_str(my_handle, "MQTTstr",(const char *)data);
                nvs_commit(my_handle);
                nvs_close(my_handle);
            }
            else
            {
                free(data);
                err = nvs_open("nvs", NVS_READWRITE, &my_handle);
                err = nvs_get_u16(my_handle, "MQTTsize", &size);
                data= (uint8_t *) malloc(size);
                err = nvs_get_str(my_handle, "MQTTstr",(const char *) data, &size);
                nvs_close(my_handle);
                uart_write_bytes(CONFIG_UART_PORT_NUM, (const char *) data, size-1);
            }
        }
    }
}

/**
 * @fn void storage_init()
 *
 * @brief initialize NVS
 *
 *  initialize NVS with for the json file for the MQTT configuration parameters
 *
 */
void storage_init(void)
{
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
}