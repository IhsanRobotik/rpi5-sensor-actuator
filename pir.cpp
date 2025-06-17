#include <wiringPi.h>
#include <iostream>

int main() {
    // Use wiringPi numbering
    wiringPiSetup();

    int pin = 29; // wiringPi pin 29 (physical pin 40 on Pi 3/4)
    pinMode(pin, INPUT);

    while (true) {
        int value = digitalRead(pin);
        std::cout << "Pin " << pin << " value: " << value << std::endl;
        delay(500); // 500 ms
    }

    return 0;
}