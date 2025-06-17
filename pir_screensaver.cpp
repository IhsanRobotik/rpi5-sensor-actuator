#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <unistd.h>
#include <time.h>
#include <iostream>

#define PIR_PIN 29 // wiringPi pin number (GPIO23 = wiringPi 4)
#define TIMEOUT_SEC 300 // 5 minutes

int main() {
    wiringPiSetup();

    pinMode(PIR_PIN, INPUT);
    time_t last_motion = time(NULL);

    while (1) {
        int motion = digitalRead(PIR_PIN);
        time_t now = time(NULL);

        if (motion) {
            last_motion = now;
            std::cout << "Motion detected! Turning screen on." << std::endl;
        }

        if (difftime(now, last_motion) >= TIMEOUT_SEC) {
            system("wlr-randr --output HDMI-A-1 --off");
            last_motion = now + 99999; // prevent retrigger
        }

        usleep(500000); // 500 ms
    }

    return 0;
}
