/**
	Author: Ryan Guilherme (github.com/ryanguilherme)
	Description: This program blink two leds, and if the defined button is pressed then blink the Raspberry Pico W onboard
    led 3 times then end the program.
**/
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

const uint LED_0 = 13;
const uint LED_1 = 15;
TaskHandle_t TaskHandle_1;
TaskHandle_t TaskHandle_2;

/** This function receives a GPIO as a parameter, set as output and blink the led (alternate between 0 and 1) every 100 ms **/
void led_blink(void * GPIO) {
    gpio_init((int)GPIO);
    gpio_set_dir((int)GPIO, GPIO_OUT);
    while(1) {
        gpio_put((int)GPIO, 1);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        gpio_put((int)GPIO, 0);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

/** This function set the GPIO 14, if this button is pressed, blink pico's onboard led 3 times then end the program **/
void reset_button() {
    const uint GPIO = 14;
    gpio_init(GPIO);
    gpio_set_dir(GPIO, GPIO_IN);
    while(1) {
        if (gpio_get(GPIO)) {
            for (int i=0; i<3; i++) {
                cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
                vTaskDelay(500 / portTICK_PERIOD_MS);
                cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
                vTaskDelay(500/ portTICK_PERIOD_MS);
            }
            vTaskDelete(&TaskHandle_1);
            vTaskDelete(&TaskHandle_2);
            gpio_put(LED_0, 0);
            gpio_put(LED_1, 0);
            vTaskDelete(NULL);
        }
    }
}

int main()
{
    if (cyw43_arch_init()) return -1;

    stdio_init_all();
    xTaskCreate(led_blink, "LED_TASK_0", 256, (void *) LED_0, 1, &TaskHandle_1);
    xTaskCreate(led_blink, "LED_TASK_1", 256, (void *) LED_1, 1, &TaskHandle_2);
    xTaskCreate(reset_button, "RST_BUTTON", 256, NULL, 1, &TaskHandle_2);

    vTaskStartScheduler();

    while(1){};
}
