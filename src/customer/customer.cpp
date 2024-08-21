#include "customer.h"

#define C_CHANNEL "stream1"
#define F_CHANNEL "stream3"


Customer::Customer(int id, std::string n)
    : customer_id(id), name(n), state(CustomerState::Idle) {}

// Getters
int Customer::getCustomerId() const { return customer_id; }
std::string Customer::getName() const { return name; }
std::string Customer::getState() const { return stateToString(state); }

// Setters
void Customer::setName(const std::string& n) { name = n; }

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
    //std::cout << "Transitioned to Idle state." << std::endl;
}

void Customer::transitionToWaitingOrderConfirm() {
    state = CustomerState::WaitingOrderConfirm;
    //std::cout << "Transitioned to WaitingOrderConfirm state." << std::endl;
}

void Customer::transitionToWaitingForDelivery() {
    state = CustomerState::WaitingForDelivery;
    //std::cout << "Transitioned to WaitingForDelivery state." << std::endl;
}

bool Customer::parseMessage(redisReply *reply) {
    bool for_me = false;

    if (reply->type == REDIS_REPLY_ARRAY) {
        for (size_t i = 0; i < reply->elements; i++) {
            redisReply *stream = reply->element[i];

            // Ogni stream dovrebbe avere un nome (prima parte) e un array di elementi (seconda parte)
            if (stream->type == REDIS_REPLY_ARRAY && stream->elements == 2) {
                redisReply *stream_name = stream->element[0];
                redisReply *entries = stream->element[1];

                printf("Stream: %s\n", stream_name->str);

                // Ogni entry nello stream
                for (size_t j = 0; j < entries->elements; j++) {
                    redisReply *entry = entries->element[j];

                    if (entry->type == REDIS_REPLY_ARRAY && entry->elements == 2) {
                        redisReply *entry_id = entry->element[0];
                        redisReply *fields = entry->element[1];

                        printf("Entry ID: %s\n", entry_id->str);

                        // Stampiamo le coppie chiave-valore
                        for (size_t k = 0; k < fields->elements; k += 2) {
                            redisReply *key = fields->element[k];
                            redisReply *value = fields->element[k + 1];

                            if (k == 0) 
                            {
                                if (strcmp(value->str, username) != 0)                                               // Controlla che il messaggio fosse per lui
                                {
                                    break;
                                }
                                else 
                                {
                                    for_me = true;
                                }
                            }
                            else if (k == 2)                                                                         // Aggiorna lo stato dell'ordine
                            {
                                if (strcmp(value->str, "CONFIRMED") == 0) 
                                {
                                    transitionToWaitingForDelivery();
                                }
                                else if (strcmp(value->str, "DELIVERED") == 0) 
                                {
                                    transitionToIdle();
                                }
                            }

                            printf("%s: %s\n", key->str, value->str);
                        }
                    }
                }
            }
        }
    }
    return for_me;
}

void Customer::simulateOrder() {
    if (dist(rng) < 0.5) { // 50% di probabilità di effettuare un ordine
        //TODO
        //Richiesta al server dei prodotti disponibili

        //TODO
        //Scelta di un prodotto randomico

        char fornitore[100];
        char prodotto[100];
        strcpy(fornitore, "apple");
        strcpy(prodotto,  "iphone");
        reply = RedisCommand(c2r, "XADD %s * fornitore %s prodotto %s utente %s",                                   // Informa i fornitori del prodotto che vuole acquistare
                                F_CHANNEL, fornitore, prodotto, username);
        assertReplyType(c2r, reply, REDIS_REPLY_STRING);
        printf("main(): pid =%d: stream %s: Added fornitore -> %s prodotto -> %s utente -> %s (id: %s)\n",
                pid, F_CHANNEL, fornitore, prodotto, username, reply->str);
        freeReplyObject(reply);

        transitionToWaitingOrderConfirm();
    } else {
        transitionToIdle();
    }
}

void Customer::handleState() 
{
    switch (state) {
        case CustomerState::Idle:
            simulateOrder();
            break;

        case CustomerState::WaitingOrderConfirm:
            reply = RedisCommand(c2r, "XREADGROUP GROUP %s %s BLOCK 10000 COUNT 10 NOACK STREAMS %s >",             // Interroga lo stream dei customer per ricevere nuovi messaggi
			                        username, username, C_CHANNEL);
            if (reply->type != 4) 
            {                                                                                 // Controlla se ha avuto risposta
                if (parseMessage(reply))                                                                            // Parsa il messaggio e controlla che fosse per lui
                {
                    std::cout << "Customer " << username << " --> order CONFIRMED!\n" << std::endl;
                }
                else 
                {
                    std::cout << "Customer " << username << "--> waiting for order confirmation...\n" << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(2)); 
                }
            }
            else 
            {
                std::cout << "Customer " << username << "--> waiting for order confirmation...\n" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(2));                                              
            }
            break;

        case CustomerState::WaitingForDelivery:
            reply = RedisCommand(c2r, "XREADGROUP GROUP %s %s BLOCK 10000 COUNT 10 NOACK STREAMS %s >", 
			                        username, username, C_CHANNEL);
            if (reply->type != 4) 
            {
                if (parseMessage(reply)) 
                {
                    std::cout << "Customer " << username << " --> product RECEIVED!\n" << std::endl;
                }
                else 
                {
                    std::cout << "Customer " << username << " --> waiting for delivery...\n" << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                }
            }
            else 
            {
                std::cout << "Customer " << username << " --> waiting for delivery...\n" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(2));                                    
            }
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
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Simula il passare del tempo
        handleState();
    }
}

int main() {
    Customer customer(1, "John Doe");
    customer.run();
    return 0;
}