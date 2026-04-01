#pragma once
#include <new>
#include "esp_heap_caps.h"

template<typename T>
T* psNew() {
    void* mem = heap_caps_malloc(sizeof(T), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!mem) return nullptr;
    return new (mem) T();
}

template<typename T>
void psDelete(T*& p) {
    if (!p) return;
    p->~T();
    heap_caps_free(p);
    p = nullptr;
}
