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
TaskHandle_t TaskHandle_3;

static unsigned long int idle_tick_counter = 0;
void vApplicationIdleHook(void)
{
    unsigned long int tick = xTaskGetTickCount();
    while (xTaskGetTickCount() == tick);
    idle_tick_counter++;
}

void taskCPUUsage(void *pvParameters)
{
    unsigned long int idle_tick_last, ticks;
    idle_tick_last = idle_tick_counter = 0;
    for (;;) {
        /* wait for 3 seconds */
        vTaskDelay(3000/portTICK_PERIOD_MS);

        /* calculate quantity of idle ticks per second */
        if (idle_tick_counter > idle_tick_last)
            ticks = idle_tick_counter - idle_tick_last;
        else
            ticks = 0xFFFFFFFF - idle_tick_last + idle_tick_counter;
        ticks /= 4;

        /* print idle ticks per second */
        printf("%ld idle ticks per second (out of %ld)\n", ticks, configTICK_RATE_HZ);

        /* calc and print CPU usage */
        ticks = (configTICK_RATE_HZ - ticks) / 10;
        printf("CPU usage: %d%%\n", ticks);

        /* update idle ticks */
        idle_tick_last = idle_tick_counter;
    }
}

void taskUseCPU(void *pvParameters)
{
    unsigned int i, j;
    for (;;) {
        for (i = 0, j = 0; i < 10000; i++){
            j *= i + 12.34;
        }
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}


/** This function receives a GPIO as a parameter, set as output and blink the led (alternate between 0 and 1) every 100 ms **/
void led_blink(void * GPIO) {
    gpio_init((int)GPIO);
    gpio_set_dir((int)GPIO, GPIO_OUT);
    while(1) {
        gpio_put((int)GPIO, 1);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        gpio_put((int)GPIO, 0);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        //printf("Led blink\n");
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
            gpio_put(LED_0, 0);
            gpio_put(LED_1, 0);
            vTaskDelete(TaskHandle_1);
            vTaskDelete(TaskHandle_2);
            vTaskDelete(NULL);
        }
    }
}

int main()
{
    stdio_init_all();
    if (cyw43_arch_init()) return -1;
    xTaskCreate(led_blink, "LED_TASK_0", 256, (void *) LED_0, 2, &TaskHandle_1);
    xTaskCreate(led_blink, "LED_TASK_1", 256, (void *) LED_1, 2, &TaskHandle_2);
    xTaskCreate(reset_button, "RST_BUTTON", 256, NULL, 1, &TaskHandle_3);

    xTaskCreate(taskCPUUsage, (signed char *)"Task CPU Usage", configMINIMAL_STACK_SIZE * 4, (void *)NULL, 0, NULL);

    vTaskStartScheduler();

    while(1){};
}