// Fornitore.cpp

#include "fornitore.h"

#define C_CHANNEL "stream1"
#define T_CHANNEL "stream2"

Fornitore::Fornitore(int id, std::string cn)
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

void Fornitore::parseCustomerMessage(redisReply *reply) {
    char *product;
    char *user;
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

                            if (k == 2) {
                                product = value->str;
                            }
                            else if (k == 4) {
                                user = value -> str;
                            }
                            
                            if (user != NULL && user[0] != '\0') {
                                processOrder(product, user);
                            }

                            printf("%s: %s\n", key->str, value->str);
                        }
                    }
                }
            }
        }
    }
}

void Fornitore::processOrder(char *product, char *user) {
    printf("Processing order for user: %s, product: %s\n", user, product);
    //TODO remove item from available products
    //TODO notify trasportatore
    //TODO notify customer
}

void Fornitore::handleState() {
    switch (state) {
        case FornitoreState::WaitingForOrder:
            //std::cout << "Fornitore" << username << " --> waiting for order..." << std::endl;
            reply = RedisCommand(c2r, "XREADGROUP GROUP %s %s BLOCK 10000 COUNT 10 NOACK STREAMS %s >", 
			  username, username, C_CHANNEL);
            if (reply->type != 4) {
                std::cout << "Fornitore " << username << " --> order received!\n" << std::endl;
                parseCustomerMessage(reply);
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