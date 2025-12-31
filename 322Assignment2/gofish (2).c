#include "deck.h"
#include "deck.c"
#include "player.h"
#include "player.c"
#include "card.c"
#include "card.h"


// Function Prototypes
int computerTurn(struct player* target, struct player* opponet);
int userTurn(struct player* target, struct player* opponet);
void gameLoop(struct player* player1, struct player* player2);

 

int main(int args, char* argv[]) 
{
  srand(time(0));
    

  struct player user;
  struct player computer;

  
  computer.playerHand = NULL;
  user.playerHand = NULL;
  computer.bookCount = 0;
  user.bookCount = 0;
   
  if(shuffle() != 0){
    return -1;
  }
  
  if(deck_size() != 52){
    return -1;
  }

  if((deal_player_cards(&user) != 0)){
    return -1;
  }
  deal_player_cards(&computer);  

  gameLoop(&user, &computer); 

  return 0;
}


int computerTurn(struct player* target, struct player* opponet){
  
  printf("Computers hand before turn \n");   
  if(target->playerHand == NULL){
  
    printf("Computer hand is empty\n");
    if(isEmpty()==1){
      printf("Deck is also empty: ending game \n");
      deckState(target,opponet);
      return 1;
    }
    deal_player_cards(target); 
    
  }
  
  printHand(target);
  
  int rank = computerPlay(target);
  int count = search(opponet, rank);
  // need 2 process character
  printf("Computer asked for %s\n", processRank(rank));

  if(count == 0){
    
    printf("Go fish! \n");
    struct card newCard = drawCard(target, opponet);
    add_card(target, newCard);
    printf("Computer draws %s\n", processCard(newCard));
    
    
    if(check_add_book(target)==0){
     // Check if the player's hand is empty after adding a book
     if(target->playerHand == NULL){
       printf("Computer's hand is now empty.\n");}
       if(isEmpty() == 1){
         printf("Deck is also empty: ending game \n");
         deckState(target, opponet);
         return 1;
       }
      if(target->playerHand == NULL){
        deal_player_cards(target);
      }
      return 0;
    }
    return 1;  
  
  }
  transferCards(opponet, target, rank);
  check_add_book(target);
  
  
  printf("Computer hand after turn \n");
  printHand(target);
  return 0;
  
}


int userTurn(struct player* target, struct player* opponet){

  
  printf("User hand before turn \n");

  if(target->playerHand == NULL){
  
    printf("PLayer hand is empty\n");
    if(isEmpty()==1){
      printf("Deck is also empty: ending game \n");
      deckState(target,opponet);
      return 1;
    }
    deal_player_cards(target); 
    
  }

  printHand(target);
  
  int rank = userPlay(target);
  int count = search(opponet, rank);
  
  printf("User asked for %s\n", processRank(rank));

  if(count == 0){
    printf("Go fish! \n");
    struct card newCard = drawCard(target, opponet);
    add_card(target, newCard);
    printf("User draws %s\n", processCard(newCard));
    
    if(check_add_book(target)==0){
      if(target->playerHand == NULL){
        deal_player_cards(target);
      }   
      return 0;
    }
    return 1;  
  
  
  }
  else{
    transferCards(opponet, target, rank);
    check_add_book(target);
  }
  printf("User hand after turn \n");
  printHand(target);
  return 0;
  
}


void gameLoop(struct player* player1, struct player* player2){
  //int gameRounds = 0;
  while(!gameOver(player1) && !gameOver(player2)){
    if(deckState(player1, player2) == 1){
      break;
    }
    
    // Player 1's turn (Computer)
    do {
      printf("P1 \n");
    } while(computerTurn(player1, player2) == 0); 
    printf("END P1 \n");
    
    // Player 2's turn (User)
    do {
      printf("P2 \n");
    } while(userTurn(player2, player1) == 0);
    printf("END P2 \n");
  }
}

// need to change game logic, if you search and return a result same user should get antoher turn.
// could achive this through having return vals and a switch
// switch based on the return value, base case return 0, next players turn

// EOF
