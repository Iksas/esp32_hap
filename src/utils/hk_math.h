/**
 * @file hk_math.h
 *
 * A math library.
 */

#pragma once

/**
 * @brief Returns the maximum of two numbers.
 */
#define MAX(a,b) ({ __typeof__ (a) _a = (a);   __typeof__ (b) _b = (b);  _a > _b ? _a : _b; })

/**
 * @brief Returns the minimum of two numbers.
 */
#define MIN(a,b) ({ __typeof__ (a) _a = (a);   __typeof__ (b) _b = (b);  _a < _b ? _a : _b; })