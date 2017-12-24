/* snowy_backlight.h
 * Backlight control implementation for Pebble Time (snowy)
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

void hw_backlight_init(void);
void hw_backlight_set(uint16_t pwmValue);
