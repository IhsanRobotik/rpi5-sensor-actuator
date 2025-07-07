#include <wiringPi.h>
#include <softPwm.h>
#include <stdio.h>
#include <stdlib.h>

#define PWM_PIN 1  // WiringPi pin 8 → BCM GPIO 2

int main(void) {
  if (wiringPiSetup() == -1)
    exit(1);

  // Create a software PWM on pin 8, initial 0, range 100
  if (softPwmCreate(PWM_PIN, 0, 100) != 0)
    exit(1);

  // Ramp brightness up and down
  while (1) {
    for (int duty = 0; duty <= 100; ++duty) {
      softPwmWrite(PWM_PIN, duty);
      delay(20);  // slower loop for visible changes
    }
    for (int duty = 100; duty >= 0; --duty) {
      softPwmWrite(PWM_PIN, duty);
      delay(20);
    }
  }
}
