#include <stdio.h>

void printBinary()
{
}

void printPositiveBinary()
{
    unsigned int num;
    printf("Please enter a positive integer:");
    scanf("%u", &num);

    if (num <= 0)
    {
        printf("Error: the numer is not positive\n");
        return;
    }

    unsigned intSizeInBits = sizeof(num) * 8;
    unsigned int mask = 1 << (intSizeInBits - 1);

    printf("Binary: ");

    for (int i = 0; i < intSizeInBits; i++)
    {
        char bit = (num & mask) ? 1 : 0;
        printf("%d", bit);
        mask = mask >> 1;
    }
}

void printNegativeBinary()
{
    int num;
    printf("Please enter a negative integer:");
    scanf("%d", &num);

    if (num >= 0)
    {
        printf("Error: the numer is not negative\n");
        return;
    }

    unsigned intSizeInBits = sizeof(num) * 8;
    unsigned int mask = 1 << (intSizeInBits - 1);

    printf("Binary: ");

    for (int i = 0; i < intSizeInBits; i++)
    {
        char bit = (num & mask) ? 1 : 0;
        printf("%d", bit);
        mask = mask >> 1;
    }
}

void printOnesNumber()
{
    unsigned int num;
    printf("Please enter a positive integer:");
    scanf("%u", &num);

    if (num <= 0)
    {
        printf("Error: the numer is not positive\n");
        return;
    }

    unsigned char onesNumber = 0;
    while (num)
    {
        num = num & (num - 1);
        onesNumber++;
    }

    printf("Number of ones is %d", onesNumber);
}

void replaceThirdByte()
{
    unsigned int num;
    printf("Please enter a positive integer:");
    scanf("%u", &num);

    if (num <= 0)
    {
        printf("Error: the numer is not positive\n");
        return;
    }

    unsigned int replacementNum;
    printf("Enter a number to replace the third byte with: ");
    scanf("%u", &replacementNum);
    if (replacementNum < 0 && replacementNum > 255)
    {
        printf("Error: value must be between 0 and 255\n");
        return;
    }

    num = num & 0xFF00FFFF;
    replacementNum <<= (8 * 2);

    num = num | replacementNum;
    printf("The number with replaced 3rd byte: %d", num);
}

int main()
{
    printPositiveBinary();
    printNegativeBinary();
    printOnesNumber();
    replaceThirdByte();

    getchar();
    return 0;
}