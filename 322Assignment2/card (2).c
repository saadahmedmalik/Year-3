#include "card.h"

/*
Prints the rank and suit of a card based off of enum values
Prams: struct card c
Return: void
*/
void print_card(struct card c){
    const char *rank_str[] = {"", "", "2", "3", "4", "5", "6", "7", "8", "9", "T", "J", "Q", "K", "A"};
    const char *suit_str[] = {"C", "D", "H", "S"};

    if (c.rank < 2 || c.rank > 14 || c.suit < 0 || c.suit > 3) {
        printf("Invalid card\n");
        exit(EXIT_FAILURE);
    }
    
    printf("Rank %s, Suit %s\n", rank_str[c.rank], suit_str[c.suit]); 
}


/*
Returns the rank and suit of a card based off of enum values
Prams: struct card c
Return: char*
*/
char* processCard(struct card c){
    // 1 character for rank, 1 for suit, 1 for null terminator. Sizeof is 3 to prevent buffer overflow
    static char returnChar[3];

    // Array of strings for ranks and suits
    const char *rank_str[] = {"", "", "2", "3", "4", "5", "6", "7", "8", "9", "T", "J", "Q", "K", "A"};
    const char *suit_str[] = {"C", "D", "H", "S"};

    // Check if card is valid and exit if not
    if (c.rank < 2 || c.rank > 14 || c.suit < 0 || c.suit > 3) {
        printf("Invalid card\n");
        exit(EXIT_FAILURE); 
    }
    
    // Print rank and suit to returnChar
    sprintf(returnChar, "%s%s", rank_str[c.rank], suit_str[c.suit]);
    return returnChar;    

}

/*
Returns the rank of a card based off of enum values
Prams: int x
Return: char*
*/
char* processRank(int x){
    // 1 character for rank, 1 for null terminator. Sizeof is 3 to prevent buffer overflow
    static char returnChar[3];
    // Array of strings for ranks
    const char *rankSTR[] = {"", "", "2", "3", "4", "5", "6", "7", "8", "9", "T", "J", "Q", "K", "A"};

    // Check if card is valid and exit if not
    if (x < 2 || x > 14){
        printf("invalid card");
        exit(EXIT_FAILURE);
    }

    // Print rank to returnChar
    sprintf(returnChar, "%s", rankSTR[x]);
    return returnChar;
}


// EOF