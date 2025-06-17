#include <wiringPi.h>
#include <iostream>
#include <cmath>
#include <unistd.h>

// PID parameters
#define KP 1.0
#define KI 0.1
#define KD 0.01

#define STEP_PIN 0       // WiringPi pin 0 (GPIO 17)
#define DT_PIN 2         // HX711 DT pin
#define SCK_PIN 3        // HX711 SCK pin
#define BASE_STEP_DELAY 10000
#define MIN_STEP_DELAY 500
#define WEIGHT_TOLERANCE 0.5  // Stop when within ±0.5 units of target

REMOVE WEIGHT_TOLERANCE

// HX711 calibration
const long OFFSET = 563887;
const float SCALE = 0.00108898;

// Read HX711 (blocking, ~1-2ms)
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

int main() {
    wiringPiSetup();
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DT_PIN, INPUT);
    pinMode(SCK_PIN, OUTPUT);
    digitalWrite(SCK_PIN, LOW);

    // Measure current weight
    double initial_weight = 0;
    std::cout << "Measuring current weight..." << std::endl;
    for (int i = 0; i < 5; ++i) { // Average a few readings
        initial_weight += readWeight();
        delay(100);
    }
    initial_weight /= 5.0;
    std::cout << "Current weight: " << initial_weight << " units" << std::endl;

    // Ask user for setpoint (amount to dispense)
    double setpoint = 0;
    std::cout << "Enter amount to dispense: ";
    std::cin >> setpoint;
    std::cin.clear();
    std::cin.ignore(10000, '\n');
    if (setpoint <= 0) {
        std::cout << "Setpoint must be positive." << std::endl;
        return 1;
    }

    double target_weight = initial_weight - setpoint;

    // PID control loop
    double input = initial_weight;
    double output = 0;
    double error, last_error = 0, integral = 0;
    unsigned int step_delay = BASE_STEP_DELAY;
    int step_count = 0;

    while (true) {
        // Only read weight every N steps to avoid timing issues
        if (step_count % 20 == 0) {
            input = readWeight();
            std::cout << "\rCurrent: " << input << " Target: " << target_weight << "      " << std::flush;
            // Stop if within tolerance
            if (std::fabs(input - target_weight) < WEIGHT_TOLERANCE) {
                std::cout << "\nTarget reached!" << std::endl;
                break;
            }
            // PID calculations
            error = input - target_weight;
            integral += error;
            double derivative = error - last_error;
            output = KP * error + KI * integral + KD * derivative;
            last_error = error;
            // Clamp output
            if (output < 0) output = 0;
            if (output > BASE_STEP_DELAY - MIN_STEP_DELAY)
                output = BASE_STEP_DELAY - MIN_STEP_DELAY;
            step_delay = BASE_STEP_DELAY - (unsigned int)output + MIN_STEP_DELAY;
        }

        // Step the motor
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(step_delay);
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(step_delay);

        ++step_count;
    }

    return 0;
}