#include <gpiod.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define CONSUMER "HC-SR04"
#define TRIG_PIN 17
#define ECHO_PIN 18

long long time_us()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000LL + ts.tv_nsec / 1000;
}

double measure_distance(struct gpiod_line *trig_line, struct gpiod_line *echo_line)
{
    gpiod_line_set_value(trig_line, 0);
    usleep(2);
    gpiod_line_set_value(trig_line, 1);
    usleep(10);
    gpiod_line_set_value(trig_line, 0);

    long long start = time_us();
    while (gpiod_line_get_value(echo_line) == 0) {
        if (time_us() - start > 1000000) return -1; // timeout
    }

    long long pulse_start = time_us();
    while (gpiod_line_get_value(echo_line) == 1) {
        if (time_us() - pulse_start > 30000) return -1; // timeout
    }
    long long pulse_end = time_us();

    long long pulse_duration = pulse_end - pulse_start;
    return (pulse_duration / 2.0) * 0.0343;
}

int main()
{
    setbuf(stdout, NULL);

    struct gpiod_chip *chip = gpiod_chip_open_by_name("gpiochip0");
    if (!chip) return 1;

    struct gpiod_line *trig_line = gpiod_chip_get_line(chip, TRIG_PIN);
    struct gpiod_line *echo_line = gpiod_chip_get_line(chip, ECHO_PIN);
    if (!trig_line || !echo_line) {
        gpiod_chip_close(chip);
        return 1;
    }

    if (gpiod_line_request_output(trig_line, CONSUMER, 0) < 0) {
        gpiod_chip_close(chip);
        return 1;
    }

    if (gpiod_line_request_input(echo_line, CONSUMER) < 0) {
        gpiod_line_release(trig_line);
        gpiod_chip_close(chip);
        return 1;
    }

    while (1) {
        double dist = measure_distance(trig_line, echo_line);
        if (dist < 0) {
            printf("Measurement timeout\n");
        } else {
            printf("Distance: %.2f cm\n", dist);
        }
        fflush(stdout);
        usleep(500000); // 0.5 second delay
    }

    gpiod_line_release(trig_line);
    gpiod_line_release(echo_line);
    gpiod_chip_close(chip);

    return 0;
}