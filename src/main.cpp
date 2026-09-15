#include "core/engine.hpp"
#include <iostream>

int main(int argc, const char** argv) {
    try {
        Engine engine;
    	engine.configure();
        engine.run();
    } catch (const std::runtime_error& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
    }
    return 0;
}
