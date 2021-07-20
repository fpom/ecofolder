#include <stdlib.h>

#include "common.h"
#include "netconv.h"

/*****************************************************************************/
/* Allocate a nodelist element. To reduce memory fragmentation, nodelist_t   */
/* elements are allocated in large contingents, and freed cells are kept in  */
/* li_freelist. New allocation requests are served from li_freelist first.   */

#define LI_ALLOC_STEP 1024 /* how many nodelist elements to grab at a time  */

typedef struct contingent_t
{
	nodelist_t *nodes;
	struct contingent_t *next;
} contingent_t;

nodelist_t *li_freelist = NULL;	  /* pointer to freed nodelist elements */
contingent_t *li_contlist = NULL; /* pointer to allocated contingents   */
int li_counter = 0;    /* num of free elements in the latest contingent */

nodelist_t* nodelist_alloc ()
{
	nodelist_t *tmp_nl;
	contingent_t *tmp_co;

	if ((tmp_nl = li_freelist))
	{
		li_freelist = tmp_nl->next;
		return tmp_nl;
	}

	if (li_counter--) return li_contlist->nodes + li_counter;

	tmp_co = MYmalloc(sizeof(contingent_t));
	tmp_co->nodes = MYmalloc(LI_ALLOC_STEP * sizeof(nodelist_t));
	tmp_co->next = li_contlist;
	li_contlist = tmp_co;
	li_counter = LI_ALLOC_STEP-1;

	return li_contlist->nodes + li_counter;
}

/*****************************************************************************/
/* add a node to the front of a list					     */

nodelist_t* nodelist_push (nodelist_t **list, void *node)
{
	nodelist_t *newlist = nodelist_alloc();
	newlist->node = node;
	newlist->next = *list;
	return *list = newlist;
}

/*****************************************************************************/
/* find and element; return 0 if element is not found, 1 otherwise    */

int nodelist_find (nodelist_t *list, void *node)
{
	while (list && list->node != node )
		list = list->next;

	return (list && list->node == node) ? 1 : 0;
}

/*****************************************************************************/
/* add a place to a list; sort by pointer in ascending order		     */

nodelist_t* nodelist_insert (nodelist_t **list, void *node)
{
	while (*list && node < (*list)->node)
		list = &((*list)->next);

	if (*list && (*list)->node == node) return *list;

	return nodelist_push(list,node);
}

/*****************************************************************************/
/* release a nodelist; move its cells to the "free" list		     */

void nodelist_delete (nodelist_t *list)
{
	nodelist_t *tmp;

	while (list)
	{
		tmp = list;
		list = list->next;
		tmp->next = li_freelist;
		li_freelist = tmp;
	}
}

/*****************************************************************************/
/* compare two lists; return 0 if equal, -1 if list1 < list2, 1 otherwise    */

char nodelist_compare (nodelist_t *list1, nodelist_t *list2)
{
	while (list1 && list2 && list1->node == list2->node)
		list1 = list1->next, list2 = list2->next;

	if (!list1 && !list2) return 0;
	if (!list1 || (list2 && list1->node < list2->node)) return -1;
	return 1;
}

/*****************************************************************************/ 	//*** NEW FUNCTION ***//
/* calculate the size of an array of pointers    */

int sizeList(nodelist_t *head)
{
    nodelist_t *ptr = head;
	int list_size = 0;
    while (ptr)
	{
		list_size++;
        ptr = ptr->next;
    }
    return list_size;
}

/*****************************************************************************/ 	//*** NEW FUNCTION ***//
/* concatenate two lists and return the join    */

nodelist_t* nodelist_concatenate(nodelist_t *list1, nodelist_t *list2)
{	
	nodelist_t *head = nodelist_alloc();
	nodelist_t *first = list1;
	nodelist_t *second = list2;

	if (!list1){
		first = list2; 
		second = list1;
	}
	
	if (first)
	{
		nodelist_t *copy_first = first;
		nodelist_t *tmp_first = first;
		int duplicated = 0;

		head->node = first->node;		
		nodelist_t *tail = head;
		first = first->next;
		while (first != NULL){
			tail->next = nodelist_alloc();
			tail = tail->next;
			tail->node = first->node;
			first = first->next;
		}
		while (second != NULL){
			while (copy_first != NULL && duplicated == 0){
				if (copy_first->node == second->node)				
					duplicated = 1;
				copy_first = copy_first->next;
			}
			if (duplicated == 0){
				tail->next = nodelist_alloc();
				tail = tail->next;
				tail->node = second->node;
			}
			duplicated = 0;
			second = second->next;
			copy_first = tmp_first;
		}
		tail->next = NULL;
	}
	
	return head->node ? head : NULL;
}

/*****************************************************************************/ 	//*** NEW FUNCTION ***//
/* check if two lists have elements in common. No tested yet.  */

int nodelist_common(cond_t *list1, nodelist_t *list2, trans_t* tr, place_t* pl)
{	
	int com = 0;
	cond_t *tmp_list1 = list1;
	if (list1 && list2){
		while (list2 && !com){
			while (list1 && !com){				
				//printf("element in common\n");
				printf("place source for the preset assessed: %s\n", pl->name);
				printf("transition source for the preset assessed: %s\n", tr->name);
				printf("mark condition: %d\n", list1->mark);
				printf("num place: %d\n", ((place_t*)(list2->node))->num);
				printf("condition name: %s\n", list1->origin->name);
				printf("place name: %s\n", ((place_t*)(list2->node))->name);
				if (list1->pre_ev)
					printf("pre_event name: %s\n", list1->pre_ev->origin->name);
				//printf("mark condition: %d", ((cond_t*)(list2->conds->node))->mark);
				if (list1->origin->name == ((place_t*)(list2->node))->name && 
					list1->mark == ((place_t*)(list2->node))->marked){
					com = 1;
				}
				//printf("hola\n");
				list1 = list1->next;
			}
			list2 = list2->next;
			list1 = tmp_list1;
		}
	}

	return com;
}
