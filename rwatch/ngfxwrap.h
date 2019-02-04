#pragma once
/* ngfxwrap.h
 * Declarations for shim between RebbleOS and Neographics
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include "context.h"

void rwatch_neographics_init(app_running_thread *thread);
n_GContext *rwatch_neographics_get_global_context(void);
