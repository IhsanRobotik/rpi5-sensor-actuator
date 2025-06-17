#include <wiringPi.h>
#include <iostream>

int main() {
    wiringPiSetup();
    int pin = 29; // wiringPi pin 29
    pinMode(pin, OUTPUT);

    digitalWrite(pin, LOW);
    std::cout << "Toggled to Low: "<< std::endl;
    return 0;
}