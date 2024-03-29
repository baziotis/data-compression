#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

uint8_t *get_byte(uint8_t *buff, size_t index) {
    size_t byte_off = index >> 3; // div 8
    uint8_t *p = (uint8_t *) (buff + byte_off);
    return p;
}

void set_bit(uint8_t *buff, size_t index) {
    uint8_t *p = get_byte(buff, index);
    size_t in_byte_off = index & 7; // 7 == 0B111, aka mod 8
    *p = (*p) | (1U << in_byte_off);
}

uint8_t get_bit(uint8_t *buff, size_t index) {
    uint8_t *p = get_byte(buff, index);
    size_t in_byte_off = index & 7; // 7 == 0B111, aka mod 8
    uint8_t ret_val = ((*p) & (1U << in_byte_off)) >> in_byte_off;
    return ret_val;
}

void print_bin_arbitrary(uint8_t *buff, size_t lim) {
    size_t len = 0;
    for(size_t i = 0; i != lim; ++i) {
        uint8_t bit = get_bit(buff, i);
        if(bit) {
            printf("1");
        } else {
            printf("0");
        }
    }
}

void print_bin_word(uint32_t v, int lim) {
    uint32_t r = 1U << 31;
    while(r && lim--) {
        if(v & r) {
            printf("1");
        } else {
            printf("0");
        }
        r >>= 1;
    }
}

// TODO(stefanos):
// 1) IMPORTANT: Better architecture and getting rid of
// very C-like code.
// 2) Investigate if the huffman tree constructed is optimal.
// 3) Decrease size (if possible) of the syms buffer.
// 4) Better decisions on when to use size_t and when to use int.

typedef struct {
    uint8_t *data;
    size_t len;
} encoded_data_t;

typedef struct {
private:
    typedef struct heap_symbol {
    public:
        int freq;
        int left_index, right_index;

        static int cmp(struct heap_symbol *a, struct heap_symbol *b) {
            return (a->freq - b->freq);
        }

        struct heap_symbol& operator=(struct heap_symbol const& copy) {
            freq = copy.freq;
            left_index = copy.left_index;
            right_index = copy.right_index;
            return *this;
        }
    } heap_symbol_t;

    typedef struct initial_symbol {
        char sym;
        uint32_t codeword;
        int code_len;
    } initial_symbol_t;

    heap_symbol_t *data;
    initial_symbol_t *initial_symbols_buffer;
    heap_symbol_t *syms;  // temp syms buffer
    int root_index;
    int capacity;
    int used;
    int (*symbol_cmp)(heap_symbol_t *, heap_symbol_t *);

    void swap(int a, int b) {
        heap_symbol_t temp = data[a];
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

    void heap_insert(heap_symbol_t s) {
        // insert into a new leaf node
        data[used] = s;
        // O(logn) heapify up
        heapify_up(used);
        ++used;
    }

    heap_symbol_t remove_min(void) {
        heap_symbol_t res;
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

    void find_vlcs_helper(int index, uint32_t v, int bit_count) {
        if(syms[index].left_index == -1) {  // leaf
            initial_symbols_buffer[syms[index].right_index].codeword = v;
            initial_symbols_buffer[syms[index].right_index].code_len = bit_count;
            return;
        }
        find_vlcs_helper(syms[index].left_index, v, bit_count+1);
        v = v | (1U << (31 - bit_count));
        find_vlcs_helper(syms[index].right_index, v, bit_count+1);
    }

    int search_symbol(uint8_t *buff, int index, size_t count, char *sym) {
        if(syms[index].left_index == -1) {  // leaf
            *sym = initial_symbols_buffer[syms[index].right_index].sym;
            return count;
        }
        if(!get_bit(buff, count)) return search_symbol(buff, syms[index].left_index, count+1, sym);
        else return search_symbol(buff, syms[index].right_index, count+1, sym);
    }

    void find_vlcs(int index) {
        find_vlcs_helper(index, 0U, 0);
    }

    // TODO(stefanos): Investigate if the tree is being built
    // in the most optimal way.
    // construct the huffman tree, return the index of the root.
    void construct_huffman_tree(void) {
        int i = 0;
        while(used > 1) {
            // TODO(stefanos): Think how we can remove duplicates.
            heap_symbol_t a = remove_min();
            syms[i] = a;
            int a_index = i;
            i++;
            heap_symbol_t b = remove_min();
            syms[i] = b;
            int b_index = i;
            i++;
            heap_symbol_t *cp = &syms[i];
            cp->freq = a.freq + b.freq;
            cp->left_index = a_index;
            cp->right_index = b_index;
            heap_insert(*cp);
            i++;
            // print();
        }
        root_index = i-1;
    }


    void search_codeword(char sym, uint32_t *out_word, int *out_len) {
        int l = 0;
        int r = capacity - 1;
        int mid = (l+r)/2;

        while(l <= r) {
            int comp = sym - initial_symbols_buffer[mid].sym;
            if(comp > 0) {
                l = mid + 1;
            } else if(comp < 0) {
                r = mid - 1;
            } else {
                *out_word = initial_symbols_buffer[mid].codeword;
                *out_len = initial_symbols_buffer[mid].code_len;
                return;
            }
            mid = (l+r)/2;
        }
    }

    void print(void) {
        for(int i = 0; i != used; ++i) {
            printf("%d ", data[i].freq);
        }
        printf("\n");
    }
public:
    void initialize(int cap) {
        size_t heap_symbols_size = cap * sizeof(heap_symbol_t);
        size_t initial_symbols_size = cap * sizeof(initial_symbol_t);
        void *mem = malloc(heap_symbols_size + initial_symbols_size);
        data = (heap_symbol_t *) mem;
        initial_symbols_buffer = (initial_symbol_t *) (((uint8_t *) mem)
                                                      + heap_symbols_size);
        capacity = cap;
        used = 0;
        symbol_cmp = heap_symbol::cmp;
    }

    void insert(char sym, int freq) {
        initial_symbols_buffer[used].sym = sym;
        heap_symbol_t s;
        s.freq = freq;
        s.left_index = -1;
        s.right_index = used;
        heap_insert(s);
    }

    void build_vlcs(void) {
        // TODO(stefanos): This is not an exact computation
        size_t num_items = 3 * capacity;
        syms = (heap_symbol_t *) malloc(num_items * sizeof(heap_symbol_t));
        construct_huffman_tree();
        find_vlcs(root_index);
    }

    void print_vlcs(void) {
        for(int i = 0; i != capacity; ++i) {
            char sym = initial_symbols_buffer[i].sym;
            uint32_t codeword = initial_symbols_buffer[i].codeword;
            uint32_t code_len = initial_symbols_buffer[i].code_len;
            printf("%c: ", sym);
            print_bin_word(codeword, code_len);
            printf("\n");
        }
    }

    encoded_data_t encode(char *s) {
        int len = strlen(s);
        // TODO(stefanos): Use actual maximum code_len for
        // this computation.
        int max_len = 6;
        size_t alloc_size = (max_len * len) / 8 + 1;
        uint8_t *out_buffer = (uint8_t *) calloc(alloc_size, sizeof(uint8_t));
        char *init_addr = s;
		size_t i = 0;
        while(*s != '\0') {
            uint32_t codeword;
            int code_len;
            search_codeword(*s, &codeword, &code_len);
            uint32_t r = 1U << 31;
            while(code_len--) {
                if(codeword & r) {
                    set_bit(out_buffer, i);
                }
                r >>= 1;
                ++i;
            }
            ++s;
        }
        encoded_data_t enc_data;
        enc_data.data = out_buffer;
        enc_data.len = i;
        return enc_data;
    }

    void decode(encoded_data_t enc_data, char *out) {
        size_t i = 0;
        size_t sym_counter = 0;
        char sym;
        while(i != enc_data.len) {
            i = search_symbol(enc_data.data, root_index, i, &sym);
            out[sym_counter++] = sym;
        }
    }

    void free_heap(void) {
        free(data);
    }
} huffman_heap_t;

typedef struct {
    char sym;
    int freq;
} hist_sym_t;

int construct_histogram(char *s, hist_sym_t hist[256], int len) {
    int hist_len = 0;
    int char_map[256];
    for(size_t i = 0; i != 256; ++i) {
        char_map[i] = -1;
    }
    for(ssize_t i = 0; i != len; ++i) {
        int hist_pos = char_map[s[i]];  // position assigned for this number in
                                        // the histogram.
        if(hist_pos == -1) {      // new symbol found, create new position
            char_map[s[i]] = hist_pos = hist_len;
            hist[hist_pos].sym = s[i];
            hist[hist_pos].freq = 0;
            ++hist_len;
        }
        hist[hist_pos].freq++;
    }
    return hist_len;
}

void read_file(char *fname, char **s_out, int *len_out) {
    FILE *fp = fopen(fname, "r");
    fseek(fp, 0L, SEEK_END);
    size_t sz = ftell(fp);
    rewind(fp);
    char *s = (char *) malloc(sz * sizeof(char));
    int c;
    size_t i = 0;
    while((c = fgetc(fp)) != EOF) {
        s[i++] = c;
    }
    s[--i] = '\0'; // terminate the string and remove the newline
    fclose(fp);
    *s_out = s;
    *len_out = i;
}

int comp_hist(const void *a, const void *b) {
    hist_sym_t *s1 = (hist_sym_t *) a;
    hist_sym_t *s2 = (hist_sym_t *) b;
    return (s1->sym - s2->sym);
}

int main(int argc, char **argv) {
    char *s;
    int len;
    read_file(argv[1], &s, &len);
    printf("Initial size: %d\n", len);

    hist_sym_t hist[256];
    int hist_len = construct_histogram(s, hist, len);
    qsort(hist, hist_len, sizeof(hist_sym_t), comp_hist);
    // for(int j = 0; j != hist_len; ++j) {
    //     printf("%c %d\n", hist[j].sym, hist[j].freq);
    // }

    huffman_heap_t huffman_heap;

    // NOTE(stefanos): Insertions should happen in
    // sorted order. (We could just do sorting inside the heap, we
    // just save time)
    huffman_heap.initialize(hist_len);
    // computed kind of randomly (you can take the shannon_entropy.c histogram construction functions
    // to compute these values).
    for(int j = 0; j != hist_len; ++j) {
        huffman_heap.insert(hist[j].sym, hist[j].freq);
    }

    printf("Build VLCs\n");
    huffman_heap.build_vlcs();
    printf("Print VLCs\n");
    // huffman_heap.print_vlcs();
    // printf("\n%s:\n", s);
    encoded_data_t enc_data = huffman_heap.encode(s);
    printf("encode\n");
    //print_bin_arbitrary(enc_data.data, enc_data.len);
    printf("Compressed size: %zd\n", enc_data.len / 8 + 1);
    char *decoded_s = (char *) malloc(len * sizeof(char));
    printf("decode\n");
    huffman_heap.decode(enc_data, decoded_s);
    printf("diff: %d\n", strcmp(s, decoded_s));
    huffman_heap.free_heap();
    return 0;
}
