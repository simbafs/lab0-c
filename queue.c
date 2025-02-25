#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

#include "queue.h"

char *valueOf(struct list_head *node)
{
    const element_t *e = list_entry(node, element_t, list);
    if (!e || !e->value)
        return "";
    return e->value;
}

/* a helper function to print the value of the given list_head */
void q_print_entry(const char *prefix, struct list_head *head)
{
    if (!head)
        return;

    if (prefix)
        printf("%s: %s\n", prefix, valueOf(head));
    else
        printf("%s\n", valueOf(head));
}

void q_print_queue(struct list_head *head)
{
    if (!head)
        return;

    struct list_head *curr = head->next;
    do {
        printf("%s ", valueOf(curr));
        curr = curr->next;
    } while (curr && curr != head);

    printf("\n");
}

#define hr() printf("-----\n")

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (!head)
        return NULL;

    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;

    element_t *entry = NULL, *safe;
    list_for_each_entry_safe (entry, safe, head, list) {
        q_release_element(entry);
    }

    free(head);
}

/* NOTE: only for internel used */
element_t *q_new_element(char *s)
{
    element_t *e = (element_t *) malloc(sizeof(element_t));
    if (!e)
        return NULL;

    INIT_LIST_HEAD(&e->list);

    int len = strlen(s) + 1;
    e->value = (char *) malloc(len);
    strncpy(e->value, s, len);

    return e;
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    element_t *e = q_new_element(s);
    if (!e)
        return false;

    list_add(&e->list, head);

    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    element_t *e = q_new_element(s);
    if (!e)
        return false;

    list_add_tail(&e->list, head);

    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head) || !sp)
        return NULL;

    element_t *e = list_entry(head->next, element_t, list);

    strncpy(sp, e->value, bufsize);

    list_del_init(head->next);

    return e;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head) || !sp)
        return NULL;

    element_t *e = list_entry(head->prev, element_t, list);

    strncpy(sp, e->value, bufsize);

    list_del_init(head->prev);

    return e;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;

    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;

    struct list_head *fast = head->next, *slow = head->next;
    while (fast && fast->next && fast->next->next && fast->next != head &&
           fast->next->next != head) {
        fast = fast->next->next;
        slow = slow->next;
    }

    list_del(slow);
    q_release_element(list_entry(slow, element_t, list));

    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    // NOTE: assume that every element are sort in the linked list
    /* q_print_queue(head); */

    if (!head || list_empty(head))
        return false;

    struct list_head *curr = head->next;

    while (curr->next != head) {
        /* q_print_entry("curr", curr); */
        /* printf("! %s %s\n", valueOf(curr), valueOf(curr->next)); */
        if (strcmp(valueOf(curr), valueOf(curr->next)) == 0) {
            element_t *e = list_entry(curr, element_t, list);
            curr = curr->next;
            struct list_head *next = NULL;

            while (curr != head && strcmp(valueOf(curr), e->value) == 0) {
                /* q_print_queue(head); */
                /* q_print_entry("\tcurr", curr); */
                /* printf("\tnext: %p\n", curr->next); */
                next = curr->next;
                list_del(curr);
                q_release_element(list_entry(curr, element_t, list));
                curr = next;
            }


            list_del(&e->list);
            q_release_element(e);
            /* printf("after delete: "); */
            /* q_print_queue(head); */
        } else {
            /* printf("move to next\n"); */
            curr = curr->next;
        }
        /* printf("is head: %s\n", curr == head ? "true" : "false"); */
    }

    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head) {}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend) {}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return 0;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return 0;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}
