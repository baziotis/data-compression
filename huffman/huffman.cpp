#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void print_bin(uint32_t v, int lim) {
    uint32_t r = 1U << 31;
    while(r && lim--) {
        if(v & r) {
            printf("1");
        } else {
            printf("0");
        }
        r >>= 1;
    }
    printf("\n");
}

// The basic data structure for Huffman encoding is a min heap.

typedef struct symbol symbol_t;

typedef struct symbol {
public:
    char *sym;
    int freq;
    symbol_t *left, *right;

    static int cmp(symbol_t *a, symbol_t *b) {
        return (a->freq - b->freq);
    }

    symbol_t& operator=(symbol_t const& copy) {
        sym = copy.sym;
        freq = copy.freq;
        left = copy.left;
        right = copy.right;
        return *this;
    }
} symbol_t;

typedef struct {
private:
    symbol_t *data;
    int capacity;
    int used;
    int (*symbol_cmp)(symbol_t *, symbol_t *);

    void swap(int a, int b) {
        symbol_t temp = data[a];
        data[a] = data[b];
        data[b] = temp;
    }

    int cmp(int a, int b) {
        return symbol_cmp(&data[a], &data[b]);
    }

// offsets start from 0
#define left(index) (2*index+1)
#define right(index) (2*index+2)
#define parent(index) ((index-1) >> 1)

    void heapify_up(int index) {
        while(index != 0) {
            int p = parent(index);
            if(cmp(p, index) > 0) {
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
            if((r = has_right(index)) && cmp(r, smallest) < 0) {
                smallest = r;
            }
            if(cmp(index, smallest) < 0) {
                break;
            } else {
                swap(index, smallest);
                index = smallest;
            }
        }
    }

public:
    void initialize(int cap, int (*sym_cmp)(symbol_t *a, symbol_t *b)) {
        data = (symbol_t *) malloc(cap * sizeof(symbol_t));
        capacity = cap;
        used = 0;
        symbol_cmp = sym_cmp;
    }

    void insert(symbol_t s) {
        // insert into a new leaf node
        data[used] = s;
        // O(logn) heapify up
        heapify_up(used);
        ++used;
    }

    symbol_t remove_min(void) {
        symbol_t res;
        res.freq = -1;
        // IMPORTANT(stefanos): value on empty heap depends on
        // usage.
        if(!used) return res;

        res = data[0];
        swap(0, used-1);
        --used;
        // O(logn) heapify down
        heapify_down();
        return res;
    }

    void print(void) {
        for(int i = 0; i != used; ++i) {
            printf("%d ", data[i].freq);
        }
        printf("\n");
    }

    void print_vlc(symbol_t *root, uint32_t v, int index) {
        if(root != NULL) {
            if(root->left == NULL && root->right == NULL) {  // leaf
                printf("%s: ", root->sym);
                print_bin(v, index);
            }
            print_vlc(root->left, v, index+1);
            v = v | (1U << (31 - index));
            print_vlc(root->right, v, index+1);
        }
    }

    void build_vlc(void) {
        symbol_t *syms = (symbol_t *) malloc(3 * capacity * sizeof(symbol_t));
        assert(syms != NULL);
        int i = 0;
        while(used > 1) {
            symbol_t a = remove_min();
            syms[i] = a;
            symbol_t *ap = &syms[i];
            i++;
            symbol_t b = remove_min();
            syms[i] = b;
            printf("a: %d, b: %d\n", a.freq, b.freq);
            symbol_t *bp = &syms[i];
            i++;
            symbol_t *cp = &syms[i];
            cp->freq = a.freq + b.freq;
            cp->left = bp;
            cp->right = ap;
            insert(syms[i]);
            i++;
            print();
        }
        symbol_t root = remove_min();
        print_vlc(&root, 0U, 0);
        free(syms);
    }

    void free_heap(void) {
        free(data);
    }
} huffman_heap_t;

int main(void) {
    huffman_heap_t huffman_heap;
    symbol_t min;
    symbol_t temp;
    temp.left = NULL;
    temp.right = NULL;

    huffman_heap.initialize(3, symbol_t::cmp);
    temp.sym = strdup("A");
    temp.freq = 4;
    huffman_heap.insert(temp);
    huffman_heap.print();
    temp.sym = strdup("B");
    temp.freq = 1;
    huffman_heap.insert(temp);
    huffman_heap.print();
    temp.sym = strdup("C");
    temp.freq = 1;
    huffman_heap.insert(temp);
    huffman_heap.print();

    printf("\nBuild VLC\n");
    huffman_heap.build_vlc();
    huffman_heap.free_heap();
    return 0;
}
