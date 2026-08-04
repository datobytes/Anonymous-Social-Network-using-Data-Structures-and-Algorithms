#ifndef CENSOR_H
#define CENSOR_H

#include "config.h"

typedef struct TrieNode {
    struct TrieNode* children[ALPHABET_SIZE];
    int              isEndOfWord;
} TrieNode;

TrieNode* createTrieNode(void);
void insertTrie(TrieNode* root, const char* word);
int searchTrie(TrieNode* root, const char* word);
void loadBadWords(TrieNode* root, const char* filename);
void censorText(TrieNode* root, char* text);
void freeTrie(TrieNode* node);

#endif // CENSOR_H
