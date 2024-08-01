// Customer.h

#ifndef CUSTOMER_H
#define CUSTOMER_H

#include "redis_helper.h"

#include <string>
#include <stdio.h>
#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include <hiredis/hiredis.h>
#include <con2redis.h>
#include <unistd.h>
#include <cstring>

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
    std::string email;
    std::string password;
    std::string address;
    std::string phone_number;
    CustomerState state;

    //Probability of order generation
    std::mt19937 rng;
    std::uniform_real_distribution<> dist;

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

    //Order generation
    void simulateOrder();

    //Status change
    void handleState();
public:
    Customer(int id, std::string n, std::string e, std::string p, std::string a, std::string ph);

    // Getters
    int getCustomerId() const;
    std::string getName() const;
    std::string getEmail() const;
    std::string getPassword() const;
    std::string getAddress() const;
    std::string getPhoneNumber() const;

    // Setters
    void setName(const std::string& n);
    void setEmail(const std::string& e);
    void setPassword(const std::string& p);
    void setAddress(const std::string& a);
    void setPhoneNumber(const std::string& ph);

    void run();

    std::string getState() const;
};

#endif // CUSTOMER_H
