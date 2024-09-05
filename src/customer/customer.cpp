#include "customer.h"

#define C_CHANNEL "stream1"
#define F_CHANNEL "stream3"

Con2DB db("localhost", "5432", "postgres", "postgres", "backend");

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
}

void Customer::transitionToWaitingOrderConfirm() {
    state = CustomerState::WaitingOrderConfirm;
}

void Customer::transitionToWaitingForDelivery() {
    state = CustomerState::WaitingForDelivery;
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

                            if (k == 0) {
                                if (strcmp(value->str, username) != 0) {      // Controlla che il messaggio fosse per lui
                                    break;
                                }
                                else {
                                    for_me = true;
                                }
                            }
                            else if (k == 2) {                                // Aggiorna lo stato dell'ordine
                                if (strcmp(value->str, "CONFIRMED") == 0) {
                                    std::cout << "Customer " << username << " --> order CONFIRMED!\n" << std::endl;
                                    transitionToWaitingForDelivery();
                                }
                                else if (strcmp(value->str, "DELIVERED") == 0) {
                                    std::cout << "Customer " << username << " --> product RECEIVED!\n" << std::endl;
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

PGresult* Customer::getAvailableProducts() {

    PGresult *res;
    PGresult *prods;
    int nrows;
    char sqlcmd[1000];

    sprintf(sqlcmd, "BEGIN");
    res = db.ExecSQLcmd(sqlcmd);
    PQclear(res);

    sprintf(sqlcmd, "SELECT * FROM availableproducts WHERE quantity > 0");
    prods = db.ExecSQLcmd(sqlcmd);
    nrows = PQntuples(prods);

    printf("\nAvailable Products: \n");
    for (int i = 0; i < nrows; i++) {
        fprintf(stderr, "(%s, %s, %d)\n",
            PQgetvalue(prods, i, PQfnumber(prods, "p_name")),
            PQgetvalue(prods, i, PQfnumber(prods, "fornitore")),
            atoi(PQgetvalue(prods, i, PQfnumber(prods, "quantity")))
        );
    }
    printf("\n");

    sprintf(sqlcmd, "COMMIT");
    res = db.ExecSQLcmd(sqlcmd);
    PQclear(res);

    return prods;

}

void Customer::simulateOrder() {
    //Probabilità del 50% di effetuare un ordine
    if (dist(rng) < 0.5) {
        int n_prods;
        int rand_prod;

        char fornitore[100];
        char prodotto[100];

        auto prods = getAvailableProducts();
        n_prods = PQntuples(prods);

        if (n_prods == 0) {
            printf("No products available\n");
            transitionToIdle();
            return;
        }
        
        std::random_device rd;  // Generatore di numeri casuali vero (o quasi)
        std::mt19937 gen(rd()); // Generatore di numeri casuali Mersenne Twister
        std::uniform_int_distribution<> distrib(0, n_prods-1); // Distribuzione uniforme tra 0 e n

        rand_prod = distrib(gen) ; // Genera un numero casuale tra 0 e n
        fprintf(stderr, "Random product: (%s, %s)\n",
            PQgetvalue(prods, rand_prod, PQfnumber(prods, "p_name")),
            PQgetvalue(prods, rand_prod, PQfnumber(prods, "fornitore"))
        );

        strcpy(fornitore, PQgetvalue(prods, rand_prod, PQfnumber(prods, "fornitore")));
        strcpy(prodotto,  PQgetvalue(prods, rand_prod, PQfnumber(prods, "p_name")));
        PQclear(prods);

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
            // Interroga lo stream dei customer per ricevere nuovi messaggi
            reply = RedisCommand(c2r, "XREADGROUP GROUP %s %s BLOCK 10000 COUNT 10 NOACK STREAMS %s >",             
			                        username, username, C_CHANNEL);
            // Controlla se ha avuto risposta e che il messaggio fosse per lui
            if (reply->type == 4 || !parseMessage(reply)) {                                                                                                                                                                 
                std::cout << "Customer " << username << "--> waiting for order confirmation...\n" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(2));  
            }
            break;

        case CustomerState::WaitingForDelivery:
            reply = RedisCommand(c2r, "XREADGROUP GROUP %s %s BLOCK 10000 COUNT 10 NOACK STREAMS %s >", 
			                        username, username, C_CHANNEL);
            if (reply->type == 4 || !parseMessage(reply)) {
                std::cout << "Customer " << username << " --> waiting for delivery...\n" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
            break;

        default:
            std::cerr << "Unknown state!" << std::endl;
            break;
    }
}

void testDBConnection() {
    
    PGresult *res;
    char sqlcmd[1000];

    sprintf(sqlcmd, "BEGIN");
    res = db.ExecSQLcmd(sqlcmd);
    PQclear(res);

    sprintf(sqlcmd, "INSERT INTO availableproducts VALUES ('iphone', 'apple', 3) ON CONFLICT DO NOTHING");
    res = db.ExecSQLcmd(sqlcmd);
    PQclear(res);

    sprintf(sqlcmd, "COMMIT");
    res = db.ExecSQLcmd(sqlcmd);
    PQclear(res);
    
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