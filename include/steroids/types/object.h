#pragma once

#define ST_CLASS(body)  \
typedef struct {        \
    st_object_t _object; \
    body;               \
}

typedef struct {
    void *owner;
    void *funcs;
} st_object_t;

inline void st_object_make(void *object, void *owner, void *funcs) {
    ((st_object_t *)object)->owner = owner;
    ((st_object_t *)object)->funcs = funcs;
}

inline void *st_object_get_owner(void *object) {
    return ((st_object_t *)object)->owner;
}

inline void *st_object_get_funcs(void *object) {
    return ((st_object_t *)object)->funcs;
}
