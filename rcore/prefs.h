#pragma once

enum prefs_key {
    PREFS_KEY_TZ,
    PREFS_KEY_IS24H,
    PREFS_KEY_WATCHFACE,
};

int prefs_get(const uint32_t key, void *buf, uint32_t bufsz);
int prefs_put(const uint32_t key, void *buf, uint32_t bufsz);
