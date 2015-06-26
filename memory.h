#ifndef STACK_H
#define STACK_H

extern void * allocMemory(size_t, size_t);
extern void reallocMemory(void **, size_t);

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

extern struct Deque *createDeque();
extern void releaseDequeEntry(struct DequeEntry *);

extern int addFirst(struct Deque *, int, void *);
extern int addLast(struct Deque *, int, void *);
extern int push(struct Deque *, int, void *);

extern struct DequeEntry *removeFirst(struct Deque *);
extern struct DequeEntry *removeLast(struct Deque *);
extern struct DequeEntry *pop(struct Deque *);

#endif /* STACK_H */
