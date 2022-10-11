/**
 * @file button_mdule.c
 * MQTT (over TCP)
 * @date 6.03.2022
 * Created on: 6 mai 2022
 * @author Thibault Sampiemon
 */
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_intr_alloc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <stdio.h>
#include <stdlib.h>
#include "button_module.h"
#include "mqtt_module.h"
/*Event group Bits used by the task tasks and interrupt to syncronise. */
// GPIOIntPC
#define GPIOIntPC_INTERRUPT_0_BIT (1 << 0)
/*to activate debug function*/
#define INTERNAL_LED_BUTTON_STATE_SYNCHRONISATION_TEST 0
#define INTERNAL_LED_SNAKE_TEST 0
#define INTERNAL_LED_CURRENT_TEST 0
#define DEBUG 0
/*i2c config*/
#define I2C_MASTER_SCL_IO GPIO_NUM_16 /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO GPIO_NUM_13 /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM                                                                                                 \
  0 /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ 400000   /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS 1000
#define GPIO1_ADDR 0x20 /*!< Slave address of the GPIO BUTTON 1 add 0x20 (a0a1a2 = 000 pour le 1 et 100 pour le 2)*/
#define GPIO2_ADDR 0x21 /*!< Slave address of the GPIO BUTTON 2  add 0x21 (a0a1a2 = 001 pour le 1 et 100 pour le 2)*/
/*button position list electrical logic (button 1 on electrical schematic = BUTTON1)*/
#define BUTTON5 0b11111101 //!< 0 mask on the button 5 (electrical logic) register from gpio 1 (program logic)
#define BUTTON6 0b11111011 //!< 0 mask on the button 6 (electrical logic) register from gpio 1 (program logic)
#define BUTTON8 0b10111111 //!< 0 mask on the button 8 (electrical logic) register from gpio 1 (program logic)
#define BUTTON7 0b11011111 //!< 0 mask on the button 7 (electrical logic) register from gpio 1 (program logic)
#define BUTTON1 0b10111111 //!< 0 mask on the button 1 (electrical logic) register from gpio 2 (program logic)
#define BUTTON2 0b01111111 //!< 0 mask on the button 2 (electrical logic) register from gpio 2 (program logic)
#define BUTTON3 0b11101111 //!< 0 mask on the button 3 (electrical logic) register from gpio 2 (program logic)
#define BUTTON4 0b11011111 //!< 0 mask on the button 4 (electrical logic) register from gpio 2 (program logic)
/*led position list electrical logic (led 1 on electrical schematic = LED1)*/
#define LED8 0b01111111      //!< 0 mask on the LED 8 (electrical logic) register from gpio 1 (program logic)
#define LED7 0b11101111      //!< 0 mask on the LED 7 (electrical logic) register from gpio 1 (program logic)
#define LED6 0b11110111      //!< 0 mask on the LED 6 (electrical logic) register from gpio 1 (program logic)
#define LED5 0b11111110      //!< 0 mask on the LED 5 (electrical logic) register from gpio 1 (program logic)
#define LED1 0b11111101      //!< 0 mask on the LED 1 (electrical logic) register from gpio 2 (program logic)
#define LED2 0b11111110      //!< 0 mask on the LED 2 (electrical logic) register from gpio 2 (program logic)
#define LED3 0b11110111      //!< 0 mask on the LED 3 (electrical logic) register from gpio 2 (program logic)
#define LED4 0b11111011      //!< 0 mask on the LED 4 (electrical logic) register from gpio 2 (program logic)
#define LED_CLEAR 0b11111111 //!< mask that clear all the LEDS
/**
 * @struct typedef struct GPIOIntPC
 *
 * @brief used to communicate button activity from interupt
 * */
typedef struct GPIOIntPC
{
  uint8_t PIN;               /*!< pin number that take the interrupt*/
  uint32_t numberKeyPresses; /*!< number of activation */
  bool pressed; /*!< actual state of the input interupt (used to notify that the interrupt have been processed)*/
} GPIOIntPC;
static GPIOIntPC GPIOIntPC1 = {GPIO_NUM_32, 0, 0}; // var used for the communication as an init
/* Use an event group to synchronise GPIOIntPC task and interrupt.  It is assumed this event
 group has already been created elsewhere. */
static EventGroupHandle_t xGPIOIntPCEventBits;            //!< FreeRTOS flag group for the flags coming from the isr
static EventGroupHandle_t xGPIO_LED_Button_Internal_synk; //!< FreeRTOS flag group for the debug function to sincro
                                                          //!< internally the led and button state
/**
 *
 * @fn static esp_err_t GPIO_register_read( uint8_t reg_addr, uint8_t *data, size_t len)
 *
 * @brief Read a sequence of bytes from a GPIO BUTTON registers
 *
 * @param uint8_t reg_addr the address of the GPIO register that must be read
 *
 * @param uint8_t *data The variable to store the current value of this register
 *
 * @param number of byte that to read
 *
 */
static esp_err_t GPIO_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
  return i2c_master_read_from_device(I2C_MASTER_NUM, reg_addr, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
}
/**
 *
 * @fn static esp_err_t GPIO_register_write_byte(uint8_t reg_addr, uint8_t data)
 *
 * @brief Write a byte to a GPIO BUTTON register
 *
 * @param uint8_t reg_addr the address of the GPIO register that must be modified
 *
 * @param uint8_t *data The value to flash in this register
 *
 */
static esp_err_t GPIO_register_write_byte(uint8_t reg_addr, uint8_t data)
{
  int ret;
  uint8_t write_buf[2] = {reg_addr, data};
  ret = i2c_master_write_to_device(I2C_MASTER_NUM, reg_addr, write_buf, sizeof(write_buf),
                                   I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
  return ret;
}
#if DEBUG
/**
 *
 * @fn static void debug_button_GPIO1(int val)
 *
 * @brief function for debugging
 *
 * it will send the button state trough the serial interface
 *
 * @param int val value tha comme from reading the GPIO1 registers
 *
 */
static void debug_button_GPIO1(int val)
{
  // printf("%d",val);
  // printf("\n");
  if ((val | BUTTON5) == BUTTON5) printf(" button 5 \n");
  if ((val | BUTTON6) == BUTTON6) printf(" button 6 \n");
  if ((val | BUTTON7) == BUTTON7) printf(" button 7 \n");
  if ((val | BUTTON8) == BUTTON8) printf(" button 8 \n");
  if ((val | BUTTON8) != BUTTON8 && (val | BUTTON7) != BUTTON7 && (val | BUTTON6) != BUTTON6 &&
      (val | BUTTON5) != BUTTON5)
    printf(" Not GPIO_1 or release \n");
}
#endif
/**
 *
 * @fn static uint8_t convert_button_GPIO1(int val)
 *
 * @brief function for converting values comming from GPIO1 into electrical logic values
 *
 * it will transform the value of the GPIOs input in an understable (electrical logic) value
 *
 * @param int val value tha comme from reading the GPIO1 registers
 *
 */
static uint8_t convert_button_GPIO1(int val)
{
  // printf("%d",val);
  // printf("\n");
  uint8_t r_value = 0x0;
  if ((val | BUTTON5) == BUTTON5) r_value |= (1 << 4);
  if ((val | BUTTON6) == BUTTON6) r_value |= (1 << 5);
  if ((val | BUTTON7) == BUTTON7) r_value |= (1 << 6);
  if ((val | BUTTON8) == BUTTON8) r_value |= (1 << 7);
  if ((val | BUTTON8) != BUTTON8 && (val | BUTTON7) != BUTTON7 && (val | BUTTON6) != BUTTON6 &&
      (val | BUTTON5) != BUTTON5)
    r_value = 0x0;
  return r_value;
}
#if DEBUG
/**
 *
 * @fn static void debug_button_GPIO2(int val)
 *
 * @brief function for debugging
 *
 * it will send the button state trough the serial interface
 *
 * @param int val value tha comme from reading the GPIO2 registers
 *
 */
static void debug_button_GPIO2(int val)
{
  // printf("%d",val);
  // printf("\n");
  if ((val | BUTTON1) == BUTTON1) printf(" button 1 \n");
  if ((val | BUTTON2) == BUTTON2) printf(" button 2 \n");
  if ((val | BUTTON3) == BUTTON3) printf(" button 3 \n");
  if ((val | BUTTON4) == BUTTON4) printf(" button 4 \n");
  if ((val | BUTTON4) != BUTTON4 && (val | BUTTON3) != BUTTON3 && (val | BUTTON2) != BUTTON2 &&
      (val | BUTTON1) != BUTTON1)
    printf(" Not GPIO_2 or release  \n");
}
#endif
/**
 *
 * @fn static uint8_t convert_button_GPIO2(int val)
 *
 * @brief function for converting values comming from GPIO2 into electrical logic values
 *
 * it will transform the value of the GPIOs input in an understable (electrical logic) value
 *
 * @param int val value tha comme from reading the GPIO1 registers
 *
 */
static uint8_t convert_button_GPIO2(int val)
{
  // printf("%d",val);
  // printf("\n");
  uint8_t r_value = 0x0;
  if ((val | BUTTON1) == BUTTON1) r_value |= (1 << 0);
  if ((val | BUTTON2) == BUTTON2) r_value |= (1 << 1);
  if ((val | BUTTON3) == BUTTON3) r_value |= (1 << 2);
  if ((val | BUTTON4) == BUTTON4) r_value |= (1 << 3);
  if ((val | BUTTON4) != BUTTON4 && (val | BUTTON3) != BUTTON3 && (val | BUTTON2) != BUTTON2 &&
      (val | BUTTON1) != BUTTON1)
    r_value = 0x0;
  return r_value;
}
/**
 *
 * @fn static void debug_LED()
 *
 * @brief function for debugging led
 *
 * make a snake animation by lightning each led one after another
 *
 */
static void debug_LED()
{
  vTaskDelay(pdMS_TO_TICKS(1000));
  ESP_ERROR_CHECK(GPIO_register_write_byte(GPIO2_ADDR, LED_CLEAR));
  ESP_ERROR_CHECK(GPIO_register_write_byte(GPIO1_ADDR, LED8));
  vTaskDelay(pdMS_TO_TICKS(1000));
  ESP_ERROR_CHECK(GPIO_register_write_byte(GPIO1_ADDR, LED7));
  vTaskDelay(pdMS_TO_TICKS(1000));
  ESP_ERROR_CHECK(GPIO_register_write_byte(GPIO1_ADDR, LED6));
  vTaskDelay(pdMS_TO_TICKS(1000));
  ESP_ERROR_CHECK(GPIO_register_write_byte(GPIO1_ADDR, LED5));
  vTaskDelay(pdMS_TO_TICKS(1000));
  ESP_ERROR_CHECK(GPIO_register_write_byte(GPIO1_ADDR, LED_CLEAR));
  ESP_ERROR_CHECK(GPIO_register_write_byte(GPIO2_ADDR, LED3));
  vTaskDelay(pdMS_TO_TICKS(1000));
  ESP_ERROR_CHECK(GPIO_register_write_byte(GPIO2_ADDR, LED4));
  vTaskDelay(pdMS_TO_TICKS(1000));
  ESP_ERROR_CHECK(GPIO_register_write_byte(GPIO2_ADDR, LED1));
  vTaskDelay(pdMS_TO_TICKS(1000));
  ESP_ERROR_CHECK(GPIO_register_write_byte(GPIO2_ADDR, LED2));
}
/**
 *
 * @fn static void max_current_LED_test()
 *
 * @brief to test the max current win a normal use of the panel
 *
 * light all the LED up (the probability that all the leds are lightened at the same time is very low
 *
 */
static void max_current_LED_test()
{
  ESP_ERROR_CHECK(GPIO_register_write_byte(GPIO1_ADDR, LED5 & LED6 & LED7 & LED8));
  ESP_ERROR_CHECK(GPIO_register_write_byte(GPIO2_ADDR, LED1 & LED2 & LED3 & LED4));
  vTaskDelay(pdMS_TO_TICKS(1000));
}
/**
 *
 * @fn void GPIOIntPC_task(void *arg)
 *
 * @brief GPIO reading task.
 *
 * Each time the interrupt occurs this task will read the state of the button and send it to the task that need it
 * (the communication task that will send it to the plc will need it)
 *
 *@param arg FreeRTOS standard argument of a task
 *
 */
void GPIOIntPC_task(void *arg)
{
  int i = 0;
  int val = 0;
  uint8_t data[2] = {0, 0};
  uint8_t converted_val = 0;
  uint8_t synk_flags = 0;
  while (1)
  {
    xEventGroupWaitBits(xGPIOIntPCEventBits, GPIOIntPC_INTERRUPT_0_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
    if (GPIOIntPC1.pressed)
    {
      // printf("GPIOIntPC 1 \n");
      data[0] = 0;
      data[1] = 0;
      val = 0;
      converted_val = 0;
      synk_flags = 0;
      /* Read the  register for button state detemination*/
      ESP_ERROR_CHECK(GPIO_register_read(GPIO1_ADDR, data, 2));
      vTaskDelay(pdMS_TO_TICKS(1));
      val = (int) data[1];
#if DEBUG
      debug_button_GPIO1(val);
#endif
      converted_val = convert_button_GPIO1(val);
      //---------------------------------------------------------------------------------
      data[0] = 0;
      data[1] = 0;
      val = 0;
      /* Read the  register for button state detemination*/
      ESP_ERROR_CHECK(GPIO_register_read(GPIO2_ADDR, data, 2));
      vTaskDelay(pdMS_TO_TICKS(1));
      val = (int) data[1];
#if DEBUG
      debug_button_GPIO2(val);
#endif
      converted_val |= convert_button_GPIO2(val);
#if DEBUG
      printf(" %d ", (int) converted_val);
#endif
      xEventGroupSetBits(xEvent_data_COM,   /* The event group being updated. */
                         SEND_BUTTON_FLAG); /* The bits being set. */
      xQueueSend(xQueue_data_Button_COM, &converted_val, pdFALSE);
#if INTERNAL_LED_BUTTON_STATE_SYNCHRONISATION_TEST
      xEventGroupSetBits(xGPIO_LED_Button_Internal_synk, /* The event group being updated. */
                         converted_val);                 /* The bits being set. */
#endif
      GPIOIntPC1.pressed = 0;
    }
  }
}
/**
 *
 * @fn void GPIOwrite_task(void *arg)
 *
 * @brief GPIO writing task.
 *
 * This task will wait a message that ask to change the state of the leds and will change it accordingly
 * (the communication task that will send it to the plc will send change to do on the led state)
 *
 *
 */
void GPIOwrite_task(void *arg)
{
  EventBits_t Button_State = 0;
  EventBits_t synk = 0;
  uint8_t data = 0;
  uint16_t val;
  while (1)
  {
#if INTERNAL_LED_BUTTON_STATE_SYNCHRONISATION_TEST
    // for an internal sychronisation of the led state to the button actual state
    Button_State = xEventGroupWaitBits(xGPIO_LED_Button_Internal_synk, 0xFF, pdTRUE, pdFALSE, portMAX_DELAY);
#elif INTERNAL_LED_SNAKE_TEST
    debug_LED();
#elif INTERNAL_LED_CURRENT_TEST
    max_current_LED_test();
#else
    xEventGroupWaitBits(xEvent_data_COM, RECEIVE_LED_FLAG, pdTRUE, pdFALSE, portMAX_DELAY);
#if DEBUG
    printf(" LED_UPDATE \n");
#endif
    xQueueReceive(xQueue_data_LED_COM, &data, portMAX_DELAY);
#if DEBUG
    printf(" LED_UPDATE_DATA=%d \n", data);
#endif
    Button_State = data;
#endif
#if (!INTERNAL_LED_SNAKE_TEST && !INTERNAL_LED_CURRENT_TEST)
#if DEBUG
    printf("button state %x", Button_State);
#endif
    val = 0xFF;
    if ((Button_State & (1 << 0)) == (1 << 0)) val = val & LED1;
    if ((Button_State & (1 << 1)) == (1 << 1)) val = val & LED2;
    if ((Button_State & (1 << 2)) == (1 << 2)) val = val & LED3;
    if ((Button_State & (1 << 3)) == (1 << 3)) val = val & LED4;
    if ((Button_State & (1 << 0)) != (1 << 0) && (Button_State & (1 << 1)) != (1 << 1) &&
        (Button_State & (1 << 2)) != (1 << 2) && (Button_State & (1 << 3)) != (1 << 3))
      val = 0xFF;
    ESP_ERROR_CHECK(GPIO_register_write_byte(GPIO2_ADDR, val));
    val = 0xFF;
    if ((Button_State & (1 << 4)) == (1 << 4)) val = val & LED5;
    if ((Button_State & (1 << 5)) == (1 << 5)) val = val & LED6;
    if ((Button_State & (1 << 6)) == (1 << 6)) val = val & LED7;
    if ((Button_State & (1 << 7)) == (1 << 7)) val = val & LED8;
    if ((Button_State & (1 << 4)) != (1 << 4) && (Button_State & (1 << 5)) != (1 << 5) &&
        (Button_State & (1 << 6)) != (1 << 6) && (Button_State & (1 << 7)) != (1 << 7))
      val = 0xFF;
    ESP_ERROR_CHECK(GPIO_register_write_byte(GPIO1_ADDR, val));
#endif
  }
}
/**
 *
 * @fn static void IRAM_ATTR isrHandler(void* arg)
 *
 * @brief handler of the interrupt that will be generated by the GPIO
 *
 * each time one of the gpio will activate the interrupt this function will send a flag and update the state on the
 * struct
 *
 * @param int val value tha comme from reading the GPIO1 registers
 *
 */
static void IRAM_ATTR isrHandler(void *arg)
{
  GPIOIntPC1.numberKeyPresses += 1;
  GPIOIntPC1.pressed = 1;
  BaseType_t xHigherPriorityTaskWoken, xResult;
  /* xHigherPriorityTaskWoken must be initialised to pdFALSE. */
  xHigherPriorityTaskWoken = pdFALSE;
  /* Set bit 0 and bit 4 in xEventGroup. */
  xResult = xEventGroupSetBitsFromISR(xGPIOIntPCEventBits,       /* The event group being updated. */
                                      GPIOIntPC_INTERRUPT_0_BIT, /* The bits being set. */
                                      &xHigherPriorityTaskWoken);
}
/**
 *
 * @fn void Button_isr_config()
 *
 * @brief function of configuration for the handling the interrupt from the GPIO chips
 *
 * Configuration of the pin that will be bind to the isr  and creation of the flag group to communicate from the handler
 *
 *@param arg FreeRTOS standard argument of a task
 *
 */
void Button_isr_config()
{
  // create event group for the GPIOIntPC isr to wakeup the GPIOIntPC task treatment and assign it a earlier created
  // reference handler
  xGPIOIntPCEventBits = xEventGroupCreate();
#if INTERNAL_LED_BUTTON_STATE_SYNCHRONISATION_TEST
  // for an internal sychronisation of the led state to the button actual state
  xGPIO_LED_Button_Internal_synk = xEventGroupCreate();
#endif
  /* config of the pin of the isr*/
  gpio_config_t GPIO_INT_PIN_CONFIG = {
      .intr_type = GPIO_INTR_NEGEDGE, // activate when a transition from 1 to 0 (all the gpio are connected to the same
                                      // interrupt line with a pull up on the line an they connect to the ground for
                                      // sending an interruption)
      .pin_bit_mask = (1ULL << GPIOIntPC1.PIN),
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
  };
  gpio_config(&GPIO_INT_PIN_CONFIG);
  gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
  gpio_set_intr_type(
      GPIOIntPC1.PIN,
      GPIO_INTR_NEGEDGE); // activate when a transition from 1 to 0 (all the gpio are connected to the same interrupt
                          // line with a pull up on the line an they connect to the ground for sending an interruption)
  gpio_isr_handler_add(GPIOIntPC1.PIN, isrHandler, NULL); // link the handler function to the isr
  gpio_intr_enable(GPIOIntPC1.PIN);
}
/**
 *
 * @fn static esp_err_t i2c_master_init(void)
 *
 * @brief i2c master initialization
 *
 * will initialize all the i2c communications with the GPIOs
 *
 * @return esp error type that will statued if the driver as been installed successfully
 *
 */
static esp_err_t i2c_master_init(void)
{
  int i2c_master_port = I2C_MASTER_NUM;
  i2c_config_t conf = {
      .mode = I2C_MODE_MASTER,
      .sda_io_num = I2C_MASTER_SDA_IO,
      .scl_io_num = I2C_MASTER_SCL_IO,
      .sda_pullup_en = GPIO_PULLUP_DISABLE,
      .scl_pullup_en = GPIO_PULLUP_DISABLE,
      .master.clk_speed = I2C_MASTER_FREQ_HZ,
  };
  i2c_param_config(i2c_master_port, &conf);
  return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE,
                            0); // no limit on the buffers, to connect to this i2c you have to access the board, but
                                // don't protect against buffer overflow reverse engineering
}
/**
 *
 * @fn void Button_i2c_config()
 *
 * @brief i2c master configuration
 *
 * will configure the i2c communication for the esp to be the master
 *
 */
void Button_i2c_config() { ESP_ERROR_CHECK(i2c_master_init()); }
/**
 *
 * @fn void Button_i2c_delte()
 *
 * @brief i2c driver erasing
 *
 * will delete the i2c driver
 *
 */
void Button_i2c_delte() { ESP_ERROR_CHECK(i2c_driver_delete(I2C_MASTER_NUM)); }
