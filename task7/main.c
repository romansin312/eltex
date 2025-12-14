#include <stdio.h>
#include "calc.h"

void print_menu() {
    printf("Menu\n");
    printf("1 - Add\n");
    printf("2 - Substract\n");
    printf("3 - Multiplication\n");
    printf("4 - Devision\n");
    printf("5 - Exit\n");
}

int main() {
    char option;
    int operand1;
    int operand2;    
    int result;
    char printResult;

    while(1) {
        printResult = 0;
        print_menu();
        
        printf("Please enter an option number: ");
        scanf(" %c", &option);

        if (option >= '1' && option <= '4') {
            printf("Enter the 1st operand: ");
            scanf("%d", &operand1);

            printf("Enter the 2nd operand: ");
            scanf("%d", &operand2);

            printResult = 1;
        }

        switch (option) {
            case '1':
                result = add(operand1, operand2);
                break;
            case '2':
                result = substract(operand1, operand2);
                break;
            case '3':
                result = multiple(operand1, operand2);
                break;
            case '4':
                result = divide(operand1, operand2);
                break;
            case '5':
                return 0;
            default:
                printf("Incorrect option number.\n");
                break;
        }
        
        if (printResult) {
            printf("Result: %d", result);
        }


        printf("\n");
    }

    return 0;
}