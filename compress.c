#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <fcntl.h>

#define ERR_CHECK_ALLOC(P) { if (!P) { printf("Could not allocate more memory\n"); exit(1); } }

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

typedef struct string string;
struct string
{
    size_t alloc_size;
    size_t size;
    unsigned char* chars;
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
      ERR_CHECK_ALLOC(q)
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

string* encode(ENCODE_T* bufin, node* root)
{
   typedef struct encoded_char encoded_char;
   struct encoded_char
   {
      int nb_bits;
      int32_t encoded;
   };
   node** lookup = calloc(ENCODE_TMAX + 1, sizeof(node*));
   ERR_CHECK_ALLOC(lookup)

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
   ERR_CHECK_ALLOC(encoded)

   unsigned char buffer = 0x0;

   size_t current_byte = 0;
   int bits_written_cur_byte = 0; //bits written in current byte
   int bits_written_cur_symbol = 0; //bits of current encoded char already written
   int symbol_index = 0; //symbol of input being encoded
   ENCODE_T current_symbol = 0;
   encoded_char encoded_symbol;

   for (symbol_index = 0; bufin[symbol_index] != '\0'; symbol_index++)
   {
       current_symbol = bufin[symbol_index];
       encoded_symbol = dict[current_symbol];
       printf("encoding symbol '%c'\n", current_symbol);

       if (current_byte == bufout_sz - 1)
       {
           bufout_sz *= 1.5;
           encoded = realloc(encoded, bufout_sz);
           ERR_CHECK_ALLOC(encoded)
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

   string* ret = (string*)calloc(1, sizeof(string));
   if (!ret)
   {
       printf("Failed to allocate more memory. buffer size is %ld\n", bufout_sz);
       exit(1);
   }

   ret->size = current_byte;
   ret->alloc_size = bufout_sz;

   ret->chars = (unsigned char*)calloc(ret->size, sizeof(unsigned char));
   ERR_CHECK_ALLOC(ret->chars)

   memcpy((char*)ret->chars, (char*)encoded, ret->size);
   free(encoded);

   return ret;
}

void serialize_tree(node* n, string* s) //string s should be allcoated beforehand
{
    if (s->size == s->alloc_size)
    {
        s->alloc_size *= 1.5;
        s->chars = realloc(s->chars, s->alloc_size);
        ERR_CHECK_ALLOC(s->chars)
    }


    if (n->is_leaf)
    {
        *(char*)(s->chars + s->size) = n->c;
        s->size++;
        return;
    }
    else
    {
        *(char*)(s->chars + s->size) = '\0';
        s->size++;
        serialize_tree(n->child_l, s);
        *(char*)(s->chars + s->size) = 0x7; //BELL char as a marker
        s->size++;
        serialize_tree(n->child_r, s);
    }
}

int main(int argc, char* argv[])
{        

   if (!argv[1] || !argv[2])
   {
       printf("Usage: compress <input_file> <output_file>");
       exit(1);
   }

   FILE* file;
   size_t fsize;
   char* buffer;

   file = fopen (argv[1] , "rb");
   if (!file)
   {
       printf("Failed to open file '%s'\n", argv[1]);
       exit(1);
   }

   fseek(file , 0L , SEEK_END); //get the number of bytes in the file
   fsize = ftell(file);
   rewind(file);

   buffer = calloc(1, fsize + 1);
   if (!buffer)
   {
       fclose(file);
       exit(1);
   }

   if (fread(buffer, sizeof(char), fsize, file) != fsize)
   {
      fclose(file);
      free(buffer);
      printf("Failed to read file\n");
      exit(1);
   }

   fclose(file);


   /* build linked list of all symbols with their frequency */
   node* freq_head = NULL;

   int occurrences[ENCODE_TMAX];

   for (int i = 0; i < ENCODE_TMAX; i++)
   {
      occurrences[i] = 0;
   }
   for (int i = 0; i < fsize; i++)
   {
      if (i == '\a') //we will not encode the BELL character as that will be used as a marker in the serialization
          continue;
      occurrences[(int)(buffer[i])]++;
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

   string* e = encode((unsigned char*)buffer, root);

   free(buffer);

   string serialized = {0};
   serialized.alloc_size = 64;
   serialized.chars = calloc(serialized.alloc_size, sizeof(char));
   serialize_tree(root, &serialized);


   /*
   for (int i = 0; i < serialized.size; i++)
   {
       char c = *(serialized.chars + i * sizeof(char));
       if (c > 39 && c != 0x7)
          printf("%c\n", c);
       else if (c && c <= 39)
           printf("%d\n", c);
       else if (c)
          printf("XX\n");
       else
           printf("\\0\n");
   } */

   dealloc_tree(root);

   /* write the output file:
    * first 4 bytes: size of the encoded text
    * next 2 bytes: size of the serialized tree
    * serialized tree
    * encoded text */

   FILE* out;
   out = fopen(argv[2], "w+b");
   if (!out)
   {
       printf("Could not create output file.\n");
       exit(1);
   }

   int32_t length = e->size;
   int16_t tree_length = serialized.size;

   int write_success = 0;

   write_success |= (fwrite(&length, sizeof(length), 1, out) == 1);
   write_success &= (fwrite(&tree_length, sizeof(tree_length), 1, out) == 1);
   write_success &= (fwrite(e->chars, 1, e->size, out) == e->size);

   if (!write_success )
   {
       printf("Could not write to output file.\n");
       exit(1);
   }
   fclose(out);

   free(e->chars);
   free(e);
   printf("input was %ld long\n", fsize);
   return 0;
}
