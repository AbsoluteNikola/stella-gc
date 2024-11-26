#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>

#include "runtime.h"
#include "gc.h"
#include "queue.h"

/** Total allocated number of bytes (over the entire duration of the program). */
int total_allocated_bytes = 0;

/** Total allocated number of objects (over the entire duration of the program). */
int total_allocated_objects = 0;

int max_allocated_bytes = 0;
int max_allocated_objects = 0;

int total_reads = 0;
int total_writes = 0;

#define MAX_GC_ROOTS 1024
#define START_HEAP_SIZE 128

int gc_roots_max_size = 0;

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
} gc_stats_t;

typedef struct gc_sweep_helper_t {
    void *next_heap;
    int next_heap_size;
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
    int current_heap_size;

    gc_sweep_helper_t sweep_helper;
} gc_t;

void *alloc_heap(size_t size);

void gc_init();

bool is_enough_place_in_current_heap(size_t size_in_bytes);

void *try_alloc(size_t size_in_bytes);

bool mark_step();

void gc_step();

void gc_full();

void mark_roots_and_their_children();

void make_stella_object_grey_if_needed(stella_object *stella_obj);

static bool is_in_current_heap(void *ptr);

static bool is_in_next_heap(void *ptr);

void sweep_cleanup();

void sweep_chase(gc_object_t *old_gc_obj);

void *sweep_forward(stella_object *stella_obj);

void gc_update_stats_after_object_alloc(size_t size_in_bytes);

bool is_enough_place_in_next_heap(size_t size_in_bytes);

void *try_alloc_in_next(size_t size_in_bytes);

gc_object_t *stella_object_to_gc_object(void *ptr);

size_t get_gc_object_size(gc_object_t *obj);

gc_t *gc = NULL; // Garbage collector instance

void gc_init_stats(gc_stats_t *stats) {
    stats->current_allocated_bytes = 0;
    stats->current_allocated_objects = 0;
    stats->max_allocated_bytes = 0;
    stats->max_allocated_objects = 0;
    stats->total_allocated_bytes = 0;
    stats->total_allocated_objects = 0;
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
    printf("For %p allocated %lu \n", ptr, bytes_to_alloc);
    ptr->color = WHITE;
    ptr->moved_to = NULL;
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
    printf("Total memory allocation: %'d bytes (%'d objects)\n", total_allocated_bytes, total_allocated_objects);
    printf("Maximum residency:       %'d bytes (%'d objects)\n", max_allocated_bytes, max_allocated_objects);
    printf("Total memory use:        %'d reads and %'d writes\n", total_reads, total_writes);
    printf("Max GC roots stack size: %'d roots\n", gc_roots_max_size);
}

void print_gc_state() {
    // TODO: not implemented
}

void gc_read_barrier(void *object, int field_index) {
    total_reads += 1;
}

void gc_write_barrier(void *object, int field_index, void *contents) {
    total_writes += 1;
}

void gc_push_root(void **ptr) {
    gc_init();
    gc->roots[gc->roots_cont++] = (stella_object *) ptr;
    if (gc->roots_cont > gc_roots_max_size) {
        gc_roots_max_size = gc->roots_cont;
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
    printf("allocated / heap_size = %f\n", allocated / heap_size);
    // heap almost full
    if (allocated / heap_size > 0.7) {
        return MAKE_BIGGER;
        // heap almost empty
    } else if (allocated / heap_size < 0.2) {
        return MAKE_SMALLER;
        // there are enough place in heap
    } else {
        return DO_NOTHING;
    }
}

static bool is_in_current_heap(void *ptr) {
    return ptr >= gc->current_heap && ptr <= gc->current_heap + gc->current_heap_size;
}

static bool is_in_next_heap(void *ptr) {
    return ptr >= gc->sweep_helper.next_heap && ptr <= gc->sweep_helper.next_heap + gc->sweep_helper.next_heap_size;
}

void *sweep_forward(stella_object *stella_obj) {
    if (!is_in_current_heap(stella_obj)) {
        return stella_obj;
    }
    gc_object_t *gc_obj = stella_object_to_gc_object(stella_obj);
    if (is_in_next_heap(gc_obj->moved_to)) {
        return gc_obj->moved_to;
    } else {
        sweep_chase(gc_obj);
        return gc_obj->moved_to;
    }
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
    } while (old_gc_obj != NULL);
}

bool sweep_step() {
    if (is_empty(gc->black_queue)) {
        return true;
    }
    gc_object_t *black_obj = get(gc->black_queue);
    if (is_in_current_heap(black_obj)) {
        printf("Sweeping object: ");
        print_stella_object(&black_obj->obj);
        printf("\n");
        sweep_forward(&black_obj->obj);
        push(gc->black_queue, black_obj->moved_to);
        printf("Swept object: ");
        print_stella_object(&black_obj->moved_to->obj);
        printf(", from %p to %p \n", black_obj, black_obj->moved_to);
    } else {
        // to change fields addresses
        // obj_to_move already moved
        printf("Swept object fields:\n object: ");
        print_stella_object(&black_obj->obj);
        int field_count = STELLA_OBJECT_HEADER_FIELD_COUNT(black_obj->obj.object_header);
        printf("\n fields count: %d\n", field_count);
        for (int i = 0; i < field_count; i++) {
            stella_object *cur_field = black_obj->obj.object_fields[i];
            printf("  field %d: ", i);
            print_stella_object(cur_field);
            printf("\n");
            if (is_in_current_heap(cur_field)) {
                gc_object_t *moved_field = stella_object_to_gc_object(cur_field)->moved_to;
                black_obj->obj.object_fields[i] = &moved_field->obj;
            }
        }
    }
    return false;
}

void *alloc_heap(size_t size) {
    printf("Size to alloc %u, ", size);
    void *heap = malloc(size);
    if (heap == NULL) {
        printf("Memory allocation for new heap failed!\n");
        exit(1);
    }
    printf("heap from %p to %p \n", heap, heap + size);
    return heap;
}

SWEEP_STRATEGY sweep_prepare(bool ignore_strategy) {
    SWEEP_STRATEGY strategy = sweep_strategy();
    if (ignore_strategy) {
        strategy = MAKE_BIGGER;
    }
    switch (strategy) {
        case MAKE_BIGGER:
            gc->sweep_helper.next_heap_size = gc->current_heap_size * 2;
            gc->sweep_helper.next_heap = alloc_heap(gc->sweep_helper.next_heap_size);
            gc->sweep_helper.next = gc->sweep_helper.next_heap;
            break;
        case MAKE_SMALLER:
            gc->sweep_helper.next_heap_size = gc->current_heap_size / 2;
            gc->sweep_helper.next_heap = alloc_heap(gc->sweep_helper.next_heap_size);
            gc->sweep_helper.next = gc->sweep_helper.next_heap;
            break;
        case DO_NOTHING:
            break;
    }
    printf("Sweeping strategy: %d\n", strategy);
    return strategy;
}

void sweep_cleanup() {
    printf("Sweep cleanup\n");
    // moving roots links
    for (int i = 0; i < gc->roots_cont; i++) {
        stella_object *current_root = *(gc->roots[i]);
        if (is_in_current_heap(current_root)) {
            printf("Sweeping root: ");
            print_stella_object(current_root);
            printf("\n from %p to %p\n", stella_object_to_gc_object(current_root), stella_object_to_gc_object(current_root)->moved_to);
            fflush(stdout);
            *(gc->roots[i]) = &stella_object_to_gc_object(current_root)->moved_to->obj;
        } else {
            int field_count = STELLA_OBJECT_HEADER_FIELD_COUNT(current_root->object_header);
            for (int j = 0; j < field_count; j++) {
                if (is_in_current_heap(current_root->object_fields[j])) {
                    gc_object_t *moved_field = stella_object_to_gc_object(current_root->object_fields[j])->moved_to;
                    current_root->object_fields[j] = &moved_field->obj;
                }
            }
        }
    }
    free(gc->current_heap);
    gc->current_heap = gc->sweep_helper.next_heap;
    gc->current_heap_size = gc->sweep_helper.next_heap_size;
    gc->next_place_in_heap = gc->sweep_helper.next;
    gc->phase = MARK;

}

gc_object_t *stella_object_to_gc_object(void *ptr) {
    return ptr - (sizeof(gc_object_t) - sizeof(stella_object));
}

size_t get_gc_object_size(gc_object_t *obj) {
    int fields_count = STELLA_OBJECT_HEADER_FIELD_COUNT(obj->obj.object_header);
    return sizeof(gc_object_t) + fields_count * sizeof(void *);
}

void make_stella_object_grey_if_needed(stella_object *stella_obj) {
    // something predefined
    printf("mark stella object: ");
    print_stella_object(stella_obj);
    printf(", ");
    if (!is_in_current_heap(stella_obj)) {
        printf(" not in current heap\n");
        return;
    }
    gc_object_t *obj = stella_object_to_gc_object(stella_obj);
    // already traversed or marked
    if (obj->color != WHITE) {
        printf(" already marked\n");
        return;
    }
    obj->color = GREY;
    push(gc->grey_queue, obj);
    printf(" marked now\n");
}

void mark_roots_and_their_children() {
    for (int i = 0; i < gc->roots_cont; i++) {
        stella_object *current_root = *(gc->roots[i]);
        // if root is allocated we can just mark it as grey and traverse it's children later
        if (current_root == NULL) {
            continue;
        }
        if (is_in_current_heap(current_root)) {
            fflush(stdout);
            make_stella_object_grey_if_needed(current_root);
        } else {
            // root is something already predefined
            int fields_count = STELLA_OBJECT_HEADER_FIELD_COUNT(current_root->object_header);
            for (int j = 0; j < fields_count; j++) {
                print_stella_object(current_root);
                fflush(stdout);
                stella_object *current_field = current_root->object_fields[j];
                make_stella_object_grey_if_needed(current_field);
            }
        }
    }
}

// returns true if everything marked, false otherwise
bool mark_step() {
    if (is_empty(gc->grey_queue)) {
        mark_roots_and_their_children();
    }
    if (!is_empty(gc->grey_queue)) {
        gc_object_t *obj = get(gc->grey_queue);
        int fields_count = STELLA_OBJECT_HEADER_FIELD_COUNT(obj->obj.object_header);
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
    sweep_prepare(true); // allocate new space
    done = sweep_step();
    while (!done) {
        done = sweep_step();
    }
    sweep_cleanup();
}

void gc_step() {
    if (gc->phase == MARK) {
        bool is_done = mark_step();
        if (is_done) {
            SWEEP_STRATEGY strategy = sweep_prepare(false);
            if (strategy != DO_NOTHING) {
                gc->phase = SWEEP;
            }
        }
    } else {
        bool is_done = sweep_step();
        if (is_done) {
            sweep_cleanup();
        }
    }
}
