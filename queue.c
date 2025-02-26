#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

#include "queue.h"

const char *value_of(struct list_head *head, struct list_head *node)
{
    if (node == head)
        return "head";
    const element_t *e = list_entry(node, element_t, list);
    return e->value;
}

/* a helper function to print the value of the given list_head */
void q_print_entry(struct list_head *head,
                   const char *prefix,
                   struct list_head *node)
{
    if (!head)
        return;

    const char *value = value_of(head, node);

    if (prefix)
        printf("%s: %s\n", prefix, value);
    else
        printf("%s\n", value);
}

void q_print_queue(struct list_head *head)
{
    if (!head)
        return;

    struct list_head *curr = head->next;
    do {
        printf("%s ", value_of(head, curr));
        curr = curr->next;
    } while (curr && curr != head);
    printf("\n");

    // check if the doublely circular linked list is correct
    int flag = 1;
    curr = head->next;
    while (curr != head) {
        if (curr->prev->next != curr || curr->next->prev != curr) {
            printf("%s is wrong\n", value_of(head, curr));
            flag = 0;
        }
        curr = curr->next;
    }
    if (flag)
        printf("the linked list is correct\n");
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
        if (strcmp(value_of(head, curr), value_of(head, curr->next)) == 0) {
            element_t *e = list_entry(curr, element_t, list);
            curr = curr->next;
            struct list_head *next = NULL;

            while (curr != head &&
                   strcmp(value_of(head, curr), e->value) == 0) {
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

// q_swap_two swap two element with each other
void q_swap_two(struct list_head *a, struct list_head *b)
{
    if (!a || !b || a == b)
        return;

    if (a->next == b || b->next == a) {
        struct list_head *first = (a->next == b) ? a : b;
        struct list_head *second = (a->next == b) ? b : a;

        list_del(first);
        list_add(first, second);
    } else {
        struct list_head *a_prev = a->prev, *b_prev = b->prev;

        list_del(a);
        list_del(b);

        list_add(a, b_prev);
        list_add(b, a_prev);
    }
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head))
        return;

    struct list_head *curr = head->next;
    while (curr != head && curr->next != head) {
        /* q_print_entry("curr", curr); */
        /* q_print_entry("next", next); */

        q_swap_two(curr, curr->next);

        /* q_print_queue(head); */

        curr = curr->next;
    }
    /* q_print_queue(head); */
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *front = head->next, *rear = head->prev, *tmp;

    while (front != rear && front->prev != rear) {
        q_swap_two(front, rear);
        /* printf("front: %s, rear: %s\n", valueOf(front), valueOf(rear)); */
        /* q_print_queue(head); */
        tmp = front->prev;
        front = rear->next;
        rear = tmp;
        /* printf("front: %s, rear: %s\n", valueOf(front), valueOf(rear)); */
    }
}

/**
 * list_next_k() - Check if there are at least k more elements in the list
 * @head: pointer to the head of the linked list
 * @curr: pointer to store the last element of the k elements
 * @k: number of elements to check for availability
 *
 * This function returns true if there are at least k more elements in the
 * linked list without reaching the end. Otherwise, it returns false.
 * If successful, @currr is updated to point to the last element among
 * the k elements.
 */
bool list_next_k(struct list_head *head, struct list_head **curr, int k)
{
    if (!head || list_empty(head) || !*curr || k <= 0)
        return false;

    struct list_head *tmp = *curr;
    for (; k > 0; k--) {
        if (tmp->next == head)
            return false;
        tmp = tmp->next;
    }
    *curr = tmp;

    return true;
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head) || k <= 1)
        return;

    if (k == 2) {
        q_reverse(head);
        return;
    }

    struct list_head *front = head->next, *rear = head, *tmp;
    if (!list_next_k(head, &rear, k))
        return;

    while (true) {
        /* printf("front: %s, rear: %s\n", value_of(head, front), */
        /*        value_of(head, rear)); */
        while (front != rear && front->prev != rear) {
            q_swap_two(front, rear);
            tmp = front->prev;
            front = rear->next;
            rear = tmp;
        }
        /* q_print_queue(head); */
        /* printf("\tfront: %s, rear: %s\n", value_of(head, front), */
        /*        value_of(head, rear)); */

        if (!list_next_k(head, &front, k - 1)) {
            /* printf("1\n"); */
            break;
        }
        rear = front;
        if (!list_next_k(head, &rear, k - 1)) {
            /* printf("1\n"); */
            break;
        }
    }
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    struct list_head list_less, list_greater;
    element_t *pivot;
    element_t *item = NULL, *is = NULL;

    if (list_empty(head) || list_is_singular(head))
        return;

    INIT_LIST_HEAD(&list_less);
    INIT_LIST_HEAD(&list_greater);

    pivot = list_first_entry(head, element_t, list);
    list_del(&pivot->list);

    list_for_each_entry_safe (item, is, head, list) {
        int cmp;
        cmp = strcmp(item->value, pivot->value);
        if (descend ? -cmp : cmp < 0)
            list_move_tail(&item->list, &list_less);
        else
            list_move_tail(&item->list, &list_greater);
    }

    q_sort(&list_less, descend);
    q_sort(&list_greater, descend);

    list_add(&pivot->list, head);
    list_splice(&list_less, head);
    list_splice_tail(&list_greater, head);
}

/**
 * q_release_element() - Common part of q_ascend and q_descend
 * @head: header of queue
 * @ascend: in ascending or descending order
 *
 * @return: the number of elements in queue after performing operation
 */
int q_ascend_descend(struct list_head *head, bool ascend)
{
    if (!head || list_empty(head))
        return 0;
    if (list_is_singular(head)) {
        return 1;
    }

    struct list_head *curr = head->prev;
    int len = 0;
    /* because there are at least one element, so max must not be NULL */
    const char *max = value_of(head, curr);

    while (curr != head) {
        int cmp = strcmp(max, value_of(head, curr));
        if (ascend ? cmp >= 0 : cmp <= 0) {
            max = value_of(head, curr);
            len++;
            curr = curr->prev;
        } else {
            struct list_head *tmp = curr->prev;
            list_del(curr);
            q_release_element(list_entry(curr, element_t, list));
            curr = tmp;
        }
    }
    return len;
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return q_ascend_descend(head, true);
}

/* Remove every node which has a node with a strictly greater value anywhere
 * to the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return q_ascend_descend(head, false);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}
