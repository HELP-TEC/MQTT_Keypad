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
#define USE_CONFIGURATIONFILE 1 //! use the config file instead of the default config



/*-----------------------------------------------------------------------
 * MQTT communication options
 * --------------------------------------------------------------------*/
#define TOPIC_POTENTIOMETER "topic/potentiometer"  //! topic where the embedded module will publish the potentiometer value ex:"topic/potentiometer4"
#define TOPIC_BUTTON "topic/buttons"  //! topic where the embedded module will publish the buttons value ex:"topic/buttons4"
#define TOPIC_LED "topic/leds"  //! topic that embedded module will subscribe to receive the LEDs value ex:"topic/leds4"


#define USER "pannel1" //! username for the connection to the broker
#define PASS "itisnotagoodpasswordbutwhocarehaha1" //! password for the connection to the broker

/*-----------------------------------------------------------------------
 * PCB options
 * --------------------------------------------------------------------*/
#define CONFIG_WITHOUT_POTENTIOMETER 0

#endif /* MAIN_CONFIG_H_ */
