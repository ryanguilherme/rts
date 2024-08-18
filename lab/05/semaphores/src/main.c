#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include <stdio.h>

// Definições de pinos
#define BUTTON_PIN 15
#define LED_PIN 16

uint button_pressed = 0;

// Tarefas
void button_task(void *pvParameters);
void led_task(void *pvParameters);
void semaphore_test_task(void *pvParameters);

// Semáforos e filas
SemaphoreHandle_t button_semaphore;
QueueHandle_t led_queue;

typedef enum {
    LED_TOGGLE // Adicionada a opção de alternar o estado do LED
} led_command_t;

void button_task(void *pvParameters) {
    (void)pvParameters;
    TickType_t xDelay = pdMS_TO_TICKS(50);

    while (1) {
        // Espera pelo semáforo
        if ((xSemaphoreTake(button_semaphore, pdMS_TO_TICKS(1000)) == pdTRUE) && button_pressed) {
            // Envia um comando para a tarefa do LED alternar seu estado
            printf("Semaphore taken by button task\n");
            led_command_t command = LED_TOGGLE;
            xQueueSend(led_queue, &command, portMAX_DELAY);

            // Debounce simples
            vTaskDelay(xDelay);
            xSemaphoreGive(button_semaphore);
            printf("Semaphore freed by button task\n");
            button_pressed = !button_pressed;
        }
    }
}

void led_task(void *pvParameters) {
    (void)pvParameters;

    led_command_t command;
    while (1) {
        // Recebe o comando da fila
        if (xQueueReceive(led_queue, &command, portMAX_DELAY) == pdTRUE) {
            if (command == LED_TOGGLE) {
                // Alterna o estado do LED
                gpio_put(LED_PIN, !gpio_get(LED_PIN));
            }
        }
    }
}
/*
void semaphore_test_task(void *pvParameters) {
    (void)pvParameters;
    TickType_t xDelay = pdMS_TO_TICKS(1000);

    while (1) {
        // Tenta pegar o semáforo
        if (xSemaphoreTake(button_semaphore, 0) == pdTRUE) {
            printf("Semaphore acquired by button task\n");
            
            // Simula o uso do semáforo
            vTaskDelay(xDelay);
            
            // Libera o semáforo
            xSemaphoreGive(button_semaphore);
            printf("Semaphore released by button task\n");
        }
    }
}*/

void semaphore_check(void *pvParameteres) {
	while (1) {
		if (xSemaphoreTake(button_semaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
			printf("Semaphore free\n");
			vTaskDelay(500);
			xSemaphoreGive(button_semaphore);
		} else {
			printf("Semaphore blocked\n");
			vTaskDelay(500);
		}
	}
}

// Interrupção do botão
void button_isr(uint gpio, uint32_t events) {
    BaseType_t higherPriorityTaskWoken = pdFALSE;
    // Libere o semáforo quando o botão for pressionado
    xSemaphoreGiveFromISR(button_semaphore, &higherPriorityTaskWoken);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
    button_pressed = !button_pressed;
}

// Configuração da interrupção do botão
void setup_button_interrupt(void) {
    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_RISE, true, button_isr);
}

int main() {
    // Inicialize o hardware
    stdio_init_all();
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_down(BUTTON_PIN); // Configura o botão como pull-down
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Crie o semáforo contador com um máximo de 1
    button_semaphore = xSemaphoreCreateCounting(3, 0); // Capacidade do semáforo: 3
    if (button_semaphore == NULL) {
        printf("Failed to create button semaphore\n");
        return -1;
    }

    // Crie a fila
    led_queue = xQueueCreate(10, sizeof(led_command_t));
    if (led_queue == NULL) {
        printf("Failed to create LED queue\n");
        return -1;
    }

    // Crie as tarefas
    xTaskCreate(button_task, "ButtonTask", 256, NULL, 1, NULL);
    xTaskCreate(led_task, "LedTask", 256, NULL, 1, NULL);
    //xTaskCreate(semaphore_test_task, "SemaphoreTestTask", 256, NULL, 1, NULL);
    xTaskCreate(semaphore_check, "SemaphoreCheckTask", 256, NULL, 1, NULL);

    // Configura a interrupção do botão
    setup_button_interrupt();

    // Inicie o scheduler
    vTaskStartScheduler();

    // O código nunca deve chegar aqui
    for (;;) {}
    return 0;
}

