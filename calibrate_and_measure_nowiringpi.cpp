#include <iostream>
#include <gpiod.h>
#include <unistd.h>
#include <chrono>
#include <thread>

long readHx711(gpiod_line* dt, gpiod_line* sck) {
    long count = 0;
    unsigned int timeout = 0;

    while (gpiod_line_get_value(dt) == 1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (++timeout > 1000) return 0;
    }

    for (int i = 0; i < 24; i++) {
        gpiod_line_set_value(sck, 1);
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        count = count << 1;
        gpiod_line_set_value(sck, 0);
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        if (gpiod_line_get_value(dt)) count++;
    }

    gpiod_line_set_value(sck, 1);
    std::this_thread::sleep_for(std::chrono::microseconds(1));
    gpiod_line_set_value(sck, 0);
    std::this_thread::sleep_for(std::chrono::microseconds(1));

    if (count & 0x800000) count |= ~0xffffff;

    return count;
}

void calibrate(gpiod_line* dt, gpiod_line* sck, int samples, double &offset, double &scale) {
    std::cout << "Remove all weight. Press Enter when ready.\n";
    std::cin.get();

    long sum = 0;
    for (int i = 0; i < samples; i++) {
        long reading = readHx711(dt, sck);
        if (reading == 0) i--;
        else sum += reading;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    offset = sum / static_cast<double>(samples);
    std::cout << "Offset (zero weight): " << offset << std::endl;

    std::cout << "Place known weight. Enter weight value in grams:\n";
    double knownWeight;
    std::cin >> knownWeight;

    sum = 0;
    for (int i = 0; i < samples; i++) {
        long reading = readHx711(dt, sck);
        if (reading == 0) i--;
        else sum += reading;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    double readingAvg = sum / static_cast<double>(samples);
    scale = knownWeight / (readingAvg - offset);

    std::cout << "Scale factor: " << scale << std::endl;
}

int main() {
    const char* chipname = "gpiochip0";
    int dt_pin = 15;
    int sck_pin = 16;

    gpiod_chip* chip = gpiod_chip_open_by_name(chipname);
    if (!chip) return 1;

    gpiod_line* dt = gpiod_chip_get_line(chip, dt_pin);
    gpiod_line* sck = gpiod_chip_get_line(chip, sck_pin);
    if (!dt || !sck) return 1;

    if (gpiod_line_request_input(dt, "hx711") < 0) return 1;
    if (gpiod_line_request_output(sck, "hx711", 0) < 0) return 1;

    const int samples = 10;
    double offset = 0.0;
    double scale = 1.0;

    calibrate(dt, sck, samples, offset, scale);

    while (true) {
        long reading = readHx711(dt, sck);
        if (reading == 0) {
            std::cerr << "Timeout reading HX711" << std::endl;
            continue;
        }
        double weight = (reading - offset) * scale;
        std::cout << "Raw: " << reading << " Weight: " << weight << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    gpiod_line_release(dt);
    gpiod_line_release(sck);
    gpiod_chip_close(chip);

    return 0;
}
