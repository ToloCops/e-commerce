// Fornitore.h

#ifndef FORNITORE_H
#define FORNITORE_H

#include "redis_helper.h"

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
    void processOrder();

    //Status change
    void handleState();
public:
    Fornitore(int id, std::string cn, std::string e, std::string p, std::string a, std::string ph);

    // Getters
    int getFornitoreId() const;
    std::string getCompany() const;

    // Setters
    void setCompany(const std::string& n);

    void run();

    std::string getState() const;
};

#endif // FORNITORE_H
