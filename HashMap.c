#include "HashMap.h"

uint64_t HashMap_FNV1A(HashMapKey const key, uint64_t const seed1, uint64_t const seed2) {
    if (!key.bytes) {
        return 0;
    }

    #define FNV_OFFSET 0xcbf29ce484222325
    #define FNV_PRIME 0x100000001b3

    uint64_t hash = FNV_OFFSET;
    for (size_t i = 0; i < key.length; ++i) {
        hash ^= (uint8_t)key.bytes[i];
        hash *= FNV_PRIME;
    }

    #undef FNV_OFFSET
    #undef FNV_PRIME

    return hash;
}

uint64_t HashMap_SipHash_2_4(HashMapKey const key, uint64_t seed1, uint64_t seed2) {
    uint8_t const* bytes = key.bytes;
    if (!bytes) {
        return 0;
    }

    #define ROTL(x, b) (uint64_t)(((x) << (b)) | ((x) >> (64 - (b))))

    #define U32TO8_LE(p, v)                                                        \
        (p)[0] = (uint8_t)((v));                                                   \
        (p)[1] = (uint8_t)((v) >> 8);                                              \
        (p)[2] = (uint8_t)((v) >> 16);                                             \
        (p)[3] = (uint8_t)((v) >> 24);

    #define U64TO8_LE(p, v)                                                        \
        U32TO8_LE((p), (uint32_t)((v)));                                           \
        U32TO8_LE((p) + 4, (uint32_t)((v) >> 32));

    #define U8TO64_LE(p)                                                           \
        (((uint64_t)((p)[0])) | ((uint64_t)((p)[1]) << 8) |                        \
         ((uint64_t)((p)[2]) << 16) | ((uint64_t)((p)[3]) << 24) |                 \
         ((uint64_t)((p)[4]) << 32) | ((uint64_t)((p)[5]) << 40) |                 \
         ((uint64_t)((p)[6]) << 48) | ((uint64_t)((p)[7]) << 56))

    #define SIPROUND                                                               \
        do {                                                                       \
            v0 += v1;                                                              \
            v1 = ROTL(v1, 13);                                                     \
            v1 ^= v0;                                                              \
            v0 = ROTL(v0, 32);                                                     \
            v2 += v3;                                                              \
            v3 = ROTL(v3, 16);                                                     \
            v3 ^= v2;                                                              \
            v0 += v3;                                                              \
            v3 = ROTL(v3, 21);                                                     \
            v3 ^= v0;                                                              \
            v2 += v1;                                                              \
            v1 = ROTL(v1, 17);                                                     \
            v1 ^= v2;                                                              \
            v2 = ROTL(v2, 32);                                                     \
        } while (0)

    uint64_t k0 = U8TO64_LE((uint8_t*)&seed1);
    uint64_t k1 = U8TO64_LE((uint8_t*)&seed2);
    uint64_t v3 = UINT64_C(0x7465646279746573) ^ k1;
    uint64_t v2 = UINT64_C(0x6c7967656e657261) ^ k0;
    uint64_t v1 = UINT64_C(0x646f72616e646f6d) ^ k1;
    uint64_t v0 = UINT64_C(0x736f6d6570736575) ^ k0;

    uint8_t const* end = bytes + key.length - (key.length % sizeof(uint64_t));
    for (; bytes != end; bytes += sizeof(uint8_t)) {
        uint64_t m = U8TO64_LE(bytes);
        v3 ^= m;
        SIPROUND;
        SIPROUND;
        v0 ^= m;
    }

    int const left = key.length & 7;
    uint64_t b = ((uint64_t)key.length) << 56;
    switch (left) {
        case 7: 
            b |= ((uint64_t)bytes[6]) << 48;
        case 6: 
            b |= ((uint64_t)bytes[5]) << 40;
        case 5: 
            b |= ((uint64_t)bytes[4]) << 32;
        case 4: 
            b |= ((uint64_t)bytes[3]) << 24;
        case 3: 
            b |= ((uint64_t)bytes[2]) << 16;
        case 2: 
            b |= ((uint64_t)bytes[1]) << 8;
        case 1: 
            b |= ((uint64_t)bytes[0]); break;
        case 0:
        default: break;
    }

    v3 ^= b;

    SIPROUND;
    SIPROUND;

    v0 ^= b;
    v2 ^= 0xff;

    SIPROUND;
    SIPROUND;
    SIPROUND;
    SIPROUND;

    b = v0 ^ v1 ^ v2 ^ v3;

    uint64_t out = 0;
    U64TO8_LE((uint8_t*)&out, b)

    #undef SIPROUND
    #undef U64TO8_LE
    #undef U8TO64_LE
    #undef U32TO8_LE
    #undef ROTL

    return out;
}

static uint32_t Murmur32Scramble(uint32_t k) {
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;

    return k;
}

uint64_t HashMap_MurmurHash32(HashMapKey key, uint64_t const seed1, uint64_t const seed2) {
    uint32_t h = (uint32_t)seed1;
    uint32_t k = 0;

    for (size_t i = key.length >> 2; i; i--) {
        memcpy(&k, key.bytes, sizeof(uint32_t));
        key.bytes += sizeof(uint32_t);
        h ^= Murmur32Scramble(k);
        h = (h << 13) | (h >> 19);
        h = h * 5 + 0xe6546b64;
    }

    for (size_t i = key.length & 3; i; i--) {
        k <<= 8;
        k |= key.bytes[i - 1];
    }
    h ^= Murmur32Scramble(k);

    h ^= key.length;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
}

HashMap HashMap_NewDefault(void) {
    // An arbitrary decent-sized prime number. Need to add automatic resizing and re-hashing.
    #define DEFAULT_NUMBER_OF_BUCKETS 67

    size_t const elementsSize = DEFAULT_NUMBER_OF_BUCKETS * sizeof(HashMapNode);
    HashMapNode* elements = malloc(elementsSize);
    if (!elements) {
        HashMap const errorHashMap = { 0, 0, 0, NULL, NULL, 0 };
        return errorHashMap;
    }
    memset(elements, 0, elementsSize);

    HashMap const hashMap = {
        .numberOfBuckets = DEFAULT_NUMBER_OF_BUCKETS,
        .seed1 = 0,
        .seed2 = 0,
        .hashFunction = &HashMap_SipHash_2_4,
        .elements = elements,
        .numberOfElements = 0
    };

    #undef DEFAULT_NUMBER_OF_BUCKETS

    return hashMap;
}

static bool HashMapStateIsValid(HashMap* hashMap) {
    return hashMap && hashMap->elements && hashMap->hashFunction && hashMap->numberOfBuckets > 0;
}

static bool HashMapBucketIsEmpty(HashMapNode* node) {
    return node->hash == 0 && node->next == NULL && node->data == NULL;
}

bool HashMap_Set(HashMap* hashMap, HashMapKey const key, void* value) {
    if (!HashMapStateIsValid(hashMap) || !key.bytes || !value) {
        return false;
    }

    uint64_t const hash = hashMap->hashFunction(key, hashMap->seed1, hashMap->seed2);
    size_t const bucketIndex = hash % hashMap->numberOfBuckets;
    HashMapNode* rootNode = &hashMap->elements[bucketIndex];
    HashMapNode const newNode = { hash, value, NULL };

    if (HashMapBucketIsEmpty(rootNode)) {
        // No value exists for this bucket yet so we can just place the element here.

        *rootNode = newNode;
        ++hashMap->numberOfElements;
    }
    else {
        // Entry already exists so we need to append to bucket by iterating through the
        // linked list. If we find a node with the same hash along the way, then we can
        // just update the value. If not, we need to append a new node to the linked list.
        HashMapNode* node = rootNode;
        if (node->hash == hash) {
            node->data = value;
            return true;
        }

        while (node->next) {
            if (node->hash == hash) {
                node->data = value;
                return true;
            }
            node = node->next;
        }

        // Append the new node.
        HashMapNode* newNodePointer = malloc(sizeof(HashMapNode));
        if (!newNodePointer) {
            return false;
        }
        *newNodePointer = newNode;
        node->next = newNodePointer;
        ++hashMap->numberOfElements;
    }

    return true;
}

void* HashMap_Get(HashMap* hashMap, HashMapKey const key) {
    if (!HashMapStateIsValid(hashMap) || !key.bytes) {
        return NULL;
    }

    uint64_t const hash = hashMap->hashFunction(key, hashMap->seed1, hashMap->seed2);
    size_t const bucketIndex = hash % hashMap->numberOfBuckets;
    HashMapNode* searchNode = &hashMap->elements[bucketIndex];

    // Check the head of the linked list.
    if (searchNode->hash != 0) {
        return searchNode->data;
    }

    // If not found, crawl the linked list and look for it.
    while (searchNode->next) {
        if (searchNode->hash == hash) {
            return searchNode->data;
        }
        searchNode = searchNode->next;
    }

    return NULL;
}

bool HashMap_Remove(HashMap* hashMap, HashMapKey const key, void** item) {
    if (!HashMapStateIsValid(hashMap)) {
        return false;
    }

    uint64_t const hash = hashMap->hashFunction(key, hashMap->seed1, hashMap->seed2);
    size_t const bucketIndex = hash % hashMap->numberOfBuckets;
    HashMapNode* rootNode = &hashMap->elements[bucketIndex];

    if (HashMapBucketIsEmpty(rootNode)) {
        return false;
    }

    // If the root node is the element to remove, then if it has no linked list, it can be zero'd
    // otherwise we need to promote the next node to the top of the bucket.
    if (rootNode->hash == hash) {
        *item = rootNode->data;
        if (rootNode->next) {
            hashMap->elements[bucketIndex] = *rootNode->next;
        }
        else {
            memset(rootNode, 0, sizeof(HashMapNode));
        }
        --hashMap->numberOfElements;
        return true;
    }

    // See if the element is in the collision linked list.
    bool found = false;
    HashMapNode* parentNode = rootNode;
    HashMapNode* deleteNode = parentNode->next;
    if (deleteNode->hash == hash) {
        found = true;
    }
    else {
        do
        {
            parentNode = deleteNode;
            deleteNode = deleteNode->next;
            if (deleteNode->hash == hash) {
                found = true;
                break;
            }
        } while (deleteNode->next);
    }

    if (!found) {
        return false;
    }

    // If the element to remove is in the linked list (but not the last element), then we have
    // to move the pointers to be pointers to the next element. If there is no next element,
    // then we should free the block and set the previous element's next pointer to NULL.
    if (deleteNode->next) {
        parentNode->next = deleteNode->next;
    }
    else {
        parentNode->next = NULL;
    }
    *item = deleteNode->data;
    --hashMap->numberOfElements;
    free(deleteNode);

    return true;
}

bool HashMap_Iterate(HashMap* hashMap, HashMap_HashMapIterationFunction const action, void* userData) {
    if (!HashMapStateIsValid(hashMap)) {
        return false;
    }

    for (size_t i = 0; i < hashMap->numberOfBuckets; ++i) {
        HashMapNode* bucket = &hashMap->elements[i];
        if (!bucket || HashMapBucketIsEmpty(bucket)) {
            continue;
        }

        // Invoke action of element at bucket.
        HashMapNode* node = bucket;
        action(node->data, userData);

        // Crawl bucket's linked list for collided entries.
        while (node->next) {
            action(node->data, userData);
            node = node->next;
        }
    }

    return true;
}

static void FreeHashMapNodeRecursively(HashMapNode* node) {
    if (node->next) {
        FreeHashMapNodeRecursively(node->next);
    }
    free(node);
}

void HashMap_Free(HashMap* hashMap)
{
    if (!HashMapStateIsValid(hashMap)) {
        return;
    }

    for (size_t i = 0; i < hashMap->numberOfBuckets; ++i) {
        HashMapNode* bucket = &hashMap->elements[i];

        // The top-level bucket is allocated in the hash map's `elements` buffer
        // so we don't want to free that here - only the collided nodes attached
        // to the buffer.
        if (bucket && bucket->next) {
            FreeHashMapNodeRecursively(bucket->next);
        }
    }

    free(hashMap->elements);
    hashMap->numberOfBuckets = 0;
    hashMap->elements = NULL;
    hashMap->hashFunction = NULL;
    hashMap->numberOfElements = 0;
}
