#include <stdio.h>

int main() {
	char inStr[100];
	printf("Please enter a string: ");
	fgets(inStr, sizeof(inStr), stdin);

	char substr[100];
	printf("Please enter a substring to search: ");
	fgets(substr, sizeof(substr), stdin);
	
	for (char *p = inStr; *p; p++) if (*p == '\n') *p = '\0';
    for (char *p = substr; *p; p++) if (*p == '\n') *p = '\0';

 	char *result = NULL;
    for (char *s = inStr; *s; s++) {
        char *str_ptr = s;
        char *sub_ptr = substr;
        
        while (*sub_ptr != '\0' && *str_ptr != '\0' && *str_ptr == *sub_ptr) {
            str_ptr++;
            sub_ptr++;
        }
        
        if (*sub_ptr == '\0') {
            result = s;
            break;
        }
    }

	if (result != NULL) {
		printf("Substring is found, ptr: %p, start: %d\n", (void*)result, (int)(result - inStr));
	} else {
		printf("Substring is not found\n");
	}
}
