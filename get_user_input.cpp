#include <iostream>
// #include <wiringPi.h>


int sus(int value){
    if (value < 0) {
        std::cout << "Value must be non-negative." << std::endl;
        return 1;
    }
    std::cout << "You entered a valid value: " << value << std::endl;
    return 0;
}

int main() {
    std::cout << "Please enter something: ";
    std::string input;
    std::getline(std::cin, input);
    std::cout << "You entered: " << input << std::endl;

    std::cout << "Please enter a non-negative integer value: ";
    int value;
    std::cin >> value;
    sus(value);

    std::cout << sus(value) << std::endl;

    return 0;
}
