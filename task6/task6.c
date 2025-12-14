#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define MAX_STR_LENGTH 10

struct contact {
    char name[MAX_STR_LENGTH];
    char second_name[MAX_STR_LENGTH];
    char tel[MAX_STR_LENGTH];

    struct contact *next;
    struct contact *prev;
};

struct contact *head = NULL;
int contactsNumber = 0;

char is_contact_empty(struct contact *contact) {
    return contact->name[0] == '\0' && contact->second_name[0] == '\0' && contact->tel[0] == '\0';
}

void add_new_contact() {
    char name[MAX_STR_LENGTH];
    printf("Enter a name: ");
    scanf("%s", name);
    
    char second_name[MAX_STR_LENGTH];
    printf("Enter a second name: ");
    scanf("%s", second_name);
    
    char tel[MAX_STR_LENGTH];
    printf("Enter a phone number: ");
    scanf("%s", tel);

    struct contact *newContact = malloc(sizeof(struct contact));
    if (newContact == NULL) {
        printf("An error occurred on contact creation.\n");
        return;
    }

    strcpy(newContact->name, name);
    strcpy(newContact->second_name, second_name);
    strcpy(newContact->tel, tel);
    newContact->next = NULL;
    newContact->prev = NULL;

    if (head == NULL) {
        head = newContact;
    } else {
        struct contact *current = head;
        while(current->next != NULL) {
            current = current->next;
        }

        current->next = newContact;
        newContact->prev = current;
    }

    contactsNumber++;

    printf("The contact has been saved successfully.\n");
}

void delete_contact() {
    if (contactsNumber < 1) {
        printf("There are no saved contacts.\n");
        return;
    }

    printf("Enter a contact number (1-%d): ", contactsNumber);
    int contactIndex;
    if(scanf("%d", &contactIndex) != 1) {
        printf("Incorrect input: not a number.\n");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');

    if (contactIndex < 0 || contactIndex > contactsNumber) {
        printf("Incorrect contact number.\n");
        return;
    }

    struct contact *current = head;
    int index = 1;
    while(index != contactIndex) {
        current = current->next;
        index++;
    }

    if (current->prev != NULL) {
        current->prev->next = current->next;
    } else {
        head = current->next;
    }

    if (current-> next != NULL) {
        current->next->prev = current->prev;
    }

    free(current);
    contactsNumber--;

    printf("The contact has been deleted successfully.\n");
}

void print_contact(int contactIndex, struct contact *contact) {
    printf("Contact #%d:\nName: %s\nSecond name: %s\nPhone: %s\n", contactIndex, contact->name, contact->second_name, contact->tel);
}

void search_contact() {
    char searchStr[MAX_STR_LENGTH];
    printf("Enter a text to search: ");
    scanf("%s", searchStr);

    char found = 0;
    struct contact *current = head;
    int index = 1;
    while (current != NULL) {
        if (strcmp(current->name, searchStr) == 0) {
            found = 1;
            print_contact(index, current);
            printf("\n");
        }
        current = current->next;
        index++;
    }

    if (!found) {
        printf("No contacts found.\n");
    }
}

void print_contacts() {
    if (head == NULL) {
        printf("There are no saved contacts.\n");
    }

    struct contact *current = head;
    int index = 1;
    while (current != NULL) {
        print_contact(index, current);
        printf("\n");
        current = current->next;
        index++;
    }
}

void print_menu() {
    printf("Menu\n");
    printf("1 - Add a new contact\n");
    printf("2 - Delete a contact\n");
    printf("3 - Search a contact by name\n");
    printf("4 - Print all contacts\n");
    printf("5 - Exit\n");
}

int free_and_exit() {
    struct contact *current = head;
    struct contact *next;
    
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }

    return 0;
}

int main() {
    char option;
    while(1) {
        print_menu();
        
        printf("Please enter an option number: ");
        scanf(" %c", &option);
        switch (option) {
            case '1':
                add_new_contact();
                break;
            case '2':
                delete_contact();
                break;
            case '3':
                search_contact();
                break;
            case '4':
                print_contacts();
                break;
            case '5':
                return free_and_exit();

            default:
                printf("Incorrect option number.\n");
                break;
        }

        printf("\n");
    }

    return 0;
}