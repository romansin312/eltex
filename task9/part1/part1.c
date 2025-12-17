#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    FILE *file;
    char input_str[] = "String from file";
    
    file = fopen("output.txt", "w");
    if (file == NULL) {
        printf("error on file open\n");
        return 1;
    }

    fwrite(&input_str, sizeof(char), strlen(input_str), file);
    fclose(file);
    
    file = fopen("output.txt", "r");
    if (file == NULL) {
        printf("error on file open");
        return 1;
    }
    
    fseek(file, -1, SEEK_END);
    int file_size = ftell(file) + 1;
    char ch;
    for (int i = 0 ; i < file_size; i++) {
        if (fread(&ch, sizeof(char), 1, file) == 0) {
            printf("error on file read");
            break;
        }
        printf("%c", ch);
        fseek(file, -(i + 2), SEEK_END);
    }
    
    fclose(file);
    printf("\n");
    
    return 0;
}