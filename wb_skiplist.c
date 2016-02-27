#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "wb_skiplist.h"


static uint32_t wb_skip_depth()
{
	uint32_t d;
	for(d = 1; d < SKIPLIST_MAXDEPTH; d++){
		if((rand() & 0xffff) > 0x4000)
			break;
	}

	return d;
}

wb_skiplist_t* skiplist_create(skiplist_compare_f compare_cb, skiplist_free_f free_cb)
{
	wb_skiplist_t* sl = (wb_skiplist_t *)calloc(1, sizeof(wb_skiplist_t));
	sl->compare_fun = compare_cb;
	sl->free_fun = free_cb;

	return sl;
}

void skiplist_clear(wb_skiplist_t* sl)
{
	wb_skiplist_iter_t* iter, *next;
	int i;

	for(iter = sl->entries[0]; iter != NULL; iter = next){
		next = iter->next[0];
		if(sl->free_fun != NULL)
			sl->free_fun(iter->key, iter->val);
		free(iter);
	}

	for(i = 0; i < SKIPLIST_MAXDEPTH; ++i)
		sl->entries[i] = 0;
	sl->size = 0;
}

void skiplist_destroy(wb_skiplist_t* sl)
{
	assert(sl != NULL);

	if(sl == NULL)
		return ;

	skiplist_clear(sl);
	free(sl);
}

/*search skiplist*/
static int skiplist_insert_stack(wb_skiplist_t* sl, wb_skiplist_iter_t*** stack, skiplist_item_t key)
{
	int i, cmp;
	wb_skiplist_iter_t** iterp;

	for(i = SKIPLIST_MAXDEPTH - 1, iterp = &sl->entries[i]; i >= 0;){
		if(*iterp == NULL){ /*empty level, dropdown a level*/
			stack[i--] = iterp --;
			continue;
		}

		/*compare key!!*/
		if(sl->compare_fun != NULL)
			cmp = sl->compare_fun((*iterp)->key, key);
		else
			cmp = memcmp(&((*iterp)->key), &key, sizeof(key));

		if(cmp > 0) /*fix pos, dropdown a level*/
			stack[i--] = iterp--;
		else if(cmp == 0) /*repeat!!*/
			return -1;
		else
			iterp = &((*iterp)->next[i]);
	}

	return 0;
}

void skiplist_insert(wb_skiplist_t* sl, skiplist_item_t key, skiplist_item_t val)
{
	wb_skiplist_iter_t* iter, **stack[SKIPLIST_MAXDEPTH];
	uint32_t i, depth;

	if(skiplist_insert_stack(sl, stack, key) != 0){
		if(sl->free_fun != NULL)
			sl->free_fun(key, val);
	}
	else{

		depth = wb_skip_depth();

		iter = (wb_skiplist_iter_t*)calloc(1, sizeof(wb_skiplist_iter_t) + depth * sizeof(wb_skiplist_iter_t*));
		iter->key = key;
		iter->val = val;
		iter->depth = depth;

		for(i = 0; i < depth; ++i){
			iter->next[i] = *stack[i];
			*stack[i] = iter;
		}

		++sl->size;
	}
}

static wb_skiplist_iter_t* skip_remove_stack(wb_skiplist_t* sl, wb_skiplist_iter_t*** stack, skiplist_item_t key)
{
	int i, cmp;
	wb_skiplist_iter_t** iterp, *ret;

	ret = NULL;

	for(i = SKIPLIST_MAXDEPTH - 1, iterp = &sl->entries[i]; i >= 0;){
		if(*iterp == NULL){ /*empty level, dropdown a level*/
			stack[i--] = iterp --;
			continue;
		}

		/*compare key!!*/
		if(sl->compare_fun != NULL)
			cmp = sl->compare_fun((*iterp)->key, key);
		else
			cmp = memcmp(&((*iterp)->key), &key, sizeof(key));

		if(cmp == 0){
			ret = *iterp;
			if(i > 0)
				stack[i--] = iterp --;
			else{
				stack[i] = iterp;
				break;
			}
		}
		else if(cmp < 0)
			iterp = &(*iterp)->next[i];
		else /*nonexistent key*/
			if(i > 0)
				stack[i--] = iterp --;
			else 
				break;
	}

	return ret;
}

void skiplist_remove(wb_skiplist_t* sl, skiplist_item_t key)
{
	wb_skiplist_iter_t **stack[SKIPLIST_MAXDEPTH], *iter;
	int i;

	iter = skip_remove_stack(sl, stack, key);
	if(iter != NULL){
		for(i = 0; i < iter->depth; ++i)
			*stack[i] = iter->next[i];

		if(sl->free_fun != NULL)
			sl->free_fun(iter->key, iter->val);
		free(iter);
		
		if(sl->size > 0)
			-- sl->size;
	}
}

static wb_skiplist_iter_t* skiplist_search_iter(wb_skiplist_t* sl, skiplist_item_t key)
{
	int i, cmp;
	wb_skiplist_iter_t** iterp;

	for(i = SKIPLIST_MAXDEPTH - 1, iterp = &sl->entries[i]; i >= 0;){
		if(*iterp == NULL){
			--i;
			--iterp;
			continue;
		}

		/*compare key!!*/
		if(sl->compare_fun != NULL)
			cmp = sl->compare_fun((*iterp)->key, key);
		else
			cmp = memcmp(&((*iterp)->key), &key, sizeof(key));

		if(cmp == 0)
			return *iterp;

		if(cmp > 0){
			--i;
			--iterp;
		}
		else
			iterp = &(*iterp)->next[i];
	}

	return NULL;
}

wb_skiplist_iter_t* skiplist_search(wb_skiplist_t* sl, skiplist_item_t key)
{
	return skiplist_search_iter(sl, key);
}

size_t skiplist_size(wb_skiplist_t* sl)
{
	return sl->size;
}


