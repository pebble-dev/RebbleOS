#pragma once
/**
 * @file notification_message_c.h
 * @author Barry Carter
 * @date 13 Feb 2018
 * @brief Notification message storage. This is a database in the absense of flash write
 *
 * Routines for creating and managing messages. We have a special message heap allocator
 * 
 */

void messages_init(void);
list_head *message_get_head(void);

/**
 * @brief Allocate memory in the messages heap
 * 
 * @param count number of \param size units to allocate
 * @param size size_t of how much to allocate
 */
void *noty_calloc(size_t count, size_t size);

/**
 * @brief Free memory on the message heap
 * 
 * @param mem pointer to the memory in message heap to free
 */
void noty_free(void *mem);

/**
 * @brief Return a count of the messages in the list
 * 
 * @return count of messages
 */
uint16_t message_count(void);

/**
 * @brief Add \ref full_msg_t message to the message stack
 * 
 * @param full_msg_t the pebble message to add
 */
void message_add(full_msg_t *msg);
