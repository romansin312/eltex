#include <stdio.h>

int main() {
	int num;
	printf("Please enter an integer: ");
	scanf("%d", &num);

    int replacementNum;
    printf("Enter a number to replace the third byte with: ");
    scanf("%u", &replacementNum);
    if (replacementNum < 0 || replacementNum > 255)
    {
        printf("Error: value must be between 0 and 255\n");
        return -1;
    }
	
	char *ptr = (char *)&num;
	ptr[2] = (char)replacementNum;
	printf("The number with replaced 3rd byte: %d\n", *(int *)ptr);
	
	return 0;
}
