#ifndef VECTOR_H
#define VECTOR_H

// refrence https://bytesbeneath.com/p/dynamic-arrays-in-c

#include <stddef.h>
#include <stdlib.h>

// https://github.com/nothings/stb/blob/master/docs/stb_howto.txt
#ifndef VECAPI
    #ifdef VECTOR_STATIC
        #define VECAPI static
    #else
        #define VECAPI extern
    #endif
#endif

#if defined(VEC_MALLOC) && defined(VEC_FREE) && defined(VEC_REALLOC)
#elif !defined(VEC_MALLOC) && !defined(VEC_FREE) && !defined(VEC_REALLOC)
#else
    #error "You need to define all VEC_MALLOC, VEC_REALLOC, and VEC_FREE or none"
#endif

#ifndef VEC_MALLOC
    #define VEC_MALLOC(size) malloc(size)
    #define VEC_FREE(ptr) free(ptr)
    #define VEC_REALLOC(ptr,new_size) realloc(ptr,new_size)
#endif

typedef struct
{
    size_t capacity;
    size_t element_size;
    size_t length;
    size_t vector__aligment; // I think it will make not cause alignmet issue for 1,2,4,16,32 (on 64bit)
}VectorHeader;

#define VECTOR_CAPACITY                 16
#define Vector(T)                       vector_init(sizeof(T), VECTOR_CAPACITY)
#define vector_header(v)                ((VectorHeader *)(v)-1)
#define vector_length(v)                ((v)?vector_header(v)->length:0)
#define vector_capacity(v)              (vector_header(v)->capacity)
#define vector_pop(v)                   (vector_header(v)->length--,v[vector_length(v)])
#define free_vector(v)                  VEC_FREE((v)?(vector_header(v)):NULL)
#define vector_append(vector, value)    ((vector) = vector_ensure_capacity(vector, 1),  \
                                        (vector)[vector_header(vector)->length] = (value),                 \
                                        &(vector)[vector_header(vector)->length++])                        

VECAPI void *vector_init(const size_t element_size, const size_t capacity);
VECAPI void *vector_ensure_capacity(void *vector, const size_t total_element);

#endif // VECTOR_H

#ifdef IMPLEMENT_VECTOR

VECAPI void *vector_init(size_t element_size, const size_t capacity)
{
    void *ptr = 0;
    VectorHeader *vec_header =
        VEC_MALLOC(sizeof(*vec_header) + capacity * element_size);

    if (vec_header)
    {
        vec_header->capacity = capacity;
        vec_header->element_size = element_size;
        vec_header->length = 0;
        ptr = vec_header + 1;
    }

    return ptr;
}

VECAPI void *vector_ensure_capacity(void *vector, const size_t total_element) 
{
    VectorHeader *vec_header = vector_header(vector);
    const size_t element_size = vec_header->element_size;
    const size_t desired_capacity = vec_header->length + total_element;
    if (vec_header->capacity < desired_capacity) 
    {
        size_t new_capacity = vec_header->capacity * 2;
        while (new_capacity < desired_capacity) new_capacity *= 2;

        const size_t new_size = sizeof(*vec_header) + new_capacity * element_size;
        VectorHeader *temp = VEC_REALLOC(vec_header, new_size);
        if (!temp) 
        {
            // todo
            return NULL;
        }

        vec_header = temp;
        vec_header->capacity = new_capacity;
    }

    vec_header += 1;
    return vec_header;
}

#endif // IMPLEMENT_VECTOR
