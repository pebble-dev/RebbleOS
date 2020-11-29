/* settings.h
 * settings for [...]
 * RebbleOS
 */ 
typedef struct SystemSettings {
    uint16_t backlight_intensity;
    uint16_t backlight_on_time;
    uint16_t vibrate_intensity;
    uint16_t vibrate_pattern;
    // may need 16t
    uint8_t modules_enabled_flag;
    uint8_t modules_error_flag;
} SystemSettings;

SystemSettings *rebbleos_get_settings(void);

