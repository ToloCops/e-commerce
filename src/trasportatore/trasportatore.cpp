// Trasportatore.cpp

#include "trasportatore.h"

#define T_CHANNEL "stream2"

Trasportatore::Trasportatore(int id) : trasportatore_id(id) {}

// Getters
int Trasportatore::getTrasportatoreId() const { return trasportatore_id; }
std::string Trasportatore::getState() const { return stateToString(state); }

std::string Trasportatore::stateToString(TrasportatoreState state) const {
    switch (state) {
        case TrasportatoreState::Idle: return "Idle";
        case TrasportatoreState::InTransit: return "InTransit";
        default: return "Unknown";
    }
}

void Trasportatore::transitionToIdle() {
    state = TrasportatoreState::Idle;
}

void Trasportatore::transitionToInTransit() {
    state = TrasportatoreState::InTransit;
}

void Trasportatore::deliverOrder() {
}

void Trasportatore::handleState() {
    switch (state) {
        case TrasportatoreState::Idle:
            reply = RedisCommand(c2r, "XREADGROUP GROUP %s %s BLOCK 10000 COUNT 10 NOACK STREAMS %s >", 
			  username, username, T_CHANNEL);
            if (reply->type != 4) {
                std::cout << "Trasportatore " << username << " --> order received!\n" << std::endl;
                transitionToInTransit();
            }
            break;

        case TrasportatoreState::InTransit:
            std::this_thread::sleep_for(std::chrono::seconds(2));
            std::cout << "Trasportatore " << username << " --> order delivered! Coming back...\n" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            std::cout << "Trasportatore " << username << " --> back to the HQ.\n" << std::endl;
            transitionToIdle();
            break;

        default:
            std::cerr << "Unknown state!" << std::endl;
            break;
    }
}

void Trasportatore::run() {
    std::cout << "Hello word from Trasportatore"  << std::endl;
    c2r = initializeRedisConnection(username, seed, pid);
    strcpy(username, "543453");
    initGroup(c2r, T_CHANNEL, username);
    while (true) {
        handleState();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main() {
    Trasportatore trasportatore1(1);
    Trasportatore trasportatore2(2);
    trasportatore1.run();
    trasportatore2.run();
    return 0;
}