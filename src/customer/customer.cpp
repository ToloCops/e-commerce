#include "customer.h"

#define C_CHANNEL "stream1"

Customer::Customer(int id, std::string n, std::string e, std::string p, std::string a, std::string ph)
    : customer_id(id), name(n), email(e), password(p), address(a), phone_number(ph), state(CustomerState::Idle) {}

// Getters
int Customer::getCustomerId() const { return customer_id; }
std::string Customer::getName() const { return name; }
std::string Customer::getEmail() const { return email; }
std::string Customer::getPassword() const { return password; }
std::string Customer::getAddress() const { return address; }
std::string Customer::getPhoneNumber() const { return phone_number; }
std::string Customer::getState() const { return stateToString(state); }

// Setters
void Customer::setName(const std::string& n) { name = n; }
void Customer::setEmail(const std::string& e) { email = e; }
void Customer::setPassword(const std::string& p) { password = p; }
void Customer::setAddress(const std::string& a) { address = a; }
void Customer::setPhoneNumber(const std::string& ph) { phone_number = ph; }

std::string Customer::stateToString(CustomerState state) const {
    switch (state) {
        case CustomerState::Idle: return "Idle";
        case CustomerState::WaitingOrderConfirm: return "WaitingOrderConfirm";
        case CustomerState::WaitingForDelivery: return "WaitingForDelivery";
        default: return "Unknown";
    }
}

void Customer::transitionToIdle() {
    state = CustomerState::Idle;
    std::cout << "Transitioned to Idle state." << std::endl;
}

void Customer::transitionToWaitingOrderConfirm() {
    state = CustomerState::WaitingOrderConfirm;
    std::cout << "Transitioned to WaitingOrderConfirm state." << std::endl;
}

void Customer::transitionToWaitingForDelivery() {
    state = CustomerState::WaitingForDelivery;
    std::cout << "Transitioned to WaitingForDelivery state." << std::endl;
}

void Customer::simulateOrder() {
    if (dist(rng) < 0.5) { // 50% di probabilità di effettuare un ordine
        //TODO
        //Richiesta al server dei prodotti disponibili

        //TODO
        //Scelta di un prodotto randomico

        //TODO
        //Invio messaggio su stream Redis customer-fornitori
        char fornitore[100];
        char prodotto[100];
        strcpy(fornitore, "apple");
        strcpy(prodotto,  "iphone");
        reply = RedisCommand(c2r, "XADD %s * %s %s", C_CHANNEL, fornitore, prodotto);
        assertReplyType(c2r, reply, REDIS_REPLY_STRING);
        printf("main(): pid =%d: stream %s: Added %s -> %s (id: %s)\n", pid, F_CHANNEL, fornitore, prodotto, reply->str);
        freeReplyObject(reply);

        transitionToWaitingOrderConfirm();
    } else {
        transitionToIdle();
    }
}

void Customer::handleState() {
    switch (state) {
        case CustomerState::Idle:
            simulateOrder();
            break;

        case CustomerState::WaitingOrderConfirm:
            std::cout << "Waiting for order confirmation..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2)); // Simula l'attesa
            transitionToWaitingForDelivery();
            break;

        case CustomerState::WaitingForDelivery:
            std::cout << "Waiting for delivery..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2)); // Simula l'attesa
            // Simula ricezione della consegna
            transitionToIdle();
            break;

        default:
            std::cerr << "Unknown state!" << std::endl;
            break;
    }
}

void Customer::run() {
    std::cout << "Hello word from " << getName() << std::endl;
    c2r = initializeRedisConnection(username, seed, pid);
    initGroup(c2r, C_CHANNEL, username);
    while (true) {
        handleState();
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Simula il passare del tempo
    }
}

int main() {
    Customer customer(1, "John Doe", "john@example.com", "password123", "123 Main St", "555-1234");
    customer.run();
    return 0;
}