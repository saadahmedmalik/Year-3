#include "player.h"

/*
 * Instance Variables: user, computer   
 * ----------------------------------
 *  
 *  We only support 2 users: a human and a computer
*/


/*
add_card: Add a new card to the player's hand.
Prams: struct player* target, struct card new_card
return: int : return 0 if no error, non-zero otherwise
*/
int add_card(struct player* target, struct card new_card){
    // check if playerHand is NULL
    if(target->playerHand == NULL){
        target->playerHand = malloc(sizeof(struct hand));
        // check if malloc failed
        if(target->playerHand == NULL){
            return -1;
        }

        // add the card to the player's hand to initialize the hand        
        target->playerHand->Payload = new_card;
        target->playerHand->next = NULL;
        return 0;
    }

    struct hand* current = target->playerHand;
   
   // find the last card in the player's hand
    while(current->next != NULL){
        current = current->next;
    }

    // add the new card to the player's hand
    struct hand* temp = malloc(sizeof(struct hand));
    if(temp == NULL){
        return -1;
    }
    temp->Payload = new_card;
    temp->next = NULL;
    current->next = temp;  

    return 0;
}

struct card returnCard(struct player* target, struct card* card2Return){

    struct hand* current = target->playerHand;
    struct hand* tempPrev = NULL;
    
    // check if playerHand is NULL
    if (current == NULL){
        fprintf(stderr, "Error: Hand is empty or uninitialized\n");
        exit(1);
    }

    // find the card to return
    while(current != NULL){
        if(current->Payload.rank == card2Return->rank && current->Payload.suit == card2Return->suit){
            if(tempPrev==NULL){
                target->playerHand = current->next;
            }
            else{
                tempPrev->next = current->next;
            }
            return current->Payload;   
        }
        tempPrev = current;
        current = current->next;
    }
    // card not found
    printf("Card not found\n");
    exit(EXIT_FAILURE);    
}


/*
removeCard: Remove a card from a player's hand
Prams: struct player* target, struct card* card2destroy
Return: int : return 0 if card is removed, -1 if card is not found
*/
int removeCard(struct player* target, struct card* card2destroy){
    
    struct hand* current = target->playerHand;
    struct hand* tempPrev = NULL;

    if (current == NULL){
        printf("Error: Hand is empty or uninitialized\n");
        return -1;
    }

    while(current != NULL){
        if(current->Payload.rank == card2destroy->rank && current->Payload.suit == card2destroy->suit){
            if(tempPrev==NULL){target->playerHand = current->next;}
            else{tempPrev->next = current->next;}
            free(current);
            //printf("Card removed\n");
            return 0;   
        }
        tempPrev = current;
        current = current->next;
    }
    printf("Card not found\n");
    return -1;
}


/*
printHand: Print the cards in a player's hand
Prams: struct player* target
Return: void
*/
void printHand(struct player* target) {
    struct hand* current = target->playerHand;
    char buffer[256] = "";  // Buffer to accumulate card representations

    while (current != NULL) {
        char* cardStr = processCard(current->Payload);
        strcat(buffer, cardStr);
        strcat(buffer, " ");  // Add a space between cards
        current = current->next;
    }
    printf("%s\n", buffer);
}



/*
check_add_book: Check if a player has all 4 cards of the same rank.
If so, remove those cards from the hand, and add the rank to the book.
Returns after finding one matching set of 4, so should be called after adding each a new card.
Prams: struct player* target
Return: int : a int that indicates the book that was added; return 0 if no book added.
*/
int check_add_book(struct player* target) {
    struct hand* curr = target->playerHand;
    char outChar[256]; // Buffer to accumulate card representations
    int rankCount[15] = {0}; // Initialize to 0
    struct hand* handArr[15] = {NULL}; // Initialize to NULL

    // When initialized, book count is a very large number -> reset it to 0
    if (target->bookCount == 0 || target->bookCount > 7) {
        target->bookCount = 0;
    }

    // Count the number of cards of each rank and store them in an array of linked lists
    while (curr != NULL) {
        int rank = curr->Payload.rank;
        rankCount[rank]++;

        struct hand* newHand = malloc(sizeof(struct hand));
        if (newHand == NULL) {
            printf("Memory allocation failed\n");
            return -1;
        }

        newHand->Payload = curr->Payload;
        newHand->next = handArr[rank];
        handArr[rank] = newHand;

        curr = curr->next;
    }

    // Check if there are any books to add
    for (int i = 0; i < 15; i++) { 
        if (rankCount[i] >= 4) {
            struct hand* current = handArr[i];
            outChar[0] = '\0';

            // Concatenate the cards of the same rank to outChar
            while (current != NULL) {
                strcat(outChar, processCard(current->Payload));
                strcat(outChar, " ");
                // Remove the card from the player's hand
                if (removeCard(target, &current->Payload) == -1) {
                    printf("Error: Card not found\n");
                    return -1;
                }
                struct hand* temp = current;
                current = current->next;
                free(temp);
            }
            
            // Add the rank to the player's book
            if(target->bookCount > 7){
                printf("failure detcted\n");
                return 0;
            }
            
            target->book[target->bookCount] = strdup(outChar);
            target->bookCount++;
            printf("Book added: %s\n", outChar);
            // Update the player's hand pointer if the hand is empty
            if (target->playerHand == NULL) {
                target->playerHand = NULL;
            }
            return 0;
        }
    }

    // Free any remaining allocated memory ->this is what was causing most stack smahes
    for (int i = 0; i < 15; i++) {
        struct hand* current = handArr[i];
        while (current != NULL) {
            struct hand* temp = current;
            current = current->next;
            free(temp);
        }
    }

    return -1;
}

/*
search: Search a player's hand for a requested rank.
Prams: struct player* target, char rank
Return: int : If the player has a card of that rank, return the count of that rank, else return 0
*/
int search(struct player* target, char rank){
    int count = 0;
    struct hand* current = target->playerHand;
    while(current != NULL){
        if(current->Payload.rank == rank){
            count++;
        }
        current = current->next;
    }
    return count;

}


/*
transferCards: Transfer cards of a given rank from the source player's hand to the destination player's hand.
Remove transferred cards from the source player's hand. Add transferred cards to the destination player's hand.
Prams: struct player* src, struct player* dest, char rank
Return: int : return 0 if no cards found/transferred, <0 if error, otherwise return value indicates number of cards transferred
*/
int transferCards(struct player* src, struct player* dest, char rank) {
    struct hand* current = src->playerHand;
    struct hand* prev = NULL;
    int count = 0;

    while (current != NULL) {
        if (current->Payload.rank == rank) {
            print_card(current->Payload);
            add_card(dest, current->Payload);

            // Remove the card from the source player's hand
            if (prev == NULL) {
                // Removing the head of the list
                src->playerHand = current->next;
            } else {
                prev->next = current->next;
            }

            struct hand* temp = current;
            current = current->next;
            free(temp); // Free the memory of the removed card
            count++;
        } else {
            prev = current;
            current = current->next;
        }
    }
    return count;
}


/*
gameOver: Check if a player has 7 books yet and the game is over
Prams: struct player* target
Return: int : return 1 if game is over, 0 if game is not over
*/
int gameOver(struct player* target){
    if(target->bookCount == 7){
        return 1;
    }
    printf("Books: %d\n", target->bookCount);
    return 0;
}


/*
userPlay: Prompt the user to enter a rank to play this turn. 
The player must have at least one card of the selected rank in their hand.
Prams: struct player* target
Return: int rank : return a valid selected rank
*/
int userPlay(struct player* target){
    char rank;
    printf("Please enter a rank: ");
    scanf(" %c", &rank);

    // Validate input
    if((rank >= '2' && rank <= '9') || rank == 'T' || rank == 'J' || rank == 'Q' || rank == 'K' || rank == 'A'){
        int char2int;
        if(rank >= '2' && rank <= '9'){
            char2int = rank - '0';
        }
        else{
            switch(rank){
                case 'T':
                    char2int = 10;
                    break;
                case 'J': 
                    char2int = 11; 
                    break;
                case 'Q':
                    char2int = 12;
                    break;
                case 'K':
                    char2int = 13;
                    break;
                case 'A':
                    char2int = 14;
                    break;
                default:
                    printf("Invalid input\n");
                    return -1;           
            }
        }
        if(search(target, char2int) == 0){
            printf("You do not have that card\n");
            return -1;
        }
        return char2int;
    }
    else{
        printf("Invalid input \n");
        return -1;
    }   
}

/*
computerPlay: Select a rank randomly to play this turn. The player must have at least
Prams: struct player* target
Return: int rank : return a valid selected rank
*/
int computerPlay(struct player* target){
    char rank; 
    do{
        rank = (rand() % 13) + 2;
    } while(search(target, rank) == 0);   
    return rank;
}

