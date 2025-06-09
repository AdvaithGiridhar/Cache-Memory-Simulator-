#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define DIRECT_MAPPED 1
#define SET_ASSOCIATIVE 2
#define FULLY_ASSOCIATIVE 3

typedef struct {
    int valid;
    int tag;
    int dirty;
    int *data;
} CacheBlock;

typedef struct {
    int cache_size;
    int block_size;
    int num_sets;
    int associativity;
    int block_bits;
    int set_bits;
    int tag_bits;
    CacheBlock *blocks;
} Cache;

// Function to convert decimal to binary string
char* decToBinary(int n, int bits) {
    char *binary = (char *)malloc((bits + 1) * sizeof(char));
    for (int i = bits - 1; i >= 0; i--) {
        int k = n >> i;
        if (k & 1)
            binary[bits - i - 1] = '1';
        else
            binary[bits - i - 1] = '0';
    }
    binary[bits] = '\0';
    return binary;
}

Cache* initializeCache(int cache_size, int block_size, int mapping_type, int main_memory_size) {
    Cache *cache = (Cache *)malloc(sizeof(Cache));
    cache->cache_size = cache_size;
    cache->block_size = block_size;
    if (mapping_type == DIRECT_MAPPED) {
        cache->num_sets = cache_size / block_size;
        cache->associativity = 1;
        cache->block_bits = log2(block_size);
        cache->set_bits = log2(cache->num_sets);
        cache->tag_bits = log2(main_memory_size) - cache->set_bits - cache->block_bits;
    } else if (mapping_type == SET_ASSOCIATIVE) {
        printf("Enter associativity:");
        scanf("%d", &cache->associativity);
        cache->num_sets = cache_size / (block_size * cache->associativity);
        cache->block_bits = log2(block_size);
        cache->set_bits = log2(cache->num_sets);
        cache->tag_bits = log2(main_memory_size) - cache->set_bits;
    } else { // Fully Associative
        cache->num_sets = 1;
        cache->associativity = cache_size / block_size;
        cache->block_bits = log2(block_size);
        cache->tag_bits = log2(main_memory_size) - cache->block_bits;
    }
    printf("Cache Configuration:\n");
    printf("Cache Size: %d\n", cache->cache_size);
    printf("Block Size: %d\n", cache->block_size);
    printf("Number of Sets: %d\n", cache->num_sets);
    printf("Associativity: %d\n", cache->associativity);
    printf("Block Bits: %d\n", cache->block_bits);
    printf("Set Bits: %d\n", cache->set_bits);
    printf("Tag Bits: %d\n", cache->tag_bits);
    cache->blocks = (CacheBlock *)malloc(cache->num_sets * cache->associativity * sizeof(CacheBlock));
    for (int i = 0; i < cache->num_sets * cache->associativity; i++) {
        cache->blocks[i].valid = 0;
        cache->blocks[i].tag = -1;
        cache->blocks[i].dirty = 0;
        cache->blocks[i].data = (int *)malloc(block_size * sizeof(int));
    }
    return cache;
}

void accessCache(Cache *cache, int address, int *hits, int *misses, int *evictions) {
    int block_index = address / cache->block_size;
    int tag = block_index; // For fully associative cache, the block index is the tag
    for (int i = 0; i < cache->associativity; i++) {
        if (cache->blocks[i].valid && cache->blocks[i].tag == tag) {
            (*hits)++;
            printf("Address: %s (Tag: %d, Word: %d), Cache Hit\n", decToBinary(address, 32), tag, block_index % cache->block_size);
            return;
        }
    }
    (*misses)++;
    printf("Address: %s (Tag: %d, Word: %d), Cache Miss\n", decToBinary(address, 32), tag, block_index % cache->block_size);
    for (int i = 0; i < cache->associativity; i++) {
        if (!cache->blocks[i].valid) {
            cache->blocks[i].valid = 1;
            cache->blocks[i].tag = tag;
            cache->blocks[i].dirty = 1;
            printf("Updated Cache Block at Index: %d, Tag: %d, Valid: %d\n", i, tag, 1);
            return;
        }
    }
    (*evictions)++;
    int random_index = rand() % cache->associativity;
    if (cache->blocks[random_index].dirty) {
        printf("Writing back dirty cache block at Index: %d, Tag: %d\n", random_index, cache->blocks[random_index].tag);
    }
    printf("Evicting cache block at Index: %d, Tag: %d, Dirty: %d\n", random_index, cache->blocks[random_index].tag, cache->blocks[random_index].dirty);
    cache->blocks[random_index].tag = tag;
    cache->blocks[random_index].dirty = 1;
    printf("Updated Cache Block at Index: %d, Tag: %d, Valid: %d\n", random_index, tag, 1);
}

void printCacheContents(Cache *cache) {
    printf("\nCache Contents:\n");
    for (int i = 0; i < cache->num_sets * cache->associativity; i++) {
        printf("Index: %d, Valid: %d, Tag: %d, Dirty: %d\n", i, cache->blocks[i].valid, cache->blocks[i].tag, cache->blocks[i].dirty);
    }
}

int main() {
    int main_memory_size, cache_size, block_size, mapping_type;
    int hits = 0, misses = 0, evictions = 0;
    int num_accesses, address;

    printf("Enter main memory size (in power of 2): ");
    scanf("%d", &main_memory_size);
    printf("Enter cache size (in power of 2): ");
    scanf("%d", &cache_size);
    printf("Enter the block size: ");
    scanf("%d", &block_size);

    printf("Select the mapping technique type:\n");
    printf("1: Direct Mapped\n");
    printf("2: Set Associative\n");
    printf("3: Fully Associative\n");
    scanf("%d", &mapping_type);
    Cache *cache = initializeCache(cache_size, block_size, mapping_type, main_memory_size);
    printf("Enter number of memory accesses: ");
    scanf("%d", &num_accesses);
    printf("Enter memory addresses one at a time in hexadecimal format:\n");
    for (int i = 0; i < num_accesses; i++) {
        scanf("%x", &address);
        accessCache(cache, address, &hits, &misses, &evictions);
    }
    float hit_ratio = (float)hits / num_accesses;
    float miss_ratio = (float)misses / num_accesses;
    printf("\nHits: %d\nMisses: %d\nEvictions: %d\n", hits, misses, evictions);
    printf("Hit Ratio: %.2f\nMiss Ratio: %.2f\n", hit_ratio, miss_ratio);
    printCacheContents(cache);
    return 0;
}