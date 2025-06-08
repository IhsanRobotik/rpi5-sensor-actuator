#include <iostream>
#include <wiringPi.h>

long readHx711(int pinDT = 2, int pinSCK = 3) {
    long count = 0;
    unsigned int timeout = 0;

    // Wait for HX711 ready (DT goes LOW)
    while (digitalRead(pinDT) == HIGH) {
        delay(1);
        if (++timeout > 1000) return 0; // timeout fail
    }

    // Read 24 bits
    for (int i = 0; i < 24; i++) {
        digitalWrite(pinSCK, HIGH);
        delayMicroseconds(1);
        count = count << 1;
        digitalWrite(pinSCK, LOW);
        delayMicroseconds(1);
        if (digitalRead(pinDT)) count++;
    }

    // 25th pulse to set gain/channel
    digitalWrite(pinSCK, HIGH);
    delayMicroseconds(1);
    digitalWrite(pinSCK, LOW);
    delayMicroseconds(1);

    // Convert from 24-bit two's complement
    if (count & 0x800000) {
        count |= ~0xffffff;
    }

    return count;
}

int main() {
    wiringPiSetup();

    int pinDT = 2;   // WiringPi pin 2 (BCM 27)
    int pinSCK = 3;  // WiringPi pin 3 (BCM 22)

    pinMode(pinDT, INPUT);
    pinMode(pinSCK, OUTPUT);
    digitalWrite(pinSCK, LOW);

    const int samples = 10;
    long sum = 0;

    const long offset = 324432; // calibrate per your sensor
    const double factor = 0.00296326;

    for (int i = 0; i < samples; i++) {
        long reading = readHx711(pinDT, pinSCK);
        if (reading == 0) {
            std::cerr << "Timeout reading HX711" << std::endl;
            continue;
        }
        sum += reading;
        double weight = (reading - offset) * factor;
        std::cout << "Sample " << i+1 << ": Raw=" << reading << " Weight=" << weight << std::endl;
        delay(50);
    }

    if (sum != 0) {
        double average = sum / static_cast<double>(samples);
        double avgWeight = (average - offset) * factor;
        std::cout << "Average: Raw=" << average << " Weight=" << avgWeight << std::endl;
    }

    return 0;
}
