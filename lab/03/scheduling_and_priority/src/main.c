/*
	Author: Ryan Guilherme (github.com/ryanguilherme)
	Description: This program is a basic FreeRTOS program on Raspberry Pico W
	using RP2040 to blink it onboard LED using a FreeRTOS Task
*/

#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

void onboard_led_toggle() {
    const uint BOARD_LED_PIN = CYW43_WL_GPIO_LED_PIN;
    while (true) {
	printf("Onboard led toggle!\n");
	cyw43_arch_gpio_put(BOARD_LED_PIN, 1);
	vTaskDelay(100);
	cyw43_arch_gpio_put(BOARD_LED_PIN, 0);
	vTaskDelay(100);
    }
}

int main()
{
    stdio_init_all();

    if (cyw43_arch_init()) return -1;

    xTaskCreate(onboard_led_toggle, "Onboard_LED_Task", 256, NULL, 1, NULL);
    vTaskStartScheduler();

    while(1){};
}
