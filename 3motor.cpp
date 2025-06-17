#include <wiringPi.h>
#include <iostream>
#include <cmath>
#include <unistd.h>

// PID parameters
#define KP 1.0
#define KI 0.1
#define KD 0.01

#define DT_PIN 2         // HX711 DT pin
#define SCK_PIN 3        // HX711 SCK pin
#define BASE_STEP_DELAY 10000
#define MIN_STEP_DELAY 500
#define WEIGHT_TOLERANCE 0.5  // Stop when within ±0.5 units of target

const long OFFSET = 563887;
const float SCALE = 0.00108898;

long readHx711(int pinDT = DT_PIN, int pinSCK = SCK_PIN) {
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

double readWeight() {
    long reading = readHx711();
    if (reading == 0) return 0;
    return (reading - OFFSET) * SCALE;
}

void run_feeder(int step_pin, double setpoint, const std::string& label) {
    pinMode(step_pin, OUTPUT);
    digitalWrite(step_pin, LOW);

    // Measure current weight
    double initial_weight = 0;
    std::cout << "\nMeasuring current weight for " << label << " on STEP_PIN " << step_pin << "..." << std::endl;
    for (int i = 0; i < 5; ++i) {
        initial_weight += readWeight();
        delay(100);
    }
    initial_weight /= 5.0;
    std::cout << "Current weight: " << initial_weight << " units" << std::endl;

    if (setpoint <= 0) {
        std::cout << "Setpoint must be positive." << std::endl;
        return;
    }

    double target_weight = initial_weight - setpoint;

    // PID control loop
    double input = initial_weight;
    double output = 0;
    double error, last_error = 0, integral = 0;
    unsigned int step_delay = BASE_STEP_DELAY;
    int step_count = 0;

    while (true) {
        if (step_count % 20 == 0) {
            input = readWeight();
            std::cout << "\rCurrent: " << input << " Target: " << target_weight << "      " << std::flush;
            if (input <= target_weight) {
                std::cout << "\nTarget reached for " << label << "!" << std::endl;
                break;
            }
            error = input - target_weight;
            integral += error;
            double derivative = error - last_error;
            output = KP * error + KI * integral + KD * derivative;
            last_error = error;
            if (output < 0) output = 0;
            if (output > BASE_STEP_DELAY - MIN_STEP_DELAY)
                output = BASE_STEP_DELAY - MIN_STEP_DELAY;
            step_delay = BASE_STEP_DELAY - (unsigned int)output + MIN_STEP_DELAY;
        }

        digitalWrite(step_pin, HIGH);
        delayMicroseconds(step_delay);
        digitalWrite(step_pin, LOW);
        delayMicroseconds(step_delay);

        ++step_count;
    }
}

int main() {
    wiringPiSetup();
    pinMode(DT_PIN, INPUT);
    pinMode(SCK_PIN, OUTPUT);
    digitalWrite(SCK_PIN, LOW);

    double coffee, sugar, creamer, water;
    std::cout << "Enter amount to dispense (coffee, sugar, creamer, water): ";
    std::cin >> coffee >> sugar >> creamer >> water;
    std::cin.clear();
    std::cin.ignore(10000, '\n');

    run_feeder(1, coffee, "Coffee");
    sleep(1);
    run_feeder(4, sugar, "Sugar");
    sleep(1);
    run_feeder(5, creamer, "Creamer");
    sleep(1);
    run_feeder(6, water, "Water");

    return 0;
}