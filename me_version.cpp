#include <wiringPi.h>
#include <softPwm.h>
#include <iostream>
#include <unistd.h>
#include <cmath>
#include <cstdlib>

double Kp = 25.0;
double Ki = 0.2;
double Kd = 2.0;

#define DT_PIN 15         // HX711 DT pin
#define SCK_PIN 16        // HX711 SCK pin

#define COFFEE_PWM_PIN 1
#define SUGAR_PWM_PIN 9
#define CREAMER_PWM_PIN 7
#define WATER_PWM_PIN 0

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

void run_pwm_feeder(int pwm_pin, double setpoint, const std::string& label) {
    if (softPwmCreate(pwm_pin, 0, 100) != 0)
        exit(1);

    double initial_weight = 0;
    for (int i = 0; i < 5; ++i) {
        initial_weight += readWeight();
        delay(100);
    }
    initial_weight /= 5.0;

    double target_weight = initial_weight - setpoint;
    double previous_error = 0;
    double integral = 0;

    while (1) {
        double current_weight = 0;
        for (int i = 0; i < 3; ++i) {
            current_weight += readWeight();
            delay(50);
        }
        current_weight /= 3.0;

        double error = target_weight + current_weight;
        integral += error;
        double derivative = error - previous_error;
        double output = Kp * error + Ki * integral + Kd * derivative;
        previous_error = error;

        int pwm_val = std::max(0, std::min(100, static_cast<int>(output)));
        softPwmWrite(pwm_pin, pwm_val);

        std::cout << "\r[" << label << "] Current: " << current_weight
                  << " Target: " << target_weight
                  << " PWM: " << pwm_val << "      " << std::flush;

        if (error < 0)
            break;
        delay(100);
    }

    softPwmWrite(pwm_pin, 0);
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
    std::cout << "Enter amount to dispense (coffee, sugar, creamer, water): ";
    std::cin >> coffee >> sugar >> creamer >> water;
    std::cin.clear();
    std::cin.ignore(10000, '\n');

    run_pwm_feeder(COFFEE_PWM_PIN, coffee, "Coffee");
    sleep(1);
    run_pwm_feeder(SUGAR_PWM_PIN, sugar, "Sugar");
    sleep(1);
    run_pwm_feeder(CREAMER_PWM_PIN, creamer, "Creamer");
    sleep(1);
    run_pwm_feeder(WATER_PWM_PIN, water, "Water");

    return 0;
}
