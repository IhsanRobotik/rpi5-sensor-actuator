#include <wiringPi.h>
#include <iostream>
#include <ctime>
#include <cstdlib>

int main() {
    wiringPiSetup();

    int pin = 15;
    pinMode(pin, INPUT);

    time_t lastActivity = time(nullptr);
    bool displayOn = true;

    while (true) {
        int value = digitalRead(pin);

        if (value == HIGH) {
            lastActivity = time(nullptr);
            if (!displayOn) {
                system("wlr-randr --output HDMI-A-1 --on");
                displayOn = true;
            }
        }

        if (difftime(time(nullptr), lastActivity) >= 300 && displayOn) {
            system("wlr-randr --output HDMI-A-1 --off");
            displayOn = false;
        }

        delay(3000);  // 10 seconds
    }

    return 0;
}
