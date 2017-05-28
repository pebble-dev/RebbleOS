#pragma once
/* driver.c
 * Driver management. Drivers are loaded and managed indirectly through
 * function pointers and callbacks. This decouples the driver core from the 
 * kernel driver and provides a hard API for drivers
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#define MAX_MODULE_NAME_LENGTH 15

/*
 * A list of loadable resources drivers may request
 */
typedef enum hw_resources_t {
    HW_RESOURCE_FPGA,
    MAX_HW_RESOURCES,
} hw_resources_t;

// The entry point for the modules callback configuration
typedef void *(*hw_driver_module_init_t)(void *handler);

// some prototype shortcuts for the various callback types
typedef void (*hw_driver_void_t)(void);
typedef int (*hw_driver_int_t)(void);
typedef uint8_t *(*hw_driver_puint8_t)(void);

/*
 * Common settings for a driver such as init, deinit and name
 */
typedef struct driver_common_t {
    char *module_name;
    hw_driver_void_t init;
    hw_driver_void_t deinit;
    hw_driver_int_t  test;
} driver_common_t;

// function prototype for loading a resource from a driver
typedef int (*hw_driver_bin_callback_t)(hw_resources_t resource_id, void *buffer, size_t offset, size_t sz);
// prototype for an ISR within a driver
typedef void (*hw_driver_isr_callback_t)(uint8_t cmd);

/*
 * These are passed to a driver on module configuration
 * The driver uses these as pointers back into the kernel
 * It is our kernel API for drivers
 */
typedef struct hw_driver_handler_t {
    hw_driver_bin_callback_t request_resource;
    hw_driver_isr_callback_t done_isr;
} hw_driver_handler_t;

// prototype for a function that knows how to load resources
typedef int (*driver_resource_loader_t)(hw_resources_t resource_id, uint8_t *buffer, size_t offset, size_t sz);

// A storage mecahanism for the driver use when loading resources
typedef struct driver_resource_t {
    hw_resources_t id;
    size_t size;
    driver_resource_loader_t resource_loader;
} driver_resource_t;


// Function prototypes
void *driver_register(hw_driver_module_init_t entry_point, hw_driver_handler_t *callbacks);
void driver_register_resource(hw_resources_t resource_id, size_t resource_size, void *resource_loader_handle);
int driver_load_resource(hw_resources_t resource_id, void *buffer, size_t offset, size_t sz);
void driver_deinit_driver(driver_common_t *driver);
