// Fornitore.cpp

#include "fornitore.h"

#define C_CHANNEL "stream1"
#define T_CHANNEL "stream2"
#define F_CHANNEL "stream3"

Con2DB db("localhost", "5432", "postgres", "postgres", "backend");

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

bool Fornitore::parseCustomerMessage(redisReply *reply) {
    char *product;
    char *user = nullptr;
    bool for_me;
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
                                if (strcmp(company_name.c_str(), value->str) != 0)                                              // Controlla che il messaggio fosse per lui
                                {
                                    break;
                                }
                                else 
                                {
                                    for_me = true;
                                }
                            }
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
    return for_me;
}

void Fornitore::processOrder(char *product, char *user) {
    PGresult *res;
    char sqlcmd[1000];
    const char* fornitore = company_name.c_str();

    transitionToProcessingOrder();
    printf("Processing order for user: %s, product: %s\n", user, product);
    
    sprintf(sqlcmd, "BEGIN");
    res = db.ExecSQLcmd(sqlcmd);
    PQclear(res);

    sprintf(sqlcmd, "UPDATE availableproducts SET quantity = quantity - 1 WHERE p_name = \'%s\' AND fornitore = \'%s\' AND quantity > 0", product, fornitore);
    printf(sqlcmd);
    res = db.ExecSQLcmd(sqlcmd);
    PQclear(res);

    sprintf(sqlcmd, "INSERT INTO transactions (customer, p_name, fornitore, quantity) VALUES (\'%s\', \'%s\', \'%s\', 1) ON CONFLICT DO NOTHING", user, product, fornitore);
    printf(sqlcmd);
    res = db.ExecSQLcmd(sqlcmd);
    PQclear(res);

    sprintf(sqlcmd, "COMMIT");
    res = db.ExecSQLcmd(sqlcmd);
    PQclear(res);

    reply = RedisCommand(c2r, "XADD %s * utente %s", T_CHANNEL, user);                                      //notifies trasportatori
    reply = RedisCommand(c2r, "XADD %s * utente %s stato CONFIRMED", C_CHANNEL, user);                      //notifies customer
}

void Fornitore::handleState() {
    switch (state) {
        case FornitoreState::WaitingForOrder:
            reply = RedisCommand(c2r, "XREADGROUP GROUP %s %s BLOCK 10000 COUNT 10 NOACK STREAMS %s >", 
			  username, username, F_CHANNEL);
            if (!parseCustomerMessage(reply)) {
                std::cout << "Fornitore " << username << " --> waiting for order...\n" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
            break;

        case FornitoreState::ProcessingOrder:
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
    initGroup(c2r, F_CHANNEL, username);
    while (true) {
        handleState();
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Simula il passare del tempo
    }
}

int main(int argc, char* argv[]) {
    // Verifica se è stato passato un argomento
    if (argc < 2) {
        std::cerr << "Errore: nessun nome di fornitore passato." << std::endl;
        return 1;
    }

    // Il nome del fornitore è il primo argomento passato
    std::string nomeFornitore = argv[1];
    Fornitore fornitore(1, nomeFornitore);
    fornitore.run();
    return 0;
}