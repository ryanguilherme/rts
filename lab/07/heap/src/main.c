#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "queue.h"
#include "semphr.h"

#define BUTTON_PIN_1    15
#define BUTTON_PIN_2    16
#define LED_PIN_1       14
#define LED_PIN_2       17

SemaphoreHandle_t semaphore;
uint button_flag = -1;

// This task changes button_flag value based on which button is being pressed
void readButton(void *params) {

    gpio_init(BUTTON_PIN_1);                gpio_init(BUTTON_PIN_2);
    gpio_set_dir(BUTTON_PIN_1, GPIO_IN);   gpio_set_dir(BUTTON_PIN_2, GPIO_IN);
    gpio_pull_up(BUTTON_PIN_1);             gpio_pull_up(BUTTON_PIN_2);

    uint flag;

    while(1) {
        if (gpio_get(BUTTON_PIN_1) && !gpio_get(BUTTON_PIN_2)) {
            flag = 1;
        } else if (!gpio_get(BUTTON_PIN_1) && gpio_get(BUTTON_PIN_2)) {
            flag = 2;
        } else if (gpio_get(BUTTON_PIN_1) && gpio_get(BUTTON_PIN_2)) {
            flag = 3;
        } else {
            flag = 0;
        }
        if (xSemaphoreTake(semaphore, portMAX_DELAY) == pdTRUE) {
            button_flag = flag;
            xSemaphoreGive(semaphore);
        } else {
            printf("ReadButton TASK TRIED TO TAKE THE SEMAPHORE, BUT COULD NOT!\n");
        }
    }
}

// This task print which button is currently pressed
void showState(void *params) {
    uint flag;

    while (1){
        if (xSemaphoreTake(semaphore, portMAX_DELAY) == pdTRUE) {
            flag = button_flag;
            xSemaphoreGive(semaphore);
            if (flag == 1) {
                printf("BUTTON 1 PRESSED!\n");
            } else if (flag == 2) {
                printf("BUTTON 2 PRESSED!\n");
            } else if (flag == 3) {
                printf("BOTH BUTTONS PRESSED\n");
            } else {
                printf("NO BUTTON PRESSED\n");
            }
            sleep_ms(250);
        } else {
            printf("ShowState TASK TRIED TO TAKE THE SEMAPHORE, BUT COULD NOT\n");
        }
    }
}

// This task turn a led on based on which button is being pressed
void main_task(void *params) {
    uint flag;
    gpio_init(LED_PIN_1);               gpio_init(LED_PIN_2);
    gpio_set_dir(LED_PIN_1, GPIO_OUT);  gpio_set_dir(LED_PIN_2, GPIO_OUT);

    while (1){
        if (xSemaphoreTake(semaphore, portMAX_DELAY) == pdTRUE) {
            
            flag = button_flag;
            xSemaphoreGive(semaphore);

            if (flag == 3) {
                gpio_put(LED_PIN_1, !gpio_get(LED_PIN_1)); gpio_put(LED_PIN_2, !gpio_get(LED_PIN_2));
            } else if (flag == 1) {
                gpio_put(LED_PIN_1, !gpio_get(LED_PIN_1));
            } else if (flag == 2) {
                gpio_put(LED_PIN_2, !gpio_get(LED_PIN_2));
            }
            vTaskDelay(50);
        } else {
            printf("MainTask TRIED TO TAKE THE SEMAPHORE, BUT COULD NOT!\n");
        }
    }
}

int main()
{
    stdio_init_all();

    semaphore =  xSemaphoreCreateMutex();

    xTaskCreate(readButton, "ReadButtonTask", 256, NULL, 2, NULL);
    xTaskCreate(main_task, "MainTask", 256, NULL, 1, NULL);
    xTaskCreate(showState, "ShowStateTask", 256, NULL, 1, NULL);
    vTaskStartScheduler();

    while(1){};
}