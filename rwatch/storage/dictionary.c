/* dictionary.c
 * Data dictionary and Tuple serialization routines
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */
/*
  Dictionary Header is 1 byte. This could be a flag store
   (read/write mode protection) or this could be a counter.
  for simplicity sake, I have left it as a counter.
  probably should be a status header  
*/

#include "rebbleos.h"
#include "dictionary.h"

uint32_t dict_calc_buffer_size(const uint8_t tuple_count, ...)
{
    if (!tuple_count)
        return 0;

    va_list l;
    va_start(l, tuple_count);
    int32_t total = 0;

    for (int i = 0; i < tuple_count; i++) {
        uint32_t t = va_arg(l, uint32_t);
        total += t + sizeof(Tuple);
    }
    va_end(l);

    return total + 1;
}

uint32_t dict_size(DictionaryIterator* iter)
{
    if (!iter)
        return 0;
    
    Tuple *t = dict_read_first(iter);

    uint32_t total = 0;
    for (int i = 0; i < iter->dictionary->entry_count; i++) {
        if (!t)
            return 0;
        
        total += sizeof(Tuple) + t->length;
        Tuple *t = dict_read_next(iter);
    }

    return total + 1;
}

DictionaryResult dict_write_begin(DictionaryIterator *iter, uint8_t * const buffer, const uint16_t size)
{
    if (!iter || !buffer || !size)
        return DICT_INVALID_ARGS;

    memset(buffer, 0, size);
    iter->dictionary = (Dictionary *)buffer;
    iter->end = (void*)iter->dictionary->data + size;
    iter->cursor = (Tuple *)iter->dictionary->data;

    return DICT_OK;
}

static DictionaryResult _dict_write(DictionaryIterator *iter, const uint32_t key, const uint8_t * const data, const uint16_t size, uint8_t dict_type)
{
    if (!iter || !iter->dictionary || !data || !size)
        return DICT_INVALID_ARGS;
    
    if (iter->dictionary->entry_count == 255)
        return DICT_NOT_ENOUGH_STORAGE;

    if ((void *)iter->cursor + size + sizeof(Tuple) > 
            (void *)iter->end)
        return DICT_NOT_ENOUGH_STORAGE;

    Tuple tuple = {
        .key = key,
        .type = dict_type,
        .length = size
    };
    
    memcpy((void *)iter->cursor, &tuple, sizeof(Tuple));
    memcpy((void *)iter->cursor + sizeof(Tuple), data, size);
    iter->cursor = (void *)iter->cursor + sizeof(Tuple) + size;
    iter->dictionary->entry_count++;
    
    return DICT_OK;
}

DictionaryResult dict_write_data(DictionaryIterator *iter, const uint32_t key, const uint8_t * const data, const uint16_t size)
{
    return _dict_write(iter, key, data, size, TUPLE_BYTE_ARRAY);
}

DictionaryResult dict_write_cstring(DictionaryIterator *iter, const uint32_t key, const char * const cstring)
{
    if (!cstring)
        return DICT_INVALID_ARGS;
    if (!strlen(cstring))
        return DICT_INVALID_ARGS;
    return _dict_write(iter, key, (uint8_t *)cstring, strlen(cstring) + 1, TUPLE_CSTRING);
}

DictionaryResult dict_write_int(DictionaryIterator *iter, const uint32_t key, const void *integer, const uint8_t width_bytes, const bool is_signed)
{
    /* must be 1,2 or 4 int8, int16, int32 */
    if (width_bytes == 0 || width_bytes > 4 || width_bytes == 3 || !integer)
        return DICT_INVALID_ARGS;

    return _dict_write(iter, key, (uint8_t *)integer, width_bytes, is_signed ? TUPLE_INT : TUPLE_UINT);
}

DictionaryResult dict_write_uint8(DictionaryIterator *iter, const uint32_t key, const uint8_t value)
{
    return _dict_write(iter, key, (uint8_t *)&value, 1, TUPLE_UINT);
}

DictionaryResult dict_write_uint16(DictionaryIterator *iter, const uint32_t key, const uint16_t value)
{
    return _dict_write(iter, key, (uint8_t *)&value, 2, TUPLE_UINT);
}

DictionaryResult dict_write_uint32(DictionaryIterator *iter, const uint32_t key, const uint32_t value)
{
    return _dict_write(iter, key, (uint8_t *)&value, 4, TUPLE_UINT);
}

DictionaryResult dict_write_int8(DictionaryIterator *iter, const uint32_t key, const int8_t value)
{
    return _dict_write(iter, key, (uint8_t *)&value, 1, TUPLE_INT);
}

DictionaryResult dict_write_int16(DictionaryIterator *iter, const uint32_t key, const int16_t value)
{
    return _dict_write(iter, key, (uint8_t *)&value, 2, TUPLE_INT);
}

DictionaryResult dict_write_int32(DictionaryIterator *iter, const uint32_t key, const int32_t value)
{
    return _dict_write(iter, key, (uint8_t *)&value, 4, TUPLE_INT);
}

uint32_t dict_write_end(DictionaryIterator *iter)
{
    if (!iter || !iter->dictionary)
        return 0;
    
    uint32_t written = (uint32_t)((void *)iter->cursor - (void *)iter->dictionary->data);
    iter->end = (void *)iter->cursor;
    iter->cursor = (Tuple *)iter->dictionary->data;

    return written;
}

Tuple *dict_read_next(DictionaryIterator *iter)
{
    if (!iter || !iter->dictionary)
        return NULL;
    
    Tuple *t = iter->cursor;

    if ((void *)t + sizeof(Tuple) >= (void *)iter->end)
        return NULL;
    
    iter->cursor = (void *)iter->cursor + iter->cursor->length + sizeof(Tuple);    
    
    return t;
}

Tuple *dict_read_first(DictionaryIterator *iter)
{
    if (!iter || !iter->dictionary)
        return NULL;
    
    iter->cursor = (Tuple *)iter->dictionary->data;
    dict_read_next(iter);

    return (Tuple *)iter->dictionary->data;
}

Tuple *dict_read_begin_from_buffer(DictionaryIterator *iter, 
    const uint8_t *const buffer, const uint16_t size)
{
    if (!iter || !iter->dictionary || !buffer || !size)
        return NULL;

    iter->dictionary = (Dictionary *)buffer;
    iter->cursor = (Tuple *)iter->dictionary->data;
    iter->end = (void*)iter->dictionary->data + size;

    return dict_read_first(iter);
}




DictionaryResult dict_serialize_tuplets(DictionarySerializeCallback callback, void *context, const Tuplet * const tuplets, const uint8_t tuplets_count)
{
    if (!callback || !tuplets || !tuplets_count)
        return DICT_INVALID_ARGS;

    uint32_t sz = dict_calc_buffer_size_from_tuplets(tuplets, tuplets_count);

    if (!sz)
        return DICT_INTERNAL_INCONSISTENCY;
    
    uint8_t buf[sz];
    int rv = dict_serialize_tuplets_to_buffer(tuplets, tuplets_count, buf, &sz);
    callback(buf, sz, context);

    return rv;
}

DictionaryResult dict_serialize_tuplets_to_buffer(const Tuplet * const tuplets, const uint8_t tuplets_count, uint8_t *buffer, uint32_t *size_in_out)
{
    if (!tuplets || !tuplets_count || !buffer || !*size_in_out)
        return DICT_INVALID_ARGS;

    DictionaryIterator iter;
    return dict_serialize_tuplets_to_buffer_with_iter(&iter, tuplets, tuplets_count, buffer, size_in_out);
}

DictionaryResult dict_serialize_tuplets_to_buffer_with_iter(DictionaryIterator *iter, const Tuplet * const tuplets, const uint8_t tuplets_count, uint8_t *buffer, uint32_t *size_in_out)
{
    if (!iter || !iter->dictionary || !tuplets || !tuplets_count || !buffer || !*size_in_out)
        return DICT_INVALID_ARGS;

    uint8_t cnt = tuplets_count;
    dict_write_begin(iter, buffer, *size_in_out);
    
    const Tuplet * t = tuplets;
    while(cnt--) {
        uint8_t *val;
        if (t->type == TUPLE_UINT || t->type == TUPLE_INT) 
            val = (uint8_t *)&t->integer.storage;
        else
            val = (uint8_t *)t->bytes.data;
        
        if (_dict_write(iter, t->key, 
                val, 
                t->bytes.length, t->type) != DICT_OK)
            return DICT_NOT_ENOUGH_STORAGE;
        
        t++;
    }

    *size_in_out = dict_write_end(iter);

    return DICT_OK;
}

DictionaryResult dict_write_tuplet(DictionaryIterator *iter, const Tuplet * const tuplet)
{
    if (!iter || !iter->dictionary || !tuplet)
        return DICT_INVALID_ARGS;
        
    return _dict_write(iter, tuplet->key, tuplet->bytes.data, tuplet->bytes.length, tuplet->type);
}

uint32_t dict_calc_buffer_size_from_tuplets(const Tuplet * const tuplets, const uint8_t tuplets_count)
{
    if (!tuplets || !tuplets_count)
        return 0;
    
    Tuplet *t = (Tuplet *)tuplets;
    uint32_t size = 0;
    uint8_t cnt = tuplets_count;
    while(cnt--) {
        size += sizeof(Tuple) + t->bytes.length;
        t++;
    }

    return size + 1;
}

DictionaryResult dict_merge(DictionaryIterator *dest, uint32_t *dest_max_size_in_out,
                             DictionaryIterator *source,
                             const bool update_existing_keys_only,
                             const DictionaryKeyUpdatedCallback key_callback, void *context)
{
    Tuple *dt; 
    Tuple *st;
    int rv = DICT_OK;
    DictionaryIterator dest_copy;
    /* pebble uses a stack alloc copy of the data to do this */
    uint8_t buf_copy[*dest_max_size_in_out];
    
    /* copy the dest */
    rv = dict_write_begin(&dest_copy, buf_copy, *dest_max_size_in_out);
    if (rv != DICT_OK)
        goto done;

    dt = dict_read_first(dest);

    while(dt) {
        rv = _dict_write(&dest_copy, dt->key, dt->value->data, dt->length, dt->type);
        if (rv != DICT_OK)
            goto done;
        
        dt = dict_read_next(dest);
    }
    rv = dict_write_end(&dest_copy);    
    if (!rv)
        goto done;
    
    st = dict_read_first(source);

    rv = dict_write_begin(dest, (uint8_t * const)dest->dictionary, *dest_max_size_in_out);
    if (rv != DICT_OK)
        goto done;
    
    /* copy in all dest
        - check against source
        - if in source, update from source
        - else update from copy
       if not just updating existing
        - copy in remaining source
    */
   dt = dict_read_first(&dest_copy);

    /* write in the dests */
    while(dt) {
        st = dict_find(source, dt->key);

        if (st) {
            rv = _dict_write(dest, st->key, st->value->data, st->length, st->type);
        } else {
            rv = _dict_write(dest, dt->key, dt->value->data, dt->length, dt->type);
        }

        if (rv != DICT_OK)
            goto done;

        if (key_callback)
            key_callback(dt->key, st, dt, context);
        dt = dict_read_next(&dest_copy);
    }

    if (update_existing_keys_only) {
        dict_write_end(dest);
        goto done;
    }

    /* write in all the remaining sources */
    st = dict_read_first(source);
    while (st) {
        rv = _dict_write(dest, st->key, st->value->data, st->length, st->type);
        if (rv != DICT_OK)
            goto done;

        if (key_callback)
            key_callback(st->key, st, NULL, context);
        
        st = dict_read_next(source);
    }
    dict_write_end(dest);

done:
    return rv;
}

Tuple *dict_find(const DictionaryIterator *iter, const uint32_t key)
{
    if (!iter || !iter->dictionary)
        return NULL;
    
    Tuple *t = dict_read_first((DictionaryIterator *)iter);
    
    while(t) {
        if (t->key == key)
            return t;
        t = dict_read_next((DictionaryIterator *)iter);
    }

    return NULL;
}

#ifdef REBBLEOS_TESTING
#include "test.h"
TEST(dictionary)
{
    static const uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    static const char *string = "Hello World";
    
    const uint32_t size = dict_calc_buffer_size(8, sizeof(data),
                                        strlen(string) + 1, 1, 2, 4, 1, 2, 4);

    uint8_t buf1[size];
    DictionaryIterator iter;

    dict_write_begin(&iter, buf1, sizeof(buf1));
    dict_write_data(&iter, 1, data, sizeof(data));
    dict_write_cstring(&iter, 2, string);
    dict_write_int8(&iter, 3, -10);
    dict_write_int16(&iter, 4, -1024);
    dict_write_int32(&iter, 5, -32000);
    dict_write_uint8(&iter, 6, 255);
    dict_write_uint16(&iter, 7, 1024);
    dict_write_uint32(&iter, 8, 32000);
    const uint32_t final_size = dict_write_end(&iter);

    Tuple *tuple = dict_read_begin_from_buffer(&iter, buf1, final_size);

    while (tuple) {
        if (tuple->key == 1 && memcmp(data, tuple->value->data, tuple->value->length) != 0) {
            return TEST_FAIL;
        }
        else if (tuple->key == 2 && strcmp(data, tuple->value->cstring) != 0) {
            return TEST_FAIL;
        }
        else if (tuple->key == 3 && tuple->value->int8 != -10) {
            return TEST_FAIL;
        }
        else if (tuple->key == 4 && tuple->value->int16 != -1024) {
            return TEST_FAIL;
        }
        else if (tuple->key == 5 && tuple->value->int32 != -32000) {
            return TEST_FAIL;
        }
        else if (tuple->key == 6 && tuple->value->uint8 != 255) {
            return TEST_FAIL;
        }
        else if (tuple->key == 7 && tuple->value->uint16 != 1024) {
            return TEST_FAIL;
        }
        else if (tuple->key == 8 && tuple->value->uint32 != 32000) {
            return TEST_FAIL;
        }
        
        tuple = dict_read_next(&iter);
    }
  
    /*  Test tuplets */
    DictionaryIterator iter2;

    Tuplet pairs[] = {
        TupletInteger(100, (int8_t) -50),
        TupletInteger(101, (int16_t) -1000),
        TupletInteger(102, (int32_t) -10000),
        TupletCString(103, "RebbleOS Rocks"),
        TupletInteger(8, (uint32_t) 32065),
    };

    uint8_t buf2[256];
    uint32_t sz = sizeof(buf2);
    dict_serialize_tuplets_to_buffer(pairs, 2, buf2, &sz);


    tuple = dict_read_begin_from_buffer(&iter2, buf2, sz);
    while (tuple) {
        else if(tuple->key == 100 && tuple->value->int8 != -50) {
            return TEST_FAIL;
        }
        else if(tuple->key == 101 && tuple->value->int16 != -1000) {
            return TEST_FAIL;
        }
        else if(tuple->key == 102 && tuple->value->cstring != -10000) {
            return TEST_FAIL;
        }
        else if (tuple->key == 103 && strcmp("RebbleOS Rocks", tuple->value->cstring) != 0) {
            return TEST_FAIL;
        }
        
        tuple = dict_read_next(&iter2);
    }

    /* dict merge test */
    uint32_t nsize = final_size + sz;
    dict_merge(&iter2, &nsize, &iter, false, NULL, NULL);

    tuple = dict_read_first(&iter2);

    int count = 0;
    while(tuple) {
        count++;
        if (tuple->key == 8 && tuple->value->uint32 != 32065) {
            return TEST_FAIL;
        }
        tuple = dict_read_next(&iter2);
    }

    if (count != 13) {
        return TEST_FAIL;
    }

    *artifact = 0;
    return TEST_PASS;
}
#endif