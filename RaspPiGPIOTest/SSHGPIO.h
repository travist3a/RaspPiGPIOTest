#pragma once
#include <string>

// Opens one persistent SSH session.
// Returns true on success or if already open.
bool gpio_open_session();

// Closes the persistent session gracefully.
void gpio_close_session();

// Sends gpioset <gpio>=<high?1:0> over the open session.
// Returns true on success, false if the write failed (session dead).
bool gpio_set_remote(int gpio, bool high);
