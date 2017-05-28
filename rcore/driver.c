/* driver.c
 * Driver management. Drivers are loaded and managed indirectly through
 * function pointers and callbacks. This decouples the driver core from the 
 * kernel driver and provides a hard API for drivers
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"
#include "driver.h"

/*

// example module
init(){
   driver_register_display(&hw_display_driver_init, &callbacks);
...
}


// module driver
hw_driver_xxxx_t _hw_xxxx_driver =  {
    .common_info.module_name = "My Driver",
    .common_info.init = hw_xxxx_init,
    .common_info.deinit = hw_xxxx_deinit,
    .my_func = hw_xxxxx_myfunc,
    ...
};

hw_driver_xxxx_t *hw_xxxx_driver_init(hw_driver_handler_t *handler...) {
    _handler = handler;
    return &_hw_display_driver;
}
*/


// prv
static void _driver_init_driver_common(void *v, hw_driver_handler_t *callbacks);

// vars
static driver_resource_t _driver_resources[MAX_HW_RESOURCES];

/*
 * Driver registertation. The driver will call this on init with a pointer to the setup function
 * The driver must then populate the function with its pointers to this API
 * In turn the driver must also give us a callback array with any isr/callbacks it defines
 */
void *driver_register(hw_driver_module_init_t entry_point, hw_driver_handler_t *callbacks)
{
    assert(callbacks && "Callbacks cannot be NULL");
    
    // call into the module to let it init
    uint32_t *v = (void *)(*(entry_point))(callbacks);
    _driver_init_driver_common(v, callbacks);
    
    return v;
}

/*
 * Register a resource binary with the driver loader. This will be generally available as a 
 * binary blob that a driver needs, such as the FPGA from flash, but it could also be a splash...
 * 
 */
void driver_register_resource(hw_resources_t resource_id, size_t resource_size, void *resource_loader_handle)
{
    _driver_resources[resource_id].id = resource_id;
    _driver_resources[resource_id].size = resource_size;
    _driver_resources[resource_id].resource_loader = resource_loader_handle;
}

/*
 * A function wrapper that allows a driver to ask for any named resource ID
 * The resource will be fetched by the entries resource loader pointer
 * 
 */
int driver_load_resource(hw_resources_t resource_id, void *buffer, size_t offset, size_t sz)
{
    assert(!(resource_id > MAX_HW_RESOURCES) && "Invalid hw bin resource");
    
    // go and get the actual implementation for this driver
    driver_resource_t *drv = &_driver_resources[resource_id];
    assert(drv != NULL && "Resource loader is NULL");
    assert(!(sz > drv->size) && "Requested hw load size > hw resource size");
    assert(drv->resource_loader && "Resource Loader address is not valid");
    
    return drv->resource_loader(resource_id, buffer, offset, sz);
}

/*
 * Deinit a driver by calling its deinit routine
 */
void driver_deinit_driver(driver_common_t *driver)
{
    if (driver->deinit)
        driver->deinit();
}


// Init the driver module and call into its init
static void _driver_init_driver_common(void *v, hw_driver_handler_t *callbacks)
{
    driver_common_t *drv = (driver_common_t *)v;
    KERN_LOG("Driver", APP_LOG_LEVEL_INFO, "Registered Driver: %s", drv->module_name);
    
    // inject our common requests such as bin loading
    callbacks->request_resource = driver_load_resource;
    
    // now call the modules real init
    drv->init();
    
    // some modules do a test
    if (drv->test)
        drv->test();
}
