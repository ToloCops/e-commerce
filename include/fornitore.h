// Fornitore.h

#ifndef FORNITORE_H
#define FORNITORE_H

#include "redis_helper.h"
#include "../../con2db/pgsql.h"

#include <string>
#include <stdio.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <hiredis/hiredis.h>
#include <con2redis.h>

class Fornitore {
private:

    //States
    enum class FornitoreState {
        WaitingForOrder,
        ProcessingOrder
    };

    //Attributes
    int fornitore_id;
    std::string company_name;
    FornitoreState state;

    //Probability of order reject
    std::mt19937 rng;
    std::uniform_real_distribution<> dist;

    //Redis connection variables
    redisContext *c2r;
    redisReply *reply;
    int pid;
    unsigned seed;
    char* username = nullptr;

    //Used to print states
    std::string stateToString(FornitoreState state) const;

    //Transition functions
    void transitionToWaitingForOrder();
    void transitionToProcessingOrder();

    //Order processing
    bool parseCustomerMessage(redisReply *reply);
    void processOrder(char *product, char *user);

    //Status change
    void handleState();
public:
    Fornitore(int id, std::string cn);

    // Getters
    int getFornitoreId() const;
    std::string getCompany() const;

    // Setters
    void setCompany(const std::string& n);

    void run();

    std::string getState() const;
};

#endif // FORNITORE_H
