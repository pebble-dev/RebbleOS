#ifndef __GRAPHICS_RES_H__
#define __GRAPHICS_RES_H__


typedef struct ResHandle
{
    void *a;
} ResHandle;

ResHandle resource_get_handle(uint16_t resource_id);
#endif
