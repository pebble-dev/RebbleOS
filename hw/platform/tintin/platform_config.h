#define DISPLAY_ROWS 168
#define DISPLAY_COLS 144

//We are a square device
#define PBL_RECT



/* Memory Configuration
 * Size of the app + stack + heap of the running app.
   IN BYTES
 */
#define MEMORY_SIZE_APP           40000
#define MEMORY_SIZE_WORKER        10000
#define MEMORY_SIZE_OVERLAY       10000

/* Size of the stack in WORDS */
#define MEMORY_SIZE_APP_STACK     800
#define MEMORY_SIZE_WORKER_STACK  100
#define MEMORY_SIZE_OVERLAY_STACK 350


#define MEMORY_SIZE_APP_HEAP      MEMORY_SIZE_APP - (MEMORY_SIZE_APP_STACK * 4)
#define MEMORY_SIZE_WORKER_HEAP   MEMORY_SIZE_WORKER - (MEMORY_SIZE_WORKER_STACK * 4)
#define MEMORY_SIZE_OVERLAY_HEAP  MEMORY_SIZE_OVERLAY - (MEMORY_SIZE_OVERLAY_STACK * 4)
//Tintin uses OC2 for backlight
#define BL_TIM_CH 2
