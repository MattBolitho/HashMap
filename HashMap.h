#ifndef HASHMAP_H
#define HASHMAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/// A key for the hash map. Keys are just some amount of unsigned bytes. During hashing, the
/// hashing algorithm will read `length` bytes from the `bytes` pointer.
typedef struct HashMapKey {
    uint8_t* bytes;
    size_t length;
} HashMapKey;

/// An entry in the hash map.
typedef struct HashMapNode {
    uint64_t hash;
    void* data;
    struct HashMapNode* next;
} HashMapNode;

/// Defines the interface for a hash function. Two seed values are also supported.
typedef uint64_t(*HashMap_HashFunction)(HashMapKey, uint64_t seed1, uint64_t seed2);

/// Defines the interface for a function that can be used to iterate through the elements of a hash map.
typedef void(*HashMap_HashMapIterationFunction)(void const*, void*);

/// A type that maps keys to values uses hashes. Depending on the chosen hash function, up to two
/// values may also be chosen to seed the hash function.
typedef struct HashMap {
    size_t numberOfBuckets;
    uint64_t seed1;
    uint64_t seed2;
    HashMap_HashFunction hashFunction;
    HashMapNode* elements;
    size_t numberOfElements;
} HashMap;

/// Performs FNV1A hashing of the given key bytes.
/// @see https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
/// @param key The key.
/// @param seed1 Unused seed value for conformance with the HashMap_HashFunction type.
/// @param seed2 Unused seed value for conformance with the HashMap_HashFunction type.
/// @returns The FNV1A hash of the key. If the key is NULL, then 0 is returned.
uint64_t HashMap_FNV1A(HashMapKey key, uint64_t seed1, uint64_t seed2);

/// Performs SipHash-2-4 hashing of the given key bytes.
/// @see https://en.wikipedia.org/wiki/SipHash
/// @see https://github.com/veorq/SipHash
/// @param key The key.
/// @param seed1 A random seed for the hash calculation.
/// @param seed2 A random seed for the hash calculation.
/// @returns The SipHash-2-4 hash of the key. If the key is NULL, then 0 is returned.
uint64_t HashMap_SipHash_2_4(HashMapKey key, uint64_t seed1, uint64_t seed2);

/// Performs Murmur hash 3 (32-bit) hashing of the given key bytes.
/// @remarks The return value is a uint64_t, but the underlying algorithm will always
/// return a uint32_t. So the range of possible values of this function is actually uint32_t.
/// @see https://en.wikipedia.org/wiki/MurmurHash
/// @param key The key.
/// @param seed1 A random seed for the hash calculation.
/// @param seed2 Unused seed value for conformance with the HashMap_HashFunction type.
/// @returns The Murmur hash 3 (32-bit) hash of the key. If the key is NULL, then 0 is returned.
uint64_t HashMap_MurmurHash32(HashMapKey key, uint64_t seed1, uint64_t seed2);

/// Creates a new HashMap struct instance using default values.
/// @returns A new HashMap with default values.
HashMap HashMap_NewDefault(void);

/// Sets (adds or updates) an entry for the key value pair in the hash map.
/// @param hashMap A reference to the hash map.
/// @param key The key.
/// @param value The value.
/// @returns True if the operation was successful, false otherwise.
bool HashMap_Set(HashMap* hashMap, HashMapKey key, void* value);

/// Gets the value associated with the key, or NULL if none exists.
/// @param hashMap A reference to the hash map.
/// @param key The key.
/// @returns The value associated with the key, or NULL if none exists.
void* HashMap_Get(HashMap* hashMap, HashMapKey key);

/// Removes the entry from the hash map with the given key. The removed item is
/// written to the `item` parameter.
/// @param hashMap A reference to the hash map.
/// @param key The key.
/// @param item The pointer to the removed item.
/// @returns True if the operation was successful, false otherwise.
bool HashMap_Remove(HashMap* hashMap, HashMapKey key, void** item);

/// Iterates the elements of the hash map and runs the given action on the value at the node.
/// @param hashMap A reference to the hash map.
/// @param action The action to run over each node.
/// @param userData An instance of data to pass into to each iteration. Can be used to manage state between iterations.
/// @returns True if the operation was successful, false otherwise.
bool HashMap_Iterate(HashMap* hashMap, HashMap_HashMapIterationFunction action, void* userData);

/// Frees the memory occupied by the hash map.
/// @remarks The user supplied value data (i.e. the values pointed to by the keys) are not freed.
/// @param hashMap The hash map.
void HashMap_Free(HashMap* hashMap);

#ifdef __cplusplus
}
#endif

#endif // HASHMAP_H
