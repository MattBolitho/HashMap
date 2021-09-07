# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [1.0.1] - 2021-09-07
### 🐛 Fixed
- `HashMap_Iterate` did not correctly iterate collided elements.

## [1.0.0] - 2021-08-24
### ✨ Added
- Initial version.
- Hashing algorithms
    - FNV1A
    - SipHash 2-4
    - Murmur3 32-bit
- `HashMap` type supporting
    - Initialize default instance
    - Set key value pair
    - Get key value pair
    - Remove key value pair
    - Free map
