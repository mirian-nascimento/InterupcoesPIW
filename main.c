#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "led_matrix.pio.h"

// Definições de hardware
#define NUM_PIXELS 25
#define LED_MATRIX_PIN 7
#define LED_VERDE 11
#define LED_AZUL 12
#define LED_VERMELHO 13
#define BOTAO_A 5
#define BOTAO_B 6
#define DEBOUNCE_DELAY_MS 200

// Variáveis globais
volatile int contador = 0;
PIO pio_instance;
uint state_machine;

// Protótipos de funções
uint32_t rgb_to_32bit(double r, double g, double b);
void display_number(int num, PIO pio, uint sm);
void gpio_callback(uint gpio, uint32_t events);
void initialize_hardware(void);

// Função principal
int main() {
    stdio_init_all();
    initialize_hardware();

    while (true) {
        gpio_put(LED_VERMELHO, !gpio_get(LED_VERMELHO));
        sleep_ms(500);
    }
    return 0;
}

// Implementação das funções

uint32_t rgb_to_32bit(double r, double g, double b) {
    unsigned char R = r * 255;
    unsigned char G = g * 255;
    unsigned char B = b * 255;
    return (G << 24) | (R << 16) | (B << 8);
}

void display_number(int num, PIO pio, uint sm) {
    static const double numeros[10][25] = {
        {1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1}, // 0
        {0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1}, // 1
        {1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1}, // 2
        {1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1}, // 3
        {1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1}, // 4
        {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1}, // 5
        {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1}, // 6
        {1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0}, // 7
        {1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1}, // 8
        {1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1}  // 9
    };

    for (int i = 0; i < NUM_PIXELS; i++) {
        pio_sm_put_blocking(pio, sm, rgb_to_32bit(numeros[num][24 - i], 0, 0));
    }
}

void gpio_callback(uint gpio, uint32_t events) {
    static uint32_t last_interrupt_time = 0;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    if (current_time - last_interrupt_time > DEBOUNCE_DELAY_MS) {
        if (gpio == BOTAO_A && contador < 9) contador++;
        if (gpio == BOTAO_B && contador > 0) contador--;
        display_number(contador, pio_instance, state_machine);
        last_interrupt_time = current_time;
    }
}

void initialize_hardware(void) {
    gpio_init(LED_VERMELHO);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);

    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);

    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);

    pio_instance = pio0;
    uint offset = pio_add_program(pio_instance, &led_matrix_program);
    state_machine = pio_claim_unused_sm(pio_instance, true);
    led_matrix_program_init(pio_instance, state_machine, offset, LED_MATRIX_PIN);

    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
}