// Trasportatore.cpp

#include "trasportatore.h"

#define T_CHANNEL "stream2"

Trasportatore::Trasportatore(int id, std::string e, std::string p, std::string a, std::string ph)
    : trasportatore_id(id), email(e), password(p), address(a), phone_number(ph), state(TrasportatoreState::Idle){}

// Getters
int Trasportatore::getTrasportatoreId() const { return trasportatore_id; }
std::string Trasportatore::getEmail() const { return email; }
std::string Trasportatore::getPassword() const { return password; }
std::string Trasportatore::getAddress() const { return address; }
std::string Trasportatore::getPhoneNumber() const { return phone_number; }
std::string Trasportatore::getState() const { return stateToString(state); }

// Setters
void Trasportatore::setEmail(const std::string& e) { email = e; }
void Trasportatore::setPassword(const std::string& p) { password = p; }
void Trasportatore::setAddress(const std::string& a) { address = a; }
void Trasportatore::setPhoneNumber(const std::string& ph) { phone_number = ph; }

std::string Trasportatore::stateToString(TrasportatoreState state) const {
    switch (state) {
        case TrasportatoreState::Idle: return "Idle";
        case TrasportatoreState::InTransit: return "InTransit";
        default: return "Unknown";
    }
}

void Trasportatore::transitionToIdle() {
    state = TrasportatoreState::Idle;
    std::cout << "Transitioned to Idle state." << std::endl;
}

void Trasportatore::transitionToInTransit() {
    state = TrasportatoreState::InTransit;
    std::cout << "Transitioned to InTransit state." << std::endl;
}

void Trasportatore::deliverOrder() {
    std::cout << "Delivering order..." << std::endl;
}

void Trasportatore::handleState() {
    switch (state) {
        case TrasportatoreState::Idle:
            std::cout << "Waiting for order..." << std::endl;
            reply = RedisCommand(c2r, "XREADGROUP GROUP %s %s BLOCK 10000 COUNT 10 NOACK STREAMS %s >", 
			  username, username, T_CHANNEL);
            if (reply->type != 4) {
                std::cout << "Order received!" << std::endl;
                transitionToInTransit();
            }
            break;

        case TrasportatoreState::InTransit:
        std::cout << "In transit..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            std::cout << "Order delivered! Coming back..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            std::cout << "Back to the HQ." << std::endl;
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
    initGroup(c2r, T_CHANNEL, username);
    while (true) {
        handleState();
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Simula il passare del tempo
    }
}

int main() {
    Trasportatore trasportatore(1, "ciao@gmail.com", "1234", "5678", "543545242");
    trasportatore.run();
    return 0;
}