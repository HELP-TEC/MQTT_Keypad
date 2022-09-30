/**
 * @file storage_module.c
 * Storage essential data for config via python script
 * @date 28.09.2022
 * Created on: 18 september 2022
 * @author Louis COLIN
 */
#include "config.h"
//#include "storage_module.h"
/**
 * @fn void storage_init()
 *
 * @brief initialize NVS
 *
 *  initialize NVS with a json file for the MQTT configuration parameters
 *
 */

static const char *TAG = "STORAGE TEST";
void NVS_config_task(void *arg)
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
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);
    uint16_t size;
    while (1) {
        // Read data from the UART
        int len = uart_read_bytes(CONFIG_UART_PORT_NUM, data, (BUF_SIZE - 1), 20 / portTICK_PERIOD_MS);
        //uart_write_bytes(CONFIG_UART_PORT_NUM, (const char *) data, len);

        if(len)
        {
            uart_write_bytes(CONFIG_UART_PORT_NUM, (const char *) &data[0], 1);
            uart_write_bytes(CONFIG_UART_PORT_NUM, (const char *) &data[1], 1);
            size = (data[1]<<8) + data[2];
            if(data[0]==WRITE_COMMAND)
            {
                size = (data[2]<<8)+data[1];
                for(int k=1;k<size;k++)
                {
                    uart_write_bytes(CONFIG_UART_PORT_NUM, (const char *) &data[2+k], 1);
                }
            }
            else
            {

            }
        }
        // Write data back to the UART
        
    }
}
