#ifndef REDIS_HELPER_H
#define REDIS_HELPER_H

#include <hiredis/hiredis.h>
#include <con2redis.h>
#include <unistd.h>
#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <chrono>
#include <cassert>
#include <ctime>
#include <iostream>
#include <random>

redisContext* initializeRedisConnection(char*& username, unsigned& seed, int& pid);

void initGroup(redisContext *c2r, const char *stream, char *username);

#endif // REDIS_HELPER_H
