#include <stdio.h>
#include <stdlib.h>

int hasP6(char * argv);
int removeComments(char * argv);
FILE * afterComments(char * argv);
int removeDimensions(char * argv);
FILE * afterDimensions(char * argv);
void printFileOut(FILE * cfptr);
int power(int base, int exponent);
int hexStrToInt(char * str, int len);
int denStrToInt(char * str, int len);
int isPixel(char * r, int lenR, char * g, int lenG, char * b, int lenB);
int isWhitespace(char c);
void getDimensions(char * argv, int * width, int * height, int * maxColor);
int myStrlen(char * s);
void myputs(char * s);
int areValidPixels(char * argv, int * numberOfPixels);
void writeHeaders(FILE * newCfptr, char * width, char * height, char * maxColor);
char * intToStr(int num);
int valAtFirst(int num);
int valToASCII(int num);
void appendPixels(FILE * newCfptr, char * argv, char * string);
int * denToBinary(int num);
char * denToHexStr(int num);
char getHexaRepresentation(int num);
int myBinaryArrayLen(int * arr);
int binaryToDen(int * arr);
FILE * skipHiddenMessageLength(FILE * newCfptr);
void appendHiddenMessage(FILE * cfptr, FILE * newCfptr, char * argv, char * string);
void printBitArray(int * arr);
FILE * skipWhitespaces(FILE * cfptr);
char * hexToHexStr(int hex);

//gcc WriteMsg.c -o WriteMsg; ./WriteMsg "Message" TU.ppm result.ppm;

//one line everything
//gcc WriteMsg.c -o WriteMsg; ./WriteMsg "Hello how are you doing? I am sending an encrypted message in a picutre. I hope you can see this!" TU.ppm result.ppm; gcc ReadMsg.c -o ReadMsg; ./ReadMsg result.ppm

int main(int argc, char * argv[])
{
    FILE * cfptr = fopen(argv[2], "r");

    if(cfptr != '\0')
    {
        //If header does not start with P6, exit
        if(!hasP6(argv[2]))
        {
            return -1;
        }

        //getting dimensions from header
        int width, height, maxColor;
        char * strWidth, * strHeight, * strMaxColor;
        getDimensions(argv[2], &width, &height, &maxColor);

        strWidth = intToStr(width);
        strHeight = intToStr(height);
        strMaxColor = intToStr(maxColor);

        //printf("width = %d, height = %d, maxColor = %d\n", width, height, maxColor);

        //if width/height are 5 digits or larger, exit
        if(width > 9999 || height > 9999)
        {
            puts("Image Too Big");
            return -1;
        }

        //if color is above 255, exit
        if(maxColor > 255)
        {
            puts("Color Value Too Big");
            return -1;
        }

        int numberOfPixels;

        //are all pixels valid? as in, are all bytes within 255
        int valid = areValidPixels(argv[2], &numberOfPixels);
        if(valid)
        {
            printf("There are %d pixels with rgb values\n", numberOfPixels);
        }
        else
        {
            puts("Not valid");
            return -1;
        }

        //does the number of pixels in the file match the product of width and height? if not, exit
        if(numberOfPixels != (width * height))
        {
            puts("Dimensions do not match amount of pixels");
            return -1;
        }
        else
        {
            puts("There is a perfect amount of pixels");
        }

        //check if there is enough space for a hidden message to be encoded
        //the first 8 bytes can only store up to 255 characters
        if(myStrlen(argv[1]) > 255)
        {
            puts("Cannot hold that many words");
            return -1;
        }
        //check if there is enough space for a hidden message to be encoded
        if(numberOfPixels < (myStrlen(argv[1]) * 3) + 3)    //1 pixel required to store length of message
        {
            puts("Not enough pixels to hide message");
            return -1;
        }

        FILE * newCfptr = fopen(argv[3], "w");
        if(newCfptr != '\0')
        {
            writeHeaders(newCfptr, strWidth, strHeight, strMaxColor);
            appendPixels(newCfptr, argv[2], argv[1]);
            fclose(newCfptr);
        }
        else
        {
            puts("Wrong Input Format For Writing");
            fclose(newCfptr);
        }

        fclose(cfptr);
    }
    else
    {
        puts("Wrong Input Format For Reading");
        fclose(cfptr);
        return -1;
    }
}

/*
Brute Force Check to see the first 2 chars and returns 1/0 depending on whether first 2 chars are P6 or not
*/
int hasP6(char * argv)
{
    FILE * cfptr = fopen(argv, "r");

    char headerCheck[3];
    headerCheck[2] = '\0';

    int c;
    for(int i = 0; i < 2; i++)
    {
        if((c = fgetc(cfptr)) != EOF)
        {
            headerCheck[i] = c;
        }
        else
        {
            //puts("Header is not p6");
            return 0;
        }
    }
    fclose(cfptr);

    if(!(headerCheck[0] == 'P' && headerCheck[1] == '6'))
    {
        //puts("Header is not p6");
        return 0;
    }
    else
    {
        //puts("Header is P6");
        return 1;
    }
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
    //printf("count = %d\n", count);

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
Just prints the whole file from start to EOF
*/
void printFileOut(FILE * cfptr)
{
    int c;
    int d;
    int e;
    char * temp1;
    char * temp2;
    char * temp3;
    while((c = fgetc(cfptr)) != EOF)
    {
        printf("%x ", c);
        //printf("%x\t%x\t%x\n", c, (d=fgetc(cfptr)), (e=fgetc(cfptr)));
        // temp1 = hexToHexStr(c);
        // temp2 = hexToHexStr(d);
        // temp3 = hexToHexStr(e);
        // printf("Hex as String: %s, %s, %s\n", temp1, temp2, temp3);
        // printf("Hex as Denary: %d, %d, %d\n", hexStrToInt(temp1, myStrlen(temp1)), hexStrToInt(temp2, myStrlen(temp2)), hexStrToInt(temp3, myStrlen(temp3)));
    }
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
Takes a hexadecimal string and the string's size and converts it to denary and returns
*/
int hexStrToInt(char * str, int len)
{
    int hex = 0;
    int exponent = 0;
    for(int i = len - 1; i >= 0; i--)
    {
        switch(str[i])
        {
            case '0':
                hex += 0;
                break;
            case '1':
                hex += power(16, exponent) * 1;
                break;
            case '2':
                hex += power(16, exponent) * 2;
                break;
            case '3':
                hex += power(16, exponent) * 3;
                break;
            case '4':
                hex += power(16, exponent) * 4;
                break;
            case '5':
                hex += power(16, exponent) * 5;
                break;
            case '6':
                hex += power(16, exponent) * 6;
                break;
            case '7':
                hex += power(16, exponent) * 7;
                break;
            case '8':
                hex += power(16, exponent) * 8;
                break;
            case '9':
                hex += power(16, exponent) * 9;
                break;
            case 'a':
                hex += power(16, exponent) * 10;
                break;
            case 'b':
                hex += power(16, exponent) * 11;
                break;
            case 'c':
                hex += power(16, exponent) * 12;
                break;
            case 'd':
                hex += power(16, exponent) * 13;
                break;
            case 'e':
                hex += power(16, exponent) * 14;
                break;
            case 'f':
                hex += power(16, exponent) * 15;
                break;
            default:
                return 256; //if char is not a hex char
        }
        exponent++;
    }
    return hex;
}

/*
Takes a denary string and the string's size and converts it to denary and returns
*/
int denStrToInt(char * str, int len)
{
    int denary = 0;
    int exponent = 0;

    for(int i = len - 1; i >= 0; i--)
    {
        switch(str[i])
        {
            case '0':
                denary += 0;
                break;
            case '1':
                denary += power(10, exponent) * 1;
                break;
            case '2':
                denary += power(10, exponent) * 2;
                break;
            case '3':
                denary += power(10, exponent) * 3;
                break;
            case '4':
                denary += power(10, exponent) * 4;
                break;
            case '5':
                denary += power(10, exponent) * 5;
                break;
            case '6':
                denary += power(10, exponent) * 6;
                break;
            case '7':
                denary += power(10, exponent) * 7;
                break;
            case '8':
                denary += power(10, exponent) * 8;
                break;
            case '9':
                denary += power(10, exponent) * 9;
                break;
            default:
                return -1;
        }
        exponent++;
    }
    return denary;
}

/*
checks if the three values of RGB are within 255 or else pixel is too large to represent
*/
int isPixel(char * r, int lenR, char * g, int lenG, char * b, int lenB)
{
    int red = hexStrToInt(r, lenR);
    int green = hexStrToInt(g, lenG);
    int blue = hexStrToInt(b, lenB);
    //printf("r=%d g=%d b=%d\n", red, green, blue);
    if(red > 255 || green > 255 || blue > 255)
    {
        return 0;
    }
    return 1;
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
All comments made by past, present, and most likely future me cuz theres gonna be bugs (whats the testcase for sanity?)
UPDATE It is lessgoo
UPDATE Maybe its solved NEED TO CHECK
Bug is not solved
For some dumb reason, len of string w is increasing after the consecutive while loops are run
Length of string w is 4, but changes to 5 afterwards
*/
void getDimensions(char * argv, int * width, int * height, int * maxColor)
{
    FILE * cfptr = afterComments(argv);

    //assume largest image size is around 8k 7680px X 4320px, or max 9999x9999
    char w[4];
    char h[4];
    char maxC[3];

    int wIndex = 0;
    int hIndex = 0;
    int maxCIndex = 0;
    int c;
    while((c = fgetc(cfptr)) != EOF)
    {
        if(wIndex > 4)
        {
            *width = 10000;
            break;
        }
        if(isWhitespace(c))
        {
            w[wIndex] = '\0';
            break;
        }
        w[wIndex] = c;
        wIndex++;
    }

    if(wIndex <= 4)
    {
        *width = denStrToInt(w, myStrlen(w));
    }

    while((c = fgetc(cfptr)) != EOF)
    {
        if(hIndex > 4)
        {
            *height = 10000;
            break;
        }
        if(c == '\n')
        {
            h[hIndex] = '\0';
            break;
        }
        h[hIndex] = c;
        hIndex++;
    }

    if(hIndex <= 4)
    {
        *height = denStrToInt(h, myStrlen(h));
    }

    while((c = fgetc(cfptr)) != EOF)
    {
        if(maxCIndex > 3)
        {
            *maxColor = 256;
            break;
        }
        if(c == '\n')
        {
            maxC[maxCIndex] = '\0';
            break;
        }
        maxC[maxCIndex] = c;
        maxCIndex++;
    }
    
    if(maxCIndex <= 3)
    {
        *maxColor = denStrToInt(maxC, myStrlen(maxC));
    }

    fclose(cfptr);
}

/*
Length of string
*/
int myStrlen(char * s)
{
    int i = 0;
    while(s[i] != '\0')
    {
        i++;
    }
    return i;
}

/*
My implementation of a binary array has -1 as terminating term instead '\0' cuz for some reason it wouldnt work
*/
int myBinaryArrayLen(int * arr)
{
    int i = 0;
    while(arr[i] != -1)
    {
        i++;
    }
    return i;
}

/*
My puts()
*/
void myputs(char * s)
{
    int i = 0;
    while(s[i] != '\0')
    {
        printf("%c", s[i]);
        i++;
    }
    printf("\n");
}

/*
Converts an integer to a string
I think I big brained this (not that impressive)
*/
char * intToStr(int num)
{
    int temp = num;
    int places = 0;
    while(temp != 0)
    {
        temp = temp / 10;
        places++;
    }

    temp = num;
    char * ptr = (char*)malloc(places + 1);
    int exponentSubtractor = 1;
    for(int i = 0; i < places; i++)
    {
        //printf("num = %d, temp = %d\n", temp, valAtFirst(temp));
        ptr[i] = valToASCII(valAtFirst(temp));
        temp = temp / power(10, 0);
        temp = temp - (valAtFirst(temp) * power(10, places - exponentSubtractor));
        exponentSubtractor++;
    }
    ptr[places] = '\0'; //indices 0 - places-1 so '\0' is places - 1 + 1
    return ptr;
}

/*
Returns left most position of number ("head" of a number lol like a linked list of numbers)
*/
int valAtFirst(int num)
{
    while(!(num < 10))
    {
        num /= 10;
    }
    return num;
}

/*
This took me longer that it should've I was about to use switch case
*/
int valToASCII(int num)
{
    return num + 48;
}

/*
Goes through the whole file and confirms whether all pixels can be used - as in are their values within 255 and that 1px = 3 bytes
*/
int areValidPixels(char * argv, int * numberOfPixels)
{
    FILE * cfptr = afterDimensions(argv);

    int areValid = 1;
    int count = 0;

    char red[3];
    red[2] = '\0';
    char green[3];
    green[2] = '\0';
    char blue[3];
    blue[2] = '\0';

    // char red[2];
    // char green[2];
    // char blue[2];
    char * temp;

    int c;
    while((c = fgetc(cfptr)) != EOF)
    {
        temp = hexToHexStr(c);
        red[0] = temp[0];
        red[1] = temp[1];

        if((c = fgetc(cfptr)) == EOF)
        {
            puts("End of File");
            *numberOfPixels = count;
            return areValid;
        }
        temp = hexToHexStr(c);
        green[0] = temp[0];
        green[1] = temp[1];
        
        if((c = fgetc(cfptr)) == EOF)
        {
            puts("End of File");
            *numberOfPixels = count;
            return areValid;
        }
        temp = hexToHexStr(c);
        blue[0] = temp[0];
        blue[1] = temp[1];

        if(!isPixel(red, myStrlen(red), green, myStrlen(green), blue, myStrlen(blue)))
        {
            puts("Color Too Big in areValidPixels");
            return 0;
        }

        count++;
    }
    fclose(cfptr);

    *numberOfPixels = count;
    return areValid;
}

/*
Kinda brute force but puts P6, width, height, and max color in file we write to
*/
void writeHeaders(FILE * newCfptr, char * width, char * height, char * maxColor)
{
    fputc('P', newCfptr);
    fputc('6', newCfptr);
    fputc('\n', newCfptr);
    int i = 0;
    int c;
    while(width[i] != '\0')
    {
        c = width[i];
        //printf("%c, %d\n", c, i);
        fputc(c, newCfptr);
        i++;
    }
    fputc(' ', newCfptr);
    i = 0;
    while(height[i] != '\0')
    {
        c = height[i];
        fputc(c, newCfptr);
        i++;
    }
    fputc('\n', newCfptr);
    i = 0;
    while(maxColor[i] != '\0')
    {
        c = maxColor[i];
        fputc(c, newCfptr);
        i++;
    }
    fputc('\n', newCfptr);
}

/*
UPDATE - I figured out how to do it better, so Im using the better way in ReadMsg.c and am kinda lazy to change it here

UPDATE - I updated
only thing changed was that i added a void function to change all the values for the string

GGs I dont want to explain this
Lemme lock in

AS OF NOW (most likely going to be changed, will have update above this)
open the file we wrote header to
we're gonna append the rest of the file to this
not going to use fopen with "a" cuz idk id rather spam fgetc fputc

first i got the string's length as a bit string (For simplicity im saying bit string but its an integer array my bad blame discrete)
since my bitstring function does not have preceeding/leading zeros, i did (8 - sizeof bit string) which will tell me how many leading
zeros there are in the bit string

before going further ill explain the for loops
what it does is
gets 2 hexadecimal chars and puts it into a string of size 2
then using helper functions, converts the string to denary value
you can play around with this denary value, as in using bitwise operators to isolate the bits of an int, or wtv you want
then using another helper function, the denary value becomes a hex string
then, fputc these two hex chars into the file
and fputc 1 fgetc because that 1 fgetc will be a whitespace

thats the gimmick of the while loops

so now there are 3 while loops
first while loop is for the preceeding/leading zeros
after getting the denary value, use AND bitwise operator & to make the last bit a 0. so use 0b11111110
do the next things in for loop and fputc into file

then next one is for the actual bit string we made
the loop condition will be as long as element in bit string is NOT -1 (this is my termination terminator idk ill be back)
this time, after getting denary value, do OR or AND bitwise operator |0b11111111 OR &0b11111110 depending on the 
element of the bit string. if element is 1, do bitwise OR | 0b00000001, else bitwise AND & 0b11111110

last while loop is AS OF NOW, the rest of the hexadecimal values
*/
void appendPixels(FILE * newCfptr, char * argv, char * string)
{
    FILE * cfptr = afterDimensions(argv);
    int c;
    char pixelColor[3];
    pixelColor[2] = '\0';
    char * temp;

    int * stringSizeInBinary = denToBinary(myStrlen(string));

    int i = 0;
    int denary = 0;
    int sizeOfBinaryStr = 8 - myBinaryArrayLen(stringSizeInBinary);
    //printf("len=%d\n", sizeOfBinaryStr);

    while(sizeOfBinaryStr > 0)
    {
        while((c=fgetc(cfptr)) != EOF)
        {
            temp = hexToHexStr(c);
            pixelColor[0] = temp[0];
            pixelColor[1] = temp[1];
            //printf("pixelColor = %s, [0] = %c, [1] = %c\n", pixelColor, pixelColor[0], pixelColor[1]);
            denary = hexStrToInt(pixelColor, myStrlen(pixelColor));
            
            denary &= 0b11111110;
            break;
        }
        
        fputc(denary, newCfptr);
        //fputc(fgetc(cfptr), newCfptr);   //whitespace
        sizeOfBinaryStr--;
    }

    while(stringSizeInBinary[i] != -1)
    {
        while((c=fgetc(cfptr)) != EOF)
        {
            temp = hexToHexStr(c);
            pixelColor[0] = temp[0];
            pixelColor[1] = temp[1];
            denary = hexStrToInt(pixelColor, myStrlen(pixelColor));

            if(stringSizeInBinary[i] == 1)
            {
                denary |= 0b00000001;
                break;
            }
            else
            {
                denary &= 0b11111110;
                break;
            }
        }
        
        fputc(denary, newCfptr);
        //fputc(fgetc(cfptr), newCfptr);   //whitespace
        i++;
    }
    
    appendHiddenMessage(cfptr, newCfptr, argv, string);
    fclose(cfptr);
}

/*
Converts denary to a an array of bitstring representation with the final terminator as -1 instead of '\0' cuz it dont work i think
*/
int * denToBinary(int num)
{
    int howManyTimes = 0;
    int temp = num;
    int size = 0;
    // while(temp > power(2, size))
    // {
    //     size++;
    //     howManyTimes++;
    // }

    // lowkey this increase efficiency instead of using while loop above... ??
    if(temp >= 256)
    {
        return '\0';
    }
    else if(temp >= 128)
    {
        size = 8;
    }
    else if(temp >= 64)
    {
        size = 7;
    }
    else if(temp >= 32)
    {
        size = 6;
    }
    else if(temp >= 16)
    {
        size = 5;
    }
    else if(temp >= 8)
    {
        size = 4;
    }
    else if(temp >= 4)
    {
        size = 3;
    }
    else if(temp >= 2)
    {
        size = 2;
    }
    else if(temp >= 0)
    {
        size = 1;
    }
    else
    {
        return '\0';
    }

    int * arr = (int*)malloc((sizeof(int) * size) + sizeof(int));
    arr[size] = -1;

    int i = size - 1;
    temp = num;
    //printf("size = %d, i = %d\n", num, i);
    // while(temp != 0)
    // {
    //     arr[i] = temp % 2;
    //     //printf("arr[%d] = %d\n", i, arr[i]);
    //     i--;
    //     temp /= 2;
    //     howManyTimes++;
    // }
    for(int i = 0; i < size; i++)
    {
        arr[size - 1 - i] = temp % 2;
        temp /= 2;
        howManyTimes++;
    }
    //printf("looped %d times\n", howManyTimes);

    return arr;
}

/*
Decimal to hexa string representation
*/
char * denToHexStr(int num)
{
    if(num < 16)
    {
        char * ptr = (char*)malloc((sizeof(int) * 3));
        ptr[0] = '0';
        ptr[1] = getHexaRepresentation(num);
        ptr[2] = '\0';
        return ptr;
    }
    int temp = num;
    int size = 0;
    while(temp != 0)
    {
        temp /= 16;
        size++;
    }
    char * ptr = (char*)malloc((sizeof(int)*size) + 1);
    ptr[size] = '\0';
    int i = size - 1;
    temp = num;
    while(temp != 0)
    {
        char c = getHexaRepresentation(temp % 16);
        ptr[i] = c;
        //printf("arr[%d] = %d\n", i, ptr[i]);
        i--;
        temp /= 16;
    }
    return ptr;
}

/*
A switch case of hexa values from 0-F
*/
char getHexaRepresentation(int num)
{
    switch(num)
    {
        case 0:
            return '0';
        case 1:
            return '1';
        case 2:
            return '2';
        case 3:
            return '3';
        case 4:
            return '4';
        case 5:
            return '5';
        case 6:
            return '6';
        case 7:
            return '7';
        case 8:
            return '8';
        case 9:
            return '9';
        case 10:
            return 'a';
        case 11:
            return 'b';
        case 12:
            return 'c';
        case 13:
            return 'd';
        case 14:
            return 'e';
        case 15:
            return 'f';
    }
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
Skip first 8 hexa values because those tell us the length
*/
FILE * skipHiddenMessageLength(FILE * newCfptr)
{
    int c;
    if((c = fgetc(newCfptr)) != EOF)
    {
        for(int i = 0; i < 7; i++)
        {
            fgetc(newCfptr);    //remove 7 more bytes after the if removed first byte
        }
    }
    return newCfptr;
}

/*
essentially the same logic as appendPixels but now taken in account for each bit of string
reffer to appendPixels
*/
void appendHiddenMessage(FILE * cfptr, FILE * newCfptr, char * argv, char * string)
{
    int len = myStrlen(string);
    //printf("len of string = %d\n", len);

    char pixelColor[3];
    pixelColor[2] = '\0';
    char * temp;
    int c;

    for(int i = 0; i < len; i++)
    {
        int * bitString = denToBinary(string[i]);   //get bitString for each char.. char will become int

        int denary = 0;
        int leadingZeros = 8 - myBinaryArrayLen(bitString);
        //printf("leading zeros = %d\n",leadingZeros);

        //leading 0s while loop
        while(leadingZeros > 0)
        {
            while((c=fgetc(cfptr)) != EOF)
            {
                temp = hexToHexStr(c);
                pixelColor[0] = temp[0];
                pixelColor[1] = temp[1];
                //printf("c= %c c= %c\n", pixelColor[0], pixelColor[1]);
                denary = hexStrToInt(pixelColor, myStrlen(pixelColor));

                // printf("denary %dth = %d\n", i, denary);
                // printf("binary %dth = %d\n", i, stringSizeInBinary[i]);
                denary &= 0b11111110;
                break;
            }
            
            fputc(denary, newCfptr);
            //fputc(fgetc(cfptr), newCfptr);   //whitespace
            leadingZeros--;
        }

        int index = 0;
        while(bitString[index] != -1)
        {
            //puts("Have we moved to bit string?");
            while((c=fgetc(cfptr)) != EOF)
            {
                temp = hexToHexStr(c);
                pixelColor[0] = temp[0];
                pixelColor[1] = temp[1];
                //printf("c= %c c= %c\n", pixelColor[0], pixelColor[1]);
                denary = hexStrToInt(pixelColor, myStrlen(pixelColor));

                // printf("denary %dth = %d\n", i, denary);
                // printf("binary %dth = %d\n", i, stringSizeInBinary[i]);

                if(bitString[index] == 1)
                {
                    denary |= 0b00000001;
                    break;
                }
                else
                {
                    denary &= 0b11111110;
                    break;
                }
            }
            
            fputc(denary, newCfptr);
            //fputc(fgetc(cfptr), newCfptr);   //whitespace
            index++;
        }
    }

    while((c=fgetc(cfptr)) != EOF)
    {
        fputc(c, newCfptr);
    }
    //fclose(cfptr);
    return;
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
Moves FILE pointer to a place that is no longer a whitespace
We start from a character after whitespace because it is assumed that this was called at a whitespace
*/
FILE * skipWhitespaces(FILE * cfptr)
{
    // we are calling this which means WE ARE AT A WHITESPACE AND WANT TO SKIP
    int c;
    int countForDebug = 0;
    while(c = fgetc(cfptr) != ' ')
    {
        countForDebug++;
    }
    return cfptr;
}

/*
Converts a hex value to a hex string
*/
char * hexToHexStr(int hex)
{
    char * ptr = (char*)malloc(2+1);
    ptr[2] = '\0';
    int temp = hex;
    ptr[1] = getHexaRepresentation(temp % 16);
    temp /= 16;
    ptr[0] = getHexaRepresentation(temp % 16);
    temp /= 16;

    //printf("hex = %s\n", ptr);

    return ptr;
}
