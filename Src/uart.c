#include "uart.h"
#include "rcc.h"
#include "nvic.h"
#include "gpio.h"

static volatile command_t last_command = CMD_NONE;

void usart2_init(void)
{
    configure_gpio_for_usart();

    *RCC_APB1ENR1 |= RCC_APB1ENR1_USART2EN;

    // 1. Desactivar la UART
    USART2->CR1 &= ~USART_CR1_UE;
    // 2. Configurar la velocidad de transmisión
    USART2->BRR = BAUD_9600_4MHZ;
    // 3. Configurar el número de bits de datos
    USART2->CR1 &= ~USART_CR1_M;
    // 4. Configurar el número de bits de parada
    USART2->CR2 &= ~USART_CR2_STOP;
    // 5. Habilitar Transmisor y Receptor
    USART2->CR1 |= USART_CR1_TE | USART_CR1_RE;
    // 6. Habilitar la UART
    USART2->CR1 |= USART_CR1_UE;

    // Activar interrupción de RXNE
    USART2->CR1 |= USART_CR1_RXNEIE; 
    NVIC->ISER[1] |= (1 << 6);
}

void usart2_send_string(const char *str)
{
    while (*str) {
        while (!(USART2->ISR & USART_ISR_TXE));
        USART2->TDR = *str++;
    }
}

command_t usart2_get_command(void)
{
    command_t cmd = last_command;
    last_command = CMD_NONE;
    return cmd;
}


void USART2_IRQHandler(void)
{
    uint32_t isr = USART2->ISR;
    if (isr & USART_ISR_RXNE) {
        char command = USART2->RDR;
        if (command == 'O') {
            last_command = CMD_OPEN;
            usart2_send_string("DOOR UNLOCKED \r\n");

        } else if (command == 'C') {
    last_command = CMD_CLOSE;
    usart2_send_string("DOOR CLOSED \r\n");     
    gpio_set_door_led_state(0); // Apagar LED de estado de la puerta
    usart2_send_string("LED APAGADO\r\n");

    // Esperar confirmación para volver a encender el LED
    while (1) {
        // Verificar comandos UART
        gpio_set_door_led_state(0); // Apagar LED de estado de la puerta
        uint8_t new_command = usart2_get_command();
        if (new_command == 'O') { // Comando para abrir la puerta
            usart2_send_string("DOOR OPENED \r\n");
            gpio_set_door_led_state(1); // Encender LED fijo
            break; // Salir del bucle y continuar
        }

        // Verificar pulsaciones de botón
        uint8_t button_pressed = button_driver_get_event();
        if (button_pressed == 1) { // Pulsación sencilla
            usart2_send_string("Single Button Press\r\n");
            gpio_set_door_led_state(1); // Encender LED fijo
            break; // Salir del bucle y continuar
        } else if (button_pressed == 2) { // Doble pulsación
            usart2_send_string("Double Button Press\r\n");
            usart2_send_string("DOOR PERMANENTLY UNLOCKED\r\n");
            gpio_set_door_led_state(1); // Encender LED fijo
            break; // Salir del bucle y continuar
        }
    }
}
    }
}
