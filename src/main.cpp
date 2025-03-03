#include "Server.hpp"

int main() {
    int port = 8080;  // Porta su cui il server ascolta
    Server server(port);
    
    try {
        server.start();
    } catch (const std::exception &e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

