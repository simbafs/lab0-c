#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

#include "queue.h"

/* ==== Beginning of Helper Functions ====*/

#define hr() printf("-----\n")

const char *element_value_of(struct list_head *head, struct list_head *node)
{
    if (node == head)
        return "head";
    const element_t *e = list_entry(node, element_t, list);
    return e->value;
}

int queue_id_of(struct list_head *node)
{
    return list_entry(node, queue_contex_t, chain)->id;
}

void print_element(struct list_head *head,
                   const char *prefix,
                   struct list_head *node)
{
    if (!head)
        return;

    const char *value = element_value_of(head, node);

    if (prefix)
        printf("%s: %s\n", prefix, value);
    else
        printf("%s\n", value);
}

// head is element_t->list
void print_queue(struct list_head *head)
{
    if (!head)
        return;

    struct list_head *curr = head->next;
    do {
        printf("%s ", element_value_of(head, curr));
        curr = curr->next;
    } while (curr && curr != head);
    printf("\n");
}

// list is queue_contex_t->chain
void print_queues(struct list_head *head)
{
    int n = 0;
    queue_contex_t *curr_q = NULL;
    list_for_each_entry (curr_q, head, chain) {
        n++;
        printf("queue %d(%d): ", curr_q->id, curr_q->size);
        element_t *item = NULL;
        list_for_each_entry (item, curr_q->q, list) {
            printf("%s ", item->value);
        }
        printf("\n");
    }
    printf("%d queue\n", n);
}

bool check_list(struct list_head *head)
{
    if (!head) {
        printf("List head is NULL\n");
        return false;
    }

    if (head->next == NULL || head->prev == NULL) {
        printf("List is corrupted: head->next or head->prev is NULL\n");
        return false;
    }

    if (head->next == head && head->prev == head) {
        printf("List is empty. Element count: 0\n");
        return true;
    }

    struct list_head *node = head->next;
    int count = 0;

    while (node != head) {
        if (!node || !node->next || !node->prev) {
            printf("List is corrupted at node %d: found NULL pointers\n",
                   count + 1);
            return false;
        }

        if (node->next->prev != node) {
            printf(
                "List is corrupted at node %d: next->prev does not point "
                "back\n",
                count + 1);
            return false;
        }

        if (node->prev->next != node) {
            printf(
                "List is corrupted at node %d: prev->next does not point "
                "forward\n",
                count + 1);
            return false;
        }

        count++;
        node = node->next;

        // 防止無窮迴圈（理論上不該發生，除非指標異常）
        if (count > 1000000) {
            printf("List check aborted: possible infinite loop detected\n");
            return false;
        }
    }

    printf("List is valid. Element count: %d\n", count);
    return true;
}

bool check_queues(struct list_head *head)
{
    hr();
    printf("  checking queues chain\n");
    if (!check_list(head))
        return false;
    queue_contex_t *queue = NULL;
    list_for_each_entry (queue, head, chain) {
        printf("  checking queue %d\n", queue->id);
        if (!check_list(queue->q))
            return false;
    }
    hr();
    return true;
}

/* ==== Ending of Helper Functions ====*/

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

element_t *new_element(char *s)
{
    element_t *e = (element_t *) malloc(sizeof(element_t));
    if (!e)
        return NULL;

    INIT_LIST_HEAD(&e->list);

    e->value = strdup(s);
    if (!e->value) {
        free(e);
        return NULL;
    }

    return e;
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    element_t *e = new_element(s);
    if (!e)
        return false;

    list_add(&e->list, head);

    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    element_t *e = new_element(s);
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
    sp[bufsize - 1] = '\0';

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
    sp[bufsize - 1] = '\0';

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

    if (!head || list_empty(head))
        return false;

    struct list_head *curr = head->next;

    while (curr->next != head) {
        if (strcmp(element_value_of(head, curr),
                   element_value_of(head, curr->next)) == 0) {
            element_t *e = list_entry(curr, element_t, list);
            curr = curr->next;
            struct list_head *next = NULL;

            while (curr != head &&
                   strcmp(element_value_of(head, curr), e->value) == 0) {
                next = curr->next;
                list_del(curr);
                q_release_element(list_entry(curr, element_t, list));
                curr = next;
            }


            list_del(&e->list);
            q_release_element(e);
        } else {
            curr = curr->next;
        }
    }

    return true;
}

// q_swap_two swap two elements with each other in linked list
void swap_two(struct list_head *a, struct list_head *b)
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

    for (struct list_head *curr = head->next;
         curr != head && curr->next != head; curr = curr->next) {
        swap_two(curr, curr->next);
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *front = head->next, *rear = head->prev, *tmp;

    while (front != rear && front->prev != rear) {
        swap_two(front, rear);

        tmp = front->prev;
        front = rear->next;
        rear = tmp;
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
        while (front != rear && front->prev != rear) {
            swap_two(front, rear);
            tmp = front->prev;
            front = rear->next;
            rear = tmp;
        }

        if (!list_next_k(head, &front, k - 1))
            break;
        rear = front;
        if (!list_next_k(head, &rear, k - 1))
            break;
    }
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (list_empty(head) || list_is_singular(head))
        return;

    struct list_head list_less, list_greater;
    INIT_LIST_HEAD(&list_less);
    INIT_LIST_HEAD(&list_greater);

    element_t *pivot;
    element_t *item = NULL, *is = NULL;

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
    const char *max = element_value_of(head, curr);

    while (curr != head) {
        int cmp = strcmp(max, element_value_of(head, curr));
        if (ascend ? cmp >= 0 : cmp <= 0) {
            max = element_value_of(head, curr);
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

// merge L1 and L2 into L1
void merge_two_list(struct list_head *L1, struct list_head *L2, bool descend)
{
    if (list_empty(L1)) {
        L1->next = L2->next;
        L1->prev = L2->prev;
        INIT_LIST_HEAD(L2);
    }
    if (list_empty(L2))
        return;

    struct list_head *curr1 = L1->next, *curr2 = L2->next;
    while (curr1 != L1 && curr2 != L2) {
        int cmp =
            strcmp(element_value_of(L1, curr1), element_value_of(L2, curr2));
        if (descend ? cmp < 0 : cmp > 0) {
            struct list_head *tmp = curr2->next;
            list_del(curr2);
            list_add_tail(curr2, curr1);
            curr2 = tmp;
        } else {
            curr1 = curr1->next;
        }
    }

    if (curr2 != L2) {
        list_splice_tail(L2, L1);
    }
    INIT_LIST_HEAD(L2);

    return;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head || list_empty(head))
        return 0;

    queue_contex_t *first = list_entry(head->next, queue_contex_t, chain);
    if (list_is_singular(head))
        return first->size;

    struct list_head *first_list =
        list_entry(head->next, queue_contex_t, chain)->q;

    for (struct list_head *curr_queue = head->next->next; curr_queue != head;
         curr_queue = curr_queue->next) {
        merge_two_list(first_list,
                       list_entry(curr_queue, queue_contex_t, chain)->q,
                       descend);
    }

    return q_size(first_list);
}
