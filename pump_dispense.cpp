
#include <wiringPi.h>
#include <softPwm.h>
#include <iostream>
#include <unistd.h>
#include <cmath>
#include <cstdlib>
#include <algorithm>

#define DT_PIN 29        // HX711 DT pin
#define SCK_PIN 28        // HX711 SCK pin

// Assign PWM pins for each feeder (change as needed)
#define WATER_PWM_PIN 26
#define SUGAR_PWM_PIN 31
#define COFFEE_PWM_PIN 10
#define CREAMER_PWM_PIN 11   

#define RELAY_PIN 27

const long OFFSET = 357864;
const float SCALE = 0.0010998;

double printWeight = 0; 

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

void run_pwm_feeder(int pwm_pin, double setpoint, const std::string& label) {
    int max_pwm = 30; // default max
    if (pwm_pin == COFFEE_PWM_PIN) max_pwm = 40;
    else if (pwm_pin == SUGAR_PWM_PIN) max_pwm = 40;
    else if (pwm_pin == CREAMER_PWM_PIN) max_pwm = 40;
    else if (pwm_pin == WATER_PWM_PIN) max_pwm = 33;

    if (setpoint <= 0) {
        std::cout << "Setpoint for " << label << " is zero or negative, skipping.\n";
        return;
    }

    // Create a software PWM on the given pin, initial 0, range 100
    if (softPwmCreate(pwm_pin, 0, 100) != 0) {
        std::cerr << "Failed to create softPwm on pin " << pwm_pin << std::endl;
        return;
    }

    std::cout << "Dispensing " << label << " (" << setpoint << " units) on PWM pin " << pwm_pin << "...\n";

    // Example: run PWM at 80% duty cycle until setpoint is reached
    double initial_weight = 0;
    for (int i = 0; i < 5; ++i) {
        initial_weight += readWeight();
        delay(100);
    }
    initial_weight /= 5.0;

    double target_weight = initial_weight + setpoint;

    double Kp = 10.0; // reduce from 20.0
    int min_pwm = 20;

    while (true) {
        double current = readWeight();
        double error = target_weight - current;
        if (error <= 0){
            std::cout << "\nTarget reached for " << label << "!\n";
            break; 
        } 

        int pwm_value = std::clamp(static_cast<int>(Kp * error), min_pwm, max_pwm);
        softPwmWrite(pwm_pin, pwm_value);
        std::cout << "\rCurrent: " << current << " Target: " << target_weight << " PWM: " << pwm_value << "   " << std::flush;
        // delay(50);
    }

    softPwmWrite(pwm_pin, 0); // Stop PWM
    softPwmStop(pwm_pin);
}

int main() {
    if (wiringPiSetup() == -1) {
        std::cerr << "Failed to initialize wiringPi\n";
        return 1;
    }
    pinMode(DT_PIN, INPUT);
    pinMode(SCK_PIN, OUTPUT);
    digitalWrite(SCK_PIN, LOW);

    double coffee, sugar, creamer, water;
    std::cout << "Enter amount to dispense (coffee, sugar, creamer, water): \n";
    std::cin >> coffee >> sugar >> creamer >> water; 
    std::cout.flush();

    if (coffee > 0) {
        std::cout << "step:coffee\n";
        std::cout.flush();
        run_pwm_feeder(COFFEE_PWM_PIN, coffee, "Coffee");
        sleep(1);
    }

    if (sugar > 0) {
        std::cout << "step:sugar\n";
        std::cout.flush();
        run_pwm_feeder(SUGAR_PWM_PIN, sugar, "Sugar");
        sleep(1);
    }

    if (creamer > 0) {
        std::cout << "step:creamer\n";
        std::cout.flush();
        run_pwm_feeder(CREAMER_PWM_PIN, creamer, "Creamer");
        sleep(1);
    }

    if (water > 0) {
        std::cout << "step:water\n";
        std::cout.flush();
        run_pwm_feeder(WATER_PWM_PIN, water, "Water");
    }
    
    return 0;

}
