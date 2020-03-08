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

// Needed to dynamically create an engine instance only when the app is opened.
#define JERRY_EXTERNAL_CONTEXT 1

// Rocky.js runs precompiled bytecode, rather than raw JS source.
# define JERRY_SNAPSHOT_EXEC 1
// This should save some flash since we don't need to parse JS source. Note: this breaks "eval".
# define JERRY_PARSER 0

// Needed to periodically handle events.
//# define JERRY_VM_EXEC_STOP 1

# define JERRY_ES2015 0
# define JERRY_BUILTINS 0
# define JERRY_ERROR_MESSAGES 1

// TODO: Evaluate the need for certain features and builtins, removing as necessary.
// TODO: Tune heap/stack/GC limits for better performance.

#include "rocky_port.h"