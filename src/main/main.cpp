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
#include <vector>
#include <string>

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

void execProgram(const char* program, const char* arg) {
    pid_t pid = fork();
    if(pid == 0) {
        // Processo figlio
        execl(program, program, arg, (char*) NULL);
        // Se execl fallisce
        std::cerr << "Errore durante l'esecuzione di " << program << std::endl;
        std::cerr << "Fork failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        std::cerr << "Fork failed: " << strerror(errno) << std::endl;
    }
}

void terminateAllProcesses() {
    std::cout << "Terminating all processes after 5 minutes..." << std::endl;
    // Usa il comando `kill` per terminare tutti i processi figli
    // Assicurati che tutti i processi siano terminati (kill all)
    system("killall customer");
    system("killall fornitore");
    system("killall trasportatore");
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

    //Genero i customer
    for (int i = 0; i <= 90; i++) {
        execProgram("/home/parallels/Documents/IngSoft/BackendEcommerce/src/customer/customer","");
    }

    std::vector<std::string> fornitori = {
        "apple", "samsung" };
        //, "dell", "hp", "lenovo", "acer", "asus", 
        //"msi", "sony", "toshiba", "lg", "huawei", "xiaomi", "nokia", "oneplus"
    //};

    for (const std::string& fornitore : fornitori) {
        execProgram("/home/parallels/Documents/IngSoft/BackendEcommerce/src/fornitore/fornitore", fornitore.c_str());
    }

    //Genero i trasportatori
    for (int i = 0; i <= 5; i++) {
        execProgram("/home/parallels/Documents/IngSoft/BackendEcommerce/src/trasportatore/trasportatore","");
    }

    // Crea un thread che attende 5 minuti e termina i processi
    std::thread timeoutThread([]() {
        std::this_thread::sleep_for(std::chrono::minutes(15));  // 5 minuti di attesa
        terminateAllProcesses();  // Termina tutti i processi
    });

    // Processo principale attende la terminazione dei processi figli
    int status;
    while (wait(&status) > 0);

    timeoutThread.join();  // Attende che il thread del timeout termini

    redisFree(c2r); // Cleanup Redis connection
    delete[] username;  // Cleanup

    return 0;
}

