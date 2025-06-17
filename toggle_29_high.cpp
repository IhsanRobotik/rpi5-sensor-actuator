#include <wiringPi.h>
#include <iostream>

int main() {
    wiringPiSetup();
    int pin = 29; // wiringPi pin 29
    pinMode(pin, OUTPUT);

    digitalWrite(pin, HIGH);
    std::cout << "Toggled to High: "<< std::endl;
    return 0;
}