#ifndef UTILITY_H
#define UTILITY_H

/**
 * !! DO NOT MODIFY THIS FILE !!
 */

#include <stddef.h>

size_t calculate_index_dblock_amount(size_t file_size);

size_t calculate_necessary_dblock_amount(size_t file_size);

dblock_index_t *cast_dblock_ptr(void *addr);

#endif