#include <Arduino.h>
#include <stdio.h>
// #include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
// #include "hardware/divider.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"


// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}