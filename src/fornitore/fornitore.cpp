// Fornitore.cpp

#include "fornitore.h"

#define C_CHANNEL "stream1"
#define T_CHANNEL "stream2"

Fornitore::Fornitore(int id, std::string cn, std::string e, std::string p, std::string a, std::string ph)
    : fornitore_id(id), company_name(cn), state(FornitoreState::WaitingForOrder) {}

// Getters
int Fornitore::getFornitoreId() const { return fornitore_id; }
std::string Fornitore::getCompany() const { return company_name; }

// Setters
void Fornitore::setCompany(const std::string& n) { company_name = n; }

std::string Fornitore::stateToString(FornitoreState state) const {
    switch (state) {
        case FornitoreState::WaitingForOrder: return "WaitingForOrder";
        case FornitoreState::ProcessingOrder: return "ProcessingOrder";
        default: return "Unknown";
    }
}

void Fornitore::transitionToWaitingForOrder() {
    state = FornitoreState::WaitingForOrder;
    //std::cout << "Transitioned to WaitingForOrder state." << std::endl;
}

void Fornitore::transitionToProcessingOrder() {
    state = FornitoreState::ProcessingOrder;
    //std::cout << "Transitioned to ProcessingOrder state." << std::endl;
}

void Fornitore::processOrder() {
    //std::cout << "Processing order..." << std::endl;
}

void Fornitore::handleState() {
    switch (state) {
        case FornitoreState::WaitingForOrder:
            //std::cout << "Fornitore" << username << " --> waiting for order..." << std::endl;
            reply = RedisCommand(c2r, "XREADGROUP GROUP %s %s BLOCK 10000 COUNT 10 NOACK STREAMS %s >", 
			  username, username, C_CHANNEL);
            if (reply->type != 4) {
                std::cout << "Fornitore " << username << " --> order received!\n" << std::endl;
                transitionToProcessingOrder();
            }
            break;

        case FornitoreState::ProcessingOrder:
            reply = RedisCommand(c2r, "XADD %s * %s %s", T_CHANNEL, "consegna", "prodotto");
            transitionToWaitingForOrder();
            break;

        default:
            std::cerr << "Unknown state!" << std::endl;
            break;
    }
}

void Fornitore::run() {
    std::cout << "Hello word from " << getCompany() << std::endl;
    c2r = initializeRedisConnection(username, seed, pid);
    initGroup(c2r, C_CHANNEL, username);
    initGroup(c2r, T_CHANNEL, username);
    while (true) {
        handleState();
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Simula il passare del tempo
    }
}

int main() {
    Fornitore apple(1, "Apple");
    Fornitore samsung(2, "Samsung");
    apple.run();
    samsung.run();
    return 0;
}