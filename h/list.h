// list.h   -- uCurses linked lists
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdbool.h>

// -----------------------------------------------------------------------

typedef struct
{
  void *head;
  void *tail;
  uint16_t count;
} list_t;

// -----------------------------------------------------------------------

typedef struct
{
  void *next;
  void *prev;
  void *payload;
  list_t *parent;
} node_t;

typedef node_t* node_t_ptr;

// -----------------------------------------------------------------------

void list_remove(list_t *l1, void *data);
bool list_append(list_t *l1, void *data);

// =======================================================================
