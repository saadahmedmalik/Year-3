#ifndef DECK_H
#define DECK_H

#include "player.h"

/*
 * Structure: deck
 * ---------------
 *  An array of 52 cards.
 */
struct deck {
  struct card list[104];
  int size;
};


/*
 * Function: shuffle
 * --------------------
 *  Initializes deck_instance and shuffles it.
 *  Resets the deck if a prior game has been played.
 * 
 *  returns: 0 if no error, and non-zero on error
 */ 
int shuffle();

/*
 * Function: deal_player_cards
 * ---------------------------
 *  Deal 7 random cards to the player specified in the function.
 *  Remove the dealt cards from the deck. 
 *
 *  target: pointer to the player to be dealt cards
 *
 *  returns: 0 if no error, and non-zero on error
 */
int deal_player_cards(struct player* target);


/*
 * Function: size
 * --------------
 *  Return the number of cards left in the current deck.
 *
 *  returns: number of cards left in the deck.
 */
int deck_size();

int isEmpty();
int deckState(struct player* user, struct player* computer);
struct card drawCard(struct player* user, struct player* computer);


#endif
//EOF