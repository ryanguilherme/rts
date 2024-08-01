#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// Definições de pinos
#define LED_PIN 13 // Pino do LED
#define ADC_PIN 26 // Pino do ADC (GPIO26)
#define BUZZER_PIN 15

// Tamanho da Queue
#define QUEUE_LENGTH 10

// Definições dos pinos dos segmentos e dígitos
#define DIGIT_1_PIN  7
#define DIGIT_2_PIN  8
#define DIGIT_3_PIN  9
#define DIGIT_4_PIN  10

#define SEGMENT_A_PIN  0
#define SEGMENT_B_PIN  1
#define SEGMENT_C_PIN  2
#define SEGMENT_D_PIN  3
#define SEGMENT_E_PIN  4
#define SEGMENT_F_PIN  5
#define SEGMENT_G_PIN  6

// Mapeamento dos segmentos para cada dígito
const int zero[] = {0, 1, 2, 3, 4, 5, -1};
const int one[] = {1, 2, -1};
const int two[] = {0, 1, 6, 4, 3, -1};
const int three[] = {0, 1, 6, 2, 3, -1};
const int four[] = {5, 6, 1, 2, -1};
const int five[] = {0, 5, 6, 2, 3, -1};
const int six[] = {0, 5, 4, 3, 2, 6, -1};
const int seven[] = {0, 1, 2, -1};
const int eight[] = {0, 1, 2, 3, 4, 5, 6, -1};
const int nine[] = {6, 5, 0, 1, 2, -1};

// Pinos dos segmentos e dígitos
const uint8_t SEGMENT_PINS[7] = {
        SEGMENT_A_PIN,
        SEGMENT_B_PIN,
        SEGMENT_C_PIN,
        SEGMENT_D_PIN,
        SEGMENT_E_PIN,
        SEGMENT_F_PIN,
        SEGMENT_G_PIN
};

const uint8_t DIGIT_PINS[4] = {
        DIGIT_1_PIN,
        DIGIT_2_PIN,
        DIGIT_3_PIN,
        DIGIT_4_PIN
};

void display_clear() {
    gpio_put(0, 1);
    gpio_put(1, 1);
    gpio_put(2, 1);
    gpio_put(3, 1);
    gpio_put(4, 1);
    gpio_put(5, 1);
    gpio_put(6, 1);
}

void update_display() {

    int last = 7;
    while (1) {
        char buffer[5];
        uint16_t adc_value = adc_read();
        sprintf(buffer, "%u", adc_value);
        for (int i = 7; buffer[i - 7] != '\0'; i++) {
            if (i != last) {
                gpio_put(last, 0);
            }
            last = i;
            gpio_put(last, 1);
            if (buffer[i - 7] == '0') {
                for (int j = 0; zero[j] != -1; j++) {
                    gpio_put(zero[j], 0);
                }
                //    sleep_ms(5);
            }
            if (buffer[i - 7] == '1') {
                for (int j = 0; one[j] != -1; j++) {
                    gpio_put(one[j], 0);
                }
                //    sleep_ms(5);
            }
            if (buffer[i - 7] == '2') {
                //printf("two\n");
                for (int j = 0; two[j] != -1; j++) {
                    gpio_put(two[j], 0);
                }
                //      sleep_ms(5);
            }
            if (buffer[i - 7] == '3') {
                for (int j = 0; three[j] != -1; j++) {
                    gpio_put(three[j], 0);
                }
                //     sleep_ms(5);
            }
            if (buffer[i - 7] == '4') {
                for (int j = 0; four[j] != -1; j++) {
                    gpio_put(four[j], 0);
                }
                //     sleep_ms(5);
            }
            if (buffer[i - 7] == '5') {
                for (int j = 0; five[j] != -1; j++) {
                    gpio_put(five[j], 0);
                }
                //    sleep_ms(5);
            }
            if (buffer[i - 7] == '6') {
                for (int j = 0; six[j] != -1; j++) {
                    gpio_put(six[j], 0);
                }
                //    sleep_ms(5);
            }
            if (buffer[i - 7] == '7') {
                for (int j = 0; seven[j] != -1; j++) {
                    gpio_put(seven[j], 0);
                }
                //       sleep_ms(5);
            }
            if (buffer[i - 7] == '8') {
                for (int j = 0; eight[j] != -1; j++) {
                    gpio_put(eight[j], 0);
                }
                //    sleep_ms(5);
            }
            if (buffer[i - 7] == '9') {
                for (int j = 0; nine[j] != -1; j++) {
                    gpio_put(nine[j], 0);
                }
                //      sleep_ms(5);
            }
            sleep_ms(5);
            display_clear();
        }
    }
}

// Função para inicializar o display
void init_display() {
    // Configura os pinos dos segmentos como saídas
    for (int i = 0; i < 7; ++i) {
        gpio_init(SEGMENT_PINS[i]);
        gpio_set_dir(SEGMENT_PINS[i], GPIO_OUT);
        //gpio_put(SEGMENT_PINS[i], 0); // Inicializa todos os segmentos apagados
    }
    // Configura os pinos dos dígitos como saídas
    for (int i = 0; i < 4; ++i) {
        gpio_init(DIGIT_PINS[i]);
        gpio_set_dir(DIGIT_PINS[i], GPIO_OUT);
        //gpio_put(DIGIT_PINS[i], 0); // Inicializa todos os dígitos apagados
    }
}

// Protótipos de funções
void adc_task(void *params);
void led_task(void *params);



void buzzer_toggle() {
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    gpio_put(BUZZER_PIN, 1);
    sleep_ms(100);
    gpio_put(BUZZER_PIN, 0);
}

// Queue Handle
QueueHandle_t adcQueue;

int main() {
    // Inicializa o padrão da Raspberry Pi Pico
    stdio_init_all();
    init_display();


    // Inicializa GPIO do LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Inicializa o ADC
    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(0); // Seleciona o canal ADC0

    // Cria a Queue
    adcQueue = xQueueCreate(QUEUE_LENGTH, sizeof(uint16_t));

    if (adcQueue == NULL) {
        printf("Não foi possível criar a Queue.\n");
        while (1); // Loop infinito em caso de falha na criação da Queue
    }

    // Cria as tarefas
    xTaskCreate(adc_task, "ADC Task", 256, NULL, 1, NULL);
    xTaskCreate(led_task, "LED Task", 256, NULL, 1, NULL);
    xTaskCreate(update_display, "Display Task", 256, NULL, 1, NULL);

    // Inicia o scheduler do FreeRTOS
    vTaskStartScheduler();

    while (1); // Loop infinito principal
}

void adc_task(void *params) {
    uint16_t adc_value;

    while (1) {
        // Lê o valor do ADC
        adc_value = adc_read();
        printf("adc value: %d\n", adc_value);

        // Envia o valor do ADC para a Queue
        if (xQueueSend(adcQueue, &adc_value, portMAX_DELAY) != pdPASS) {
            printf("Falha ao enviar para a Queue.\n");
        }

        // Espera 300ms
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

void led_task(void *params) {
    uint16_t adc_value;
    //char buffer[5];
    uint16_t buzzer_flag;

    while (1) {
        // Recebe o valor da Queue
        if (adc_read() <= 2000) {
            buzzer_flag = 0;
        } else {
            buzzer_flag = 1;
        }
        if (xQueueReceive(adcQueue, &adc_value, portMAX_DELAY) == pdPASS) {
            //sprintf(buffer, "%u", adc_value);
            // Aciona o LED dependendo do valor do ADC
            if ((buzzer_flag == 0 && adc_value > 2000) || (buzzer_flag == 1 && adc_value < 2000)) {
                buzzer_toggle();
            }
            if (adc_value > 2000) {
                gpio_put(LED_PIN, 1); // Acende o LED
                //buzzer_toggle();
            } else {
                gpio_put(LED_PIN, 0); // Apaga o LED
            }
            //update_display(buffer);

        } else {
            printf("Falha ao receber da Queue.\n");
        }
    }
}

