#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "Person.h"

int main() {
    std::unordered_map<std::string, int> expertise;
    std::unordered_map<std::string, double> needs;
    Person person(expertise, 21, Person::HEALTHY, needs);
    std::cout << person.health_status << std::endl;
    return EXIT_SUCCESS;
}
