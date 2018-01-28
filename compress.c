#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct node node;
typedef struct freq_list freq_list;

struct node
{   
	char c;
	short int freq;
	node* parent;
	node* child_l;
	node* child_r;
};

struct freq_list
{
	freq_list* next;
	char c;
	short int freq;
};

//void swap(node* a, node* b) //only for nodes right next to each other =>a<=>b<=
//{
//	a.next = b.next;
//	b.prev = a.prev;
//
//	if (a.next)
//		a.next.prev = a;
//	if (b.prev)
//		b.prev.next = b;
//	b.next = a;
//	a.prev = b;
//}

node* tree_head;
freq_list* freq_head;
freq_list* sorted_freq_head;

void node_init(node* n)
{
	n->parent = n->child_r = n->child_l = NULL;
	n->freq = n->c = 0;
}

void freq_list_init(freq_list* p)
{
	p->next = NULL;
	p->freq = p->c = 0;
}

int main(int argc, char* argv[])
{
	int16_t occurrences[255];

	for (int i = 0; i < 256; i++)
	{
		occurrences[i] = 0;
	}
	for (int32_t i = 0; i < strlen(argv[1]); i++)
	{
		occurrences[(int)(argv[1][i])]++;
	}

	freq_list* current;
	freq_list* prev;
   for (int c = 0; (int)c < 256; c++)
	{
		current = malloc(sizeof(freq_list));
		if (!current)
		{
			printf("Could not allocate more memory\n");
			exit(1);
		}
		freq_list_init(current);

		current->c = (char)c;
		current->freq = occurrences[c];
		if (c == 0)
			freq_head = current;
		else
			prev->next = current;

		prev = current;
	}

	//for (freq_list* p = freq_head; p->next; p = p->next)
	  // printf("%d ", p->freq);

	freq_list* up;
	sorted_freq_head = freq_head;
	freq_head = freq_head->next;
	sorted_freq_head->next = NULL;
	int i = 0;
	while (freq_head)
	{
		//Sonderfall: in sorted list nur head
		//freqhead next zwischenspeichern
		//wenn head groesser als freqhead
		//freqhead next = head, head = freqhead
		//else head kleiner als freqhead
		//head next = freqhead, freqhead next = null
		freq_list* unsorted_next = freq_head->next;
		
		if (!sorted_freq_head->next)
		{
			if (sorted_freq_head->freq > freq_head->freq)
			{
				freq_head->next = sorted_freq_head;
				sorted_freq_head = freq_head;
			}
			else if (sorted_freq_head->freq <= freq_head->freq)
			{
				sorted_freq_head->next = freq_head;
				freq_head->next = NULL;
			}
		}

		//wir nehmen freq_head, gehen durch sortierte list bis das naechste groesser als freq_head oder current.next == null
		//last_smaller = das letzte davor
		//freqhead next auf last_smaller->next
		//last_smaller next auf freqhead
		//freqhead auf freqhead next
		else
		{         
		  	freq_list* sorted_cur = sorted_freq_head;
			while (sorted_cur->next)
			{
				if (freq_head->freq < sorted_cur->next->freq)
					break;
				sorted_cur = sorted_cur->next;
			}
			if (!sorted_cur->next)
			{
				sorted_cur->next = freq_head;
				freq_head->next = NULL;
			}
			else
			{
				freq_head->next = sorted_cur->next;
				sorted_cur->next = freq_head;
			}
		}
		freq_head = unsorted_next;
	}
	for (freq_list* p = sorted_freq_head; p; p = p->next)
		printf("%d", p->freq);
}
