#include <wiringPi.h>
#include <iostream>

int main() {
    while(1){
    wiringPiSetup();
    int pin = 15; // wiringPi pin 29
    pinMode(pin, INPUT);
    int value = digitalRead(pin);
    std::cout << "Read value: " << value << std::endl;
    delay(100); // Sleep for 1 second before reading again
}
    return 0;
}