#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint
{
	int NO;
	struct watchpoint *next;

	/* TODO: Add more members if necessary */
	word_t pre_val;
	char expression[32];
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool()
{
	int i;
	for (i = 0; i < NR_WP; i++)
	{
		wp_pool[i].NO = i;
		wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
	}

	head = NULL;
	free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

static WP *new_wp()
{
	if (free_ == NULL)
	{
		assert(0);
		return NULL;
	}
	WP *curr = free_;
	free_ = free_->next;
	return curr;
}

static void free_wp(WP *wp)
{
	memset(wp->expression, 0, 32U);
	wp->next = free_;
	free_ = wp;
}

char *check_watch_point()
{
	WP *curr = head;
	while (curr)
	{
		bool success = false;
		word_t val = expr(curr->expression, &success);
		if (val != curr->pre_val)
		{
			curr->pre_val = val;
			return curr->expression;
		}
	}
	return NULL;
}

int new_watch_point(char *e)
{
	bool success = 0;
	word_t val = expr(e, &success);
	if (!success)
	{
		return -1;
	}
	WP *new_node = new_wp();
	new_node->pre_val = val;
	strcpy(new_node->expression, e);
	new_node->next = head;
	head = new_node;
	return new_node->NO;
}

bool delete_watch_point(int no)
{
	WP *curr = head;
	WP *delete_node = NULL;
	while (curr)
	{
		if (curr->NO == no)
		{
			delete_node = curr;
			break;
		}
		curr = curr->next;
	}
	if (!delete_node)
	{
		return false;
	}
	if (delete_node == head)
	{
		head = head->next;
		free_wp(delete_node);
		return true;
	}
	curr = head;
	while (curr->next != delete_node)
	{
		curr = curr->next;
	}
	curr->next = delete_node->next;
	free_wp(delete_node);
	return true;
}

void display_watch_point()
{
	WP *curr = head;
	while(curr){
		printf("Watch point no.%2d:\n\texpression:%s\n\tval:%ld\n", curr->NO, curr->expression, curr->pre_val);
		curr = curr->next;
	}
	return ;
}