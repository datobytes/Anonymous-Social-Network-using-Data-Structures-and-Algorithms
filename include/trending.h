#ifndef TRENDING_H
#define TRENDING_H

#include "config.h"
#include "post.h"
#include "user.h"

typedef struct HeapNode {
    int   postId;
    long  score;
    Post* ref;
} HeapNode;

typedef struct MaxHeap {
    HeapNode* nodes;
    int       size;
    int       capacity;
} MaxHeap;

MaxHeap* createHeap(int initCapacity);
long calcScore(const Post* p);
int heapPush(MaxHeap* h, Post* p);
void heapifyUp(MaxHeap* h, int i);
void heapifyDown(MaxHeap* h, int i);
int  heapFindIndex(MaxHeap* h, int postId);
void heapUpdateOnReact(MaxHeap* h, int postId);
void heapRebuild(MaxHeap* h);
void printTrending(MaxHeap* h, int topK, UserStore* users);
void freeHeap(MaxHeap* h);

#endif // TRENDING_H
