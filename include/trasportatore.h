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
    std::string email;
    std::string password;
    std::string address;
    std::string phone_number;
    TrasportatoreState state;

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

    //Notify delivery
    void deliverOrder();

    //Status change
    void handleState();
public:
    Trasportatore(int id, std::string e, std::string p, std::string a, std::string ph);

    // Getters
    int getTrasportatoreId() const;
    std::string getEmail() const;
    std::string getPassword() const;
    std::string getAddress() const;
    std::string getPhoneNumber() const;

    // Setters
    void setEmail(const std::string& e);
    void setPassword(const std::string& p);
    void setAddress(const std::string& a);
    void setPhoneNumber(const std::string& ph);

    void run();

    std::string getState() const;
};

#endif // TRASPORTATORE_H
