#include <stdio.h>
#include <string.h>

#define MAX_STR_LENGTH 10
#define MAX_CONTACTS 100

struct contact {
    char name[MAX_STR_LENGTH];
    char second_name[MAX_STR_LENGTH];
    char tel[MAX_STR_LENGTH];
};

struct contact contacts[MAX_CONTACTS] = {0};

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

    for (int i = 0; i < MAX_CONTACTS; i++) {
        struct contact contact = contacts[i];
        if (is_contact_empty(&contacts[i])) {
            strcpy(contacts[i].name, name);
            strcpy(contacts[i].second_name, second_name);
            strcpy(contacts[i].tel, tel);

            printf("The contact has been saved successfully.\n");

            break;
        }
    }
}

void delete_contact() {
    printf("Enter a contact number (0-%d): ", MAX_CONTACTS - 1);
    int contactIndex;
    if(scanf("%d", &contactIndex) != 1) {
        printf("Incorrect input: not a number.\n");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');

    if (contactIndex < 0 || contactIndex > MAX_CONTACTS - 1) {
        printf("Incorrect contact number.\n");
        return;
    }

    struct contact contact = contacts[contactIndex];
    if (is_contact_empty(&contact)) {
        printf("Contact with number %d does not exist.\n", contactIndex);
        return;
    }

    contacts[contactIndex].name[0] = contacts[contactIndex].second_name[0] = contacts[contactIndex].tel[0] = '\0';
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
    for (int i = 0; i < MAX_CONTACTS; i++) {
        struct contact contact = contacts[i];
        if (strcmp(contact.name, searchStr) == 0) {
            found = 1;
            print_contact(i, &contact);
            printf("\n");
        }
    }

    if (!found) {
        printf("No contacts found.\n");
    }
}

void print_contacts() {
    char hasContacts = 0;
    for (int i = 0; i < MAX_CONTACTS; i++) {
        struct contact contact = contacts[i];
        if (!is_contact_empty(&contact)) {
            hasContacts = 1;
            print_contact(i, &contact);
            printf("\n");
        }
    }

    if (!hasContacts) {
        printf("There are no saved contacts.\n");
    }
}

void print_menu() {
    printf("Menu\n");
    printf("1 - Add a new contact\n");
    printf("2 - Delete a contact\n");
    printf("3 - Seach a contact by name\n");
    printf("4 - Print all contacts\n");
    printf("5 - Exit\n");
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
                return 0;
                break;
            default:
                printf("Incorrect option number.\n");
                break;
        }

        printf("\n");
    }

    return 0;
}