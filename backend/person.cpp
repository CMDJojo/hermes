#include "personType.h"
#include <iostream>
#include <vector>

int main() {
    std::vector<Person> persons = Person::load("../../backend/data/raw/Ast_bost.txt");
    std::cout << persons[0].home_coord.latitude << std::endl;
    std::cout << persons[0].home_coord.longitude << std::endl;

}