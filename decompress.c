#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <fcntl.h>

#define ERR_CHECK_ALLOC(P) { if (!P) { printf("Could not allocate more memory\n"); exit(1); } }
#define ERR_FILEREAD       printf("Could not read from file.\n");\
                           exit(1);

#define MARKER             0x7 //BELL character


typedef struct node node;
struct node
{
   node* parent;
   node* child_l;
   node* child_r;
   char c;
};

typedef struct string string;
struct string
{
    size_t alloc_size;
    size_t size;
    unsigned char* chars;
};

node* deserialize_tree(unsigned char* serialized, size_t length)
{
    node* new_node = 0;
    node* next_parent = 0;
    node* root = 0;
    int next_node_is_right = 0;


    for (size_t i = 0; i < length; i++)
    {
        if (serialized[i] == MARKER)
        {
            if (next_node_is_right)
            {
                return 0;
            }
            next_node_is_right = 1;
            continue;
        }

        //WRONG WRONG WRONG DOESNT WORK
        new_node = calloc(1, sizeof(node));
        ERR_CHECK_ALLOC(new_node);


        new_node->c = serialized[i];
        if (i == 0)
        {
            next_parent = new_node;
            root = new_node;
            continue;
        }

        new_node->parent = next_parent;
        if (next_node_is_right)
        {
            new_node->parent->child_r = new_node;
            next_node_is_right = 0;
        }
        else
        {
            new_node->parent->child_l = new_node;
        }


        if (i < length - 1 && serialized[i + 1] != MARKER)
        {
            next_parent = new_node;
        }
        else if (i < length - 1 && serialized[i + 1] == MARKER)
        {
           while (next_parent->child_r)
              next_parent = next_parent->parent;
        }

    }
    return root;
}

char* decode(char* encoded, int32_t length, node* root)
{
    char* decoded = calloc(length, sizeof(char));
    int32_t enc_byte = 0;
    int32_t dec_byte = 0;

    node* current_node = root;
    uint8_t bit_offset = 0;

    while (dec_byte < length)
    {
        for (bit_offset = 0; bit_offset < sizeof(char) * 8; bit_offset++)
        {
            uint8_t bit = (encoded[enc_byte] >> (sizeof(char) * 8 - 1 - bit_offset)) & 0x1;
            current_node = bit ? current_node->child_r : current_node->child_l;

            if (!current_node->child_l && !current_node->child_r)
            {
                decoded[dec_byte] = current_node->c;
                dec_byte++;
                if (dec_byte == length)
                {
                   goto done;
                }
                current_node = root;
            }
        }
        enc_byte++;
    }
done:
    return decoded;
}

void dealloc_tree(node* n)
{
    if (!n)
        return;
    if (!n->child_l && !n->child_r)
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

int main(int argc, char* argv[])
{
    if (!argv[1])
    {
        printf("Usage: %s <file>\n", argv[0]);
        exit(1);
    }
    FILE* input_file = fopen(argv[1], "r+b");
    if (!input_file)
    {
        printf("Could not open file for reading.\n");
        exit(1);
    }

    int32_t text_length = 0; //length of the unencoded text
    int16_t tree_length = 0;


    size_t fsize = 0; //length of the encoded text in the file

    if (!fread(&text_length, sizeof(int32_t), 1, input_file))
    {
        ERR_FILEREAD;
    }
    if (!fread(&tree_length, sizeof(int16_t), 1, input_file))
    {
        ERR_FILEREAD;
    }


    /*find start of encoded text after tree*/
    size_t bytes_before_text = sizeof(text_length) + sizeof(tree_length) + tree_length * sizeof(char);

    long int offset = ftell(input_file);
    fseek(input_file, 0L, SEEK_END);
    fsize = ftell(input_file) - bytes_before_text;

    fseek(input_file, offset, SEEK_SET);

    unsigned char* serialized_tree = calloc(tree_length, sizeof(char));
    ERR_CHECK_ALLOC(serialized_tree);

    if (fread(serialized_tree, sizeof(char), tree_length, input_file) != tree_length)
    {
        ERR_FILEREAD;
    }

    char* buffer = calloc(fsize, sizeof(unsigned char));
    if (!buffer)
    {
       fclose(input_file);
       exit(1);
    }

    if(fread(buffer, sizeof(char), fsize, input_file) != fsize)
    {
        ERR_FILEREAD;
    }

    node* root = deserialize_tree(serialized_tree, tree_length);
    if (!root)
    {
        printf("Corrupted tree in file. Aborting.\n");
        exit(1);
    }

    char* text = decode(buffer, text_length, root);
    printf("%.*s", text_length, text);

    fclose(input_file);
    dealloc_tree(root);
    free(serialized_tree);
    free(text);
    free(buffer);
    return 0;
}
