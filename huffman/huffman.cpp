#include <stdio.h>
#include <stdlib.h>

// The basic data structure for Huffman encoding is a min heap.

typedef struct {
private:
    int *data;
    int capacity;
    int used;

    void swap(int a, int b) {
        int temp = data[a];
        data[a] = data[b];
        data[b] = temp;
    }

// offsets start from 0
#define left(index) (2*index+1)
#define right(index) (2*index+2)
#define parent(index) ((index-1) >> 1)

    void heapify_up(int index) {
        while(index != 0) {
            int p = parent(index);
            if(data[p] > data[index]) {
                swap(p, index);
                index = p;
            } else {
                return;
            }
        }
    }

// Little preprocessor metaprogramming
// Note that we can use 0 to essentially return 'false'
// because 0 is never a left child of a node.
#define has_child(c, i) ( (c(index) < used) ? c(index) : 0 )
#define has_left(i) has_child(left, i)
#define has_right(i) has_child(right, i)

    void heapify_down(void) {
        // Start from the root. As long as there are children, find the smallest
        // of the children and check if root is smaller than this. If yes, then
        // we're finished. If not, swap with the smallest and do the same thing
        // for the child.
        int index = 0, l, r;
        // if there's no left child, then certainly there's no right child.
        while(l = has_left(index)) {
            int smallest = l;
            if((r = has_right(index)) && data[r] < data[smallest]) {
                smallest = r;
            }
            if(data[index] < data[smallest]) {
                break;
            } else {
                swap(index, smallest);
                index = smallest;
            }
        }
    }

public:
    void initialize(int capacity) {
        data = (int *) malloc(capacity * sizeof(int));
        capacity = capacity;
        used = 0;
    }

    void insert(int v) {
        // insert into a new leaf node
        data[used] = v;
        // O(logn) heapify up
        heapify_up(used);
        ++used;
    }

    int remove_min(void) {
        // IMPORTANT(stefanos): value on empty heap depends on
        // usage.
        if(!used) return -1;

        int res = data[0];
        swap(0, used-1);
        --used;
        // O(logn) heapify down
        heapify_down();
        return res;
    }

    void print(void) {
        for(int i = 0; i != used; ++i) {
            printf("%d ", data[i]);
        }
        printf("\n");
    }

    void free_heap(void) {
        free(data);
    }
} min_heap_t;

int main(void) {
    min_heap_t min_heap;
    int min;

    min_heap.initialize(8);
    min_heap.insert(45);
    min_heap.print();
    min_heap.insert(3);
    min_heap.print();
    min_heap.insert(7);
    min_heap.print();
    min_heap.insert(1);
    min_heap.print();
    min_heap.insert(9);
    min_heap.print();

    min = min_heap.remove_min();
    printf("min: %d\n", min);
    min_heap.print();
    min = min_heap.remove_min();
    printf("min: %d\n", min);
    min_heap.print();
    min = min_heap.remove_min();
    printf("min: %d\n", min);
    min_heap.print();
    min = min_heap.remove_min();
    printf("min: %d\n", min);
    min_heap.print();
    min = min_heap.remove_min();
    printf("min: %d\n", min);
    min_heap.print();
    min_heap.free_heap();
    return 0;
}
