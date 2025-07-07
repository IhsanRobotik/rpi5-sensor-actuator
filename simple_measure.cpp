#include <iostream>
#include <wiringPi.h>

long readHx711(int pinDT = 16, int pinSCK = 1) {
    long count = 0;
    unsigned int timeout = 0;

    while (digitalRead(pinDT) == HIGH) {
        delay(1);
        if (++timeout > 1000) return 0;
    }

    for (int i = 0; i < 24; i++) {
        digitalWrite(pinSCK, HIGH);
        delayMicroseconds(1);
        count = count << 1;
        digitalWrite(pinSCK, LOW);
        delayMicroseconds(1);
        if (digitalRead(pinDT)) count++;
    }

    digitalWrite(pinSCK, HIGH);
    delayMicroseconds(1);
    digitalWrite(pinSCK, LOW);
    delayMicroseconds(1);

    if (count & 0x800000) count |= ~0xffffff;

    return count;
}

int main() {
    wiringPiSetup();

    int pinDT = 16;
    int pinSCK = 1;

    pinMode(pinDT, INPUT);
    pinMode(pinSCK, OUTPUT);
    digitalWrite(pinSCK, LOW);

    const long offset = 563887;
    const float factor = 0.00108898;

    while (true) {
        long reading = readHx711(pinDT, pinSCK);
        if (reading == 0) {
            std::cerr << "Timeout reading HX711" << std::endl;
            continue;
        }
        double weight = (reading - offset) * factor;
        std::cout << "Raw=" << reading << " Weight=" << weight << std::endl;
        delay(100);
    }

    return 0;
}
