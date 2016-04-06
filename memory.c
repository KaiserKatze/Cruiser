#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "java.h"
#include "log.h"

extern void *
allocMemory(size_t count, size_t size)
{
    void *ptr;
    size_t cap;

    cap = count * size;
    ptr = malloc(cap);
    if (!ptr)
    {
        logError("Fail to allocate memory!\r\n");
        return (void *) 0;
    }
    bzero(ptr, cap);

    return ptr;
}

extern void
reallocMemory(void **mem, size_t newsize)
{
    void *ptr;

    ptr = realloc(*mem, newsize);
    if (!ptr)
    {
        free(*mem);
        *mem = (void *) 0;
        return;
    }
    *mem = ptr;
}

extern void
freeMemory(void *mem)
{
    free(mem);
}

extern void *
trimMemory(void *mem)
{
    int len;
    void *res;
    
    len = strlen((char *) mem);
    res = allocMemory(len + 1, sizeof (u1));
    if (!res)
        return (void *) 0;
    memcpy(res, mem, len);
    freeMemory(mem);
    
    return res;
}

extern struct Deque *
deque_createDeque()
{
    struct Deque *deque;

    deque = (struct Deque *) allocMemory(1, sizeof (struct Deque));
    if (!deque) return (struct Deque *) 0;
    deque->first = deque->last = (struct DequeEntry *) 0;

    return deque;
}

extern void
deque_releaseEntry(struct DequeEntry *entry)
{
    freeMemory(entry);
}

extern struct DequeEntry *
deque_removeFirst(struct Deque *deque)
{
    struct DequeEntry *first;

    if (!deque)
    {
        logError("Parameter 'deque' is NULL in method %s!\r\n", __func__);
        return (struct DequeEntry *) 0;
    }
    if (first = deque->first)
        if (deque->first = first->next)
            deque->first->prev = first->prev = first->next = (struct DequeEntry *) 0;
        else
            deque->last = (struct DequeEntry *) 0;

    return first;
}

extern struct DequeEntry *
deque_removeLast(struct Deque *deque)
{
    struct DequeEntry *last;

    if (!deque)
    {
        logError("Parameter 'deque' is NULL in method %s!\r\n", __func__);
        return (struct DequeEntry *) 0;
    }
    if (last = deque->last)
        if (deque->last = last->prev)
            deque->last->next = last->prev = last->next = (struct DequeEntry *) 0;
        else
            deque->first = (struct DequeEntry *) 0;

    return last;
}

extern struct DequeEntry *
deque_pop(struct Deque *deque)
{
    return deque_removeFirst(deque);
}

extern int
deque_addFirst(struct Deque *deque, int size, void *value)
{
    struct DequeEntry *entry;

    if (!deque)
    {
        logError("Parameter 'deque' is NULL in method %s!\r\n", __func__);
        return -1;
    }
    entry = (struct DequeEntry *) allocMemory(1, sizeof (struct DequeEntry));
    if (!entry) return -1;
    entry->size = size;
    entry->value = value;
    entry->prev = (struct DequeEntry *) 0;
    entry->next = deque->first;
    if (deque->first)
        deque->first->prev = entry;
    else // deque was empty
        deque->last = entry;
    deque->first = entry;

    return 0;
}

extern int
deque_addLast(struct Deque *deque, int size, void *value)
{
    struct DequeEntry *entry;

    if (!deque)
    {
        logError("Parameter 'deque' is NULL in method %s!\r\n", __func__);
        return -1;
    }
    entry = (struct DequeEntry *) allocMemory(1, sizeof (struct DequeEntry));
    if (!entry) return -1;
    entry->size = size;
    entry->value = value;
    entry->prev = deque->last;
    entry->next = (struct DequeEntry *) 0;
    if (deque->last)
        deque->last->next = entry;
    else // deque is empty
        deque->first = entry;
    deque->last = entry;

    return 0;
}

extern int
deque_push(struct Deque *deque, int size, void *value)
{
    return deque_addFirst(deque, size, value);
}

// Horner's hash method
static int hash_str(int m, int len, u1 *str)
{
    int h, i;
    
    h = 0;
    for (i = 0; i < len; i++)
        h = str[i] + (31 * h);
    h &= 0x7fffffff;
    h %= m;
    
    return h;
}
