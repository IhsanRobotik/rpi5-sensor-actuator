#include <wiringPi.h>
#include <stdio.h>

#define STEP_PIN 0       // WiringPi pin 0 (GPIO 17)
#define DELAY_START 10000 // Start delay in microseconds
#define DELAY_END 400     // End delay in microseconds
#define DELAY_STEP 100    // Decrement step in microseconds

int main(void) {
    wiringPiSetup();

    pinMode(STEP_PIN, OUTPUT);

    while (1) {
         // Wait for 1 second before starting the ramp
        // Ramp down delay from DELAY_START to DELAY_END
        for (int delayVal = DELAY_START; delayVal >= DELAY_END; delayVal -= DELAY_STEP) {
            digitalWrite(STEP_PIN, HIGH);
            delayMicroseconds(delayVal);
            digitalWrite(STEP_PIN, LOW);
            delayMicroseconds(delayVal);
            delay(1); // Optional delay to observe the ramp effect
        }
        // Optionally, ramp back up for a full cycle:
        for (int delayVal = DELAY_END; delayVal <= DELAY_START; delayVal += DELAY_STEP) {
            digitalWrite(STEP_PIN, HIGH);
            delayMicroseconds(delayVal);
            digitalWrite(STEP_PIN, LOW);
            delayMicroseconds(delayVal);
            delay(1); // Optional delay to observe the ramp effect
        }
    }

    return 0;
}
