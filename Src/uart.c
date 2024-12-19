#include "uart.h"
#include "rcc.h"
#include "nvic.h"
#include "gpio.h"


static volatile command_t last_command = CMD_NONE;

void usart2_init(void)
{
    configure_gpio_for_usart();

    *RCC_APB1ENR1 |= RCC_APB1ENR1_USART2EN;

    // Configurar UART2
    USART2->CR1 = 0;  // Clear CR1 register
    USART2->CR2 = 0;  // Clear CR2 register
    USART2->CR3 = 0;  // Clear CR3 register

    // Set baud rate to 9600 (assuming 4MHz APB clock)
    USART2->BRR = BAUD_9600_4MHZ;

    // Configure word length (8 bits), no parity, and oversampling by 16
    USART2->CR1 &= ~(USART_CR1_M | USART_CR1_PCE | USART_CR1_OVER8);

    // Configure 1 stop bit
    USART2->CR2 &= ~USART_CR2_STOP;

    // Enable transmitter and receiver
    USART2->CR1 |= (USART_CR1_TE | USART_CR1_RE);

    // Enable USART
    USART2->CR1 |= USART_CR1_UE;

    // Wait for USART to be ready
    while (!(USART2->ISR & USART_ISR_TEACK));
    while (!(USART2->ISR & USART_ISR_REACK));

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
        } else if (command == 'C') {
            last_command = CMD_CLOSE;
        }
    }
}

