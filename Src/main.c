#include "gpio.h"
#include "systick.h"
#include "uart.h"

#define TEMP_UNLOCK_DURATION 5000 // Duration in ms for temporary unlock

typedef enum {
    LOCKED,
    TEMP_UNLOCK,
    PERM_UNLOCK
} DoorState_t;

DoorState_t current_state = LOCKED;
uint32_t unlock_timer = 0;
uint32_t heartbeat_tick = 0;

void handle_locked_state(void) {
    usart2_send_string("System is LOCKED. Waiting for events...\r\n");

    while (current_state == LOCKED) {
        // Parpadeo del LED en estado bloqueado
        if (systick_GetTick() - heartbeat_tick >= 500) {
            heartbeat_tick = systick_GetTick();
            gpio_set_door_led_state(2); // LED parpadeante
        }

        // Verificar eventos de botón
        uint8_t button_pressed = button_driver_get_event();
        if (button_pressed != 0) {
            handle_event(button_pressed);
            return; // Salir de la función al manejar un evento
        }

        // Verificar comandos UART
        uint8_t rx_byte = usart2_get_command();
        if (rx_byte != 0) {
            handle_event(rx_byte);
            return; // Salir de la función al manejar un comando
        }
    }
}

void run_state_machine(void) {
    switch (current_state) {
        case LOCKED:
            gpio_set_door_led_state(0); // Asegurar que el LED de estado está apagado
            handle_locked_state();     // Manejar el estado bloqueado
            break;

        case TEMP_UNLOCK:
            gpio_set_door_led_state(1); // Encender LED fijo
            if (systick_GetTick() - unlock_timer >= TEMP_UNLOCK_DURATION) {
                usart2_send_string("Door Locked Automatically\r\n");
                gpio_set_door_led_state(0); // Apagar LED de estado de la puerta
                current_state = LOCKED;
            }
            break;

        case PERM_UNLOCK:
            gpio_set_door_led_state(1); // LED fijo para desbloqueo permanente
            // No hay acciones periódicas para desbloqueo permanente
            break;
    }
}

void handle_event(uint8_t event) {
    if (event == 1) { // Pulsación sencilla
        usart2_send_string("Single Button Press\r\n");
        gpio_set_door_led_state(1); // Encender LED fijo
        current_state = TEMP_UNLOCK;
        unlock_timer = systick_GetTick();
        usart2_send_string("DOOR UNLOCKED \r\n");

        // Mostrar los 5 segundos restantes
        for (uint32_t remaining_time = 5; remaining_time > 0; --remaining_time) {
            char message[20] = "Time left: ";
            char time_char[2] = {remaining_time + '0', '\0'};
            usart2_send_string(message);
            usart2_send_string(time_char);
            usart2_send_string(" seconds\r\n");

            uint32_t start_time = systick_GetTick();
            while (systick_GetTick() - start_time < 1000) {
                uint8_t rx_byte = usart2_get_command();
                if (rx_byte == 'C') {
                    usart2_send_string("Close Command Received\r\n");
                    gpio_set_door_led_state(0);
                    current_state = LOCKED;
                    return;
                }
                uint8_t button_pressed = button_driver_get_event();
                if (button_pressed == 2) {
                    usart2_send_string("Double Button Press\r\n");
                    usart2_send_string("DOOR PERMANENTLY UNLOCKED\r\n");
                    current_state = PERM_UNLOCK;
                    return;
                }
            }
        }

        // Bloquear automáticamente si no hay interrupciones
        gpio_set_door_led_state(0);
        usart2_send_string("Door Locked Automatically\r\n");
        current_state = LOCKED;

    } else if (event == 2) { // Doble pulsación
        usart2_send_string("Double Button Press\r\n");
        gpio_set_door_led_state(1);
        current_state = PERM_UNLOCK;

    } else if (event == 'O') { // Comando UART OPEN
        usart2_send_string("Open Command Received\r\n");
        gpio_set_door_led_state(1);
        current_state = TEMP_UNLOCK;
        unlock_timer = systick_GetTick();

    } else if (event == 'C') { // Comando UART CLOSE
        usart2_send_string("Close Command Received\r\n");
        gpio_set_door_led_state(0);
        current_state = LOCKED;
    }
}

int main(void) {
    configure_systick_and_start();
    configure_gpio();
    usart2_init();

    usart2_send_string("System Initialized\r\n");

    while (1) {
        run_state_machine(); // Ejecutar máquina de estados
    }
}
