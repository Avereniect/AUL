#include "containers/Slot_map.hpp"

#include <cstdlib>
#include <iostream>

using std::cout;
using std::endl;

using aul::Slot_map;

int main() {
    Slot_map<double> map;
    map.push_back(1);

    cout << map[0] << endl;

	return EXIT_SUCCESS;
}
