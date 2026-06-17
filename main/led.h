#pragma once

#include <stdbool.h>

// Initialise the LED GPIO. Call once at startup.
void led_init(void);

// Set the LED on/off and remember the state.
void led_set(bool on);

// Toggle the LED and return the new state.
bool led_toggle(void);

// Get the current LED state.
bool led_get(void);
