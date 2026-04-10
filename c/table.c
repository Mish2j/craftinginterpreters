//> Hash Tables table-c
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

//> max-load
#define TABLE_MAX_LOAD 0.75

//< max-load

static bool isEmptyEntry(Entry* entry) {
  return IS_BOOL(entry->key) && !AS_BOOL(entry->key) && IS_NIL(entry->value);
}

static bool isTombstoneEntry(Entry* entry) {
  return IS_BOOL(entry->key) && AS_BOOL(entry->key) && IS_NIL(entry->value);
}

static uint32_t hashBits(uint64_t bits) {
  bits ^= bits >> 33;
  bits *= 0xff51afd7ed558ccdULL;
  bits ^= bits >> 33;
  bits *= 0xc4ceb9fe1a85ec53ULL;
  bits ^= bits >> 33;
  return (uint32_t)bits;
}

static uint32_t hashNumber(double num) {
  if (num == 0.0) num = 0.0;

  union {
    double num;
    uint64_t bits;
  } data;
  data.num = num;

  return hashBits(data.bits);
}

static uint32_t hashValue(Value key) {
  if (IS_BOOL(key)) {
    return AS_BOOL(key) ? 3 : 1;
  }

  if (IS_NIL(key)) {
    return 2;
  }

  if (IS_NUMBER(key)) {
    double num = AS_NUMBER(key);

    // Normalize -0.0 and 0.0
    if (num == 0.0) num = 0.0;

    union {
      double num;
      uint64_t bits;
    } data;

    data.num = num;

    return hashBits(data.bits);
  }

  if (IS_OBJ(key)) {
    Obj* object = AS_OBJ(key);

    if (object->type == OBJ_STRING) {
      return AS_STRING(key)->hash;
    }

    // identity-based fallback
    return hashBits((uint64_t)(uintptr_t)object);
  }

  return 0;
}

void initTable(Table* table) {
  table->count = 0;
  table->capacity = 0;
  table->entries = NULL;
}
//> free-table
void freeTable(Table* table) {
  FREE_ARRAY(Entry, table->entries, table->capacity);
  initTable(table);
}
//< free-table
//> find-entry
//> omit
// NOTE: The "Optimization" chapter has a manual copy of this function.
// If you change it here, make sure to update that copy.
//< omit
static Entry* findEntry(Entry* entries, int capacity,
                        Value key) {
/* Hash Tables find-entry < Optimization initial-index
  uint32_t index = key->hash % capacity;
*/
//> Optimization initial-index
  uint32_t index = hashValue(key) & (capacity - 1);
//< Optimization initial-index
//> find-entry-tombstone
  Entry* tombstone = NULL;
  
//< find-entry-tombstone
  for (;;) {
    Entry* entry = &entries[index];
/* Hash Tables find-entry < Hash Tables find-tombstone
    if (entry->key == key || entry->key == NULL) {
      return entry;
    }
*/
//> find-tombstone
   if (isEmptyEntry(entry)) {
      return tombstone != NULL ? tombstone : entry;
    } else if (isTombstoneEntry(entry)) {
      if (tombstone == NULL) tombstone = entry;
    } else if (valuesEqual(entry->key, key)) {
      return entry;
    }
//< find-tombstone

/* Hash Tables find-entry < Optimization next-index
    index = (index + 1) % capacity;
*/
//> Optimization next-index
    index = (index + 1) & (capacity - 1);
//< Optimization next-index
  }
}
//< find-entry
//> table-get
bool tableGet(Table* table, Value key, Value* value) {
  if (table->count == 0) return false;

  Entry* entry = findEntry(table->entries, table->capacity, key);
  if (isEmptyEntry(entry) || isTombstoneEntry(entry)) return false;

  *value = entry->value;
  return true;
}
//< table-get
//> table-adjust-capacity
static void adjustCapacity(Table* table, int capacity) {
  Entry* entries = ALLOCATE(Entry, capacity);
  for (int i = 0; i < capacity; i++) {
    entries[i].key = BOOL_VAL(false);
    entries[i].value = NIL_VAL;
  }
//> re-hash

//> resize-init-count
  table->count = 0;
//< resize-init-count
  for (int i = 0; i < table->capacity; i++) {
    Entry* entry = &table->entries[i];
    if (isEmptyEntry(entry) || isTombstoneEntry(entry)) continue;

    Entry* dest = findEntry(entries, capacity, entry->key);
    dest->key = entry->key;
    dest->value = entry->value;
//> resize-increment-count
    table->count++;
//< resize-increment-count
  }
//< re-hash

//> Hash Tables free-old-array
  FREE_ARRAY(Entry, table->entries, table->capacity);
//< Hash Tables free-old-array
  table->entries = entries;
  table->capacity = capacity;
}
//< table-adjust-capacity
//> table-set
bool tableSet(Table* table, Value key, Value value) {
//> table-set-grow
  if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
    int capacity = GROW_CAPACITY(table->capacity);
    adjustCapacity(table, capacity);
  }

//< table-set-grow
  Entry* entry = findEntry(table->entries, table->capacity, key);
  bool isNewKey = isEmptyEntry(entry) || isTombstoneEntry(entry);
/* Hash Tables table-set < Hash Tables set-increment-count
  if (isNewKey) table->count++;
*/
//> set-increment-count
  if (isNewKey && isEmptyEntry(entry)) table->count++;
//< set-increment-count

  entry->key = key;
  entry->value = value;
  return isNewKey;
}
//< table-set
//> table-delete
bool tableDelete(Table* table, Value key) {
  if (table->count == 0) return false;

  // Find the entry.
  Entry* entry = findEntry(table->entries, table->capacity, key);
  if (isEmptyEntry(entry) || isTombstoneEntry(entry)) return false;

  // Place a tombstone in the entry.
  entry->key = BOOL_VAL(true);
  entry->value = NIL_VAL;
  return true;
}
//< table-delete
//> table-add-all
void tableAddAll(Table* from, Table* to) {
  for (int i = 0; i < from->capacity; i++) {
    Entry* entry = &from->entries[i];
    if (!isEmptyEntry(entry) && !isTombstoneEntry(entry)) {
      tableSet(to, entry->key, entry->value);
    }
  }
}
//< table-add-all
//> table-find-string
ObjString* tableFindString(Table* table, const char* chars,
                           int length, uint32_t hash) {
  if (table->count == 0) return NULL;

/* Hash Tables table-find-string < Optimization find-string-index
  uint32_t index = hash % table->capacity;
*/
//> Optimization find-string-index
  uint32_t index = hash & (table->capacity - 1);
//< Optimization find-string-index
  for (;;) {
    Entry* entry = &table->entries[index];
    if (isEmptyEntry(entry)) return NULL;

    if (!isTombstoneEntry(entry) &&
        IS_OBJ(entry->key) &&
        OBJ_TYPE(entry->key) == OBJ_STRING) {
      ObjString* key = AS_STRING(entry->key);
      if (key->length == length &&
          key->hash == hash &&
          memcmp(key->chars, chars, length) == 0) {
        return key;
      }
    }

/* Hash Tables table-find-string < Optimization find-string-next
    index = (index + 1) % table->capacity;
*/
//> Optimization find-string-next
    index = (index + 1) & (table->capacity - 1);
//< Optimization find-string-next
  }
}
//< table-find-string
//> Garbage Collection table-remove-white
void tableRemoveWhite(Table* table) {
  for (int i = 0; i < table->capacity; i++) {
    Entry* entry = &table->entries[i];
    if (!isEmptyEntry(entry) && !isTombstoneEntry(entry) &&
        IS_OBJ(entry->key) && !AS_OBJ(entry->key)->isMarked) {
      tableDelete(table, entry->key);
    }
  }
}
//< Garbage Collection table-remove-white
//> Garbage Collection mark-table
void markTable(Table* table) {
  for (int i = 0; i < table->capacity; i++) {
    Entry* entry = &table->entries[i];
    if (!isEmptyEntry(entry) && !isTombstoneEntry(entry)) {
      markValue(entry->key);
      markValue(entry->value);
    } 
  }
}
//< Garbage Collection mark-table