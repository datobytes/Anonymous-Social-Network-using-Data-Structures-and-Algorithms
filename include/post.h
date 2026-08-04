#ifndef POST_H
#define POST_H

#include "config.h"
#include "user.h"

typedef struct Post {
    int          id;
    int          authorId;
    int          isAnonymous;
    char         content[MAX_CONTENT_LEN];
    int          reacts;
    time_t       postTime;
    struct Post* next;
} Post;

typedef struct PostList {
    Post* head;
    int   count;
    int   nextId;
} PostList;

PostList* createPostList(void);
Post* createPost(PostList* list, int authorId, const char* content, time_t postTime, int isAnon);
Post* findPostById(PostList* list, int postId);
void printNewsFeed(PostList* list, UserStore* users);
void printPostRow(Post* p, UserStore* users);
int reactPost(Post* p);
long hoursSincePost(const Post* p);
void freePostList(PostList* list);

#endif // POST_H
