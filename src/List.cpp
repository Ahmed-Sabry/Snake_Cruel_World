#include "List.h"
#include <stdlib.h>

void InitList(List* pl)
{
	pl->head = NULL;
	pl->size = 0;
} // (1)

int ListEmpty(List* pl)
{
	return (pl->size == 0);
	//or return !pl->head
} // (1)

int ListSize(List* pl)
{
	return pl->size;
} // (1)

void DestroyList(List* pl)
{
	ListNode* q;
	while (pl->head)
	{
		q = pl->head->next;
		free(pl->head);
		pl->head = q;
	}
	pl->size = 0;
} // (n)

void TraverseList(List* pl, void (*Visit)(ListEntry))
{
	ListNode* p = pl->head;
	while (p)
	{
		(*Visit)(p->entry);
		p = p->next;
	}
} // (n)

int PushList(ListEntry e, List* pl)
{
	int pos = ListSize(pl);
	return InsertList(pos, e, pl);
}

ListEntry PopList(List* pl)
{
	int pos = ListSize(pl) - 1;

	if (pos < 0)
		return { -999, -999 }; // Value for Error Handling
	else
		return DeleteList(pos, pl);
}

int InsertList(int pos, ListEntry e, List* pl)
{
	ListNode *p, *q;
	int i;

	if (pos == -1)
		pos = ListSize(pl);

	p = (ListNode*)malloc(sizeof(ListNode));

	if (p != NULL)
	{
		p->entry = e;
		p->next = NULL;

		if (pos == 0)
		{ //works also for head = NULL
			p->next = pl->head;
			pl->head = p;
		}

		else
		{
			for (q = pl->head, i = 0; i < pos - 1; i++)
				q = q->next;

			p->next = q->next;
			q->next = p;
		}

		pl->size++;

		return 1;
	}

	else
		return 0;
}

ListEntry DeleteList(int pos, List* pl)
{
	int i;
	ListNode *q, *tmp;
	ListEntry e;

	if (pos == 0)
	{
		e = pl->head->entry;
		tmp = pl->head->next;
		free(pl->head);
		pl->head = tmp;
	} // it works also for one node
	else
	{
		for (q = pl->head, i = 0; i < pos - 1; i++)
			q = q->next;

		e = q->next->entry;
		tmp = q->next->next;
		free(q->next);
		q->next = tmp;
	} // check for pos=size-1 (tmp will be NULL)
	pl->size--;

	return e;
} //O(n) but without shifting elements.

void RetrieveList(int pos, ListEntry* pe, List* pl)
{
	int i;
	ListNode* q;
	for (q = pl->head, i = 0; i < pos; i++)
		q = q->next;
	*pe = q->entry;
}

void ReplaceList(int pos, ListEntry e, List* pl)
{
	int i;
	ListNode* q;
	for (q = pl->head, i = 0; i < pos; i++)
		q = q->next;
	q->entry = e;
}
