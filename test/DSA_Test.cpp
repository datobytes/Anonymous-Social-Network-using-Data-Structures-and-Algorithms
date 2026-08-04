/*
    DO AN CUOI KY - MON CAU TRUC DU LIEU VA GIAI THUAT
    De tai 12: Mang xa hoi an danh va goi y ket ban
               (Campus Confessions & Connections)
*/

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/* ---------------- CAU TRUC DU LIEU ---------------- */

/* ---- Hang so gioi han ---- */
#define MAX_USERS        100
#define MAX_NAME_LEN      50
#define MAX_CONTENT_LEN  256
#define ALPHABET_SIZE    256

#define MAX_REACTS 100000000

#define REL_NONE    0
#define REL_FRIEND  1
#define REL_PENDING 2

#define MIN_MUTUAL_FRIENDS 3

/* ---- Nguoi dung ---- */
typedef struct User {
    int  id;
    char name[MAX_NAME_LEN];
    int  alias;
} User;

/* ---- Bai viet ---- */
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

/* ---- Node cua cay tien to ---- */
typedef struct TrieNode {
    struct TrieNode* children[ALPHABET_SIZE];
    int              isEndOfWord;
} TrieNode;

/* ---- Phan tu cua Max-Heap ---- */
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

/* ---------------- NGUYEN MAU HAM ---------------- */

typedef struct UserStore {
    User** items;
    int    count;
    int    capacity;
} UserStore;

UserStore* createUserStore(int initCapacity);
User* addUser(UserStore* store, const char* name);
User* findUserById(UserStore* store, int id);
const char* getUserName(UserStore* store, int id);
int        getAlias(UserStore* store, int id);
void       xaoTronBiDanh(UserStore* store);
void       printAllUsers(UserStore* store);
void       freeUserStore(UserStore* store);

PostList* createPostList(void);

Post* createPost(PostList* list, int authorId, const char* content,
    time_t postTime, int isAnon);

Post* findPostById(PostList* list, int postId);

void printNewsFeed(PostList* list, UserStore* users);

void printPostRow(Post* p, UserStore* users);

int reactPost(Post* p);

long hoursSincePost(const Post* p);

void freePostList(PostList* list);

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

TrieNode* createTrieNode(void);

void insertTrie(TrieNode* root, const char* word);

int searchTrie(TrieNode* root, const char* word);

void loadBadWords(TrieNode* root, const char* filename);

void censorText(TrieNode* root, char* text);

void freeTrie(TrieNode* node);

void initGraph(int users);

void setRelation(int u, int v, int status);

int getRelation(int u, int v);

int countMutualFriends(int u, int v);

void recommendFriends(UserStore* users, int u, int minMutual);

void printAdjMatrix(void);

typedef struct AppContext {
    UserStore* users;
    PostList* posts;
    MaxHeap* trending;
    TrieNode* badWords;
    int        currentUser;
} AppContext;

AppContext* initApp(const char* badWordsFile);

void initMockData(AppContext* app);

void cleanupAll(AppContext* app);

/* ---------------- QUAN LY NGUOI DUNG ---------------- */

UserStore* createUserStore(int initCapacity)
{
    UserStore* store;

    if (initCapacity <= 0) initCapacity = 8;

    store = (UserStore*)malloc(sizeof(UserStore));
    if (store == NULL) {
        printf("Loi: Khong du bo nho de tao UserStore!\n");
        return NULL;
    }

    store->items = (User**)malloc(sizeof(User*) * initCapacity);
    if (store->items == NULL) {
        printf("Loi: Khong du bo nho cho mang User!\n");
        free(store);
        return NULL;
    }

    store->count = 0;
    store->capacity = initCapacity;
    return store;
}

/* Thêm 1 người dùng mới */
User* addUser(UserStore* store, const char* name)
{
    User* u;
    User** tmp;
    int    i;

    if (store == NULL || name == NULL) return NULL;

    if (store->count >= MAX_USERS) {
        printf("Loi: Da dat gioi han %d nguoi dung!\n", MAX_USERS);
        return NULL;
    }

    if (store->count == store->capacity) {
        tmp = (User**)realloc(store->items,
            sizeof(User*) * store->capacity * 2);
        if (tmp == NULL) {
            printf("Loi: Khong the mo rong mang User!\n");
            return NULL;
        }
        store->items = tmp;
        store->capacity *= 2;
    }

    u = (User*)malloc(sizeof(User));
    if (u == NULL) {
        printf("Loi: Khong du bo nho de tao User!\n");
        return NULL;
    }

    u->id = store->count;
    u->alias = store->count;

    for (i = 0; i < MAX_NAME_LEN - 1 && name[i] != '\0'; i++) {
        u->name[i] = name[i];
    }
    u->name[i] = '\0';

    store->items[store->count] = u;
    store->count++;
    return u;
}

User* findUserById(UserStore* store, int id)
{
    if (store == NULL || id < 0 || id >= store->count) return NULL;
    return store->items[id];
}

const char* getUserName(UserStore* store, int id)
{
    User* u = findUserById(store, id);
    return (u != NULL) ? u->name : "??";
}
int getAlias(UserStore* store, int id)
{
    User* u = findUserById(store, id);
    return (u != NULL) ? u->alias : -1;
}

void xaoTronBiDanh(UserStore* store)
{
    int i, j, tmp;
    int* bang;

    if (store == NULL || store->count <= 0) return;

    bang = (int*)malloc(sizeof(int) * store->count);
    if (bang == NULL) return;

    for (i = 0; i < store->count; i++) bang[i] = i;

    srand((unsigned int)time(NULL));
    for (i = store->count - 1; i > 0; i--) {
        j = rand() % (i + 1);
        tmp = bang[i]; bang[i] = bang[j]; bang[j] = tmp;
    }

    if (store->count > 1) {
        for (i = 0; i < store->count; i++) {
            if (bang[i] == i) {
                j = (i + 1) % store->count;
                tmp = bang[i]; bang[i] = bang[j]; bang[j] = tmp;
            }
        }
    }

    for (i = 0; i < store->count; i++) {
        store->items[i]->alias = bang[i];
    }
    free(bang);
}
void printAllUsers(UserStore* store)
{
    int i;

    if (store == NULL || store->count == 0) {
        printf("  (Chua co nguoi dung nao)\n");
        return;
    }

    printf("\n+------+----------------------------+\n");
    printf("|  ID  | TEN NGUOI DUNG             |\n");
    printf("+------+----------------------------+\n");
    for (i = 0; i < store->count; i++) {
        printf("| %4d | %-26s |\n", store->items[i]->id, store->items[i]->name);
    }
    printf("+------+----------------------------+\n");
    printf("Tong cong: %d nguoi dung.\n", store->count);
}

void freeUserStore(UserStore* store)
{
    int i;

    if (store == NULL) return;

    for (i = 0; i < store->count; i++) {
        free(store->items[i]);
        store->items[i] = NULL;
    }
    free(store->items);
    free(store);
}

/* ---------------- BAI VIET - DANH SACH LIEN KET ---------------- */

PostList* createPostList(void)
{
    PostList* list = (PostList*)malloc(sizeof(PostList));
    if (list == NULL) {
        printf("Loi: Khong du bo nho de tao PostList!\n");
        return NULL;
    }
    list->head = NULL;
    list->count = 0;
    list->nextId = 1;
    return list;
}

/* createPost(): cấp phát động 1 node Post và NỐI CON TRỎ vào ĐẦU danh sách */
Post* createPost(PostList* list, int authorId, const char* content,
    time_t postTime, int isAnon)
{
    Post* p;
    int   i;

    if (list == NULL || content == NULL) return NULL;

    p = (Post*)malloc(sizeof(Post));
    if (p == NULL) {
        printf("Loi: Khong du bo nho de dang bai!\n");
        return NULL;
    }

    p->id = list->nextId;
    p->authorId = authorId;
    p->reacts = 0;
    p->postTime = (postTime == 0) ? time(NULL) : postTime;
    p->isAnonymous = isAnon;
    for (i = 0; i < MAX_CONTENT_LEN - 1 && content[i] != '\0'; i++) {
        p->content[i] = content[i];
    }
    p->content[i] = '\0';

    p->next = list->head;
    list->head = p;

    list->count++;
    list->nextId++;
    return p;
}

Post* findPostById(PostList* list, int postId)
{
    Post* cur;

    if (list == NULL) return NULL;

    for (cur = list->head; cur != NULL; cur = cur->next) {
        if (cur->id == postId) return cur;
    }
    return NULL;
}

/* Số giờ đã trôi qua kể từ khi đăng */
long hoursSincePost(const Post* p)
{
    double seconds;
    long   hours;

    if (p == NULL) return 0;

    seconds = difftime(time(NULL), p->postTime);
    hours = (long)(seconds / 3600.0);
    if (hours < 0) hours = 0;
    return hours;
}

int reactPost(Post* p)
{
    if (p == NULL) return 0;

    if (p->reacts >= MAX_REACTS) {
        printf("Canh bao: Bai viet da dat nguong react toi da (%d)!\n",
            MAX_REACTS);
        return 0;
    }
    p->reacts++;
    return 1;
}

void printPostRow(Post* p, UserStore* users)
{
    char timeBuf[32];
    struct tm* lt;

    if (p == NULL) return;

    lt = localtime(&p->postTime);
    if (lt != NULL) {
        strftime(timeBuf, sizeof(timeBuf), "%d/%m %H:%M", lt);
    }
    else {
        timeBuf[0] = '\0';
    }

    printf("+--------------------------------------------------------------+\n");
    if (p->isAnonymous) {
        printf("| Bai #%-3d | Tac gia: [An danh #%-11d] | %s |\n",
            p->id, getAlias(users, p->authorId), timeBuf);
    }
    else {
        printf("| Bai #%-3d | Tac gia: %-26s | %s |\n",
            p->id, getUserName(users, p->authorId), timeBuf);
    }

    printf("| Noi dung: %-50s |\n", p->content);
    printf("| React: %-4d | Da dang: %-3ld gio truoc                         |\n",
        p->reacts, hoursSincePost(p));
}

void printNewsFeed(PostList* list, UserStore* users)
{
    Post* cur;

    printf("\n============== BANG TIN (MOI NHAT TRUOC) ==============\n");

    if (list == NULL || list->head == NULL) {
        printf("  (Chua co bai viet nao)\n");
        return;
    }

    for (cur = list->head; cur != NULL; cur = cur->next) {
        printPostRow(cur, users);
    }
    printf("+--------------------------------------------------------------+\n");
    printf("Tong cong: %d bai viet.\n", list->count);
}

/* Giải phóng toàn bộ node: giữ con trỏ tmp trước khi free để không mất liên kết */
void freePostList(PostList* list)
{
    Post* cur, * tmp;

    if (list == NULL) return;

    cur = list->head;
    while (cur != NULL) {
        tmp = cur->next;
        free(cur);
        cur = tmp;
    }
    list->head = NULL;
    list->count = 0;
    free(list);
}

/* ---------------- BANG XEP HANG - MAX-HEAP ---------------- */

MaxHeap* createHeap(int initCapacity)
{
    MaxHeap* h;

    if (initCapacity <= 0) initCapacity = 8;

    h = (MaxHeap*)malloc(sizeof(MaxHeap));
    if (h == NULL) {
        printf("Loi: Khong du bo nho de tao Heap!\n");
        return NULL;
    }

    h->nodes = (HeapNode*)malloc(sizeof(HeapNode) * initCapacity);
    if (h->nodes == NULL) {
        printf("Loi: Khong du bo nho cho mang Heap!\n");
        free(h);
        return NULL;
    }

    h->size = 0;
    h->capacity = initCapacity;
    return h;
}

/* calcScore(): TRỌNG SỐ ĐỘNG tính tại thời điểm gọi hàm (On-the-fly) */
long calcScore(const Post* p)
{
    if (p == NULL) return 0;
    return (long)p->reacts * 10L - hoursSincePost(p) * 2L;
}

static void swapNode(HeapNode* a, HeapNode* b)
{
    HeapNode tmp = *a;
    *a = *b;
    *b = tmp;
}

/* heapifyUp(): node i có trọng số vừa TĂNG -> ngoi dần lên đỉnh */
void heapifyUp(MaxHeap* h, int i)
{
    int parent;

    if (h == NULL) return;

    while (i > 0) {
        parent = (i - 1) / 2;

        if (h->nodes[i].score <= h->nodes[parent].score) break;

        swapNode(&h->nodes[i], &h->nodes[parent]);
        i = parent;
    }
}

/* heapifyDown(): node i có trọng số vừa GIẢM (bài viết cũ đi theo thời gian) */
void heapifyDown(MaxHeap* h, int i)
{
    int left, right, largest;

    if (h == NULL) return;

    while (1) {
        left = 2 * i + 1;
        right = 2 * i + 2;
        largest = i;

        if (left  < h->size && h->nodes[left].score  > h->nodes[largest].score)
            largest = left;
        if (right < h->size && h->nodes[right].score > h->nodes[largest].score)
            largest = right;

        if (largest == i) break;

        swapNode(&h->nodes[i], &h->nodes[largest]);
        i = largest;
    }
}

/* heapPush(): thêm bài viết vào cuối mảng rồi vun lên - O(log n) */
int heapPush(MaxHeap* h, Post* p)
{
    HeapNode* tmp;

    if (h == NULL || p == NULL) return 0;

    if (h->size == h->capacity) {
        tmp = (HeapNode*)realloc(h->nodes,
            sizeof(HeapNode) * h->capacity * 2);
        if (tmp == NULL) {
            printf("Loi: Khong the mo rong Heap!\n");
            return 0;
        }
        h->nodes = tmp;
        h->capacity *= 2;
    }

    h->nodes[h->size].postId = p->id;
    h->nodes[h->size].ref = p;
    h->nodes[h->size].score = calcScore(p);
    h->size++;

    heapifyUp(h, h->size - 1);
    return 1;
}

int heapFindIndex(MaxHeap* h, int postId)
{
    int i;

    if (h == NULL) return -1;

    for (i = 0; i < h->size; i++) {
        if (h->nodes[i].postId == postId) return i;
    }
    return -1;
}

void heapUpdateOnReact(MaxHeap* h, int postId)
{
    int idx;

    if (h == NULL) return;

    idx = heapFindIndex(h, postId);
    if (idx < 0) return;

    h->nodes[idx].score = calcScore(h->nodes[idx].ref);
    heapifyUp(h, idx);
}

/* heapRebuild(): mốc thời gian đã thay đổi -> trọng số MỌI bài viết đều giảm */
void heapRebuild(MaxHeap* h)
{
    int i;

    if (h == NULL || h->size == 0) return;

    for (i = 0; i < h->size; i++) {
        h->nodes[i].score = calcScore(h->nodes[i].ref);
    }
    for (i = h->size / 2 - 1; i >= 0; i--) {
        heapifyDown(h, i);
    }
}

/* printTrending(): lấy Top-K bài hot nhất */
void printTrending(MaxHeap* h, int topK, UserStore* users)
{
    MaxHeap  copy;
    HeapNode* buf;
    int       i, rank;

    printf("\n============== TOP %d BAI VIET THINH HANH ==============\n", topK);

    if (h == NULL || h->size == 0) {
        printf("  (Chua co bai viet nao)\n");
        return;
    }

    heapRebuild(h);

    buf = (HeapNode*)malloc(sizeof(HeapNode) * h->size);
    if (buf == NULL) {
        printf("Loi: Khong du bo nho de xep hang Trending!\n");
        return;
    }
    for (i = 0; i < h->size; i++) buf[i] = h->nodes[i];

    copy.nodes = buf;
    copy.size = h->size;
    copy.capacity = h->size;

    rank = 1;
    while (copy.size > 0 && rank <= topK) {
        printf("\n--- HANG %d | Diem = %ld ---\n", rank, copy.nodes[0].score);
        printPostRow(copy.nodes[0].ref, users);

        copy.nodes[0] = copy.nodes[copy.size - 1];
        copy.size--;
        heapifyDown(&copy, 0);
        rank++;
    }
    printf("+--------------------------------------------------------------+\n");

    free(buf);
}

void freeHeap(MaxHeap* h)
{
    if (h == NULL) return;
    free(h->nodes);
    free(h);
}

/* ---------------- BO LOC TU NGU - CAY TIEN TO ---------------- */

TrieNode* createTrieNode(void)
{
    TrieNode* node = (TrieNode*)malloc(sizeof(TrieNode));
    if (node) {
        int i;
        node->isEndOfWord = 0;
        for (i = 0; i < ALPHABET_SIZE; i++) {
            node->children[i] = NULL;
        }
    }
    return node;
}

/* Chèn từ vào Trie (chuyển sang chữ thường để xử lý không phân biệt hoa/thường) */
void insertTrie(TrieNode* root, const char* word)
{
    TrieNode* p = root;
    int i;

    if (root == NULL || word == NULL) return;

    for (i = 0; word[i] != '\0'; i++) {
        unsigned char index = (unsigned char)tolower((unsigned char)word[i]);
        if (!p->children[index]) {
            p->children[index] = createTrieNode();
            if (!p->children[index]) return;
        }
        p = p->children[index];
    }
    p->isEndOfWord = 1;
}

int searchTrie(TrieNode* root, const char* word)
{
    TrieNode* p = root;
    int i;

    if (root == NULL || word == NULL) return 0;

    for (i = 0; word[i] != '\0'; i++) {
        unsigned char index = (unsigned char)tolower((unsigned char)word[i]);
        if (!p->children[index]) {
            return 0;
        }
        p = p->children[index];
    }
    return (p != NULL && p->isEndOfWord);
}

void loadBadWords(TrieNode* root, const char* filename)
{
    FILE* file = fopen(filename, "r");
    char  word[256];
    int   count = 0;

    if (!file) {
        printf("Canh bao: Khong the mo file %s! Bo loc tu cam se khong hoat dong.\n",
            filename);
        return;
    }

    while (fscanf(file, "%255s", word) == 1) {
        insertTrie(root, word);
        count++;
    }
    fclose(file);
    printf("Da nap %d tu cam tu file %s.\n", count, filename);
}

static int isDelimiter(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' ||
        c == '.' || c == ',' || c == '!' || c == '?' ||
        c == ';' || c == ':' || c == '"' || c == '\'' ||
        c == '(' || c == ')' || c == '-';
}

/* Che từ độc hại TRỰC TIẾP trên vùng nhớ (in-place) bằng dấu '*' */
void censorText(TrieNode* root, char* text)
{
    char        temp[1024];
    const char* delimiters = " \t\n\r.,!?;:\"'()-";
    char* token;

    if (!text || !root) return;

    strncpy(temp, text, sizeof(temp));
    temp[sizeof(temp) - 1] = '\0';

    token = strtok(temp, delimiters);
    while (token != NULL) {
        if (searchTrie(root, token)) {
            char* match = text;
            int   len = (int)strlen(token);

            while ((match = strstr(match, token)) != NULL) {
                int isStartBoundary = (match == text) || isDelimiter(*(match - 1));
                int isEndBoundary = isDelimiter(*(match + len)) ||
                    *(match + len) == '\0';
                if (isStartBoundary && isEndBoundary) {
                    int i;
                    for (i = 0; i < len; i++) match[i] = '*';
                }
                match += len;
            }
        }
        token = strtok(NULL, delimiters);
    }
}

/* Giải phóng đệ quy toàn bộ bộ nhớ động của cây Trie */
void freeTrie(TrieNode* node)
{
    int i;

    if (!node) return;

    for (i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i]) {
            freeTrie(node->children[i]);
        }
    }
    free(node);
}

/* ---------------- MANG LUOI BAN BE - DO THI VO HUONG ---------------- */

static int adjMatrix[MAX_USERS][MAX_USERS];
static int numUsers = 0;

void initGraph(int users)
{
    int i, j;

    if (users > MAX_USERS) users = MAX_USERS;
    if (users < 0)         users = 0;
    numUsers = users;

    for (i = 0; i < numUsers; i++) {
        for (j = 0; j < numUsers; j++) {
            adjMatrix[i][j] = REL_NONE;
        }
    }
}

/* Thiết lập trạng thái quan hệ giữa 2 người dùng */
void setRelation(int u, int v, int status)
{
    if (u >= 0 && u < numUsers && v >= 0 && v < numUsers && u != v) {
        adjMatrix[u][v] = status;

        if (status == REL_FRIEND) {
            adjMatrix[v][u] = REL_FRIEND;
        }
        if (status == REL_NONE) {
            adjMatrix[v][u] = REL_NONE;
        }
    }
}

int getRelation(int u, int v)
{
    if (u < 0 || u >= numUsers || v < 0 || v >= numUsers) return REL_NONE;
    return adjMatrix[u][v];
}

/* Đếm số bạn chung giữa u và v = giao hai tập đỉnh kề. Độ phức tạp O(V). */
int countMutualFriends(int u, int v)
{
    int count = 0;
    int i;

    if (u < 0 || u >= numUsers || v < 0 || v >= numUsers || u == v) {
        return 0;
    }

    for (i = 0; i < numUsers; i++) {
        if (adjMatrix[u][i] == REL_FRIEND && adjMatrix[v][i] == REL_FRIEND) {
            count++;
        }
    }
    return count;
}

/* Gợi ý kết bạn: duyệt mọi đỉnh chưa quen, đếm bạn chung, lọc theo ngưỡng */
void recommendFriends(UserStore* users, int u, int minMutual)
{
    int v, mutual, count = 0;

    printf("\n--- GOI Y KET BAN (tu %d ban chung tro len) ---\n", minMutual);

    if (u < 0 || u >= numUsers) {
        printf("  Loi: Ma nguoi dung khong ton tai!\n");
        return;
    }

    for (v = 0; v < numUsers; v++) {
        if (u == v) continue;

        if (adjMatrix[u][v] == REL_NONE && adjMatrix[v][u] == REL_NONE) {
            mutual = countMutualFriends(u, v);
            if (mutual >= minMutual) {
                printf("  [ID %d] %-20s - %d ban chung\n",
                    v, getUserName(users, v), mutual);
                count++;
            }
        }
    }

    if (count == 0) {
        printf("  Khong co goi y ket ban phu hop.\n");
    }
}

void printAdjMatrix(void)
{
    int i, j;

    printf("\n--- MA TRAN KE MANG LUOI BAN BE ---\n");
    printf("(0: Chua ket ban | 1: Da ket ban | 2: Cho duyet)\n");

    printf("    ");
    for (i = 0; i < numUsers; i++) printf("U%-2d", i);
    printf("\n");

    for (i = 0; i < numUsers; i++) {
        printf("U%-2d ", i);
        for (j = 0; j < numUsers; j++) {
            printf("%-3d", adjMatrix[i][j]);
        }
        printf("\n");
    }
}

/* ---------------- DU LIEU MAU VA GIAI PHONG BO NHO ---------------- */

AppContext* initApp(const char* badWordsFile)
{
    AppContext* app = (AppContext*)malloc(sizeof(AppContext));
    if (app == NULL) {
        printf("Loi nghiem trong: khong du bo nho khoi tao chuong trinh!\n");
        return NULL;
    }

    app->users = NULL;
    app->posts = NULL;
    app->trending = NULL;
    app->badWords = NULL;
    app->currentUser = 0;

    app->users = createUserStore(16);
    app->posts = createPostList();
    app->trending = createHeap(32);
    app->badWords = createTrieNode();

    if (app->users == NULL || app->posts == NULL ||
        app->trending == NULL || app->badWords == NULL) {
        cleanupAll(app);
        return NULL;
    }

    loadBadWords(app->badWords, badWordsFile);

    return app;
}

static const char* MOCK_NAMES[10] = {
    "Thanh Dat", "Phu", "Phat", "Thai Dat", "Hung",
    "Hoang Dat", "Khoi", "Hieu", "Lan Anh", "Minh Thu"
};

static const char* MOCK_CONTENTS[20] = {
    "Thu vien tang 3 hom nay dong qua, ai biet cho nao trong khong?",
    "Co ai on thi Cau truc du lieu cung minh khong?",
    "Canteen doi mon moi an ngon that su!",
    "Deadline do an dang den gan, moi nguoi co gang len nhe.",
    "Ai lam mat the sinh vien mau xanh o san B khong, minh nhat duoc.",
    "Thay day Giai thuat giang de hieu ghe.",
    "Hom nay troi mua, nho mang ao mua nhe cac ban.",
    "Ban nao co tai lieu on tap Toan roi rac cho minh xin voi.",
    "Cau lac bo IT tuyen thanh vien moi, ai quan tam inbox nhe.",
    "May chieu phong A305 hong roi, bao ky thuat gium nhe.",
    "Do ngu that, minh vua nop nham file bai tap.",
    "Ai di xe bus tuyen 08 cho minh di ghep voi.",
    "Hom qua thuc khuya code den 3h sang, hom nay ngu gat ca buoi.",
    "Spam nhom lop nhieu qua, mong moi nguoi han che.",
    "Ban nao mat chia khoa tu do o phong Lab thi len bao ve nhan.",
    "Chuc moi nguoi thi tot nhe, co len!",
    "Sinh vien nam nhat co nen tham gia nhieu cau lac bo khong ta?",
    "Quan cafe doi dien cong truong moi mo, wifi manh lam.",
    "Minh muon tim nhom lam do an mon Lap trinh huong doi tuong.",
    "Hom nay la ngay dep troi de hoc bai, cac ban oi!"
};

/* initMockData(): sinh 10 người dùng + 20 bài đăng + mạng lưới bạn bè mẫu. */
void initMockData(AppContext* app)
{
    char    buffer[MAX_CONTENT_LEN];
    time_t  now;
    Post* p;
    int     i, hoursAgo, reacts;

    if (app == NULL) return;

    now = time(NULL);

    for (i = 0; i < 10; i++) {
        addUser(app->users, MOCK_NAMES[i]);
    }

    initGraph(app->users->count);

    for (i = 0; i < 20; i++) {
        strncpy(buffer, MOCK_CONTENTS[i], MAX_CONTENT_LEN - 1);
        buffer[MAX_CONTENT_LEN - 1] = '\0';

        censorText(app->badWords, buffer);

        hoursAgo = (19 - i) * 2 + 1;

        reacts = (i * 7) % 25;
        int isAnon = (i % 3 == 0) ? 0 : 1;

        p = createPost(app->posts, i % app->users->count, buffer,
            now - (time_t)hoursAgo * 3600, isAnon);
        if (p != NULL) {
            p->reacts = reacts;
            heapPush(app->trending, p);
        }
    }

    setRelation(0, 1, REL_FRIEND);
    setRelation(0, 2, REL_FRIEND);
    setRelation(0, 3, REL_FRIEND);
    setRelation(0, 4, REL_FRIEND);
    setRelation(5, 1, REL_FRIEND);
    setRelation(5, 2, REL_FRIEND);
    setRelation(5, 3, REL_FRIEND);
    setRelation(5, 4, REL_FRIEND);
    setRelation(6, 1, REL_FRIEND);
    setRelation(6, 2, REL_FRIEND);
    setRelation(7, 8, REL_FRIEND);
    setRelation(9, 0, REL_PENDING);

    printf("Da khoi tao du lieu mau: %d nguoi dung, %d bai viet.\n",
        app->users->count, app->posts->count);
}

/* cleanupAll(): GIẢI PHÓNG TOÀN BỘ BỘ NHỚ ĐỘNG */
void cleanupAll(AppContext* app)
{
    if (app == NULL) return;

    freeHeap(app->trending);
    app->trending = NULL;

    freePostList(app->posts);
    app->posts = NULL;

    freeUserStore(app->users);
    app->users = NULL;

    freeTrie(app->badWords);
    app->badWords = NULL;

    free(app);
    printf("Da giai phong toan bo bo nho dong. Tam biet!\n");
}

/* ---------------- GIAO DIEN VA HAM MAIN ---------------- */

#define BADWORDS_FILE   "badwords.txt"
#define TOP_TRENDING    5

#define CHE_DO_KIEM_THU 0

static void clearScreen(void)
{
    if (CHE_DO_KIEM_THU) return;
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

static void pauseScreenMsg(const char* msg)
{
    int c;
    if (CHE_DO_KIEM_THU) return;
    printf("\n   [ %s ]", msg);
    while ((c = getchar()) != '\n' && c != EOF) {
    }
}

static void pauseScreen(void)
{
    pauseScreenMsg("Nhan ENTER de quay lai menu");
}

static void clearInputBuffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
    }
}

/* Đọc 1 số nguyên an toàn. Trả về 1 nếu hợp lệ, 0 nếu nhập sai/hết dữ liệu. */
static int readInt(const char* prompt, int* out)
{
    int rc;

    printf("%s", prompt);
    rc = scanf("%d", out);

    if (rc == EOF) return 0;
    if (rc != 1) {
        clearInputBuffer();
        printf(">> Loi: Vui long nhap MOT SO NGUYEN!\n");
        return 0;
    }
    clearInputBuffer();
    return 1;
}

/* Đọc 1 dòng văn bản an toàn bằng fgets (không dùng gets - dễ tràn bộ đệm) */
static int readLine(const char* prompt, char* buf, int size)
{
    int len;

    printf("%s", prompt);
    if (fgets(buf, size, stdin) == NULL) return 0;

    len = (int)strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    }
    else {
        clearInputBuffer();
        printf(">> Canh bao: Noi dung qua dai, da tu dong cat bot con %d ky tu.\n",
            size - 1);
    }
    return 1;
}

static void printHeader(AppContext* app, const char* title)
{
    printf("==========================================================\n");
    printf("   %s\n", title);
    printf("   Nguoi dung: [ID %d] %s\n",
        app->currentUser, getUserName(app->users, app->currentUser));
    printf("==========================================================\n");
}

/* Màn hình xem bài viết + thả react ngay tại chỗ */
static void viewAndReact(AppContext* app, int trending)
{
    int   postId;
    Post* p;

    while (1) {
        clearScreen();

        if (trending) {
            printTrending(app->trending, TOP_TRENDING, app->users);
        }
        else {
            printNewsFeed(app->posts, app->users);
        }

        if (!readInt("\nNhap ma bai viet de tha react (0 = quay lai menu): ",
            &postId)) {
            return;
        }
        if (postId == 0) return;

        p = findPostById(app->posts, postId);
        if (p == NULL) {
            printf(">> Loi: Khong tim thay bai viet #%d!\n", postId);
            pauseScreen();
            continue;
        }

        if (reactPost(p)) {
            heapUpdateOnReact(app->trending, postId);
            printf(">> Da tha react cho bai #%d. Tong react: %d | Diem moi: %ld\n",
                postId, p->reacts, calcScore(p));
        }
        pauseScreen();
    }
}

/* Đăng bài mới - luồng tích hợp Trie + Linked List + Max-Heap */
static void doCreatePost(AppContext* app)
{
    char  buffer[MAX_CONTENT_LEN];
    char  original[MAX_CONTENT_LEN];
    Post* p;

    clearScreen();
    printHeader(app, "DANG BAI VIET MOI");

    if (!readLine("\nNhap noi dung bai viet: ", buffer, MAX_CONTENT_LEN)) return;

    if (buffer[0] == '\0') {
        printf(">> Loi: Noi dung khong duoc de trong!\n");
        pauseScreen();
        return;
    }

    strncpy(original, buffer, MAX_CONTENT_LEN - 1);
    original[MAX_CONTENT_LEN - 1] = '\0';

    censorText(app->badWords, buffer);
    if (strcmp(original, buffer) != 0) {
        printf("\n>> Canh bao: Bai viet chua tu ngu khong phu hop, da duoc loc.\n");
        printf("   Truoc : %s\n", original);
        printf("   Sau   : %s\n", buffer);
    }
    int isAnon = 1;
    readInt("\nBan muon dang an danh khong? (1 = Co, 0 = Khong): ", &isAnon);

    p = createPost(app->posts, app->currentUser, buffer, 0, isAnon);
    if (p == NULL) {
        pauseScreen();
        return;
    }

    heapPush(app->trending, p);

    printf("\n>> Dang bai thanh cong! Ma bai viet: #%d\n", p->id);
    pauseScreen();
}

static void doSwitchUser(AppContext* app)
{
    int id;

    clearScreen();
    printHeader(app, "DOI TAI KHOAN DANG NHAP");
    printAllUsers(app->users);

    if (!readInt("\nNhap ID nguoi dung muon dang nhap: ", &id)) {
        pauseScreen();
        return;
    }

    if (findUserById(app->users, id) == NULL) {
        printf(">> Loi: Khong ton tai nguoi dung co ID = %d!\n", id);
    }
    else {
        app->currentUser = id;
        printf(">> Da chuyen sang tai khoan: %s\n", getUserName(app->users, id));
    }
    pauseScreen();
}

static void doViewFriends(AppContext* app)
{
    int me = app->currentUser;
    int i, count = 0;

    printf("\n--- DANH SACH BAN BE CUA %s ---\n", getUserName(app->users, me));

    for (i = 0; i < app->users->count; i++) {
        if (getRelation(me, i) == REL_FRIEND) {
            printf("  [ID %d] %s\n", i, getUserName(app->users, i));
            count++;
        }
    }

    if (count == 0) {
        printf("  (Ban chua ket ban voi ai)\n");
    }
    else {
        printf("  Tong cong: %d nguoi ban.\n", count);
    }
}

static void doSendRequest(AppContext* app)
{
    int me = app->currentUser;
    int other;

    printf("\n--- GUI LOI MOI KET BAN ---\n");
    printAllUsers(app->users);

    if (!readInt("\nNhap ID nguoi muon ket ban: ", &other)) return;

    if (findUserById(app->users, other) == NULL) {
        printf(">> Loi: Khong ton tai nguoi dung co ID = %d!\n", other);
        return;
    }
    if (other == me) {
        printf(">> Loi: Khong the tu ket ban voi chinh minh!\n");
        return;
    }
    if (getRelation(me, other) == REL_FRIEND) {
        printf(">> Cac ban da la ban be tu truoc roi.\n");
        return;
    }
    if (getRelation(me, other) == REL_PENDING) {
        printf(">> Ban da gui loi moi cho nguoi nay, dang cho ho duyet.\n");
        return;
    }
    if (getRelation(other, me) == REL_PENDING) {
        setRelation(me, other, REL_FRIEND);
        printf(">> %s da moi ban truoc do. He thong tu dong ket ban 2 nguoi!\n",
            getUserName(app->users, other));
        return;
    }

    setRelation(me, other, REL_PENDING);
    printf(">> Da gui loi moi ket ban toi %s.\n", getUserName(app->users, other));
}

static void doAcceptRequest(AppContext* app)
{
    int me = app->currentUser;
    int i, found = 0, from, traLoi;

    printf("\n--- LOI MOI KET BAN DANG CHO DUYET ---\n");
    for (i = 0; i < app->users->count; i++) {
        if (getRelation(i, me) == REL_PENDING) {
            printf("  [ID %d] %s\n", i, getUserName(app->users, i));
            found++;
        }
    }

    if (found == 0) {
        printf("  (Khong co loi moi nao)\n");
        return;
    }

    if (!readInt("\nNhap ID nguoi muon xu ly (-1 = quay lai): ", &from)) return;
    if (from == -1) return;

    if (getRelation(from, me) != REL_PENDING) {
        printf(">> Loi: Nguoi nay khong gui loi moi ket ban cho ban!\n");
        return;
    }

    if (!readInt("  1 = Dong y, 0 = Tu choi: ", &traLoi)) return;

    if (traLoi == 1) {
        setRelation(me, from, REL_FRIEND);
        printf(">> Da ket ban voi %s.\n", getUserName(app->users, from));
    }
    else if (traLoi == 0) {
        setRelation(from, me, REL_NONE);
        printf(">> Da tu choi loi moi cua %s.\n", getUserName(app->users, from));
    }
    else {
        printf(">> Da huy thao tac.\n");
    }
}

/* Huỷ kết bạn - xoá cạnh khỏi đồ thị vô hướng */
static void doHuyKetBan(AppContext* app)
{
    int me = app->currentUser;
    int i, n = 0, chon;

    printf("\n--- HUY KET BAN ---\n");
    printf("\n  Danh sach ban be hien tai:\n");

    for (i = 0; i < app->users->count; i++) {
        if (getRelation(me, i) == REL_FRIEND) {
            printf("    [ID %d] %s\n", i, getUserName(app->users, i));
            n++;
        }
    }

    if (n == 0) {
        printf("    (Ban chua ket ban voi ai)\n");
        return;
    }

    if (!readInt("\nNhap ID nguoi muon huy ket ban (-1 = quay lai): ", &chon))
        return;
    if (chon == -1) return;

    if (findUserById(app->users, chon) == NULL) {
        printf(">> Loi: Khong ton tai nguoi dung co ID = %d!\n", chon);
        return;
    }
    if (getRelation(me, chon) != REL_FRIEND) {
        printf(">> Nguoi nay khong nam trong danh sach ban be cua ban!\n");
        return;
    }

    setRelation(me, chon, REL_NONE);
    printf(">> Da huy ket ban voi %s.\n", getUserName(app->users, chon));
}

static void friendMenu(AppContext* app)
{
    int choice;

    while (1) {
        clearScreen();
        printHeader(app, "MANG LUOI BAN BE (DO THI VO HUONG)");
        printf("  1. Xem danh sach ban be cua toi\n");
        printf("  2. Gui loi moi ket ban\n");
        printf("  3. Duyet loi moi dang cho\n");
        printf("  4. Goi y ket ban (tu %d ban chung tro len)\n",
            MIN_MUTUAL_FRIENDS);
        printf("  5. Huy ket ban\n");
        printf("  6. Xem ma tran ke\n");
        printf("  0. Quay lai menu chinh\n");
        printf("==========================================================\n");

        if (!readInt("Moi ban chon: ", &choice)) {
            if (feof(stdin)) return;
            pauseScreen();
            continue;
        }

        switch (choice) {
        case 1: doViewFriends(app);    break;
        case 2: doSendRequest(app);    break;
        case 3: doAcceptRequest(app);  break;
        case 4: recommendFriends(app->users, app->currentUser, MIN_MUTUAL_FRIENDS); break;
        case 5: doHuyKetBan(app);      break;
        case 6: printAdjMatrix();      break;
        case 0: return;
        default:
            printf(">> Loi: Khong co muc %d, vui long chon lai!\n", choice);
            break;
        }
        pauseScreen();
    }
}

int main(void)
{
    AppContext* app;
    int choice;

    printf("Dang khoi tao he thong...\n");

    app = initApp(BADWORDS_FILE);
    if (app == NULL) {
        printf("Khoi tao that bai. Chuong trinh ket thuc.\n");
        return 1;
    }

    initMockData(app);
    xaoTronBiDanh(app->users);
    pauseScreenMsg("Nhan ENTER de vao chuong trinh");

    /* --------- VÒNG LẶP GIAO DIỆN CHÍNH --------- */
    while (1) {
        heapRebuild(app->trending);

        clearScreen();
        printHeader(app, "CAMPUS CONFESSIONS & CONNECTIONS - NHOM 12");
        printf("  1. Bang tin moi nhat\n");
        printf("  2. Bai viet thinh hanh\n");
        printf("  3. Dang bai viet moi\n");
        printf("  4. Mang luoi ban be\n");
        printf("  5. Doi tai khoan dang nhap\n");
        printf("  0. Thoat chuong trinh\n");
        printf("==========================================================\n");

        if (!readInt("Moi ban chon chuc nang: ", &choice)) {
            if (feof(stdin)) break;
            pauseScreen();
            continue;
        }

        switch (choice) {
        case 1:
            viewAndReact(app, 0);
            break;

        case 2:
            viewAndReact(app, 1);
            break;

        case 3:
            doCreatePost(app);
            break;

        case 4:
            friendMenu(app);
            break;

        case 5:
            doSwitchUser(app);
            break;

        case 0:
            printf("\nDang thoat chuong trinh...\n");
            cleanupAll(app);
            return 0;

        default:
            printf(">> Loi: Khong co chuc nang %d, vui long chon lai!\n", choice);
            pauseScreen();
            break;
        }
    }

    cleanupAll(app);
    return 0;
}

