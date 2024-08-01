#include "redis_helper.h"

redisContext* initializeRedisConnection(char*& username, unsigned& seed, int& pid) {
    seed = (unsigned) time(NULL) + getpid();  // Aggiungi il PID al seme
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


void initGroup(redisContext *c2r, const char *stream, char *username) {
    redisReply *r = RedisCommand(c2r, "XGROUP CREATE %s %s $", stream, username);
    assertReply(c2r, r);
    freeReplyObject(r);
}
