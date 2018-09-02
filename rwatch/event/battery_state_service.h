/* routines that implement the PebbleOS battery api
*/

/**
 * @brief Battery charge and level state
 */
typedef struct {
  uint8_t charge_percent;  /* A percentage (0-100) of how full the battery is. */
  bool is_charging;        /* True if the battery is currently being charged. False if not. */
  bool is_plugged;         /* True if the charger cable is connected. False if not. */
} BatteryChargeState;

/**
 * @brief Callback type for battery state change events.
 * 
 * @param charge the state of the battery @ref BatteryChargeState
 */
typedef void(* BatteryStateHandler)(BatteryChargeState charge);

/**
 * @brief Peek at the last known battery state.
 * 
 * @returns a @ref BatteryChargeState containing the last known data 
 */
BatteryChargeState battery_state_service_peek(void);

/**
 * @brief Subscribe to the battery state event service. Once subscribed, the handler gets called on every battery state change. 
 * 
 * @param handler A callback to be executed on battery state change event
 */
void battery_state_service_subscribe(BatteryStateHandler handler);

/**
 * @brief Unsubscribe from the battery state event service. Once unsubscribed, the previously registered handler will no longer be called.
 */
void battery_state_service_unsubscribe(void);
