idf_component_register(SRCS "app_main.c" "adc_module.c" "button_module.c" "mqtt_module.c" "storage_module.c"
                    INCLUDE_DIRS ".")

#spiffs_create_partition_image(storage ../spiffs_image FLASH_IN_PROJECT)
