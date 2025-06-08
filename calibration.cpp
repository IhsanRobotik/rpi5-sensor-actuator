#include <iostream>
#include <wiringPi.h>

long readHx711(int pinDT = 2, int pinSCK = 3) {
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

void calibrate(int pinDT, int pinSCK, int samples, double &offset, double &scale) {
    std::cout << "Remove all weight. Press Enter when ready.\n";
    std::cin.get();

    long sum = 0;
    for (int i = 0; i < samples; i++) {
        long reading = readHx711(pinDT, pinSCK);
        if (reading == 0) i--;
        else sum += reading;
        delay(50);
    }
    offset = sum / static_cast<double>(samples);
    std::cout << "Offset (zero weight): " << offset << std::endl;

    std::cout << "Place known weight. Enter weight value in grams:\n";
    double knownWeight;
    std::cin >> knownWeight;

    sum = 0;
    for (int i = 0; i < samples; i++) {
        long reading = readHx711(pinDT, pinSCK);
        if (reading == 0) i--;
        else sum += reading;
        delay(50);
    }
    double readingAvg = sum / static_cast<double>(samples);
    scale = knownWeight / (readingAvg - offset);

    std::cout << "Scale factor: " << scale << std::endl;
}

int main() {
    wiringPiSetup();

    int pinDT = 2;
    int pinSCK = 3;

    pinMode(pinDT, INPUT);
    pinMode(pinSCK, OUTPUT);
    digitalWrite(pinSCK, LOW);

    const int samples = 10;
    double offset = 0.0;
    double scale = 1.0;

    calibrate(pinDT, pinSCK, samples, offset, scale);

    while (true) {
        long reading = readHx711(pinDT, pinSCK);
        if (reading == 0) {
            std::cerr << "Timeout reading HX711" << std::endl;
            continue;
        }
        double weight = (reading - offset) * scale;
        std::cout << "Raw: " << reading << " Weight: " << weight << std::endl;
        delay(200);
    }

    return 0;
}
