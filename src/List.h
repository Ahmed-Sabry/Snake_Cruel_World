#ifndef List_H_
#define List_H_

typedef struct position
{
	int x;
	int y;
} Position;

typedef Position ListEntry;

typedef struct listnode
{
	ListEntry entry;
	struct listnode* next;
} ListNode;

typedef struct list
{
	ListNode* head;
	int size;
} List;

void InitList(List*);
int ListEmpty(List*);
int ListSize(List*);
void DestroyList(List*);
int InsertList(int, ListEntry, List*);
int PushList(ListEntry e, List* pl);
ListEntry DeleteList(int, List*);
ListEntry PopList(List* pl);
void TraverseList(List*, void (*Visit)(ListEntry));
void RetrieveList(int, ListEntry*, List*);
void ReplaceList(int, ListEntry, List*);

#endif /* List_H_ */
