// Trasportatore.h

#ifndef TRASPORTATORE_H
#define TRASPORTATORE_H

#include "redis_helper.h"

#include <string>
#include <cstring>
#include <stdio.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <hiredis/hiredis.h>
#include <con2redis.h>

class Trasportatore {
private:

    //States
    enum class TrasportatoreState {
        Idle,
        InTransit
    };

    //Attributes
    int trasportatore_id;
    TrasportatoreState state;
    char *customer;

    //Redis connection variables
    redisContext *c2r;
    redisReply *reply;
    int pid;
    unsigned seed;
    char* username = nullptr;

    //Used to print states
    std::string stateToString(TrasportatoreState state) const;

    //Transition functions
    void transitionToIdle();
    void transitionToInTransit();

    void parseFornitoreMessage(redisReply *reply);

    //Notify delivery
    void deliverOrder();

    //Status change
    void handleState();
public:
    Trasportatore(int id);

    // Getters
    int getTrasportatoreId() const;
    std::string getState() const;

    void run();

};

#endif // TRASPORTATORE_H
