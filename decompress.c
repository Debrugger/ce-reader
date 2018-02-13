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
   int8_t is_leaf;
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
    node* higher_node = 0;
    node* root = 0;
    int next_node_is_right = 0;

    printf("deserializing len %lu\n", length);
    for (size_t i = 0; i < length; i++)
    {
        printf("deserializing %d\n", serialized[i]);
        if (serialized[i] == MARKER)
        {
            printf("found marker\n");
            next_node_is_right = 1;
            continue;
        }

        new_node = calloc(1, sizeof(node));
        ERR_CHECK_ALLOC(new_node)

        if (i == 0)
        {
            root = new_node;
        }


        new_node->parent = higher_node;
        new_node->c = (char)serialized[i];

        if(next_node_is_right)
        {
            next_node_is_right = 0;
            higher_node->child_r = new_node;
        }
        else if (i != 0)
        {
            higher_node->child_l = new_node;
        }
        if (i < length - 1 && serialized[i + 1] != MARKER)
        {
            higher_node = new_node;
        }
        else if (i != 0 && serialized[i - 1] == MARKER)
        {
            higher_node = higher_node->parent;
        }
    }
    return root;
}

char* decode(char* encoded, int32_t length, node* root)
{
    char* decoded = calloc(length, sizeof(char));
    int32_t enc_byte = 0;
    int32_t dec_byte = 0;
    printf("decoding length %d\n", length);

    node* current_node = root;
    uint8_t bit_offset = 0;

    while (dec_byte < length)
    {
        for (bit_offset = 0; bit_offset < sizeof(char) * 8; bit_offset++)
        {
            uint8_t bit = (encoded[enc_byte] >> (sizeof(char) * 8 - 1 - bit_offset)) & 0x1;
            current_node = bit ? current_node->child_r : current_node->child_l;

            printf("bit %d\n", bit);
            if (!current_node->child_l && !current_node->child_r)
            {
                printf("no children\n");
                decoded[dec_byte] = current_node->c;
                dec_byte++;
                current_node = root;
            }
        }
        printf("decoding byte #%d: %c\n", dec_byte, decoded[dec_byte]);
        enc_byte++;
    }
    return decoded;
}

int main(int argc, char* argv[])
{
    if (!argv[1])
    {
        printf("Usage: decompress <file>\n");
        exit(1);
    }
    FILE* input_file = fopen(argv[1], "r");
    if (!input_file)
    {
        printf("Couldn not open file for reading.\n");
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

    size_t bytes_before_text = sizeof(text_length) + sizeof(tree_length) + tree_length * sizeof(char);

    long int offset = ftell(input_file);
    printf("bytes before text %d\n", bytes_before_text);
    fseek(input_file, 0L, SEEK_END);
    fsize = ftell(input_file) - bytes_before_text;

    fseek(input_file, offset, SEEK_SET);

    unsigned char* serialized_tree = calloc(tree_length, sizeof(char));
    ERR_CHECK_ALLOC(serialized_tree);

    printf("tree length %d\n", tree_length);

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

    printf("size of encoded text: %ld\n", fsize);
    if(fread(buffer, sizeof(unsigned char), fsize, input_file) != fsize)
    {
        ERR_FILEREAD;
    }
    printf("first byte in buffer is %d\n", buffer[0]);

    node* root = deserialize_tree(serialized_tree, tree_length);

    char* text = decode(buffer, text_length, root);
    printf("%s", text);

    fclose(input_file);
    free(buffer);
    return 0;
}
