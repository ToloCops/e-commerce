// Trasportatore.cpp

#include "trasportatore.h"

#define C_CHANNEL "stream1"
#define T_CHANNEL "stream2"
#define F_CHANNEL "stream3"

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

void Trasportatore::parseFornitoreMessage(redisReply *reply) {
    if (reply->type == REDIS_REPLY_ARRAY) {
        for (size_t i = 0; i < reply->elements; i++) {
            redisReply *stream = reply->element[i];

            // Ogni stream dovrebbe avere un nome (prima parte) e un array di elementi (seconda parte)
            if (stream->type == REDIS_REPLY_ARRAY && stream->elements == 2) {
                redisReply *stream_name = stream->element[0];
                redisReply *entries = stream->element[1];

                //printf("Stream: %s\n", stream_name->str);

                // Ogni entry nello stream
                for (size_t j = 0; j < entries->elements; j++) {
                    redisReply *entry = entries->element[j];

                    if (entry->type == REDIS_REPLY_ARRAY && entry->elements == 2) {
                        redisReply *entry_id = entry->element[0];
                        redisReply *fields = entry->element[1];

                        //printf("Entry ID: %s\n", entry_id->str);

                        // Stampiamo le coppie chiave-valore
                        for (size_t k = 0; k < fields->elements; k += 2) {
                            redisReply *key = fields->element[k];
                            redisReply *value = fields->element[k + 1];

                            if (value->str != NULL && (value->str)[0] != '\0') {
                                customer = value->str;
                            }

                            //printf("%s: %s\n", key->str, value->str);
                        }
                    }
                }
            }
        }
    }
}

void Trasportatore::deliverOrder() {
}

void Trasportatore::handleState() {
    switch (state) {
        case TrasportatoreState::Idle:
            reply = RedisCommand(c2r, "XREADGROUP GROUP %s %s BLOCK 10000 COUNT 10 NOACK STREAMS %s >", 
			  username, username, T_CHANNEL);
            if (reply->type != 4) {
                parseFornitoreMessage(reply);
                std::cout << GREEN << "Trasportatore " << username << " --> order received! Deliver to user: " << customer << "\n" << RESET << std::endl;
                transitionToInTransit();
            }
            break;

        case TrasportatoreState::InTransit: {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> distr(1, 5);
            int tempoConsegna = distr(gen);
            std::this_thread::sleep_for(std::chrono::seconds(tempoConsegna));
            reply = RedisCommand(c2r, "XADD %s * utente %s stato DELIVERED", C_CHANNEL, customer);                      //notifies customer
            printf(GREEN "ORDER DELIVERED IN %d DAYS" RESET, tempoConsegna);
            std::cout << GREEN << "Trasportatore " << username << " --> order delivered! Coming back...\n" << RESET << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            std::cout << GREEN << "Trasportatore " << username << " --> back to the HQ.\n" << RESET << std::endl;
            transitionToIdle();
            break;
        }
        default:
            std::cerr << "Unknown state!" << std::endl;
            break;
    }
}

void Trasportatore::run() {
    std::cout << GREEN << "Hello word from Trasportatore" << RESET << std::endl;
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