/**
 * @file storage_module.h
 * Storage essential data for config via python script
 * @date 28.09.2022
 * Created on: 18 september 2022
 * @author Louis COLIN
 */
#ifndef MAIN_MQTT_MODULE_H_
#define MAIN_MQTT_MODULE_H_

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

void Storage_init();
#endif /* MAIN_STORAGE_MODULE_H_ */