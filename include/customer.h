// Customer.h

#ifndef CUSTOMER_H
#define CUSTOMER_H

#include "redis_helper.h"
#include "../../con2db/pgsql.h"

#include <string>
#include <stdio.h>
#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include <ctime>
#include <hiredis/hiredis.h>
#include <con2redis.h>
#include <unistd.h>
#include <cstring> 

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define ORANGE  "\033[38;5;208m"

class Customer {
private:

    //States
    enum class CustomerState {
        Idle,
        WaitingOrderConfirm,
        WaitingForDelivery
    };

    //Attributes
    int customer_id;
    std::string name;
    CustomerState state;

    //Probability of order generation
    std::mt19937 rng;
    std::uniform_real_distribution<> dist;

    //Performance monitoring variables
    std::chrono::time_point<std::chrono::steady_clock> request_sent;
    std::chrono::time_point<std::chrono::steady_clock> request_confirmed;

    //Redis connection variables
    redisContext *c2r;
    redisReply *reply;
    int pid;
    unsigned seed;
    char* username = nullptr;

    //Used to print states
    std::string stateToString(CustomerState state) const;

    //Transition functions
    void transitionToIdle();
    void transitionToWaitingOrderConfirm();
    void transitionToWaitingForDelivery();

    bool parseMessage(redisReply *redis); //Parses messages from fornitori or trasportatori to update order status, returns TRUE if the message was for him
    
    //DB Functions
    PGresult* getAvailableProducts();

    //Order generation
    void simulateOrder();

    //Status change
    void handleState();
public:
    Customer(int id, std::string n);

    // Getters
    int getCustomerId() const;
    std::string getName() const;

    // Setters
    void setName(const std::string& n);

    void run();

    std::string getState() const;
};

#endif // CUSTOMER_H
