#include <wiringPi.h>
#include <stdio.h>

#define STEP_PIN 0       // WiringPi pin 0 (GPIO 17)
#define STEP_DELAY 500   // Delay in microseconds

int main(void) {
    wiringPiSetup();

    pinMode(STEP_PIN, OUTPUT);

    while (1) {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(STEP_DELAY);
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(STEP_DELAY);
    }

    return 0;
}
