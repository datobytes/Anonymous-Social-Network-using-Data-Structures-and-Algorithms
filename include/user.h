#ifndef USER_H
#define USER_H

#include "config.h"

typedef struct User {
    int  id;
    char name[MAX_NAME_LEN];
    int  alias;
} User;

typedef struct UserStore {
    User** items;
    int    count;
    int    capacity;
} UserStore;

UserStore* createUserStore(int initCapacity);
User* addUser(UserStore* store, const char* name);
User* findUserById(UserStore* store, int id);
const char* getUserName(UserStore* store, int id);
int         getAlias(UserStore* store, int id);
void        xaoTronBiDanh(UserStore* store);
void        printAllUsers(UserStore* store);
void        freeUserStore(UserStore* store);

#endif // USER_H
