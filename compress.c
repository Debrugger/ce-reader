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

typedef struct encoded_string encoded_string;
struct encoded_string
{
    size_t size;
    unsigned char* string;
};

void node_init(node* p)
{
   p->parent = p->child_l = p->child_r = 0;
   p->freq = p->c = p->is_leaf = 0;
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

   for (node* i = lh; i; i = i->parent)
   {
      i->child_l = i->child_r = NULL;
   }

   while (uh != lh)
   {
      node* lh_next = lh->parent->parent;
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


      if (a->freq <= b->freq)
      {
         q->child_l = a;
         q->child_r = b;
      }
      else
      {
         q->child_l = b;
         q->child_r = a;
      }
      a->parent = q;
      b->parent = q;

      if (!lh_next)
      {
         return q;
      }

      node* n = lh_next;
      if (n->freq == q->freq) //this part to make sure that if they're equal it gets inserted below
      {
          q->parent = n;
          lh = q;
          continue;
      }
      while (n->parent)
      {
         if(n->parent->freq >= q->freq)
            break;
         n = n->parent;
      }
      if (!n->parent)
      {
         n->parent = q;
         uh = q;
         q->parent = NULL;
      }
      //else if (n->freq == q->freq)
      //{
      //    q->parent = n;
      //    lh = q;
      //}
      else
      {
         q->parent = n->parent;
         n->parent = q;
      }
      if (!lh_next)
         return uh;
      lh = lh_next;
   }
   return uh;
}

void dealloc_tree(node* n)
{
    if (n->is_leaf)
    {
        free(n);
        return;
    }
    else
    {
        dealloc_tree(n->child_l);
        dealloc_tree(n->child_r);
    }
    free(n);
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

void printBits(size_t const size, void const * const ptr) //from https://stackoverflow.com/a/3974138
{
   unsigned char *b = (unsigned char*) ptr;
   unsigned char byte;
   int i, j;

   for (i=size-1;i>=0;i--)
   {
      for (j=7;j>=0;j--)
      {
         byte = (b[i] >> j) & 1;
         printf("%u", byte);
      }
   }
   puts("");
}

encoded_string* encode(ENCODE_T* bufin, node* root)
{
   typedef struct encoded_char encoded_char;
   struct encoded_char
   {
      int nb_bits;
      int32_t encoded;
   };
   node** lookup = calloc(ENCODE_TMAX + 1, sizeof(node*));
   get_nodes_by_char(&lookup[0], root);

   encoded_char dict[ENCODE_TMAX + 1];
   for (int c = 0; c < ENCODE_TMAX + 1; c++)
   {
      node* n = lookup[c];

      dict[c].nb_bits = 0;
      dict[c].encoded = 0x0;

      if (!n)
         continue;

      if (c != n->c)
      {
         printf("LOOKUP IS WRONG!!\n");
      }

      while (n != root)
      {
         dict[c].encoded <<= 1;
         dict[c].encoded |= (n == n->parent->child_l) ? 0x0 : 0x1; //writing the bits in reverse order starting from LSB. TODO reverse this

         dict[c].nb_bits++;
         n = n->parent;
      }
   }

   free(lookup);

   //printing out the new alphabet
   //for (int s = 0; s <= ENCODE_TMAX; s++)
   //{
   //   if (!dict[s].nb_bits)
   //      continue;
   //   printf("%c: %d bits ", s, dict[s].nb_bits);
   //   printBits(sizeof(int32_t), &dict[s].encoded);
   //   printf("\n");
   //}



   size_t bufout_sz = 512;
   unsigned char* encoded = (unsigned char*)(calloc(bufout_sz, sizeof(unsigned char)));
   if (!encoded)
   {
       printf("Could not allocate memory.\n");
       exit(1);
   }
   unsigned char buffer = 0x0;

   size_t current_byte = 0;
   int bits_written_cur_byte = 0; //bits written in current byte
   int bits_written_cur_symbol = 0; //bits of current encoded char already written
   int symbol_index = 0; //symbol of input being encoded
   ENCODE_T current_symbol = 0;
   encoded_char encoded_symbol;

   for (symbol_index = 0; bufin[symbol_index] != 0; symbol_index++)
   {
       current_symbol = bufin[symbol_index];
       encoded_symbol = dict[current_symbol];
       printf("encoding symbol '%c'\n", current_symbol);

       if (current_byte == bufout_sz - 1)
       {
           bufout_sz *= 1.5;
           encoded = realloc(encoded, bufout_sz);
           if (!encoded)
           {
               printf("Failed to allocate more memory. buffer size is %ld\n", bufout_sz);
               exit(1);
           }
       }

       for (int i = 0; i < encoded_symbol.nb_bits; i++)
       {
           int bit_to_write = (encoded_symbol.encoded >> bits_written_cur_symbol) & 0x1;
           buffer |= bit_to_write;
           bits_written_cur_byte++;
           bits_written_cur_symbol++;
           if (bits_written_cur_byte == 8 || (bufin[symbol_index + 1] == 0 /*&& bits_written_cur_byte != 0*/))
           {
               bits_written_cur_byte = 0;

               printf("flushing buffer: current byte is %ld\n", current_byte);

               encoded[current_byte] = buffer;
               buffer = 0x0;
               current_byte++;
           }
           buffer <<= 1;
           if (bits_written_cur_symbol == encoded_symbol.nb_bits)
           {
               bits_written_cur_symbol = 0;
               break;
           }
       }
   }

   encoded_string* ret = (encoded_string*)calloc(1, sizeof(encoded_string));
   if (!ret)
   {
       printf("Failed to allocate more memory. buffer size is %ld\n", bufout_sz);
       exit(1);
   }

   ret->size = current_byte;

   ret->string = (unsigned char*)calloc(ret->size, sizeof(unsigned char));
   if (!ret->string)
   {
       printf("Failed to allocate more memory. buffer size is %ld\n", bufout_sz);
       exit(1);
   }

   memcpy((char*)ret->string, (char*)encoded, ret->size);
   free(encoded);

   return ret;
}

void serialize_tree(node* n)
{
    if (n->is_leaf)
    {
        printf("%c", n->c);
    }
    else
    {
        printf("\\0");
        serialize_tree(n->child_l);
        printf("X");
        serialize_tree(n->child_r);
    }
}

int main(int argc, char* argv[])
{        
   char text[256];
   strcpy(text, argv[1]);
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

   node* sorted_freq_head = insertion_sort(freq_head);
   node* root = build_huff_tree(sorted_freq_head);

   encoded_string* e = encode((unsigned char*)text, root);

   serialize_tree(root);

   dealloc_tree(root);

   printf("\n\nENCODED STRING:\n");
   for (int i = 0; i < e->size; i++)
      printBits(sizeof(unsigned char), &e->string[i]);

   free(e->string);
   free(e);
   printf("input was %ld long\n", strlen(text));
   return 0;
}
