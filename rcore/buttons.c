/* buttons.c
 * routines for Debouncing and sending buttons through a click, multi, long press handler
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

/*
 * MODULE TODO
 * 
 * Button Multi click support
 * buttonholder -> clickrecognizer convert / typedef
 * Theres a couple of bytes of ram to be shaved
 * 
 * review task priorities
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "buttons.h"
#include "overlay_manager.h"
#include "rtoswrap.h"

//#define LOG_LEVEL RBL_LOG_LEVEL_DEBUG
#define LOG_LEVEL RBL_LOG_LEVEL_NONE
#define MODULE_NAME "buttons"
#define MODULE_TYPE "KERN"

typedef struct ButtonHolder {
    uint8_t button_id;
    ClickConfig click_config;
    uint8_t pressed:1;
    uint8_t did_long_click:1;
    uint8_t is_repeating:1;
    TickType_t last_transition;
    TickType_t last_repeat_time;
    TickType_t press_time;
    uint8_t click_thread_affinity_bitfield; /* track which handler is on which thread. Not super ideal but memory efficient. */
} ButtonHolder;

/* Stack size is tuned with the assumption logging is on/present
 * Stack can be chopped with no logging enabled */

static void _button_thread(void *pvParameters);
THREAD_DEFINE(button, configMINIMAL_STACK_SIZE, tskIDLE_PRIORITY + 5UL, _button_thread);

static ButtonMessage _button_message;
static ButtonHolder _button_holders[NUM_BUTTONS];

void button_send_app_click(ButtonHolder *button, uint8_t bit, void *callback, void *recognizer, void *context);

static void _button_isr(hw_button_t button_id);
static uint8_t _button_pressed(ButtonId button_id);


#define BUTTON_CLICK_THREAD_S_C (1 << 0)
#define BUTTON_CLICK_THREAD_R_C (1 << 1)
#define BUTTON_CLICK_THREAD_M_C (1 << 2)
#define BUTTON_CLICK_THREAD_L_C (1 << 3)
#define BUTTON_CLICK_THREAD_N_C (1 << 4)
#define BUTTON_CLICK_THREAD_D_C (1 << 5)

/*
 * Start the button processor
 */
uint8_t rcore_buttons_init(void)
{
    hw_button_init();

    // Initialise the button click configs
    for (uint8_t i = 0; i < NUM_BUTTONS; i++)
    {
        memset(&_button_holders[i], 0, sizeof(_button_holders[i]));
        _button_holders[i].button_id = i;
    }
    
    THREAD_CREATE(button);
    
    KERN_LOG("buttons", APP_LOG_LEVEL_INFO, "Button Task Created");
    
    return 0;
}

/*
 * Callback function for the button ISR in hardware.
 */
static void _button_isr(hw_button_t /* which is definitionally the same as a ButtonID */ button_id)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    vTaskNotifyGiveFromISR(THREAD_HANDLE(button), &xHigherPriorityTaskWoken);

    /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
    should be performed to ensure the interrupt returns directly to the highest
    priority task.  The macro used for this purpose is dependent on the port in
    use and may be called portEND_SWITCHING_ISR(). */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/*
 * Handle debouncing and the button all at once -- updating buttons whenever
 * we need to, and looking at all of them all the time (rather than trying
 * to divine what happened).
 */
static void _button_thread(void *pvParameters)
{
    /* Don't set up the ISR until we're alive, lest we fall down go boom! */
    hw_button_set_isr(_button_isr);
    
    TickType_t next_wakeup = portMAX_DELAY;
    
    for( ;; )
    {
        /* Wait for a button press (or for it to be time to do useful work). */
        TickType_t curticks = xTaskGetTickCount(); /* avoid TOCTTOU race! */
        if (next_wakeup > curticks) {
            int wait = (next_wakeup == portMAX_DELAY) ? portMAX_DELAY : (next_wakeup - curticks);
            LOG_DEBUG("going to sleep for %d ticks", wait);
            ulTaskNotifyTake(pdTRUE, wait);
        }
        
        next_wakeup = portMAX_DELAY;
        
        /* Rather than trying to be clever, we just scan all of the buttons
         * every time we wake up.  It doesn't cost much CPU time, and it
         * happens relatively infrequently.  */
        for (int btni = 0; btni < NUM_BUTTONS; btni++)
        {
            ButtonHolder *btn = &_button_holders[btni];
            
            /* Has something changed? */
            if ((btn->pressed != hw_button_pressed(btni)) && 
                (xTaskGetTickCount() > (btn->last_transition + butDEBOUNCE_DELAY)))
            {
                btn->pressed = hw_button_pressed(btni);
                btn->last_transition = xTaskGetTickCount();
                
                /* We know there was an edge here -- which was it? */
                if (btn->pressed /* released -> pressed */) {
                    btn->press_time = btn->last_repeat_time = xTaskGetTickCount();
                    btn->did_long_click = 0;
                    btn->is_repeating = 0;
                    LOG_DEBUG("PRESS: for button %d", btn->button_id);

                    /* raw handlers happen, no matter what */
                    if (btn->click_config.raw.down_handler) {
                        LOG_DEBUG("CLICK: raw down for button %d", btn->button_id);
                        button_send_app_click(btn, BUTTON_CLICK_THREAD_D_C, btn->click_config.raw.down_handler, btn, btn->click_config.context);
                    }
                    
                    /* we only greedy-trigger the single-click if we don't
                     * have to wait to decide if it was a long-click */
                    if (btn->click_config.click.handler && !btn->click_config.long_click.handler) {
                        LOG_DEBUG("CLICK: greedy single click for button %d", btn->button_id);
                        button_send_app_click(btn, BUTTON_CLICK_THREAD_S_C, btn->click_config.click.handler, btn, btn->click_config.context);
                    }
                } else /* pressed -> released */ {
                    LOG_DEBUG("RELEASE: for button %d", btn->button_id);

                    /* raw handlers happen, no matter what */
                    if (btn->click_config.raw.up_handler) {
                        LOG_DEBUG("CLICK: raw up for button %d", btn->button_id);
                        button_send_app_click(btn, BUTTON_CLICK_THREAD_N_C, btn->click_config.raw.up_handler, btn, btn->click_config.context);
                    }
                    
                    /* we need to send a long-click xor a short-click */
                    if (btn->click_config.click.handler &&
                        btn->click_config.long_click.handler &&
                        !btn->did_long_click) {
                        LOG_DEBUG("CLICK: single click (for aborted long click) for button %d", btn->button_id);
                        button_send_app_click(btn, BUTTON_CLICK_THREAD_S_C, btn->click_config.click.handler, btn, btn->click_config.context);
                    }
                    
                    if (btn->did_long_click &&
                        btn->click_config.long_click.release_handler) {
                        LOG_DEBUG("CLICK: long click release for button %d", btn->button_id);
                        button_send_app_click(btn, BUTTON_CLICK_THREAD_L_C, btn->click_config.long_click.release_handler, btn, btn->click_config.context);
                    }
                }
            }
            
            /* Now see if there's timed work to do, and update the next time
             * we need to do timed work.  */
             
            /* Waiting for a long press? */
            if (btn->pressed && !btn->did_long_click &&
                btn->click_config.long_click.handler) {
                
                TickType_t lc_wakeup = btn->press_time + pdMS_TO_TICKS(btn->click_config.long_click.delay_ms);
                
                if (xTaskGetTickCount() >= lc_wakeup) {
                    /* it's time */
                    btn->did_long_click = 1;
                    LOG_DEBUG("CLICK: long click for button %d", btn->button_id);
                    button_send_app_click(btn, BUTTON_CLICK_THREAD_L_C, btn->click_config.long_click.handler, btn, btn->click_config.context);
                } else if (lc_wakeup < next_wakeup) {
                    LOG_DEBUG("next wakeup will be for %d msec long-click", btn->click_config.long_click.delay_ms);
                    next_wakeup = lc_wakeup;
                }
            }
            
            /* Waiting for a repeat? */
            if (btn->pressed && 
                btn->click_config.click.handler &&
                btn->click_config.click.repeat_interval_ms) {
                
                TickType_t repeat_wakeup = btn->last_repeat_time + pdMS_TO_TICKS(btn->click_config.click.repeat_interval_ms);
                if (xTaskGetTickCount() >= repeat_wakeup) {
                    /* time for a repeat */
                    btn->is_repeating = 1;
                    btn->last_repeat_time = xTaskGetTickCount();
                    LOG_DEBUG("CLICK: repeat click for button %d", btn->button_id);
                    button_send_app_click(btn, BUTTON_CLICK_THREAD_R_C, btn->click_config.click.handler, btn, btn->click_config.context);
                    repeat_wakeup = btn->last_repeat_time + pdMS_TO_TICKS(btn->click_config.click.repeat_interval_ms);
                }
                
                if (repeat_wakeup < next_wakeup) {
                    LOG_DEBUG("next wakeup will be for %d msec repeat", btn->click_config.click.repeat_interval_ms);
                    next_wakeup = repeat_wakeup;
                }
            }
        }
    }
}

/*
 * Convenience function to send a button click to the main application processor
 */
void button_send_app_click(ButtonHolder *button, uint8_t bit, void *callback, void *recognizer, void *context)
{   
    _button_message.callback = callback;
    _button_message.clickref = recognizer; // TODO
    _button_message.context  = context;

    rcore_backlight_on(100, 3000);

    if (button->click_thread_affinity_bitfield & bit)
        appmanager_post_button_message(&_button_message);
    else
        overlay_window_post_button_message(&_button_message);
}


/*
 * These are the subscription handlers for the button inputs and their various settings
 */


/* When a thread sets a callback, the thread type is tracked in a bitfield
 * this way we can later post it to the correct queue */
void _button_set_is_app_thread_bit(ButtonHolder *button, uint8_t bit)
{
    if (appmanager_is_thread_app())
        button->click_thread_affinity_bitfield |= bit;
    else
        button->click_thread_affinity_bitfield &= ~bit;
}

void button_single_click_subscribe(ButtonId button_id, ClickHandler handler)
{
    if (button_id >= NUM_BUTTONS)
        return;
    
    ButtonHolder *holder = &_button_holders[button_id]; // get the button
    holder->click_config.click.handler = handler;
    holder->click_config.click.repeat_interval_ms = 0;
    
    _button_set_is_app_thread_bit(holder, BUTTON_CLICK_THREAD_S_C);
}

void button_single_repeating_click_subscribe(ButtonId button_id, uint16_t repeat_interval_ms, ClickHandler handler)
{
    if (button_id >= NUM_BUTTONS)
        return;
    
    ButtonHolder *holder = &_button_holders[button_id]; // get the button
    holder->click_config.click.handler = handler;
    holder->click_config.click.repeat_interval_ms = repeat_interval_ms;
    
    _button_set_is_app_thread_bit(holder, BUTTON_CLICK_THREAD_R_C);
}

void button_multi_click_subscribe(ButtonId button_id, uint8_t min_clicks, uint8_t max_clicks, uint16_t timeout, bool last_click_only, ClickHandler handler)
{
    if (button_id >= NUM_BUTTONS)
        return;

    // XXX Todo
}

void button_long_click_subscribe(ButtonId button_id, uint16_t delay_ms, ClickHandler down_handler, ClickHandler up_handler)
{
    if (button_id >= NUM_BUTTONS)
        return;
    
    ButtonHolder *holder = &_button_holders[button_id]; // get the button
    holder->click_config.long_click.handler = down_handler;
    holder->click_config.long_click.release_handler = up_handler;
    holder->click_config.long_click.delay_ms = delay_ms;
    
    _button_set_is_app_thread_bit(holder, BUTTON_CLICK_THREAD_L_C);
}

void button_raw_click_subscribe(ButtonId button_id, ClickHandler down_handler, ClickHandler up_handler, void * context)
{
    if (button_id >= NUM_BUTTONS)
        return;
    
    ButtonHolder *holder = &_button_holders[button_id]; // get the button
    holder->click_config.raw.up_handler = up_handler;
    holder->click_config.raw.down_handler = down_handler;
    if (context != NULL)
        holder->click_config.context = context;
    
    _button_set_is_app_thread_bit(holder, BUTTON_CLICK_THREAD_N_C);
}


void button_unsubscribe_all(void)
{
    for(uint8_t i = 0; i < NUM_BUTTONS; i++)
    {
        ButtonHolder *holder = &_button_holders[i]; // get the button
        memset(holder, 0, sizeof(ButtonHolder));
        holder->button_id = i;
    }
}

void button_set_click_context(ButtonId button_id, void *context)
{
    if (button_id >= NUM_BUTTONS)
        return;

    _button_holders[button_id].click_config.context = context;
}

uint8_t button_short_click_is_subscribed(ButtonId button_id)
{
    if (button_id >= NUM_BUTTONS)
        return 0;
    
    ButtonHolder *holder = &_button_holders[button_id];
    
    return holder->click_config.click.handler != NULL;
}

bool click_recognizer_is_repeating(ClickRecognizerRef recognizer)
{
    return ((ButtonHolder *)recognizer)->is_repeating;
}

ButtonId click_recognizer_get_button_id(ClickRecognizerRef recognizer)
{
    return ((ButtonHolder *)recognizer)->button_id;
}