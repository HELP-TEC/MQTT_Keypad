/**
 * @file adc_module.c
 * MQTT (over TCP)
 * @date 6.03.2022
 * Created on: 6 mai 2022
 * @author Thibault Sampiemon
 */

#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "freertos/event_groups.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "adc_module.h"
#include "mqtt_module.h"

#define DEBUG 0 //!< 1 to activate serial debug messages

/**
 * ADC Variables
 * */
//-----------Generic adc const----------------------------------------------------------------------------------------------------------
//:: Disclaymer this time is not exact and depand of the sceduling
//if problemetic chek that the priority of the tasck is the highest and the interuptions are quick and their data processed in a task
//!< MUST:MEAN_NO_OF_SAMPLES*MEAN_TIME_INTERVAL << SAMPLING_PERIOD !!
#define SAMPLING_PERIOD 25 //!< sampling period in [ms]
//------------Multisampling MEAN FILTER Constant--------------------------------------------------------------------------------------
//This mean filter is a fixed windows filter each T period it makes a mean of N sample
//and and make a mean of it the result is the real sample that we take each T constant period
#define MEAN_NO_OF_SAMPLES 5         //!< MUST:MEAN_NO_OF_SAMPLES*MEAN_TIME_INTERVAL << SAMPLING_PERIOD !! Number of sample in the first mean
#define MEAN_TIME_INTERVAL 1			//!< In [ms] Time between 2 simples of the mean
//----------Low pass (pseudo expodentialy ponderated mean)--------------------------------------------------------------------------------
//!< MUST:L_P_PRESENT_NOMINATOR+L_P_PAST_NOMINATOR=L_P_PROPORTION_DENOMINATOR !!!!
#define L_P_PRESENT_NOMINATOR 1.0
//!< MUST:L_P_PRESENT_NOMINATOR+L_P_PAST_NOMINATOR=L_P_PROPORTION_DENOMINATOR !!!!
#define L_P_PAST_NOMINATOR 9.0
//----------Output limit (minimum and maximum of desired out)--------------------------------------------------------------------------------
#define OUT_MIN 0 		//!< minimum value that you can send to the broker the value will NEVER be lower
#define OUT_MAX 100		//!< maximum value that you can send to the broker the value will NEVER be higher
//----------Input limit (minimum and maximum of in for desired mechanical range)--------------------------------------------------------------
#define IN_MIN 300 //!< minimum value coming from adc (you can calibrate it to use the full range of the potentiometer (can variety a little bit because of the tolerancy of the electronicall component)
#define IN_MAX 2300 //!< maximum value coming from adc (you can calibrate it to use the full range of the potentiometer (can variety a little bit because of the tolerancy of the electronicall component)
//-------------------------------------------------------------------------------------------------------------------------------------------------
//----------calculated Const--------------------------------------------------------------------------------------------
//--GENERIC--
//!< MUST:MEAN_NO_OF_SAMPLES*MEAN_TIME_INTERVAL << SAMPLING_PERIOD !!
#define SAMPLING_TIME_CONST (SAMPLING_PERIOD-MEAN_NO_OF_SAMPLES*MEAN_TIME_INTERVAL) //time delay to reach the samppling period
//--LOW_PASS--
//!< MUST:L_P_PRESENT_NOMINATOR+L_P_PAST_NOMINATOR=L_P_PROPORTION_DENOMINATOR
#define L_P_PROPORTION_DENOMINATOR (L_P_PRESENT_NOMINATOR+L_P_PAST_NOMINATOR) //!< denominator of the past and present value proportion
#define L_P_PAST_PROP (L_P_PAST_NOMINATOR/L_P_PROPORTION_DENOMINATOR)
#define L_P_PRESENT_PROP (L_P_PRESENT_NOMINATOR/L_P_PROPORTION_DENOMINATOR)
//--Downscaling--
#define BIAS (IN_MIN/OUT_MAX) //!< bias after downscaling
#define PROPORTIONALITY_FACTOR (IN_MAX/OUT_MAX) //!< conversion factor (conversion line slope)
//------------calculation 1 time (economy of running time)-------------------
const float c_L_P_PAST_PROP = (float) L_P_PAST_PROP;
const float c_L_P_PRESENT_PROP = (float) L_P_PRESENT_PROP;
const uint32_t c_SAMPLING_TIME_CONST = (uint32_t) SAMPLING_TIME_CONST;
const uint32_t c_BIAS = (uint32_t) BIAS;
const uint32_t c_PROPORTIONALITY_FACTOR = (uint32_t) PROPORTIONALITY_FACTOR;
//---------------------------------------------------------------------------------------------------------------------------------------------------

static esp_adc_cal_characteristics_t *adc_chars; //!< ADC strukt value from esp adc driver

#define DEFAULT_VREF    1100        					//!< adc voltage referece (adc2_vref_to_gpio() best if change to adc2)

static const adc_channel_t channel = ADC_CHANNEL_7; //!< GPIO35 if ADC1 our potentiometer connected pin
static const adc_bits_width_t width = ADC_WIDTH_BIT_12; //!< our adc value definition
static const adc_atten_t atten = ADC_ATTEN_DB_11; //!< input voltage of ADC will be attenuated extending the range of measurement by about 11 dB
static const adc_unit_t unit = ADC_UNIT_1; //!< esp32 dont suport othe options must be ADC_UNIT_1

/**
 *
 * @fn void potentiometer_task(void *arg)
 *
 * @brief potentiometer value reading task
 *
 *
 * task that read the value of the potentiometer (with the esp adc) by pooling, process it, filter it, and then if the converted value that will be send further have changed will send it further (if not dont send anything)
 *
 * @param arg FreeRTOS standard argument of a task
 *
 *
 */
void potentiometer_task(void *arg) {
	//Characterize ADC
	adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
	esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, adc_chars);
	static uint32_t adc_reading_old = 0; 	//memory of mesurment at t-1
	static int32_t adc_schmit = 0; //last value out of schmitt trigger (output value)
	uint32_t adc_reading = 0;			//current mesurment
	int32_t adc_transformed = 0;		//val out of filteing and downscaling
	uint32_t i = 0;					//increment of the multisimpling for loop
	//Continuously sample ADC1
	while (1) {

		//reinitialisation of value that are recalculed at each (multi)sampling
		adc_reading = 0;
		adc_transformed = 0;

		//-----------------Multisampling MEAN Filter-------------------
		for (i = 0; i < MEAN_NO_OF_SAMPLES; i++) {
			adc_reading += adc1_get_raw((adc1_channel_t) channel);
			vTaskDelay(pdMS_TO_TICKS(MEAN_TIME_INTERVAL));
		}
		adc_reading /= MEAN_NO_OF_SAMPLES;
		//-------------------------------------------------------------

		//----------Low pass (pseudo expodentialy ponderated mean)-----------
		adc_reading = c_L_P_PAST_PROP * adc_reading_old
				+ c_L_P_PRESENT_PROP * adc_reading;
		adc_reading_old = adc_reading;
		//---------------------------------------------------------------------

		//-----------Convert adc_reading to voltage in mV--------------------
		uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
		//-----------------------------------------------------------------------

		//------Down scaling of the resolution (down-quantisation)-----
		//(supression of the bias to have a purly proportional value)
		adc_transformed = ((adc_reading / c_PROPORTIONALITY_FACTOR) - c_BIAS);
		//--------------------------------------------------------------

		//--------------limit definition for the output value---------------
		adc_transformed =
				(adc_transformed < OUT_MIN) ? OUT_MIN : adc_transformed;
		adc_transformed =
				(adc_transformed > OUT_MAX) ? OUT_MAX : adc_transformed;
		//------------------------------------------------------------------

		//--------------------------------Schmitt trigger-----------------------------------------------
		if (adc_schmit != adc_transformed) {
			if (((adc_reading
					< ((adc_schmit * c_PROPORTIONALITY_FACTOR)
							+ c_BIAS * c_PROPORTIONALITY_FACTOR
							- (c_PROPORTIONALITY_FACTOR / 2)))
					&& (adc_transformed < adc_schmit))
					|| ((adc_reading
							> ((adc_schmit * c_PROPORTIONALITY_FACTOR)
									+ c_PROPORTIONALITY_FACTOR
									+ (c_PROPORTIONALITY_FACTOR / 2)))
							&& (adc_transformed > adc_schmit))) {
				adc_schmit = adc_transformed;
#if DEBUG
					printf("Raw: %d\t Raw down res: %d\tVoltage: %dmV\n",adc_reading, adc_schmit, voltage);
#endif

				xEventGroupSetBits(xEvent_data_COM, /* The event group being updated. */
				SEND_POTENTIOMETRE_FLAG); /* The bits being set. */
				xQueueSend(xQueue_data_Potentimeter_COM, &adc_schmit, pdFALSE);
			}
		}
		//--------------------------------------------------------------------------------------
		
		vTaskDelay(pdMS_TO_TICKS(c_SAMPLING_TIME_CONST));
	}

}

/**
 *  @fn void potentiometer_config()
 *
 *  @brief potentiometer configuration
 *
 *  configure the adc for the potentiometer reading
 *
 *
 */

void potentiometer_config() {
	adc1_config_width(width);
	adc1_config_channel_atten(channel, atten);
}
