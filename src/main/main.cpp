#include <iostream>
#include <pqxx/pqxx>
#include <hiredis/hiredis.h>
#include <thread>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <con2redis.h>
#include <sys/wait.h>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <errno.h>

#define C_CHANNEL "stream1"
#define T_CHANNEL "stream2"
#define F_CHANNEL "stream3"

redisContext* initializeRedisConnection(char*& username, unsigned &seed, int &pid) {

    seed = (unsigned) time(NULL);
    srand(seed);

    username = new char[100];

    sprintf(username, "%u", rand());
 
    pid = getpid();

    printf("main(): pid %d: user %s: connecting to redis ...\n", pid, username);

    redisContext* c2r = redisConnect("localhost", 6379);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    if (c2r == NULL || c2r->err) {
        if (c2r) {
            printf("Connection error: %s\n", c2r->errstr);
            redisFree(c2r);
        } else {
            printf("Connection error: can't allocate redis context\n");
        }
        return NULL;
    }
    
    printf("main(): pid %d: user %s: connected to redis\n", pid, username);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    return c2r;

}

void deleteOldStreams(redisContext* c2r, redisReply* reply) {

    reply = RedisCommand(c2r, "DEL %s", F_CHANNEL);
    assertReply(c2r, reply);
    dumpReply(reply, 0);

    reply = RedisCommand(c2r, "DEL %s", T_CHANNEL);
    assertReply(c2r, reply);
    dumpReply(reply, 0);

    reply = RedisCommand(c2r, "DEL %s", C_CHANNEL);
    assertReply(c2r, reply);
    dumpReply(reply, 0);

    std::cout << "\nDeleted old streams" << std::endl;

}

void initializeStreams(redisContext* c2r) {
    initStreams(c2r, F_CHANNEL);
    initStreams(c2r, T_CHANNEL);
    initStreams(c2r, C_CHANNEL);
}

void execProgram(const char* program) {
    pid_t pid = fork();
    if(pid == 0) {
        // Processo figlio
        execl(program, program, (char*) NULL);
        // Se execl fallisce
        std::cerr << "Errore durante l'esecuzione di " << program << std::endl;
        std::cerr << "Fork failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        std::cerr << "Fork failed: " << strerror(errno) << std::endl;
    }
}

void setupDatabase() {
    try {
        pqxx::connection C("dbname=ecommerce user=youruser password=yourpassword hostaddr=127.0.0.1 port=5432");
        pqxx::work W(C);

        W.exec("DROP TABLE IF EXISTS Ordini, Prodotti, Customers, Fornitori, Trasportatori CASCADE");
        W.exec("CREATE TABLE Customers (customer_id SERIAL PRIMARY KEY, name VARCHAR(100), email VARCHAR(100) UNIQUE, password VARCHAR(100), address VARCHAR(255), phone_number VARCHAR(15))");
        W.exec("CREATE TABLE Fornitori (fornitore_id SERIAL PRIMARY KEY, company_name VARCHAR(100), email VARCHAR(100) UNIQUE, password VARCHAR(100), address VARCHAR(255), phone_number VARCHAR(15))");
        W.exec("CREATE TABLE Trasportatori (trasportatore_id SERIAL PRIMARY KEY, company_name VARCHAR(100), email VARCHAR(100) UNIQUE, password VARCHAR(100), address VARCHAR(255), phone_number VARCHAR(15))");
        W.exec("CREATE TABLE Prodotti (prodotto_id SERIAL PRIMARY KEY, name VARCHAR(100), description TEXT, price DECIMAL(10, 2), fornitore_id INT REFERENCES Fornitori(fornitore_id))");
        W.exec("CREATE TABLE Ordini (ordine_id SERIAL PRIMARY KEY, customer_id INT REFERENCES Customers(customer_id), prodotto_id INT REFERENCES Prodotti(prodotto_id), quantity INT, order_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP, status VARCHAR(50))");

        W.commit();
        std::cout << "Database setup complete." << std::endl;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

void testRedisConnection() {
    redisContext *c = redisConnect("127.0.0.1", 6379);
    if (c != NULL && c->err) {
        std::cerr << "Error: " << c->errstr << std::endl;
    } else {
        std::cout << "Connected to Redis." << std::endl;
    }
    redisFree(c);
}

/*void simulateOperations() {
    Customer customer1(1, "John Doe", "john@example.com", "password123", "123 Main St", "555-1234");
    Customer customer2(2, "Jane Smith", "jane@example.com", "password456", "456 Elm St", "555-5678");

    Fornitore fornitore1(1, "Tech Corp", "tech@example.com", "password789", "789 Oak St", "555-8765");
    Fornitore fornitore2(2, "Gadget Inc", "gadget@example.com", "password101", "101 Pine St", "555-1011");

    Trasportatore trasportatore1(1, "Fast Delivery", "fast@example.com", "password112", "112 Cedar St", "555-1213");
    Trasportatore trasportatore2(2, "Quick Ship", "quick@example.com", "password131", "131 Maple St", "555-1415");

    std::cout << "Operations simulation complete." << std::endl;
}*/

void monitorTransactions() {
    try {
        pqxx::connection C("dbname=ecommerce user=youruser password=yourpassword hostaddr=127.0.0.1 port=5432");
        pqxx::nontransaction N(C);
        pqxx::result R(N.exec("SELECT COUNT(*) FROM Ordini"));

        if (!R.empty()) {
            std::cout << "Number of transactions: " << R[0].at(0).as<int>() << std::endl;
        } else {
            std::cout << "No transactions found." << std::endl;
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

void monitorPerformance() {
    try {
        pqxx::connection C("dbname=ecommerce user=youruser password=yourpassword hostaddr=127.0.0.1 port=5432");
        auto start = std::chrono::high_resolution_clock::now();

        pqxx::nontransaction N(C);
        pqxx::result R(N.exec("SELECT * FROM Prodotti"));

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;

        std::cout << "Time taken to fetch products: " << elapsed.count() << " seconds" << std::endl;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

int main() {
    redisContext *c2r;
    redisReply *reply;
    int read_counter = 0;
    int send_counter = 0;
    int block = 1000000000;
    int pid;
    unsigned seed;
    char* username = nullptr;
    char key[100];
    char value[100];
    char streamname[100];
    char msgid[100];
    int numstreams;
    int k, i, h;
    char fval[100];

    std::cout << "Setting up the environment..." << std::endl;

    c2r = initializeRedisConnection(username, seed, pid);

    deleteOldStreams(c2r, reply);

    initializeStreams(c2r);

    execProgram("/home/parallels/Documents/IngSoft/BackendEcommerce/src/customer/customer");
    execProgram("/home/parallels/Documents/IngSoft/BackendEcommerce/src/fornitore/fornitore");
    execProgram("/home/parallels/Documents/IngSoft/BackendEcommerce/src/trasportatore/trasportatore");

    //execProgram("../src/customer/customer");
    //execProgram("../src/fornitore/fornitore");
    //execProgram("../src/trasportatore/trasportatore");


    // Parent process waits for child processes
    int status;
    while (wait(&status) > 0);

    redisFree(c2r); // Cleanup Redis connection
    return 0;

    delete[] username;

    return 0;
}

