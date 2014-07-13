/*
 * sim_cache.c
 *
 *  Created on: 25/12/2013
 *      Author: Diego Nieto Muñoz
 */

#include "sim_cache.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <xmmintrin.h>

#ifdef DEBUG
#include <assert.h>
#endif

static inline uint32_t log2_int(const uint32_t x) {
    uint32_t y;
    asm ( "\tbsr %1, %0\n"
    : "=r"(y)
    : "r" (x)
    );
    return y;
}

// Struct for list
struct double_list {
    struct double_list *prev, *next;
    unsigned short pos;
};

// Struct type
struct Tlru_queue {
    unsigned int *v_pos, *v;
};

struct Tcache {
    unsigned int tag;
    char  valid, padding;
};

// Cache parameters
unsigned int cache_size;
unsigned int line_size;
unsigned int cache_assoc;
unsigned int n_sets;
unsigned int n_sets_1;
unsigned int n_lines;
unsigned char t_line; // 1 -> array, 1 -> list

// Pointers to front of the lists
struct Tlru_queue  *lru_queue;

// Address structure
unsigned int bits_index;
unsigned int bits_offset;

// Cache data
struct Tcache *cache;

// Pointers to front of the lists
struct double_list **lru_queue2;

// List of the pointers to each element
struct double_list **index_address;


// Cache stats
long long int access;
long long int misses;

long long int Access();
long long int Misses();

void list_init(const short n,
                struct Tlru_queue *queue) {
    int i;
    struct Tlru_queue *aux = queue;
    aux->v = (unsigned int*) _mm_malloc(sizeof(unsigned int)*n, 8);
    aux->v_pos = (unsigned int*) _mm_malloc(sizeof(unsigned int)*n, 8);
    for (i = 0; i < n; i++) {
        aux->v[i] = i;
        aux->v_pos[i] = i;
    }
}

/*
 * @brief Initialize a double linked list with n elements
 *
 * @param[in] n Number of elements in the list
 * @param[inout] front Pointer to pointer to the first element of the list
 * @param[inout] rear Pointer to pointer to the last element of the list
 * @param[inout] index_address Pointer to
 *
 */
void list_init_l(const short n,
                struct double_list **front,
                struct double_list **rear,
                struct double_list **index_address) {
    int i;
    struct double_list *ptr, *aux;
    ptr = (struct double_list*) _mm_malloc(sizeof(struct double_list), 8);
    *front = ptr;
    ptr->pos = 0;
    ptr->prev = NULL;
    *index_address = ptr;
    for (i = 1; i < n; i++) {
        ptr->next = (struct double_list*) _mm_malloc(sizeof(struct double_list), 8);
        aux = ptr;
        ptr = ptr->next;
        ptr->prev = aux;
        ptr->pos = i;
        index_address++;
        *index_address = ptr;
    }
    ptr->next = NULL;
    *rear = ptr;
}

/*
 * Free the elements of the list
 */
void list_free(struct Tlru_queue *queue, int n) {
    struct Tlru_queue *ptr, *aux;
    int i;
    ptr = queue;
    for(i=0; i<n; i++) {
        aux = ptr;
        ptr++;
        _mm_free(aux->v);
        _mm_free(aux->v_pos);
    }
}

void list_free_l(struct double_list *list) {
    struct double_list *ptr, *aux;
    ptr = list;
    while (ptr != NULL) {
        aux = ptr;
        ptr = ptr->next;
        _mm_free(aux);
    }
}

/*
 * Performs a LRU update in the list (unroll)
 */
/*
void lru_update(const int elem,
                const struct Tlru_queue *queue) {
    int i;
    const int n = queue->v[elem];
    char pair = (n) % 2;

    for(i=n-1; i>=pair; i-=2) {
        queue->v_pos[i+1] = queue->v_pos[i];
        queue->v_pos[i] = queue->v_pos[i-1];
        queue->v[queue->v_pos[i+1]]++;
        queue->v[queue->v_pos[i]]++;
    }
    if(pair) {
        queue->v_pos[1] = queue->v_pos[0];
        queue->v[queue->v_pos[1]]++;
    }

    queue->v[elem] = 0;
    queue->v_pos[0] = elem;
}
*/

/*
 * Performs a LRU update in the list
 */
void lru_update(const int elem,
                const struct Tlru_queue *queue) {
    int i;
    const int n = queue->v[elem];

    for(i=n-1; i>=0; i--) {
        queue->v_pos[i+1] = queue->v_pos[i];
        queue->v[queue->v_pos[i+1]]++;
    }

    queue->v[elem] = 0;
    queue->v_pos[0] = elem;
}

void lru_update_l(struct double_list **elem,
                struct double_list **front,
                struct double_list **rear) {
    struct double_list *ptr = *elem;
    if (ptr->prev == NULL)
        return;
    ptr->prev->next = ptr->next;
    if (ptr->next != NULL)
        ptr->next->prev = ptr->prev;
    else
        *rear = ptr->prev;
    ptr->next = *front;
    ptr->next->prev = ptr;
    ptr->prev = NULL;
    *front = ptr;
}



/*
 * @brief Initializes the simulator
 ∗
 ∗ @param[in] cs Cache size in bytes
 ∗ @param[in] ls Line size in bytes. It must be a power of 2
 ∗ @param[in] assoc Associativity of the cache.
 ∗
 ∗ @pre The number of sets in the cache must be a power of 2,
 ∗ that is, (cs/(ls∗assoc)) must be a power of 2.
 ∗ The line size must be also a power of 2
 */
void config_sim(unsigned int cs, unsigned int ls, unsigned int assoc) {
    int i;

#ifdef DEBUG
    tests();
#endif

    cache_size = cs;
    line_size = ls;
    cache_assoc = assoc;
    n_sets = cache_size / (line_size * assoc);
    n_lines = cache_size / line_size;
    n_sets_1 = n_sets-1;

    // Stats init
    misses = 0;
    access = 0;

    // Bits calc
    bits_offset = log2_int(line_size);
    bits_index = log2_int(n_sets);

    cache = (struct Tcache *) _mm_malloc(
             sizeof(struct Tcache) * n_lines, 8);
    for (i = 0; i < n_lines; i++) {
        cache[i].tag = 0;
        cache[i].valid = 0;
    }

    if(assoc < 16)
        t_line = 0;
    else
        t_line = 1;

    if(!t_line) {
        lru_queue = (struct Tlru_queue*) _mm_malloc(
        sizeof(struct Tlru_queue) * n_sets, 8);
        for (i = 0; i < n_sets; i++) {
            list_init(cache_assoc, lru_queue+i);
        }

    } else {
        lru_queue2 = (struct double_list**) _mm_malloc(
        sizeof(struct double_list*) * n_sets * 2, 8);

        index_address = (struct double_list**) _mm_malloc(sizeof(struct double_list*)*n_lines, 8);

        for (i = 0; i < n_sets; i++) {
            list_init_l(cache_assoc, lru_queue2+i*2, lru_queue2+i*2+1, index_address+i*cache_assoc);
        }
    }
}

/**
 * @brief Simulates an access to the cache
 ∗
 ∗ @param[in] address Address of the memory position accessed
 */
void sim_access(unsigned int address) {
    int i;
    struct double_list *aux;
    const unsigned index = address >> bits_offset;
    unsigned int set = index & n_sets_1;
    unsigned int label = address >> (bits_index + bits_offset);
    unsigned int base = set * cache_assoc;
    // Check if the label belongs to any line
    for (i = 0; i < cache_assoc; i++) {
        if (label == cache[base + i].tag && cache[base + i].valid) {
            break;
        }
    }
    if (i == cache_assoc) {
        // Miss
        misses++;
        if(!t_line) {
            lru_update((lru_queue+set)->v_pos[cache_assoc-1], lru_queue+set);
            cache[(lru_queue+set)->v_pos[0]+set*cache_assoc].tag = label;
            cache[(lru_queue+set)->v_pos[0]+set*cache_assoc].valid = 1;
        } else {
            lru_update_l(lru_queue2+2*set+1, lru_queue2+2*set, lru_queue2+2*set+1);
            aux = *(lru_queue2+2*set);
            cache[aux->pos+set*cache_assoc].tag = label;
            cache[aux->pos+set*cache_assoc].valid = 1;
        }
    } else {
        // Hit
        if(!t_line)
            lru_update(i, lru_queue+set);
        else
            lru_update_l(index_address+cache_assoc*set+i, lru_queue2+2*set, lru_queue2+2*set+1);
    }
    access++;
}

/**
 *  @brief Dumps simulation results
 ∗
 ∗ Prints the number of accesses and the number of misses
 */
void end_sim() {
    _mm_free(cache);
}

#ifdef DEBUG
void tests() {
    const int n = 4;
    int i;
    struct Tlru_queue *queue, *aux;
    queue = (struct Tlru_queue*)malloc(sizeof(struct Tlru_queue));

    // Initialization
    list_init(n, queue);

    aux = queue;

    // Check initialization
    for(i=0; i<n; i++) {
        assert(aux->v[i] == i);
        assert(aux->v_pos[i] == i);
    }

    // Update 2 --> 2,0,1,3
    lru_update(2, queue);

    // Check after update
    assert(aux->v[2] == 0);
    assert(aux->v_pos[0] == 2);

    assert(aux->v[0] == 1);
    assert(aux->v_pos[1] == 0);

    assert(aux->v[1] == 2);
    assert(aux->v_pos[2] == 1);

    assert(aux->v[3] == 3);
    assert(aux->v_pos[3] == 3);

    // Update 1 --> 1,2,0,3
    lru_update(1, queue);

    // Check after update
    assert(aux->v[1] == 0);
    assert(aux->v_pos[0] == 1);

    assert(aux->v[2] == 1);
    assert(aux->v_pos[1] == 2);

    assert(aux->v[0] == 2);
    assert(aux->v_pos[2] == 0);

    assert(aux->v[3] == 3);
    assert(aux->v_pos[3] == 3);

    // Update 3 --> 3,1,2,0
    lru_update(3, queue);

    // Check after update
    assert(aux->v[3] == 0);
    assert(aux->v_pos[0] == 3);

    assert(aux->v[1] == 1);
    assert(aux->v_pos[1] == 1);

    assert(aux->v[2] == 2);
    assert(aux->v_pos[2] == 2);

    assert(aux->v[0] == 3);
    assert(aux->v_pos[3] == 0);

    // Update 3 --> 3,1,2,0
    lru_update(3, queue);

    // Check after update
    assert(aux->v[3] == 0);
    assert(aux->v_pos[0] == 3);

    assert(aux->v[1] == 1);
    assert(aux->v_pos[1] == 1);

    assert(aux->v[2] == 2);
    assert(aux->v_pos[2] == 2);

    assert(aux->v[0] == 3);
    assert(aux->v_pos[3] == 0);

    list_free(queue, 1);
    free(queue);
    printf("Test finished\n");
}
#endif

long long int Access() {
    return access;
}

long long int Misses() {
    return misses;
}
