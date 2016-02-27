#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "wb_skiplist.h"

/*test skip level*/
static void test_skip_depth()
{
	uint32_t d;
	int i = 0;
	for(i = 0; i < 32; i ++){
		for(d = 1; d < SKIPLIST_MAXDEPTH; d++){
			if((rand() & 0xffff) > 0x4000)
				break;
		}

		printf("depth = %d\n", d);
	}
}

static int compare_fun(skiplist_item_t k1, skiplist_item_t k2)
{
	if(k1.u32 > k2.u32)
		return 1;
	else if(k1.u32 < k2.u32)
		return -1;
	else
		return 0;
}

int main(int argc, const char* argv[])
{
	uint32_t i;
	wb_skiplist_t* sl;
	skiplist_item_t v;
	wb_skiplist_iter_t* iter;

	srand((uint32_t)time(NULL));

	sl = skiplist_create(compare_fun, NULL);

	for(i = 10; i > 0; i--){
		v.u32 = i;
		skiplist_insert(sl, v, v);
	}

	printf("values = ");
	SKIPLIST_FOREACH(sl, iter){
		printf("%d ", iter->key.u32);
	}

	printf("\nremove v = 5\n");
	v.u32 = 5;
	skiplist_remove(sl, v);
	v.u32 = 8;
	printf("remove v = 8\n");
	skiplist_remove(sl, v);

	printf("values = ");
	SKIPLIST_FOREACH(sl, iter){
		printf("%d ", iter->key.u32);
	}
	printf("\n");
	v.u32 = 8;
	iter = skiplist_search(sl, v);
	if(iter != NULL)
		printf("find k = %d OK, v = %d\n", v.u32, iter->val.u32);
	else
		printf("find k = %d failed!\n", v.u32);

	skiplist_destroy(sl);

	test_skip_depth();

	return 0;
}
