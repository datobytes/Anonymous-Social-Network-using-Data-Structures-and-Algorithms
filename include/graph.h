#ifndef GRAPH_H
#define GRAPH_H

#include "config.h"
#include "user.h"

void initGraph(int users);
void setRelation(int u, int v, int status);
int getRelation(int u, int v);
int countMutualFriends(int u, int v);
void recommendFriends(UserStore* users, int u, int minMutual);
void printAdjMatrix(void);

#endif // GRAPH_H
