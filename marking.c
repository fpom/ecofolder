#include <stdio.h>
#include <string.h>

#include "common.h"
#include "netconv.h"
#include "unfold.h"

hashcell_t **hash;
querycell_t **query;
int hash_buckets;
int *rep_marking;


/******************************************************/

void marking_init ()
{
  hash_buckets = net->numpl*4 + 1;
  hash = MYcalloc(hash_buckets * sizeof(hashcell_t*));
  query = MYcalloc(1 * sizeof(querycell_t*));
}

/******************************************************/
/* 
  Compute the hash value of a marking. TODO: Is this a 
  good hash function?? If not, replace by something better.
  Also, this might be the only place that really needs 
  pl->num.
*/

int marking_hash (nodelist_t *marking)
{
  unsigned int val = 0, i = 0;
  while (marking)
  {
    val += ((place_t*)(marking->node))->num * ++i;
    marking = marking->next;
  }
  return val % hash_buckets;
}

/******************************************************/
/*
  Check if a marking is already present in the hash table.
  Return number of times the marking is repeated, negative 
  values are returned when the marking is present but its 
  corresponding instance does not match with the requested 
  one by the user when searching. If the marking is not 
  present at all, it returns 0. The given marking is left 
  unchanged.
*/

int find_marking (nodelist_t *marking, int m_query)
{
  hashcell_t **buck = hash + marking_hash(marking);
  int cmp = 2;
  nodelist_t* list = NULL;
  int tmp_repeat = 0;

  while (*buck && (cmp = nodelist_compare(marking,(*buck)->marking)) > 0)
    buck = &((*buck)->next);

  if (m_query && !cmp){
    for(list = marking; list && !cmp; list = list->next)
      if(!((place_t*)(list->node))->queried)  cmp = 1;
    if(!cmp && (*buck)->repeat != m_repeat)
      tmp_repeat = (*buck)->repeat*-1;
    else if(!cmp && (*buck)->repeat == m_repeat)
      tmp_repeat = (*buck)->repeat;
  }

  return !cmp && m_query ? tmp_repeat : !cmp;
}

/******************************************************/
/* 
  Inspecting the cone of an event to see if its 
  corresponding marking was seen before
*/

int check_back(cond_t **conds, int size, event_t *ev)
{
  int i, found = 0;
  for(i = 0; i < size && !found; i++)
  {
    //printf("i: %d, co name: %s and num: %d\n", i, conds[i]->origin->name, conds[i]->num + 1);
    if(conds[i]->pre_ev && ev &&
      conds[i]->pre_ev->mark != ev_mark)
      {
        if(conds[i]->pre_ev == ev)
          found = 1;
        else
        { 
          //printf("conds[i]->pre_ev->preset, ev->id: %s, %d\n", conds[i]->pre_ev->origin->name, ev->id);
          found = check_back(conds[i]->pre_ev->preset,
            conds[i]->pre_ev->preset_size, ev);
        }
        conds[i]->pre_ev->mark = ev_mark;
      }
  }
  return found;
}

/******************************************************/
/* 
  Add a marking to the hash table. It is assumed that 
  marking = Mark([ev]). Return 1 if the marking was not 
  yet present; otherwise, add ev to the list of cut-off 
  events and return 0.
*/

int add_marking (nodelist_t *marking, event_t *ev)
{
  hashcell_t *newbuck;
  hashcell_t **buck = hash + marking_hash(marking);
  char cmp = 2;
  nodelist_t* list = NULL;
  int not_present = 0, checked_back = 0;

  while (*buck && (cmp = nodelist_compare(marking,(*buck)->marking)) > 0)
    buck = &((*buck)->next);
  
  /* printf("\nhola\n");
  if(ev) printf("creating marking: %s, %d\n", ev->origin->name, ev->id);
  for(list = marking; list; list = list->next)
    printf("%s, ", ((place_t*)(list->node))->name); */
  
  if(!cmp && mcmillan) /* marking is already present */
  {
    list = (*buck)->pre_evs;
    if(!list)
    {
      checked_back = 1;
      nodelist_push(&cutoff_list,ev);
      nodelist_push(&corr_list, NULL);
    }
    else
      for(; list && !checked_back; list = list->next)
      {
        ev_mark++;
        if ((checked_back = check_back(ev->preset, ev->preset_size,
          list->node)))
        {
          nodelist_push(&cutoff_list,ev);
          nodelist_push(&corr_list, ((event_t*)(list->node)));
        }
      }
    not_present = !checked_back;
    (*buck)->repeat++;
    /* printf("\nrepetition: %d\n", (*buck)->repeat);
    printf("chao\n"); */
    nodelist_push(&((*buck)->pre_evs),ev);
  }
  else if (!cmp && !mcmillan)
  {
    (*buck)->repeat++;
    //nodelist_delete(marking);
    nodelist_push(&cutoff_list,ev);
    if ((*buck)->pre_evs)
      nodelist_push(&corr_list,((event_t*)((*buck)->pre_evs->node)));
    else
      nodelist_push(&corr_list, NULL);
    nodelist_push(&((*buck)->pre_evs),ev);
  }

  if(!!cmp) /* marking is not present */
  {
    newbuck = MYmalloc(sizeof(hashcell_t));
    newbuck->marking = marking;
    newbuck->pre_evs = NULL;
    if (ev) nodelist_push(&(newbuck->pre_evs),ev);
    newbuck->repeat = 1;
    /* printf("\nrepetition: %d\n", newbuck->repeat);
    printf("chao\n"); */
    newbuck->next = *buck;
    *buck = newbuck;
    not_present = 1;
  }
  return not_present;
}

/******************************************************/
/* Collect the initial marking.     */

nodelist_t* retrieve_list (char* attribute)
{
  place_t *pl;
  nodelist_t *list = NULL;

  for (pl = net->places; pl; pl = pl->next)
    if (((!strcmp("marked",attribute) && pl->marked) !=
     (!strcmp("queried",attribute) && pl->queried)) !=
     (!strcmp("harmful",attribute) && pl->harmful))
      nodelist_insert(&list,pl);

  return list;
}

/******************************************************/

void print_marking_pl (nodelist_t* list)
{
  if (!list) return;
  printf("%s ",((place_t*)(list->node))->name);
  print_marking_pl(list->next);
}

void print_marking_co (nodelist_t* list)
{
  if (!list) return;
  printf("%s ",((cond_t*)(list->node))->origin->name);
  print_marking_co(list->next);
}
