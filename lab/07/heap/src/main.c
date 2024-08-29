#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <stdlib.h>
#include "portable.h"
#include "pico/stdlib.h"
#include "queue.h"

#define LED_PIN     13
#define BUTTON_PIN  16
#define BUTTON2_PIN 15

QueueHandle_t queue;

void show_free_heap_size(void *pvParameters) {
    unsigned int freeHeap;
    float percentage;
    while (1) {
        freeHeap = xPortGetFreeHeapSize();
        percentage = ((float)freeHeap / (float)configTOTAL_HEAP_SIZE) * 100;
        if (xQueueSend(queue, &percentage, portMAX_DELAY) != pdPASS) {
            printf("[ERROR!] show_free_heap_size: Could not send percentage value to queue!\n");
        }
        printf("%ul\n", freeHeap);
        vTaskDelay(500);
    }
}

void check_heap_percentage(void *pvParameters) {
    float percentage;
    while (1) {
        if (xQueueReceive(queue, &percentage, portMAX_DELAY) == pdPASS) {
            printf("Current free heap percentage: %.2f\n", percentage);
            if (percentage <= 10) {
                gpio_put(LED_PIN, 1);
            } else {
                gpio_put(LED_PIN, 0);
            }
        } else {
            printf("[ERROR!] check_heap_percentage: Could not receive percentage value from queue!\n");
        }

        vTaskDelay(500);
    }
}

void fill_heap_memory(void *pvParameters) {
    int *consumer;
    float alloc_amount = 0.7;
    while (1) {
        if (gpio_get(BUTTON_PIN) == 1) {
            consumer = (int*)pvPortMalloc(alloc_amount * sizeof(int));
            vTaskDelay(20);
        } else if (gpio_get(BUTTON2_PIN) == 1) {
            if (consumer != NULL) {
                vPortFree(consumer);
            }
            vTaskDelay(20);
        }
    }
}

int main()
{
    stdio_init_all();
    gpio_init(LED_PIN);             gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_init(BUTTON_PIN);          gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_init(BUTTON2_PIN);         gpio_set_dir(BUTTON2_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);       gpio_pull_up(BUTTON2_PIN);
    queue = xQueueCreate(10, sizeof(float));

    xTaskCreate(show_free_heap_size, "ShowFreeHeapSizeTask", 256, NULL, 1, NULL);
    xTaskCreate(check_heap_percentage, "CheckHeapPercentageTask", 256, NULL, 1, NULL);
    xTaskCreate(fill_heap_memory, "FillHeapMemoryTask", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while(1){};
}