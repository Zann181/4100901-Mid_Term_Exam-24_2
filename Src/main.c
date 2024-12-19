// Access Control System Implementation

#include "gpio.h"
#include "systick.h"
#include "uart.h"

#define TEMP_UNLOCK_DURATION 5000 // Duration in ms for temporary unlock

// Enumeration for the states of the door
typedef enum {
    LOCKED,         // Door is locked
    TEMP_UNLOCK,    // Door is temporarily unlocked
    PERM_UNLOCK     // Door is permanently unlocked
} DoorState_t;

DoorState_t current_state = LOCKED; // Initial state of the door
uint32_t unlock_timer = 0;          // Timer to track temporary unlock duration

// State machine to handle door states
void run_state_machine(void) {
    switch (current_state) {
        case LOCKED:
            // No periodic action needed in the locked state
            break;
        case TEMP_UNLOCK:
            // Check if the temporary unlock duration has expired
            if (systick_GetTick() - unlock_timer >= TEMP_UNLOCK_DURATION) {
                gpio_set_door_led_state(0); // Turn off door state LED
                current_state = LOCKED;    // Transition back to locked state
            }
            break;
        case PERM_UNLOCK:
            // No periodic action needed in the permanent unlock state
            break;
    }
}

// Handle events from buttons or UART commands
void handle_event(uint8_t event) {
    if (event == 1) { // Single button press
        gpio_set_door_led_state(1); // Turn on door state LED
        current_state = TEMP_UNLOCK; // Transition to temporary unlock state
        unlock_timer = systick_GetTick(); // Start the unlock timer
    } else if (event == 2) { // Double button press
        gpio_set_door_led_state(1); // Turn on door state LED
        current_state = PERM_UNLOCK; // Transition to permanent unlock state
    } else if (event == 'O') { // UART OPEN command
        gpio_set_door_led_state(1); // Turn on door state LED
        current_state = TEMP_UNLOCK; // Transition to temporary unlock state
        unlock_timer = systick_GetTick(); // Start the unlock timer
    } else if (event == 'C') { // UART CLOSE command
        gpio_set_door_led_state(0); // Turn off door state LED
        current_state = LOCKED;    // Transition to locked state
    }
}

// Handle events specific to Button 2
void handle_button2_event(uint8_t event) {
    if (event == 1) { // Single button press
        gpio_toggle_led2(); // Toggle LED2
        usart2_send_string("Button 2 single press\r\n"); // Log event
    } else if (event == 2) { // Double button press
        gpio_toggle_led2(); // Toggle LED2 twice to emphasize the event
        gpio_toggle_led2();
        usart2_send_string("Button 2 double press\r\n"); // Log event
    }
}

int main(void) {
    configure_systick_and_start(); // Initialize systick timer
    configure_gpio();              // Configure GPIOs
    usart2_init();                 // Initialize UART

    usart2_send_string("System Initialized\r\n"); // Indicate system startup

    uint32_t heartbeat_tick = 0; // Timer for heartbeat LED
    while (1) {
        // Toggle heartbeat LED every 500 ms
        if (systick_GetTick() - heartbeat_tick >= 500) {
            heartbeat_tick = systick_GetTick();
            gpio_toggle_heartbeat_led();
        }

        // Check for Button 1 events
        uint8_t button_pressed = button_driver_get_event();
        if (button_pressed != 0) {
            handle_event(button_pressed); // Handle the event
            usart2_send_string("System button \r\n"); // Log event
            button_pressed = 0; // Reset event flag
        }

        // Check for Button 2 events
        uint8_t button2_pressed = button2_driver_get_event();
        if (button2_pressed != 0) {
            handle_button2_event(button2_pressed); // Handle the event
            button2_pressed = 0; // Reset event flag
        }

        // Check for UART commands
        uint8_t rx_byte = usart2_get_command();
        if (rx_byte != 0) {
            handle_event(rx_byte); // Handle the command
            rx_byte = 0; // Reset command flag
        }

        // Run the state machine to manage door state
        run_state_machine();
    }
}
