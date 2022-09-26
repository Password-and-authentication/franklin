#ifndef _LIST_
#define _LIST_

#define SLIST_ENTRY(type)                                                      \
  struct                                                                       \
  {                                                                            \
    struct type* next;                                                         \
  }

#define SLIST_HEAD(name, type)                                                 \
  struct name                                                                  \
  {                                                                            \
    struct type* first;                                                        \
  }

#define SLIST_NEXT(item, field) ((item)->field.next)
#define SLIST_FIRST(head) ((head)->first)

// insert item at head
#define SLIST_INSERT_HEAD(item, head, field)                                   \
  do {                                                                         \
    (item)->field.next = (head)->first;                                        \
    (head)->first = (item);                                                    \
  } while (0)

// insert an item after any item
#define SLIST_INSERT_AFTER(item, prev, field)                                  \
  do {                                                                         \
    (prev)->field.next = (item);                                               \
    (prev) = (prev)->field.next;                                               \
  } while (0)

// remove head from a list
#define SLIST_REMOVE_HEAD(item, head, field)                                   \
  do {                                                                         \
    (item) = (head)->first;                                                    \
    SLIST_FIRST((head)) = SLIST_NEXT((head)->first, field);                    \
  } while (0)

// remove any item from a list
#define SLIST_REMOVE_ITEM(item, head, type, field)                             \
  do {                                                                         \
    if (item == (head)->first) {                                               \
      SLIST_REMOVE_HEAD(item, (head), field);                                  \
    } else {                                                                   \
      struct type* current = (head)->first;                                    \
                                                                               \
      /* loop will iterate until current->next is equal                        \
         to the item that will be removed */                                   \
      while (current->field.next != (item))                                    \
        current = current->field.next;                                         \
                                                                               \
      /*remove the item*/                                                      \
      current->field.next = current->field.next->field.next;                   \
    }                                                                          \
  } while (0)

#define SLIST_FOREACH(item, head, field)                                       \
  for (item = SLIST_FIRST((head)); item; item = SLIST_NEXT(item, field))

#endif
