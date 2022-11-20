#include "personType.h"
#include <iostream>
#include <vector>

int main() {
    std::vector<Person> persons = Person::load("C:\\Users\\Alexb\\Dev\\hermes\\backend\\data\\raw");
    std::cout << persons[0].home_coord.latitude << std::endl;
    std::cout << persons[0].home_coord.longitude << std::endl;
}

