#pragma once

/* rockyjs.h
 *
 * Rocky.js (On-watch JavaScript) Implementation
 * JerryScript Configuration
 * 
 * RebbleOS
 *
 * Author: Davit Markarian <davit@udxs.me>
 */

// RebbleOS provides allocation functions.
#define JERRY_SYSTEM_ALLOCATOR 1
// System Allocation requires using full 32-bit pointers.
# define JERRY_CPOINTER_32_BIT 0

// RebbleOS works with soft-floats. In the interest of speed, JerryScript is told to use less precise 32-bit floats.
// # define JERRY_NUMBER_TYPE_FLOAT64 0

// Rocky.js runs precompiled bytecode, rather than raw JS source.
# define JERRY_SNAPSHOT_EXEC 1
// This should save some flash since we don't need to parse JS source. Note: this breaks "eval".
# define JERRY_PARSER 0

// Needed to periodically handle events.
# define JERRY_VM_EXEC_STOP 1

# define JERRY_ES2015 0

// TODO: Evaluate the need for certain features and builtins, removing as necessary.
// TODO: Tune heap/stack/GC limits for better performance.

#include "jerryscript-config.h"

#include "rocky_port.h"
#include "jerryscript.h"