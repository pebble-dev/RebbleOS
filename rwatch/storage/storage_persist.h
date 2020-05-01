#pragma once


//! @addtogroup Storage
//! \brief A mechanism to store persistent application data and state
//!
//! The Persistent Storage API provides you with a mechanism for performing a variety of tasks,
//! like saving user settings, caching data from the phone app, or counting high scores for
//! Pebble watchapp games.
//!
//! In Pebble OS, storage is defined by a collection of fields that you can create, modify or delete.
//! In the API, a field is specified as a key with a corresponding value.
//!
//! Using the Storage API, every app is able to get its own persistent storage space. Each value
//! in that space is associated with a uint32_t key.
//!
//! Storage supports saving integers, strings and byte arrays. The maximum size of byte arrays and
//! strings is defined by PERSIST_DATA_MAX_LENGTH (currently set to 256 bytes). You call the function
//! persist_exists(key), which returns a boolean indicating if the key exists or not.
//! The Storage API enables your app to save its state, and when compared to using \ref AppMessage to
//! retrieve values from the phone, it provides you with a much faster way to restore state.
//! In addition, it draws less power from the battery.
//!
//! Note that the size of all persisted values cannot exceed 4K per app.
//! @{

//! The maximum size of a persist value in bytes
#define PERSIST_DATA_MAX_LENGTH 256

//! The maximum size of a persist string in bytes including the NULL terminator
#define PERSIST_STRING_MAX_LENGTH PERSIST_DATA_MAX_LENGTH

//! Status codes. See \ref status_t
typedef enum StatusCode {
  //! Operation completed successfully.
  S_SUCCESS = 0,

  //! An error occurred (no description).
  E_ERROR = -1,

  //! No idea what went wrong.
  E_UNKNOWN = -2,

  //! There was a generic internal logic error.
  E_INTERNAL = -3,

  //! The function was not called correctly.
  E_INVALID_ARGUMENT = -4,

  //! Insufficient allocatable memory available.
  E_OUT_OF_MEMORY = -5,

  //! Insufficient long-term storage available.
  E_OUT_OF_STORAGE = -6,

  //! Insufficient resources available.
  E_OUT_OF_RESOURCES = -7,

  //! Argument out of range (may be dynamic).
  E_RANGE = -8,

  //! Target of operation does not exist.
  E_DOES_NOT_EXIST = -9,

  //! Operation not allowed (may depend on state).
  E_INVALID_OPERATION = -10,

  //! Another operation prevented this one.
  E_BUSY = -11,

  //! Operation not completed; try again.
  E_AGAIN = -12,

  //! Equivalent of boolean true.
  S_TRUE = 1,

  //! Equivalent of boolean false.
  S_FALSE = 0,

  //! For list-style requests.  At end of list.
  S_NO_MORE_ITEMS = 2,

  //! No action was taken as none was required.
  S_NO_ACTION_REQUIRED = 3,

} StatusCode;

//! Return value for system operations. See \ref StatusCode for possible values.
typedef int32_t status_t;

//! Checks whether a value has been set for a given key in persistent storage.
//! @param key The key of the field to check.
//! @return true if a value exists, otherwise false.
bool persist_exists(const uint32_t key);

//! Gets the size of a value for a given key in persistent storage.
//! @param key The key of the field to lookup the data size.
//! @return The size of the value in bytes or \ref E_DOES_NOT_EXIST
//! if there is no field matching the given key.
int persist_get_size(const uint32_t key);

//! Reads a bool value for a given key from persistent storage.
//! If the value has not yet been set, this will return false.
//! @param key The key of the field to read from.
//! @return The bool value of the key to read from.
bool persist_read_bool(const uint32_t key);

//! Reads an int value for a given key from persistent storage.
//! @note The int is a signed 32-bit integer.
//! If the value has not yet been set, this will return 0.
//! @param key The key of the field to read from.
//! @return The int value of the key to read from.
int32_t persist_read_int(const uint32_t key);

//! Reads a blob of data for a given key from persistent storage into a given buffer.
//! If the value has not yet been set, the given buffer is left unchanged.
//! @param key The key of the field to read from.
//! @param buffer The pointer to a buffer to be written to.
//! @param buffer_size The maximum size of the given buffer.
//! @return The number of bytes written into the buffer or \ref E_DOES_NOT_EXIST
//! if there is no field matching the given key.
int persist_read_data(const uint32_t key, void *buffer, const size_t buffer_size);

//! Reads a string for a given key from persistent storage into a given buffer.
//! The string will be null terminated.
//! If the value has not yet been set, the given buffer is left unchanged.
//! @param key The key of the field to read from.
//! @param buffer The pointer to a buffer to be written to.
//! @param buffer_size The maximum size of the given buffer. This includes the null character.
//! @return The number of bytes written into the buffer or \ref E_DOES_NOT_EXIST
//! if there is no field matching the given key.
int persist_read_string(const uint32_t key, char *buffer, const size_t buffer_size);

//! Writes a bool value flag for a given key into persistent storage.
//! @param key The key of the field to write to.
//! @param value The boolean value to write.
//! @return The number of bytes written if successful, a value from \ref StatusCode otherwise.
status_t persist_write_bool(const uint32_t key, const bool value);

//! Writes an int value for a given key into persistent storage.
//! @note The int is a signed 32-bit integer.
//! @param key The key of the field to write to.
//! @param value The int value to write.
//! @return The number of bytes written if successful, a value from \ref StatusCode otherwise.
status_t persist_write_int(const uint32_t key, const int32_t value);

//! Writes a blob of data of a specified size in bytes for a given key into persistent storage.
//! The maximum size is \ref PERSIST_DATA_MAX_LENGTH
//! @param key The key of the field to write to.
//! @param data The pointer to the blob of data.
//! @param size The size in bytes.
//! @return The number of bytes written if successful, a value from \ref StatusCode otherwise.
int persist_write_data(const uint32_t key, const void *data, const size_t size);

//! Writes a string a given key into persistent storage.
//! The maximum size is \ref PERSIST_STRING_MAX_LENGTH including the null terminator.
//! @param key The key of the field to write to.
//! @param cstring The pointer to null terminated string.
//! @return The number of bytes written if successful, a value from \ref StatusCode otherwise.
int persist_write_string(const uint32_t key, const char *cstring);

//! Deletes the value of a key from persistent storage.
//! @param key The key of the field to delete from.
status_t persist_delete(const uint32_t key);






status_t persist_write(const uint32_t key, const void *data, size_t size);
status_t persist_read(const uint32_t key, const void *buffer, const size_t size);
