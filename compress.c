#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct node node;

struct node
{
	node* parent;
	node* child_l;
	node* child_r;
	char c;
	long int freq;
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
node* freq_head;
node* sorted_freq_head;

void node_init(node* p)
{
	p->parent = p->child_l = p->child_r = NULL;
	p->freq = p->c = 0;
}

node* insertion_sort(node* unsorted_head)
{
	node* sorted_head;

	sorted_head = unsorted_head;
	unsorted_head = unsorted_head->parent;
	sorted_head->parent = NULL;
	while (unsorted_head)
	{
		//Sonderfall: in sorted list nur head
		//freqhead next zwischenspeichern
		//wenn head groesser als freqhead
		//freqhead next = head, head = freqhead
		//else head kleiner als freqhead
		//head next = freqhead, freqhead next = null
		node* unsorted_next = unsorted_head->parent;
		
		if (!sorted_head->parent)
		{
			if (sorted_head->freq > unsorted_head->freq)
			{
				unsorted_head->parent = sorted_head;
				unsorted_head->child_l = NULL;
				sorted_head->child_l = unsorted_head;
				sorted_head = unsorted_head;
			}
			else if (sorted_head->freq <= unsorted_head->freq)
			{
				sorted_head->parent = unsorted_head;
				unsorted_head->child_l = sorted_head;
				unsorted_head->parent = NULL;
			}
		}

		//wir nehmen freq_head, gehen durch sortierte list bis das naechste groesser als freq_head oder current.next == null
		//last_smaller = das letzte davor
		//freqhead next auf last_smaller->next
		//last_smaller next auf freqhead
		//freqhead auf freqhead next
		else
		{         
		  	node* sorted_cur = sorted_head;
			while (sorted_cur->parent)
			{
				if (unsorted_head->freq < sorted_cur->parent->freq)
					break;
				sorted_cur = sorted_cur->parent;
			}
			if (!sorted_cur->parent)
			{
				sorted_cur->parent = unsorted_head;
//				unsorted_head->child_l = sorted_cur;
				unsorted_head->parent = NULL;
			}
			else
			{
				unsorted_head->parent = sorted_cur->parent;
//				sorted_cur->parent->child_l = unsorted_head;
				sorted_cur->parent = unsorted_head;
//				unsorted_head->child_l = sorted_cur;
			}
		}
		unsorted_head = unsorted_next;
	}
	return sorted_head;
}

node* build_huff_tree(node* lh) //lh is the least frequent node
{
	node *uh, *a, *b, *q;
	for (uh = lh; uh->parent; uh = uh->parent);

	node* r;
	for (r = lh; r; r = r->parent)
	{
		if (!r->freq)
			free(r);
		else
			break;
	}
	lh = r;

	while (uh != lh)
	{
		if (!lh->parent)
		{
			printf("%c breaking\n", lh->c);
			break;
		}
		a = lh;
		b = lh->parent;
		q = (node*)calloc(1, sizeof(node));
		q->freq = a->freq + b->freq;

		q->child_l = (a->freq <= b->freq) ? a : b;
		q->child_r = (a->freq > b->freq) ? a : b;

		if (lh->parent == uh)
		{
			uh = q;
			return uh;
		}
		lh = lh->parent->parent;
		node* n = lh;
		while (n->freq <= q->freq)
		{
			if (!n->parent)
				break;
			n = n->parent;
		}

		if (n->parent && n != lh) //now insert above n
		{
			q->parent = n->parent;
			n->parent = q;
		}
		else if (!n->parent && n == lh)
		{
			if (n->freq < q->freq)
			{
				n->parent = q;
				lh =n;
				uh = q;
			}
			else
			{
				q->parent = n;
            n->parent = NULL;
				uh = n;
				lh = q;
			}
		}
		else if (n == lh)
		{
			lh = q;
			q->parent = n;
		}
		else if (!n->parent)
		{
			uh = q;
			n->parent = q;
		}
	}
	return uh;
}

int main(int argc, char* argv[])
{
	int16_t occurrences[255];

	//TODO replace this with calloc so its already initialized
	for (int i = 0; i < 256; i++)
	{
		occurrences[i] = 0;
	}
	for (int32_t i = 0; i < strlen(argv[1]); i++)
	{
		if (argv[1][i])                           //we're not encoding ascii null
			occurrences[(int)(argv[1][i])/* - 1*/]++;
	}

	node* current;
	node* prev = NULL;
   for (int c = 0; (int)c < 255; c++)
	{
		int created = 0;
		if (occurrences[c])
		{
			printf("freq %d, allocating %zd\n", occurrences[c], sizeof(node));
			current = (node*)calloc(1, sizeof(node));
			if (!current)
			{
				printf("Could not allocate more memory\n");
				exit(1);
			}

			current->c = (char)c;
			current->freq = occurrences[c];
			if (created == 0)
				freq_head = current;
			else
				prev->parent = current;

			prev = current;
			created++;
		}
	}
	
	node* sorted_freq_head = insertion_sort(freq_head);

	//node* p;
	//for (p = sorted_freq_head; p; p = p->parent)
	//	printf("%d ", p->freq);
	//printf("\n\n");

	//node* q;
	//for (q = sorted_freq_head; q->parent; q = q->parent);

	//for (q; q->child_l; q = q->child_l)
	//	printf("%d ", q->freq);
	//printf("\n\n");
	
	node* root = build_huff_tree(sorted_freq_head);
	node* p;
	for (p = root; p; p = p->child_l)
		printf("%c\n", p->c);

	return 0;
}
