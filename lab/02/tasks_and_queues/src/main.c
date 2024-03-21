/*
	Author: Ryan Guilherme (github.com/ryanguilherme)
	Description: This program is a basic FreeRTOS program on Raspberry Pico W
	using RP2040 to comunicate two tasks using FreeRTOS, the task 1 blinks a led
    and then sends to the queue the state of the led (0/1), the task 2 read the data
    on the queue and then print the current led state to the serial output via usb
*/

#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "queue.h"

static QueueHandle_t xQueue = NULL;

void onboard_led_toggle() {
    const uint BOARD_LED_PIN = CYW43_WL_GPIO_LED_PIN;
    uint uIValueToSend = 0;
    while (true) {
	printf("Onboard led toggle!\n");
        cyw43_arch_gpio_put(BOARD_LED_PIN, 1);
        uIValueToSend = 1;
        xQueueSend(xQueue, &uIValueToSend, 0U);
        vTaskDelay(100);

        cyw43_arch_gpio_put(BOARD_LED_PIN, 0);
        uIValueToSend = 0;
        xQueueSend(xQueue, &uIValueToSend, 0U);
        vTaskDelay(100);
    }
}

void usb_task(void *pvParameters) {
    uint uIReceivedValue;

    while (1) {
        xQueueReceive(xQueue, &uIReceivedValue, portMAX_DELAY);
        if (uIReceivedValue) printf("LED IS ON! \n");
        else                 printf("LED IS OFF! \n");
    }
}

int main()
{
    stdio_init_all();

    xQueue = xQueueCreate(1, sizeof(uint));

    if (cyw43_arch_init()) return -1;

    xTaskCreate(onboard_led_toggle, "Onboard_LED_Task", 256, NULL, 1, NULL);
    xTaskCreate(usb_task, "USB_Task", 256, NULL, 1, NULL);
    vTaskStartScheduler();

    while(1){};
}
