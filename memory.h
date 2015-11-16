#ifndef MEMORY_H
#define MEMORY_H

extern void * allocMemory(size_t, size_t);
extern void reallocMemory(void **, size_t);
extern void freeMemory(void *);
extern void *trimMemory(void *);

struct Deque;
struct DequeEntry;

struct Deque
{
    struct DequeEntry *first;
    struct DequeEntry *last;
};

struct DequeEntry
{
    struct DequeEntry *prev;
    struct DequeEntry *next;
    int size;
    void *value;
};

extern struct Deque *deque_createDeque();
extern void deque_releaseEntry(struct DequeEntry *);

extern int deque_addFirst(struct Deque *, int, void *);
extern int deque_addLast(struct Deque *, int, void *);
extern int deque_push(struct Deque *, int, void *);

extern struct DequeEntry *deque_removeFirst(struct Deque *);
extern struct DequeEntry *deque_removeLast(struct Deque *);
extern struct DequeEntry *deque_pop(struct Deque *);

#endif /* MEMORY_H */
