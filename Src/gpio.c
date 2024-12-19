#include "gpio.h"
#include "rcc.h"

#include "systick.h"

#define EXTI_BASE 0x40010400
#define EXTI ((EXTI_t *)EXTI_BASE)

#define EXTI15_10_IRQn 40
#define NVIC_ISER1 ((uint32_t *)(0xE000E104)) // NVIC Interrupt Set-Enable Register


#define SYSCFG_BASE 0x40010000
#define SYSCFG ((SYSCFG_t *)SYSCFG_BASE)


#define GPIOA ((GPIO_t *)0x48000000) // Base address of GPIOA
#define GPIOC ((GPIO_t *)0x48000800) // Base address of GPIOC

#define LED_PIN 5 // Pin 5 of GPIOA
#define BUTTON_PIN 13 // Pin 13 of GPIOC

#define LED_PIN2 8 // Pin 8 of GPIOA
#define BUTTON_PIN2 11 // Pin 11 of GPIOC

void configure_gpio_for_usart(void)
{
    // Enable GPIOA clock
    *RCC_AHB2ENR |= (1 << 0);

    // Configure PA2 (TX) as alternate function
    GPIOA->MODER &= ~(3U << (2 * 2)); // Clear mode bits for PA2
    GPIOA->MODER |= (2U << (2 * 2));  // Set alternate function mode for PA2

    // Configure PA3 (RX) as alternate function
    GPIOA->MODER &= ~(3U << (3 * 2)); // Clear mode bits for PA3
    GPIOA->MODER |= (2U << (3 * 2));  // Set alternate function mode for PA3

    // Set alternate function to AF7 for PA2 and PA3
    GPIOA->AFR[0] &= ~(0xF << (4 * 2)); // Clear AFR bits for PA2
    GPIOA->AFR[0] |= (7U << (4 * 2));   // Set AFR to AF7 for PA2
    GPIOA->AFR[0] &= ~(0xF << (4 * 3)); // Clear AFR bits for PA3
    GPIOA->AFR[0] |= (7U << (4 * 3));   // Set AFR to AF7 for PA3

    // Configure PA2 and PA3 as very high speed
    GPIOA->OSPEEDR |= (3U << (2 * 2)); // Very high speed for PA2
    GPIOA->OSPEEDR |= (3U << (3 * 2)); // Very high speed for PA3

    // Configure PA2 and PA3 as no pull-up, no pull-down
    GPIOA->PUPDR &= ~(3U << (2 * 2)); // No pull-up, no pull-down for PA2
    GPIOA->PUPDR &= ~(3U << (3 * 2)); // No pull-up, no pull-down for PA3
}

void init_gpio_pin(GPIO_t *GPIOx, uint8_t pin, uint8_t mode)
{
    GPIOx->MODER &= ~(0x3 << (pin * 2)); // Clear MODER bits for this pin
    GPIOx->MODER |= (mode << (pin * 2)); // Set MODER bits for this pin
}
/*
void configure_gpio(void)
{
    *RCC_AHB2ENR |= (1 << 0) | (1 << 2); // Enable clock for GPIOA and GPIOC

    // Enable clock for SYSCFG
    *RCC_APB2ENR |= (1 << 0); // RCC_APB2ENR_SYSCFGEN

    // Configure SYSCFG EXTICR to map EXTI13 to PC13
    SYSCFG->EXTICR[3] &= ~(0xF << 4); // Clear bits for EXTI13
    SYSCFG->EXTICR[3] |= (0x2 << 4);  // Map EXTI13 to Port C

    // Configure EXTI13 for falling edge trigger
    EXTI->FTSR1 |= (1 << BUTTON_PIN);  // Enable falling trigger
    EXTI->RTSR1 &= ~(1 << BUTTON_PIN); // Disable rising trigger

    // Unmask EXTI13
    EXTI->IMR1 |= (1 << BUTTON_PIN);

    init_gpio_pin(GPIOA, LED_PIN, 0x1); // Set LED pin as output
    init_gpio_pin(GPIOC, BUTTON_PIN, 0x0); // Set BUTTON pin as input

    // Enable EXTI15_10 interrupt
    *NVIC_ISER1 |= (1 << (EXTI15_10_IRQn - 32));

    configure_gpio_for_usart();
    
}
*/
void configure_gpio(void)
{
    *RCC_AHB2ENR |= (1 << 0) | (1 << 2); // Enable clock for GPIOA and GPIOC

    // Enable clock for SYSCFG
    *RCC_APB2ENR |= (1 << 0); // RCC_APB2ENR_SYSCFGEN

    // Configure SYSCFG EXTICR to map EXTI13 to PC13
    SYSCFG->EXTICR[3] &= ~(0xF << 4); // Clear bits for EXTI13
    SYSCFG->EXTICR[3] |= (0x2 << 4);  // Map EXTI13 to Port C

    // Configure SYSCFG EXTICR to map EXTI11 to PC11
    SYSCFG->EXTICR[2] &= ~(0xF << 12); // Clear bits for EXTI11
    SYSCFG->EXTICR[2] |= (0x2 << 12);  // Map EXTI11 to Port C


    // Configure EXTI13, EXTI11, and EXTI10 for falling edge trigger
    EXTI->FTSR1 |= (1 << BUTTON_PIN) | (1 << BUTTON_PIN2);  // Enable falling trigger
    EXTI->RTSR1 &= ~((1 << BUTTON_PIN) | (1 << BUTTON_PIN2)); // Disable rising trigger

    // Unmask EXTI13, EXTI11, and EXTI10
    EXTI->IMR1 |= (1 << BUTTON_PIN) | (1 << BUTTON_PIN2);

    init_gpio_pin(GPIOA, LED_PIN, 0x1); // Set LED pin as output
    init_gpio_pin(GPIOA, LED_PIN2, 0x1); // Set second LED pin as output
    init_gpio_pin(GPIOC, BUTTON_PIN, 0x0); // Set BUTTON pin as input
    init_gpio_pin(GPIOC, BUTTON_PIN2, 0x0); // Set second BUTTON pin as input

    // Enable EXTI15_10 interrupt
    *NVIC_ISER1 |= (1 << (EXTI15_10_IRQn - 32));

    configure_gpio_for_usart();
}



// Emula el comprtamiento de la puerta
void gpio_set_door_led_state(uint8_t state) {
    if (state) {
        GPIOA->ODR |= (1 << 4); // encender LED estado puerta
    } else {
        GPIOA->ODR &= ~(1 << 4); // apagar LED estado puerta
    }
}

void gpio_toggle_heartbeat_led(void) {
    GPIOA->ODR ^= (1 << 5);
}

void gpio_toggle_led2(void) {
    GPIOA->ODR ^= (1 << 8); // Alternar LED 2
}

volatile uint8_t button_pressed = 0; // Flag to indicate button press
volatile uint8_t button2_pressed = 0; // Flag for second button


uint8_t button_driver_get_event(void)
{
    return button_pressed;
}

uint8_t button2_driver_get_event(void)
{
    return button2_pressed;
}

uint32_t b1_tick = 0;
void detect_button_press(void)
{
    if (systick_GetTick() - b1_tick < 50) {
        return; // Ignore bounces of less than 50 ms
    } else if (systick_GetTick() - b1_tick > 500) {
        button_pressed = 1; // single press
    } else {
        button_pressed = 2; // double press
    }

    b1_tick = systick_GetTick();
}

uint32_t b2_tick = 0;
void detect_button2_press(void) {
    if (systick_GetTick() - b2_tick < 50) {
        return; // Ignore bounces of less than 50 ms
    } else if (systick_GetTick() - b2_tick > 500) {
        button2_pressed = 1; // single press
    } else {
        button2_pressed = 2; // double press
    }

    b2_tick = systick_GetTick();
}




void EXTI15_10_IRQHandler(void) {
    if (EXTI->PR1 & (1 << BUTTON_PIN)) {
        EXTI->PR1 = (1 << BUTTON_PIN); // Clear pending bit
        detect_button_press();
        gpio_toggle_heartbeat_led();
    }

    if (EXTI->PR1 & (1 << BUTTON_PIN2)) {
        EXTI->PR1 = (1 << BUTTON_PIN2); // Clear pending bit
        detect_button2_press();
        gpio_toggle_led2();
    }
}