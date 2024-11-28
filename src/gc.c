#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>

#include "runtime.h"
#include "gc.h"
#include "queue.h"

#define MAX_GC_ROOTS 2048
#define START_HEAP_SIZE 1024
// enables a lot of debug output during gc work
// #define STELLA_DEBUG


typedef enum COLOR {
    WHITE,
    GREY,
    BLACK,
} COLOR;

typedef enum GC_PHASE {
    MARK,
    SWEEP,
} GC_PHASE;

typedef struct gc_object_t {
    COLOR color;
    struct gc_object_t *moved_to;
    stella_object obj;
} gc_object_t;

typedef struct gc_stats_t {
    unsigned long total_allocated_bytes;
    unsigned long total_allocated_objects;

    unsigned long max_allocated_bytes;
    unsigned long max_allocated_objects;

    unsigned long current_allocated_bytes;
    unsigned long current_allocated_objects;

    unsigned long total_reads;
    unsigned long total_writes;

    unsigned long gc_roots_max_size;

    unsigned long mark_steps;
    unsigned long sweep_steps;
    unsigned long sweep_phase_count;
    unsigned long mark_phase_count;
    unsigned long marked_objects;
} gc_stats_t;

typedef struct gc_sweep_helper_t {
    void *next_heap;
    size_t next_heap_size;
    int sweep_allocated_bytes;
    int sweep_allocated_objects;
    void *next;
} gc_sweep_helper_t;


typedef struct gc_t {
    // roots info
    stella_object **roots[MAX_GC_ROOTS];
    int roots_cont;

    // in what phase GC now
    GC_PHASE phase;

    // queue for mark phase
    queue_t *grey_queue;

    // queue for sweep phase
    queue_t *black_queue;

    // garbage collector statistic
    gc_stats_t stats;

    // where to store current objects and where to move them in sweep phase
    void *current_heap;
    void *next_place_in_heap;
    size_t current_heap_size;

    gc_sweep_helper_t sweep_helper;
} gc_t;

void *alloc_heap(size_t size);

void gc_init();

bool is_enough_place_in_current_heap(size_t size_in_bytes);

void *try_alloc(size_t size_in_bytes);

bool mark_step();

void gc_step();

void gc_full();

void mark_roots();

void make_stella_object_grey_if_needed(stella_object *stella_obj);

static bool is_in_current_heap(void *ptr);

static bool is_in_next_heap(void *ptr);

void sweep_cleanup();

void sweep_chase(gc_object_t *old_gc_obj);

void *sweep_forward(stella_object *stella_obj);

void gc_update_stats_after_object_alloc(size_t size_in_bytes);

bool is_enough_place_in_next_heap(size_t size_in_bytes);

void *try_alloc_in_next(size_t size_in_bytes);

void has_ill_fields_rec(gc_object_t *object);

void gc_init_sweep_helper(size_t size_in_bytes);

gc_object_t *stella_object_to_gc_object(void *ptr);

size_t get_gc_object_size(gc_object_t *obj);

gc_t *gc = NULL; // Garbage collector instance

void gc_init_sweep_helper(size_t size_in_bytes) {
    void *new_heap = alloc_heap(size_in_bytes);
    gc->sweep_helper.next_heap = new_heap;
    gc->sweep_helper.next_heap_size = size_in_bytes;
    gc->sweep_helper.next = new_heap;
    gc->sweep_helper.sweep_allocated_bytes = 0;
    gc->sweep_helper.sweep_allocated_objects = 0;
}

void gc_init_stats(gc_stats_t *stats) {
    stats->current_allocated_bytes = 0;
    stats->current_allocated_objects = 0;
    stats->max_allocated_bytes = 0;
    stats->max_allocated_objects = 0;
    stats->total_allocated_bytes = 0;
    stats->total_allocated_objects = 0;
    stats->total_reads = 0;
    stats->total_writes = 0;
    stats->gc_roots_max_size = 0;
}

void gc_init() {
    if (gc != NULL) {
        return;
    }
    gc = malloc(sizeof(gc_t));

    gc_init_stats(&gc->stats);

    // gc->roots already allocated
    gc->roots_cont = 0;

    gc->phase = MARK;

    gc->grey_queue = create_queue();
    gc->black_queue = create_queue();

    gc->current_heap = alloc_heap(START_HEAP_SIZE);
    gc->current_heap_size = START_HEAP_SIZE;
    gc->next_place_in_heap = gc->current_heap;
}

bool is_enough_place_in_current_heap(size_t size_in_bytes) {
    return gc->next_place_in_heap + size_in_bytes < gc->current_heap + gc->current_heap_size;
}

void *try_alloc(size_t size_in_bytes) {
    if (is_enough_place_in_current_heap(size_in_bytes)) {
        void *res = gc->next_place_in_heap;
        gc->next_place_in_heap += size_in_bytes;
        return res;
    }
    return NULL;
}

bool is_enough_place_in_next_heap(size_t size_in_bytes) {
    return gc->sweep_helper.next + size_in_bytes < gc->sweep_helper.next_heap + gc->sweep_helper.next_heap_size;
}

void *try_alloc_in_next(size_t size_in_bytes) {
    if (is_enough_place_in_next_heap(size_in_bytes)) {
        void *res = gc->sweep_helper.next;
        gc->sweep_helper.next += size_in_bytes;
        return res;
    }
    return NULL;
}

void gc_update_stats_after_object_alloc(size_t size_in_bytes) {
    gc->stats.total_allocated_bytes += size_in_bytes;
    gc->stats.total_allocated_objects += 1;
    gc->stats.current_allocated_bytes += size_in_bytes;
    gc->stats.current_allocated_objects += 1;
    if (gc->stats.max_allocated_bytes < gc->stats.total_allocated_bytes) {
        gc->stats.max_allocated_bytes = gc->stats.total_allocated_bytes;
    }
    if (gc->stats.max_allocated_objects < gc->stats.total_allocated_objects) {
        gc->stats.max_allocated_objects = gc->stats.total_allocated_objects;
    }
}

void *gc_alloc(size_t size_in_bytes_for_stella) {
    // printf("stella object size = %d\n, p", sizeof(stella_object), (void *)NULL + (size_t)4);
    // printf("gc object size = %d\n", sizeof(gc_object_t));
    size_t bytes_to_alloc = sizeof(gc_object_t) - sizeof(stella_object) + size_in_bytes_for_stella;
    gc_object_t *ptr = try_alloc(bytes_to_alloc);
    while (ptr == NULL) {
        gc_full();
        fflush(stdout);
        ptr = try_alloc(bytes_to_alloc);
    }
    gc_update_stats_after_object_alloc(bytes_to_alloc);
#ifdef STELLA_DEBUG
    printf("For %p allocated %lu \n", ptr, bytes_to_alloc);
#endif
    ptr->color = WHITE;
    ptr->moved_to = NULL;
    // STELLA_OBJECT_INIT_FIELDS_COUNT((&ptr->obj), 0);
    make_stella_object_grey_if_needed(&ptr->obj);
    gc_step();
    return &ptr->obj;
}

void print_gc_roots() {
    printf("ROOTS: ");
    for (int i = 0; i < gc->roots_cont; i++) {
        printf("%p ", gc->roots[i]);
    }
    printf("\n");
}

void print_gc_alloc_stats() {
    printf("Total memory allocation:            %ld'd bytes (%lu'd objects)\n", gc->stats.total_allocated_bytes, gc->stats.total_allocated_objects);
    printf("Maximum residency:                  %lu'd bytes (%lu'd objects)\n", gc->stats.max_allocated_bytes, gc->stats.max_allocated_objects);
    printf("Total memory use:                   %lu'd reads and %lu'd writes\n", gc->stats.total_reads, gc->stats.total_writes);
    printf("Allocations after last sweep:       %lu'd bytes and %lu'd objects\n", gc->stats.current_allocated_bytes, gc->stats.current_allocated_objects);
    printf("Max GC roots stack size:            %lu roots\n", gc->stats.gc_roots_max_size);
    printf("Marked objects:                     %lu\n", gc->stats.marked_objects);
    printf("Mark phases done:                   %lu\n", gc->stats.mark_phase_count);
    printf("Mark steps done:                    %lu\n", gc->stats.mark_steps);
    printf("Sweep phases done:                  %lu\n", gc->stats.sweep_phase_count);
    printf("Sweep steps done:                   %lu\n", gc->stats.sweep_steps);
}

void print_gc_state() {
    // TODO: not implemented
}

void gc_read_barrier(void *object, int field_index) {
    gc->stats.total_reads += 1;
}

void gc_write_barrier(void *object, int field_index, void *contents) {
    // gc_object_t *obj = stella_object_to_gc_object((stella_object*) contents);
    make_stella_object_grey_if_needed((stella_object *) contents);
    gc->stats.total_writes += 1;
}

void gc_push_root(void **ptr) {
    gc_init();
    gc->roots[gc->roots_cont++] = (stella_object *) ptr;
#ifdef STELLA_DEBUG
    printf("Root (%d): %p\n", gc->roots_cont - 1, *ptr);
#endif
    if (gc->roots_cont > gc->stats.gc_roots_max_size) {
        gc->stats.gc_roots_max_size = gc->roots_cont;
    }
}

void gc_pop_root(void **ptr) {
    gc->roots_cont--;
}

typedef enum SWEEP_STRATEGY {
    MAKE_BIGGER,
    MAKE_SMALLER,
    DO_NOTHING
} SWEEP_STRATEGY;

SWEEP_STRATEGY sweep_strategy() {
    float allocated = gc->stats.current_allocated_bytes;
    float heap_size = gc->current_heap_size;
#ifdef STELLA_DEBUG
    printf("allocated / heap_size = %f\n", allocated / heap_size);
#endif
    // heap almost full
    if (allocated / heap_size > 0.7) {
        return MAKE_BIGGER;
        // heap almost empty
    }
    if (allocated / heap_size < 0.2) {
        return MAKE_SMALLER;
        // there are enough place in heap
    }
    return DO_NOTHING;
}

static bool is_in_current_heap(void *ptr) {
    return ptr >= gc->current_heap && ptr < gc->current_heap + gc->current_heap_size;
}

static bool is_in_next_heap(void *ptr) {
    return ptr >= gc->sweep_helper.next_heap && ptr < gc->sweep_helper.next_heap + gc->sweep_helper.next_heap_size;
}

void has_ill_fields_rec(gc_object_t *object) {
    // printf("check ill: %p\n", object);
    if (is_in_current_heap(object)) {
        printf("Ill object (cur heap): %p", object);
        printf("\n object: ");
        print_stella_object(&object->obj);
        printf("\n");
    } else if (!is_in_next_heap(object)) {
        // printf("Ill object (not next not cur): %p", object);
        // printf("\n object: ");
        // print_stella_object(&object->obj);
        // printf("\n");
        // fflush(stdout);
    } else {
        int fields_count = STELLA_OBJECT_HEADER_FIELD_COUNT(object->obj.object_header);
        for (int i = 0; i < fields_count; i++) {
            has_ill_fields_rec(stella_object_to_gc_object(object->obj.object_fields[i]));
        }
    }
}


void *sweep_forward(stella_object *stella_obj) {
    if (!is_in_current_heap(stella_obj)) {
        return stella_obj;
    }
    gc_object_t *gc_obj = stella_object_to_gc_object(stella_obj);
    if (is_in_next_heap(gc_obj->moved_to)) {
        return gc_obj->moved_to;
    }
    sweep_chase(gc_obj);
    return gc_obj->moved_to;
}

void sweep_chase(gc_object_t *old_gc_obj) {
    do {
        gc_object_t *q = try_alloc_in_next(get_gc_object_size(old_gc_obj));
        if (q == NULL) {
            printf("Failed to allocate gc_object in sweep phase\n");
            exit(1);
        }
        const int field_count = STELLA_OBJECT_HEADER_FIELD_COUNT(old_gc_obj->obj.object_header);
        void *r = NULL;

        q->moved_to = NULL;
        q->color = WHITE;
        q->obj.object_header = old_gc_obj->obj.object_header;
        for (int i = 0; i < field_count; i++) {
            q->obj.object_fields[i] = old_gc_obj->obj.object_fields[i];

            if (is_in_current_heap(q->obj.object_fields[i])) {
                gc_object_t *potentially_forwarded = stella_object_to_gc_object(q->obj.object_fields[i]);

                if (!is_in_next_heap(potentially_forwarded->moved_to)) {
                    r = potentially_forwarded;
                }
            }
        }

        old_gc_obj->moved_to = q;
        old_gc_obj = r;
        // to fix fields addresses after sweep
        push(gc->black_queue, q);
    } while (old_gc_obj != NULL);
}

bool sweep_step() {
    if (is_empty(gc->black_queue)) {
        return true;
    }
    gc_object_t *black_obj = get(gc->black_queue);
    gc->stats.sweep_steps += 1;
    if (is_in_current_heap(black_obj)) {
#ifdef STELLA_DEBUG
        printf("Sweeping object: ");
        print_stella_object(&black_obj->obj);
        printf("\n");
#endif
        sweep_forward(&black_obj->obj);
        push(gc->black_queue, black_obj->moved_to);
#ifdef STELLA_DEBUG
        printf("Swept object: ");
        print_stella_object(&black_obj->moved_to->obj);
        printf(", from %p to %p \n", black_obj, black_obj->moved_to);
#endif
    } else {
        // to change fields addresses
        // black_obj already moved
#ifdef STELLA_DEBUG
        printf("Swept object fields:\n ptr: %p\n object: ", black_obj);
        print_stella_object(&black_obj->obj);
#endif
        int field_count = STELLA_OBJECT_HEADER_FIELD_COUNT(black_obj->obj.object_header);
#ifdef STELLA_DEBUG
        printf("\n fields count: %d\n", field_count);
#endif
        for (int i = 0; i < field_count; i++) {
            stella_object *cur_field = black_obj->obj.object_fields[i];
#ifdef STELLA_DEBUG
            printf("  field %d: ", i);
            print_stella_object(cur_field);
            printf("\n");
#endif
            if (is_in_current_heap(cur_field)) {
                gc_object_t *moved_field = stella_object_to_gc_object(cur_field)->moved_to;
                black_obj->obj.object_fields[i] = &moved_field->obj;
            }
        }
    }
    return false;
}

void *alloc_heap(size_t size) {
#ifdef STELLA_DEBUG
    printf("Size to alloc %u, ", size);
#endif
    void *heap = malloc(size);
    if (heap == NULL) {
        printf("Memory allocation for new heap failed!\n");
        exit(1);
    }
#ifdef STELLA_DEBUG
    printf("heap from %p to %p \n", heap, heap + size);
#endif
    return heap;
}

SWEEP_STRATEGY sweep_prepare(bool ignore_strategy) {
    SWEEP_STRATEGY strategy = sweep_strategy();
    if (ignore_strategy) {
        strategy = MAKE_BIGGER;
    }
    switch (strategy) {
        case MAKE_BIGGER:
            gc_init_sweep_helper(gc->current_heap_size * 2);
            break;
        case MAKE_SMALLER:
            gc_init_sweep_helper(gc->current_heap_size / 2);
            break;
        case DO_NOTHING:
            break;
    }
#ifdef STELLA_DEBUG
    printf("Sweeping strategy: %d\n", strategy);
#endif
    return strategy;
}

void sweep_cleanup() {
#ifdef STELLA_DEBUG
    printf("Sweep cleanup\n");
#endif
    // moving roots links
    for (int i = 0; i < gc->roots_cont; i++) {
        stella_object *current_root = *(gc->roots[i]);
        if (is_in_current_heap(current_root)) {
#ifdef STELLA_DEBUG
            printf("Sweeping root (%d): ", i);
            if (i == 12) {
                printf("Anime!");
            }
            print_stella_object(current_root);
            printf("\n from %p to %p\n", stella_object_to_gc_object(current_root), stella_object_to_gc_object(current_root)->moved_to);
            fflush(stdout);
            has_ill_fields_rec(stella_object_to_gc_object(current_root)->moved_to);
#endif
            *(gc->roots[i]) = &stella_object_to_gc_object(current_root)->moved_to->obj;
        }
    }
    free(gc->current_heap);
    gc->current_heap = gc->sweep_helper.next_heap;
    gc->current_heap_size = gc->sweep_helper.next_heap_size;
    gc->stats.current_allocated_bytes = 0;
    gc->stats.current_allocated_objects = 0;
    gc->next_place_in_heap = gc->sweep_helper.next;
    gc->phase = MARK;
    gc->stats.mark_phase_count += 1;
}

gc_object_t *stella_object_to_gc_object(void *ptr) {
    return ptr - (sizeof(gc_object_t) - sizeof(stella_object));
}

size_t get_gc_object_size(gc_object_t *obj) {
    const int fields_count = STELLA_OBJECT_HEADER_FIELD_COUNT(obj->obj.object_header);
    return sizeof(gc_object_t) + fields_count * sizeof(void *);
}

void make_stella_object_grey_if_needed(stella_object *stella_obj) {
    // something predefined
#ifdef STELLA_DEBUG
    printf("mark stella object: ");
    print_stella_object(stella_obj);
    printf(", ");
#endif
    if (!is_in_current_heap(stella_obj)) {
#ifdef STELLA_DEBUG
        printf(" not in current heap\n");
#endif
        return;
    }
    gc_object_t *obj = stella_object_to_gc_object(stella_obj);
    // already traversed or marked
    if (obj->color != WHITE) {
#ifdef STELLA_DEBUG
        printf(" already marked\n");
#endif
        return;
    }
    gc->stats.marked_objects += 1;
    obj->color = GREY;
    push(gc->grey_queue, obj);
#ifdef STELLA_DEBUG
    printf(" marked now\n");
#endif
}

void mark_roots() {
    for (int i = 0; i < gc->roots_cont; i++) {
        stella_object *current_root = *(gc->roots[i]);
        // if root is allocated we can just mark it as grey and traverse it's children later
        if (is_in_current_heap(current_root)) {
            fflush(stdout);
            make_stella_object_grey_if_needed(current_root);
        }
    }
}

// returns true if everything marked, false otherwise
bool mark_step() {
    gc->stats.mark_steps += 1;
    if (is_empty(gc->grey_queue)) {
        mark_roots();
    }
    if (!is_empty(gc->grey_queue)) {
        gc_object_t *obj = get(gc->grey_queue);
        const int fields_count = STELLA_OBJECT_HEADER_FIELD_COUNT(obj->obj.object_header);
        for (int i = 0; i < fields_count; i++) {
            make_stella_object_grey_if_needed(obj->obj.object_fields[i]);
        }
        obj->color = BLACK;
        push(gc->black_queue, obj);
        // there are something to do
        return false;
    } else {
        // everything was marked
        return true;
    }
}

void gc_full() {
    bool done = mark_step();
    while (!done) {
        done = mark_step();
    }
    gc->phase = SWEEP;
    gc->stats.sweep_phase_count += 1;
    sweep_prepare(true); // allocate new space
    done = sweep_step();
    while (!done) {
        done = sweep_step();
    }
    sweep_cleanup();
}

void gc_step() {
    if (gc->phase == MARK) {
        const bool is_done = mark_step();
        if (is_done) {
            const SWEEP_STRATEGY strategy = sweep_prepare(false);
            if (strategy != DO_NOTHING) {
                gc->phase = SWEEP;
                gc->stats.sweep_phase_count += 1;
            }
        }
    } else {
        const bool is_done = sweep_step();
        if (is_done) {
            sweep_cleanup();
        }
    }
    fflush(stdout);
}
