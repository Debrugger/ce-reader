#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <fcntl.h>

#define ENCODE_T     unsigned char
#define ENCODE_TSIZE sizeof(ENCODE_TYPE)
#define ENCODE_TMAX  UCHAR_MAX
typedef struct node node;

struct node
{
   node* parent;
   node* child_l;
   node* child_r;
   int is_leaf;
   ENCODE_T c;
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


void node_init(node* p)
{
   p->parent = p->child_l = p->child_r = 0;
   p->freq = p->c = p->is_leaf = 0;
}

void get_nodes_by_char(node** dict, node* n)
{
   if (!n->is_leaf)
   {
      get_nodes_by_char(dict, n->child_l);
      get_nodes_by_char(dict, n->child_r);
   }
   else
   {
      dict[n->c] = n;
   }
}

char* encode(ENCODE_T* bufin, node* root)
{
   typedef struct encoded_char encoded_char;
   struct encoded_char
   {
      int nb_bits;
      int32_t encoded;
   };
   node** lookup = calloc(ENCODE_TMAX, sizeof(node*));
   get_nodes_by_char(&lookup[0], root);

   encoded_char dict[ENCODE_TMAX];
   for (ENCODE_T c = 0; c < ENCODE_TMAX; c++)
   {
      node* n = lookup[c];
      if (!n)
         continue;
      dict[c].nb_bits = 0;
         printf("encoding %c\n", c);
      while (n != root)
      {

         dict[c].encoded |= (n == n->parent->child_l) ? 0 : 1; //writing the bits in reverse order starting from LSB. TODO reverse this
         dict[c].encoded	<<= 1;

         dict[c].nb_bits++;
         n = n->parent;
      }
      //Reversing the bits so it starts at MSB
      int32_t temp = 0;
      for (int i = 0; i < 8 * sizeof(int32_t); i++)
      {
         if ((dict[c].encoded >> i) % 2) //if after shifting lsb is set
            temp |= (1 << (8 * sizeof(int32_t) - i));
      }
      dict[c].encoded = temp;
   }

   char* encoded = (char*)(calloc(512, sizeof(char)));
   int index = 0;
   char buffer = 0;
   int bits_written = 0; //number of bits from previous symbol we already wrote to the buffer
   int bits_to_write = 0; //number of bits of current symbol to write

   for (ENCODE_T symbol = bufin[0]; symbol; symbol++)
   {
      bits_to_write = dict[symbol].nb_bits;

      while (bits_to_write >= sizeof(unsigned char) && bits_written == 0)
      {
         buffer |= (dict[symbol].encoded >> (sizeof(int32_t) * 8 - 8));
         encoded[index++] = buffer;
         buffer = 0;
         bits_to_write -= sizeof(unsigned char);
      }
      if (bits_written && bits_to_write >= 8)
      {
      }
      if (bits_to_write < 8)
      {
         buffer |= dict[symbol].encoded >> (sizeof(int32_t) * 8 - (8 - bits_written));
         if (bits_to_write + bits_written == 8)
         {
            bits_written = 0;
            bits_to_write = 0;
            encoded[index++] = buffer;
            buffer = 0;
         }
         else if (bits_written + bits_written > 8)
         {
            bits_to_write -= 8 - bits_written;
            bits_written = 0;
            encoded[index++] = buffer;
            buffer = 0;
         }
         else
         {
            bits_written += bits_to_write;
            bits_to_write = 0;
         }
      }
   }
   return encoded;
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
            printf("sorted head is %c, sorted head freq is %d, unsrted head %c freq is %d\n", sorted_head->c, sorted_head->freq, unsorted_head->c, unsorted_head->freq);
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

         if (sorted_cur->parent)
            sorted_cur->parent->child_l = unsorted_head;
         unsorted_head->parent = sorted_cur->parent;
         sorted_cur->parent = unsorted_head;
         unsorted_head->child_l = sorted_cur;
      }
      unsorted_head = unsorted_next;
   }
   return sorted_head;
}

node* build_huff_tree(node* lh) //lh is the least frequent node
{
   node *uh, *a, *b, *q;
   for (uh = lh; uh->parent; uh = uh->parent);

   //pretty sure not needed? if frequency is 0 then node is not even created in main
   //node* r;
   //for (r = lh; r; r = r->parent)
   //{
   //	if (!r->freq)
   //	{
   //		printf("freeing in build tree\n");
   //		free(r);
   //	}
   //	else
   //		break;
   //}
   //lh = r;

   while (uh != lh)
   {
      if (!lh->parent)
      {
         printf("%c breaking\n", lh->c);
         break;
      }
      a = lh;
      b = lh->parent;
      q = (node*)malloc(sizeof(node));
      node_init(q);
      q->freq = a->freq + b->freq;

      q->child_l = (a->freq <= b->freq) ? a : b;
      q->child_r = (a->freq > b->freq) ? a : b;
      a->parent = b->parent = q;
      a->child_l = a->child_r = b->child_l = b->child_r = NULL;

      if (!lh->parent->parent)
      {
          uh = q;
          return uh;
      }
      lh = lh->parent->parent;
      if (lh == uh)
      {
         uh = q;
         return uh;
      }
      node* n = lh;
      while (n->parent)
      {
          if (n->parent->freq > q->freq)
              break;
          n = n->parent;
      }

      q->parent = n->parent;
      n->parent = q;

   }
   return uh;
}

int main(int argc, char* argv[])
{        
   char* text = argv[1];
   node* freq_head = NULL;

   int occurrences[ENCODE_TMAX];

   for (int i = 0; i < ENCODE_TMAX; i++)
   {
      occurrences[i] = 0;
   }
   for (int i = 0; i < strlen(text); i++)
   {
      occurrences[(int)(text[i])]++;
   }

   node* current;
   node* prev = NULL;
   int created = 0;
   for (int c = 0; (int)c < ENCODE_TMAX; c++)
   {
      if (occurrences[c])
      {
         printf("freq %d %c, allocating %zd\n", occurrences[c], c, sizeof(node));
         current = (node*)malloc(sizeof(node));
         if (!current)
         {
            printf("Could not allocate more memory\n");
            exit(1);
         }

         node_init(current);
         current->c = (ENCODE_T)c;
         current->is_leaf = 1;
         current->freq = occurrences[c];
         if (created == 0)
            freq_head = current;
         else
            prev->parent = current;

         prev = current;
         created++;
      }
   }

   node* q;

   node* sorted_freq_head = insertion_sort(freq_head);

   for (q = sorted_freq_head; q; q = q->parent)
    printf("%li ", q->freq);
   printf("\n\n");

   node* root = build_huff_tree(sorted_freq_head);


   //for (q = sorted_freq_head; q->parent; q = q->parent);

   //for ( ; q->child_l; q = q->child_l)
   //	printf("%li ", q->freq);
   //printf("\n\n");

   //int i = 2;
   //for (q = root; q; q = i % 3 ? q->child_l : q->child_r)
   //{
   //   i++;
   //   printf("%c\n", q->c);
   //}

   //char* e = encode(text, root);
   //printf("encoded:'%s'\n", e);

   return 0;
}
