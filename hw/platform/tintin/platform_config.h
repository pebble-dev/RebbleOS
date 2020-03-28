#define DISPLAY_ROWS 168
#define DISPLAY_COLS 144

//We are a square device
#define PBL_RECT

/* Memory Configuration
 * Size of the app + stack + heap of the running app.
   IN BYTES
 */
#define MEMORY_SIZE_SYSTEM         8192
#define MEMORY_SIZE_APP           40000
#define MEMORY_SIZE_WORKER        10000
#define MEMORY_SIZE_OVERLAY       16000

/* Size of the stack in WORDS */
#define MEMORY_SIZE_APP_STACK     4000
#define MEMORY_SIZE_WORKER_STACK  100
#define MEMORY_SIZE_OVERLAY_STACK 450


#define MEMORY_SIZE_APP_HEAP      MEMORY_SIZE_APP - (MEMORY_SIZE_APP_STACK * 4)
#define MEMORY_SIZE_WORKER_HEAP   MEMORY_SIZE_WORKER - (MEMORY_SIZE_WORKER_STACK * 4)
#define MEMORY_SIZE_OVERLAY_HEAP  MEMORY_SIZE_OVERLAY - (MEMORY_SIZE_OVERLAY_STACK * 4)
//Tintin uses OC2 for backlight
#define BL_TIM_CH 2

/* Bluetooth config */
//#define BLUETOOTH_MODULE_TYPE BLUETOOTH_MODULE_TYPE_NONE
#define BLUETOOTH_MODULE_TYPE        BLUETOOTH_MODULE_TYPE_CC2564
#define BLUETOOTH_MODULE_NAME_LENGTH 0x09
#define BLUETOOTH_MODULE_LE_NAME     'P', 'e', 'b', 'b', 'l', 'e', ' ', 'L', 'E'
#define BLUETOOTH_MODULE_GAP_NAME    "Pebble RblOs"
