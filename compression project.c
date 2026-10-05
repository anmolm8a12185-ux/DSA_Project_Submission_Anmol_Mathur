#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 256

struct Node
{
    unsigned char data;
    unsigned long frequency;
    struct Node *left;
    struct Node *right;
};

struct Node* createNode(unsigned char data, unsigned long frequency)
{
    struct Node *newNode;
    newNode = (struct Node*)malloc(sizeof(struct Node));
    newNode->data = data;
    newNode->frequency = frequency;
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}

int isLeaf(struct Node *root)
{
    if(root->left == NULL && root->right == NULL)
        return 1;

    return 0;
}

int findSmallest(struct Node *nodes[], int count)
{
    int i;
    int smallest = 0;

    for(i = 1; i < count; i++)
    {
        if(nodes[i]->frequency < nodes[smallest]->frequency)
        {
            smallest = i;
        }
    }

    return smallest;
}

struct Node* buildTree(unsigned long frequency[])
{
    struct Node *nodes[SIZE];

    int count = 0;
    int i;

    for(i = 0; i < SIZE; i++)
    {
        if(frequency[i] > 0)
        {
            nodes[count] = createNode((unsigned char)i,frequency[i]);

            count++;
        }
    }

    if(count == 0)
        return NULL;

    if(count == 1)
    {
        struct Node *parent;

        parent = createNode(0, nodes[0]->frequency);

        parent->left = nodes[0];

        return parent;
    }

    while(count > 1)
    {
        int first;
        int second;

        struct Node *firstNode;
        struct Node *secondNode;
        struct Node *newNode;

        first = findSmallest(nodes, count);

        firstNode = nodes[first];

        nodes[first] = nodes[count - 1];
        count--;

        second = findSmallest(nodes, count);

        secondNode = nodes[second];

        newNode = createNode(0,firstNode->frequency +secondNode->frequency);

        newNode->left = firstNode;
        newNode->right = secondNode;

        nodes[second] = nodes[count - 1];

        count--;

        nodes[count] = newNode;
        count++;
    }

    return nodes[0];
}

void generateCodes(struct Node *root,
                   char code[],
                   int position,
                   char codes[SIZE][SIZE])
{
    if(root == NULL)
        return;

    if(isLeaf(root))
    {
        code[position] = '\0';

        strcpy(codes[root->data], code);

        return;
    }

    code[position] = '0';

    generateCodes(root->left,
                  code,
                  position + 1,
                  codes);

    code[position] = '1';

    generateCodes(root->right,
                  code,
                  position + 1,
                  codes);
}

void freeTree(struct Node *root)
{
    if(root == NULL)
        return;

    freeTree(root->left);
    freeTree(root->right);

    free(root);
}

void removeQuotes(char path[])
{
    int length;

    length = strlen(path);

    if(length >= 2)
    {
        if(path[0] == '"' &&
           path[length - 1] == '"')
        {
            path[length - 1] = '\0';

            memmove(path,
                    path + 1,
                    length - 1);
        }
    }
}

unsigned long getFileSize(char filename[])
{
    FILE *file;
    unsigned long size;

    file = fopen(filename, "rb");

    if(file == NULL)
        return 0;

    fseek(file, 0, SEEK_END);

    size = ftell(file);

    fclose(file);

    return size;
}

void compressFile()
{
    FILE *input;
    FILE *output;

    char inputName[500];
    char folderPath[500];
    char outputName[600];

    unsigned long frequency[SIZE] = {0};

    struct Node *root;

    char codes[SIZE][SIZE] = {{0}};
    char code[SIZE];

    int ch;
    int i;

    unsigned long originalSize;
    unsigned long compressedSize;

    printf("\nEnter file to compress: ");

    scanf(" %499[^\n]", inputName);

    removeQuotes(inputName);

    input = fopen(inputName, "rb");

    if(input == NULL)
    {
        printf("\nERROR: File could not be opened.\n");
        return;
    }

    while((ch = fgetc(input)) != EOF)
    {
        frequency[(unsigned char)ch]++;
    }

    fseek(input, 0, SEEK_END);

    originalSize = ftell(input);

    rewind(input);

    if(originalSize == 0)
    {
        printf("\nFile is empty.\n");

        fclose(input);

        return;
    }

    root = buildTree(frequency);

    if(root == NULL)
    {
        printf("\nCould not build Huffman tree.\n");

        fclose(input);

        return;
    }

    generateCodes(root,
                  code,
                  0,
                  codes);

    printf("\nEnter folder where you want to save the compressed file: ");

    scanf(" %499[^\n]", folderPath);

    removeQuotes(folderPath);

    char *fileName;

    fileName = strrchr(inputName, '\\');

    if(fileName == NULL)
    {
        fileName = strrchr(inputName, '/');
    }

    if(fileName == NULL)
    {
        fileName = inputName;
    }
    else
    {
        fileName++;
    }

    strcpy(outputName, folderPath);

    int length;

    length = strlen(outputName);

    if(outputName[length - 1] != '\\' &&
       outputName[length - 1] != '/')
    {
        strcat(outputName, "\\");
    }

    strcat(outputName, fileName);

    strcat(outputName, ".huf");

    output = fopen(outputName, "wb");

    if(output == NULL)
    {
        printf("\nERROR: Could not create output file.\n");
        printf("Please check the folder path.\n");

        fclose(input);
        freeTree(root);

        return;
    }

    fwrite(&originalSize,
           sizeof(unsigned long),
           1,
           output);

    fwrite(frequency,
           sizeof(unsigned long),
           SIZE,
           output);

    unsigned char buffer = 0;

    int bitCount = 0;

    while((ch = fgetc(input)) != EOF)
    {
        char *currentCode;

        currentCode = codes[(unsigned char)ch];

        for(i = 0;
            currentCode[i] != '\0';
            i++)
        {
            buffer = buffer << 1;

            if(currentCode[i] == '1')
            {
                buffer = buffer | 1;
            }

            bitCount++;

            if(bitCount == 8)
            {
                fwrite(&buffer,
                       sizeof(unsigned char),
                       1,
                       output);

                buffer = 0;

                bitCount = 0;
            }
        }
    }

    if(bitCount > 0)
    {
        buffer = buffer << (8 - bitCount);

        fwrite(&buffer,
               sizeof(unsigned char),
               1,
               output);
    }

    fclose(input);
    fclose(output);
    compressedSize = getFileSize(outputName);
    printf("\n       COMPRESSION COMPLETE\n");
    printf("\nOriginal Size   : %lu bytes",
           originalSize);

    printf("\nCompressed Size : %lu bytes",
           compressedSize);

    if(originalSize > 0)
    {
        double saved;

        saved = ((double)(originalSize -
                          compressedSize)
                          / originalSize) * 100;

        printf("\nSpace Saved     : %.2f%%",
               saved);
    }

    printf("\n\nCompressed file saved at:");
    printf("\n%s\n", outputName);

    freeTree(root);
}

void decompressFile()
{
    FILE *input;
    FILE *output;

    char inputName[500];
    char outputName[500];

    unsigned long frequency[SIZE];

    unsigned long originalSize;

    struct Node *root;
    struct Node *current;

    unsigned long decoded = 0;

    int ch;
    int i;

    printf("\nEnter .huf file to decompress: ");

    scanf(" %499[^\n]", inputName);

    removeQuotes(inputName);

    input = fopen(inputName, "rb");

    if(input == NULL)
    {
        printf("\nERROR: File could not be opened.\n");
        return;
    }

    fread(&originalSize,
          sizeof(unsigned long),
          1,
          input);

    fread(frequency,
          sizeof(unsigned long),
          SIZE,
          input);

    root = buildTree(frequency);

    if(root == NULL)
    {
        printf("\nERROR: Invalid compressed file.\n");

        fclose(input);

        return;
    }

    printf("\nEnter output file name: ");

    scanf(" %499[^\n]", outputName);

    removeQuotes(outputName);

    output = fopen(outputName, "wb");

    if(output == NULL)
    {
        printf("\nERROR: Could not create output file.\n");

        fclose(input);
        freeTree(root);

        return;
    }

    current = root;

    while((ch = fgetc(input)) != EOF &&
          decoded < originalSize)
    {
        unsigned char byte;

        byte = (unsigned char)ch;

        for(i = 7; i >= 0; i--)
        {
            int bit;

            bit = (byte >> i) & 1;

            if(bit == 0)
            {
                current = current->left;
            }
            else
            {
                current = current->right;
            }

            if(isLeaf(current))
            {
                fputc(current->data,
                      output);

                decoded++;

                current = root;

                if(decoded == originalSize)
                    break;
            }
        }
    }

    fclose(input);
    fclose(output);
    printf("\n      DECOMPRESSION COMPLETE\n");

    printf("\nRestored File : %s",
           outputName);

    printf("\nRestored Size : %lu bytes\n",
           decoded);

    freeTree(root);
}

int main()
{
    int choice;

    do
    {
        printf("\n        FILE COMPRESSOR\n");

        printf("\n1. Compress File");
        printf("\n2. Decompress File");
        printf("\n3. Exit");

        printf("\n\nEnter your choice: ");

        scanf("%d", &choice);

        switch(choice)
        {
            case 1:
                compressFile();
                break;

            case 2:
                decompressFile();
                break;

            case 3:
                printf("\nThank you!\n");
                break;

            default:
                printf("\nInvalid choice!\n");
        }

    } while(choice != 3);

    return 0;
}
