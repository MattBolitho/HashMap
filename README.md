# Hash Map Implementation
This repository contains a basic C hash map implementation.
The motivation for creating this was a combination of use in personal projects and learning.

## Getting Started
This hash map implementation depends only on the C standard library.
It should be C99 compatible.

There is a CMake project for building a static library containing the implementation.
As it's a single header and source file, it could be easier to directly include the files in compilation.

General notes:
- Keys to the hash map are unordered bytes and values are `void*`.
- Most functions defined on `HashMap` return `bool` to indicate success status.
- When the hash map is freed, the `void*` values will not be affected.
- Custom hashing algorithms are supported. See the `HashMap_HashFunction` `typedef`.
- Some hashing functions are provided:
    - FNV1A
    - SipHash-2-4
    - MurmurHash3 32-bit

### API example
```cpp
#include "HashMap.h"

typedef struct Point { int x; int y; } Point;

// Iteration functions should have 2 arguments:
// - The pointer to the element.
// - Optionally, the pointer to user data. This is shared across all iterations.
static void PrintPoint(void const* element, void* userData) {
    Point const* point = element;
    printf("( %d, %d )\n", point->x, point->y);
}

int main(void) {
    // Declare a test key/value pair for this example.
    HashMapKey const key1 = { "test", 4 };
    Point test = { 2, 2 };

    // Initialize a new instance of a hash map.
    HashMap hashMap = HashMap_NewDefault();

    // Set a key value pair.
    HashMap_Set(&hashMap, key1, &test);

    // Remove an element from the hash map. This function will also
    // place the pointer to the removed value in the 3rd argument.
    Point testRemove = { 0, 0 };
    void* testRemovePtr = &testRemove;
    HashMap_Remove(&hashMap, key1, &testRemovePtr);
    testRemove = *(Point*)testRemovePtr;

    // Iterates through every element of hash map. In this case, the PrintPoint
    // function will be called on each value.
    HashMap_Iterate(&hashMap, &PrintPoint, NULL);

    // When you're done with the hash map, make sure to free the memory.
    HashMap_Free(&hashMap);

    return EXIT_SUCCESS;
}
```

## Future Work
Future work for this hash map implementation includes:
- Automatic resizing and rehashing when a given threshold of buckets are full.
- Optimizing collision search.
- More hash functions.
- Unit tests.

## License
This repository is licensed under the [MIT license](https://choosealicense.com/licenses/mit/).
For more information, please refer to the [`LICENSE`](./LICENSE) file.

## Acknowledgements & References
- MurmurHash [Wikipedia](https://en.wikipedia.org/wiki/MurmurHash)
- SipHash-2-4 [Wikipedia](https://en.wikipedia.org/wiki/SipHash) / [Repository](https://github.com/veorq/SipHash)
- FNV1A [Wikipedia](https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function)
