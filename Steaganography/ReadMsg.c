#include <stdio.h>
#include <stdlib.h>

int readStringSize(char * argv);
void readHiddenString(char * argv);
void printString(char * str);

//common to both files, here for now
int isWhitespace(char c);
int removeComments(char * argv);
FILE * afterComments(char * argv);
int removeDimensions(char * argv);
FILE * afterDimensions(char * argv);
int power(int base, int exponent);
int binaryToDen(int * arr);
void printBitArray(int * arr);

//gcc ReadMsg.c -o ReadMsg; ./ReadMsg result.ppm

//User wants to read hidden message in a file
int main(int argc, char * argv[])
{
    FILE * readingFile = fopen(argv[1], "r");
    if(readingFile == '\0')
    {
        puts("Wrong Input Format For Reading");
        fclose(readingFile);
        return -1;
    }
    else
    {
        readHiddenString(argv[1]);
        fclose(readingFile);
    }
}

/*
We want to read the first 8 bytes of the file after the header
Make an array for a bitstring, with terminator as -1
Get each byte which is the same as get each hex
Check the last bit of the byte we got using bitwise operator AND (& 0b00000001)
If it results in 1, add 1 to the bitstring, else add 0
By the end of the 8 iterations, we would have a filled bitstring and convert this value into denary
Return this denary value
*/
int readStringSize(char * argv)
{
    FILE * newCfptr = afterDimensions(argv);

    int * arr = (int *)malloc(sizeof(int) * 9);
    arr[8] = -1;
    
    
    int c;
    for(int i = 0; i < 8; i++)
    {
        if((c = fgetc(newCfptr)) != EOF)
        {
            if(c & 0b00000001 == 0b00000001)
            {
                arr[i] = 1;
            }
            else
            {
                arr[i] = 0;
            }
        }
    }

    int returnDenaryValue = binaryToDen(arr);
    fclose(newCfptr);
    return returnDenaryValue;
}

/*
skip first 8 because we dont need to read it. separate function tells us the size

make a string of size len + 1 for '\0'
make a for loop, 'string size' times iteration
inside, we will make bitstrings for the next 8 hexa values
convert the bistrings to denary and put that in the string
the denary value we get will be an ASCII value
do this iteratively
and ggs assignment finished
*/
void readHiddenString(char * argv)
{
    int size = readStringSize(argv);
    //printf("size = %d\n", size);
    FILE * newCfptr = afterDimensions(argv);

    int c; 
    int pos = 0;
    while(pos < 8 && ((c = fgetc(newCfptr)) != EOF))
    {
        //fgetc(newCfptr);    //remove 7 more bytes after the if removed first byte
        pos++;
    }

    int index = 0;
    char * ptr = (char *)malloc(size + 1);
    ptr[size] = '\0';
    
    for(int i = 0; i < size; i++)
    {        
        int * bitString = (int*)malloc(sizeof(int) * 9);
        bitString[8] = -1;

        for(int i = 0; i < 8; i++)
        {
            if((c = fgetc(newCfptr)) != EOF)
            {
                bitString[i] = c & 0b00000001;
            }
        }
        //printBitArray(bitString);
        ptr[index] = binaryToDen(bitString);
        index++;
    }

    printf("Hidden Message: %s\n", ptr);
    fclose(newCfptr);
}

/*
print string
*/
void printString(char * str)
{
    int i = 0;
    while(str[i] != '\0')
    {
        printf("%c", str[i]);
        i++;
    }
    puts("");
}


/*
is whitespace? ye or nah
*/
int isWhitespace(char c)
{
    if(c != ' ')
    {
        return 0;
    }
    return 1;
}

/*
Counts and returns how many fgetc we need to reach a position in the file that has no comments
*/
int removeComments(char * argv)
{
    FILE * cfptr = fopen(argv, "r");
    int c;
    int count = 0;

    //first count to remove header -> P6
    while((c = fgetc(cfptr)) != EOF)
    {
        if(c == '\n')
        {
            count++;
            break;
        }
        count++;
    }

    //then count to remove comments -> #
    while((c = fgetc(cfptr)) != EOF)
    {
        if(c != '#')
        {
            count++;
            return count;
        }
        else if(c == '#')
        {
            while((c = fgetc(cfptr)) != EOF)
            {
                if(c == '\n')
                {
                    count++;
                    break;
                }
                count++;
            }
        }
        count++;
    }
    fclose(cfptr);
    return count;
}

/*
Moves the FILE pointer to the position after comments
*/
FILE * afterComments(char * argv)
{
    FILE * cfptr = fopen(argv, "r");

    int count = removeComments(argv);

    for(int i = 0; i < count - 1; i++)
    {
        fgetc(cfptr);
    }
    return cfptr;
}

/*
Similar to removeComments, where it counts how many fgetc to be at position of max color
This starts from pointer being already shifted to after comments use afterComments function
NOTE: DO NOT CALL THIS IF YOU DONT HAVE COMMENTS IN YOUR FILE OR ELSE YOU'LL WASTE YOUR TIME (i was stupid)
*/
int removeDimensions(char * argv)
{
    FILE * cfptr = afterComments(argv);
    int count = 0;
    int c;
    while((c = fgetc(cfptr)) != EOF)
    {
        if(isWhitespace(c))
        {
            count++;
            break;
        }
        count++;
    }
    while((c = fgetc(cfptr)) != EOF)
    {
        if(c == '\n')
        {
            count++;
            break;
        }
        count++;
    }
    while((c = fgetc(cfptr)) != EOF)
    {
        if(c == '\n')
        {
            count++;
            break;
        }
        count++;
    }
    fclose(cfptr);
    return count;
}

/*
Moves the FILE pointer to the position after dimensions
*/
FILE * afterDimensions(char * argv) //working confirmed?
{
    FILE * cfptr = fopen(argv, "r");

    int count = removeComments(argv) + removeDimensions(argv);
    
    for(int i = 0; i < count - 1; i++)
    {
        fgetc(cfptr);   
    }

    return cfptr;
}

/*
Computes the binary array into a denary value and returns
*/
int binaryToDen(int * arr)
{
    int i = 0;
    int exponent = 7;
    int sum = 0;
    while(arr[i] != -1)
    {
        sum += power(2, exponent) * arr[i];
        i++;
        exponent--;
    }
    return sum;
}

/*
Simple (without recursion cuz im not crazy) power function
b^x
*/
int power(int base, int exponent)
{
    if(exponent == 0)
    {
        return 1;
    }

    int product = 1;
    for(int i = 0; i < exponent; i++)
    {
        product *= base;
    }
    return product;
}

/*
printing bitstring
*/
void printBitArray(int * arr)
{
    int i = 0;
    while(arr[i] != -1)
    {
        printf("%d", arr[i]);
        i++;
    }
    puts("");
}
