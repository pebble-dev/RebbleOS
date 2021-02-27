#pragma once
/* watchface.h
 * Routines for loading watchfaces based on
 * user preferences.
 * 
 * RebbleOS
 *
 */

void watchface_enter();

void watchface_init();

void watchface_set_pref_by_uuid(Uuid *uuid);