cmake_minimum_required(VERSION 3.12)

project(HashMap
    VERSION 1.0.1
    DESCRIPTION "A basic C hash map implementation."
    HOMEPAGE_URL "https://github.com/MattBolitho/HashMap"
    LANGUAGES C)

add_library(hashmap HashMap.c)