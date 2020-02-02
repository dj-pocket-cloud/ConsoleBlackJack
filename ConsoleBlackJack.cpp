// ConsoleBlackJack.cpp : it's simply blackjack... in a console
//

#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <fstream>
#include <string>
using namespace std;

const int SCOREDEFAULT = 4;
const char DECKDEFAULT[52] =
{ 'A', 'A', 'A', 'A', '2', '2', '2', '2', '3', '3', '3', '3', '4', '4', '4', '4', '5', '5', '5', '5', '6', '6', '6', '6', 
'7', '7', '7', '7', '8', '8', '8', '8', '9', '9', '9', '9', '0', '0', '0', '0', 'J', 'J', 'J', 'J', 'Q', 'Q', 'Q', 'Q', 'K', 'K', 'K', 'K', };
random_device rng;

void nextScreen();
void LoadDeckFromDefault(vector<char> &deck);
void LoadDeckFromCustom(vector<char> &deck);
void PutDiscardIntoDeck(vector<char>& deck, vector<char>& discard);
int mainMenu(vector<char>& deck);
void game(vector<char>& deck);
int confirmCustomDeckLoad();
void loadReadme();
void showAvailableWagers(int& score, bool &cancel);
void drawCard(vector<char>& deck, vector<char>& hand, vector<char> &discard);
int checkValue(vector<char>& hand);
bool checkBlackjack(vector<char>& hand);
void displayHands(vector<char>& playerHand, vector<char>& dealerHand, bool showDealer);
void shuffleDeck(vector<char>& deck);
void discardHand(vector<char>& hand, vector<char>& deck);
void showAvailableMoves(vector<char>& player, vector<char>& dealer, vector<char>& deck, vector<char>& discard);

int main()
{
    //game variables that can be saved, loaded, and modified
    vector<char> deck;
    vector<char> memDeck;
    
    //misc variables and decalrations
    int mainMenuSelection;
    int confirm;

    LoadDeckFromDefault(deck);

    while (true) {
        cout << "\n\tC O N S O L E - B L A C K J A C K\n"
            << "\t\t    ver. 0.1\n\n\n\n\n\n\n\n\n\n\n\n\n";

        mainMenuSelection = mainMenu(deck);

        switch (mainMenuSelection) {

        case 1: //start game
            game(deck); //loop through this until player exits
            break;
        case 2: //readme
            nextScreen();
            loadReadme();
            nextScreen();
            break;
        case 3: //customize deck
            nextScreen();
            confirm = confirmCustomDeckLoad();
            nextScreen();
            if (confirm == 1) {
                LoadDeckFromCustom(deck);
            }
            else if (confirm == 2) {
                LoadDeckFromDefault(deck); //option to use standard deck
                cout << "deck reset to default\n\n\n";
            }
            
            break;
        case 4: //unused

            break;
        default:
            break;
        }

    }

    //init variables: 
    // 'deck' array of all possible cards in the game, represented by chars
        // A = ace, 2-1[0] = numbers, J = jack, Q = queen, K = king, other = joker
        // bonus: make deck editable before playing, able to be saved or loaded from .txt file
    // 'DECKDEFAULT' final array that's the deck that is replaced in the event the deck config gets deleted
    // 'deckDiscard' all discarded cards, shuffled back in 'deckPlay' at some point
    // 'playerHand' all cards in player's hand
    // 'dealerHand' likewise all cards in dealer's hand. cards are shown or hidden based on array index
    // 'score' integer variable denoting how much "money" the player has
        //bonus: automatically save score for future play sessions
    // 'SCOREDEFAULT' final int that's the amount the player starts with on starting a new game
    // 'wager' int array that's equal to amount being wagered
    // 'multiplier' int array determining how much should be paid out for each hand

    //general game rules/loop:
    // taken from https://www.blackjackapprenticeship.com/how-to-play-blackjack/
    // 1. player starts with 4 score on brand new game, can bet in these denominations:
        // white - 1
        // pink - 2
        // red - 4
        // blue - 8
        // green - 16
        // black - 32
    // 2. cards are dealt to all hands as appropriate, removing them from the deck. the topmost dealer card is shown
        // all card arrays are FIFO except in the case a specific card is selected
    // 3. check for blackjack (10 and ace)
        // if only player: pay out 2.5x (3 to 2)
        // if only dealer: pay out 0x
        // if both: pay out 1x (push)
        // go to next turn
    // 4. if no blackjack, get player input for all hands
        // stand (go to next step)
        // hit (take a card, repeat this step if not above 21. if above pay out 0x and remove hand from play)
        // double down (multiply wager by 2, take one additional card and go to next step)
        // surrender (forfeit hand, pay out 0.5x and remove hand from play)
        // insurance?
    // 5. if at least one hand is in play, dealer goes
        // show face-down card, start drawing cards until hand => 17
        // on soft 17 (ace and 7) dealer will draw once more
    // 6. pay out
        // if dealer busted pay out 2x
        // if player hand > dealer hand 2x
        // likewise if player < dealer 0x
        // on push deal 1x
        // start new set
    //any time deck becomes empty, shuffle discard pile into it
    //bonus: make options menu to configure certain gameplay aspects
}

/*
    Most if not all the game logic is here
*/
void game(vector<char>& deck) {

    //core gameplay variables
    //vector<char> deck
    vector<char> discard;
    vector<char> player;
    vector<char> dealer;
    int score = SCOREDEFAULT;
    int wager = 0;
    int insurance = 0;
    int playerValue = 0;
    int dealerValue = 0;
    bool playerBlackjack = false;
    bool dealerBlackjack = false;
    float multiplier = 0;
    bool shuffled = false;
    bool placedInsurance = false;
    bool bust = false;

    //other variables and declarations
    char charIn;
    bool loop1;
    bool cancel;
    shuffleDeck(deck);

    //gameplay loop starts at this point
    while (true) {
        loop1 = true;
        cancel = false;
        nextScreen();
        wager = 0;
        playerValue = 0;
        dealerValue = 0;
        playerBlackjack = false;
        dealerBlackjack = false;
        shuffled = false;

        /* get player wager */
        while (loop1) {

            if (score == 0) {
                loop1 = false; //go directly to next step if no score
                break;
            }
            
            cout << "score: " << score << "\nwager: " << wager << "\n\n";
            showAvailableWagers(score, cancel);

            //get and respond to input
            cin >> charIn;
            switch (toupper(charIn)) {
            case 'W':
                if (score >= 1) {
                    wager += 1;
                    score -= 1;
                    cancel = true;
                    nextScreen();
                }
                else {
                    nextScreen();
                    cout << "invalid input\n\n";
                }
                break;
            case 'P':
                if (score >= 2) {
                    wager += 2;
                    score -= 2;
                    cancel = true;
                    nextScreen();
                }
                else {
                    nextScreen();
                    cout << "invalid input\n\n";
                }
                break;
            case 'R':
                if (score >= 4) {
                    wager += 4;
                    score -= 4;
                    cancel = true;
                    nextScreen();
                }
                else {
                    nextScreen();
                    cout << "invalid input\n\n";
                }
                break;
            case 'B':
                if (score >= 8) {
                    wager += 8;
                    score -= 8;
                    cancel = true;
                    nextScreen();
                }
                else {
                nextScreen();
                cout << "invalid input\n\n";
                }
                break;
            case 'G':
                if (score >= 16) {
                    wager += 16;
                    score -= 16;
                    cancel = true;
                    nextScreen();
                }
                else {
                    nextScreen();
                    cout << "invalid input\n\n";
                }
                break;
            case 'L':
                if (score >= 32) {
                    wager += 32;
                    score -= 32;
                    cancel = true;
                    nextScreen();
                }
                else {
                    nextScreen();
                    cout << "invalid input\n\n";
                }
                break;
            case 'A':
                if (cancel) {
                    loop1 = false; //go to next step if cancel active
                    nextScreen();
                }
                else {
                    nextScreen();
                    cout << "invalid input\n\n";
                }
                break;
            default:
                nextScreen();
                cout << "really return to menu?"
                    << "   - [Y]es"
                    << "      - [N]o\n\n";
                cin >> charIn;
                nextScreen();

                if (toupper(charIn) == 'Y') {
                    return; //end game and return to menu
                }
                break;
            }

        }

        //deal initial cards, get initial hand values and check for blackjack
        drawCard(deck, player, discard);
        drawCard(deck, dealer, discard); //face down
        drawCard(deck, player, discard);
        drawCard(deck, dealer, discard);
        playerValue = checkValue(player);
        dealerValue = checkValue(dealer);
        playerBlackjack = checkBlackjack(player);
        dealerBlackjack = checkBlackjack(dealer);

        cout << "wager: " << wager << "\n\n\n";
        //ask for insurance if up card is A
        if (dealer[1] == 'A') {
            displayHands(player, dealer, false);
            if (!playerBlackjack) {
                cout << "insurance? ->\n"
                    << "   - [Y]es\n"
                    << "      - [*]no\n\n";
                cin >> charIn;

                switch (toupper(charIn)) {
                case 'Y':
                    placedInsurance = true;
                    if ((score - (wager * 0.5)) > 0) {
                        insurance += (wager * 0.5);
                        score -= (wager * 0.5);
                    }
                    else {
                        insurance += score;
                        score = 0;
                    }
                    
                    break;
                default:
                    break;
                }
                nextScreen();
            }
            else {
                cout << "even money? ->\n"
                    << "   - [Y]es\n"
                    << "      - [*]no\n\n";
                cin >> charIn;

                switch (toupper(charIn)) {
                case 'Y':
                    placedInsurance = true;
                    break;
                default:
                    break;
                }
            }
            nextScreen();
            cout << "wager: " << wager << "\n\n\n";
        }

        //do blackjack stuff first
        if (playerBlackjack && dealerBlackjack) {
            displayHands(player, dealer, true);
            if (placedInsurance) {
                cout << "even money\n\n";
                multiplier = 2;
            }
            else {
                cout << "push!\n\n";
                multiplier = 1;
            }
            cin >> charIn;
        }
        else if (playerBlackjack && !dealerBlackjack) {
            displayHands(player, dealer, true);
            if (placedInsurance) {
                cout << "even money\n\n";
                multiplier = 2;
            }
            else {
                cout << "B L A C K J A C K\n\n";
                multiplier = 2.5;
            }
            cin >> charIn;
        }
        else if (!playerBlackjack && dealerBlackjack) {
            displayHands(player, dealer, true);
            if (placedInsurance) {
                cout << "insurance\n\n";
                multiplier = 0;
                insurance += insurance;
            }
            else {
                cout << "you lose...\n\n";
                multiplier = 0;
            }
            cin >> charIn;
        }
        else { //if no blackjack, continue in this else
            if (placedInsurance) {
                cout << "dealer doesn't have 21, insurance taken...\n\n\n";
                placedInsurance = false;
                insurance *= -1;
            }
            loop1 = true;

            //player plays their hand
            while (loop1) {
                displayHands(player, dealer, false);
                showAvailableMoves(player, dealer, deck, discard);

                cin >> charIn;
                switch (toupper(charIn)) {
                case 'S':
                    loop1 = false;
                    nextScreen();
                    break;
                case 'H':
                    if (!(deck.empty() && discard.empty())) {
                        drawCard(deck, player, discard);
                        playerValue = checkValue(player);
                        if (playerValue > 21) {
                            nextScreen();

                            cout << "bust...\n\n\n";
                            displayHands(player, dealer, false);

                            cin >> charIn;
                            loop1 = false;
                            bust = true;
                            multiplier = 0;
                        }
                        nextScreen();
                    }
                    else {
                        nextScreen();
                        cout << "invalid input\n\n";
                    }
                    break;
                case 'D':
                    if (!(deck.empty() && discard.empty())) {
                        if ((score - wager) > 0) {
                            score -= wager;
                            wager += wager;
                        }
                        else {
                            wager += score;
                            score = 0;
                        }
                        drawCard(deck, player, discard);
                        playerValue = checkValue(player);
                        if (playerValue > 21) {
                            nextScreen();

                            cout << "bust...\n\n\n";
                            displayHands(player, dealer, false);

                            cin >> charIn;
                            bust = true;
                            multiplier = 0;
                        }
                        loop1 = false;
                        nextScreen();
                    }
                    else {
                        nextScreen();
                        cout << "invalid input\n\n";
                    }
                    break;
                case 'U': //surrender
                    multiplier = 0.5;
                    bust = true;
                    loop1 = false;
                    nextScreen();
                    break;
                default:
                    nextScreen();
                    cout << "invalid input\n\n";
                    break;
                }
            }

            //dealer plays their hand if player hasn't busted or surrendered
            if (!bust) {
                loop1 = true;
                cout << "dealer's turn\n\n\n";
                while (loop1) {
                    displayHands(player, dealer, true);
                    cin >> charIn;
                    
                    //draw cards until more than 17 reached
                    if (dealerValue < 17) {
                        drawCard(deck, dealer, discard);
                    }
                    else if (dealerValue == 17) { //draw on soft 17
                        bool hasAce = false;
                        for (char x : dealer) {
                            if (x == 'A') {
                                hasAce = true;
                            }
                        }
                        if (hasAce) {
                            drawCard(deck, dealer, discard);
                        }
                        loop1 = false;
                    }
                    else {
                        loop1 = false;
                    }
                    

                    dealerValue = checkValue(dealer);
                    nextScreen();
                }

                //check if dealer busted, then check player and dealer values
                displayHands(player, dealer, true);
                if (dealerValue > 21) {
                    cout << "dealer bust\n\n";
                    multiplier = 2;
                }
                else {
                    if (playerValue == dealerValue) {
                        cout << "push!\n\n";
                        multiplier = 1;
                    }
                    else if (playerValue > dealerValue) {
                        cout << "you win\n\n";
                        multiplier = 2;
                    }
                    else {
                        cout << "you lose...\n\n";
                        multiplier = 0;
                    }
                }
                cin >> charIn;
                nextScreen();
            }

        }

        //report earning/losses before next round, sweep table
        discardHand(player, discard);
        discardHand(dealer, discard);
        if (placedInsurance) {
            if (!playerBlackjack) {
                cout << "insurance paid: " << insurance;
            }
            else {
                cout << "even money earned: " << wager * multiplier;
            }
        }
        else {
            if (multiplier == 0) {
                cout << "losings: " << wager + insurance;
            }
            else {
                cout << "earnings: " << (wager * multiplier) + insurance;
            }
        }
        wager *= multiplier;
        score += (wager + insurance);
        if (score <= 0) {
            score = 0;
        }
        cout << "\n\nnew score: " << score << "\n\n\n";
        cin >> charIn;
        nextScreen();

        if (score == 0) {
            cout << "no more money...\n\n\n";
            cin >> charIn;
            nextScreen();
            LoadDeckFromDefault(deck);
            return; //go to title
        }
        

    }


}

/*
    method to dynamically show menu input for wagers
*/
void showAvailableWagers(int& score, bool& cancel) {
    cout << "take wager tokens ->\n";
    int menuspaces = 3;
    if (score >= 1) {
        for (int i = 0; i < menuspaces; i++) { cout << " "; }
        cout << "- [W]hite - 1\n";
        menuspaces += 3;
    } if (score >= 2) {
        for (int i = 0; i < menuspaces; i++) { cout << " "; }
        cout << "- [P]ink - 2\n";
        menuspaces += 3;
    } if (score >= 4) {
        for (int i = 0; i < menuspaces; i++) { cout << " "; }
        cout << "- [R]ed - 4\n";
        menuspaces += 3;
    } if (score >= 8) {
        for (int i = 0; i < menuspaces; i++) { cout << " "; }
        cout << "- [B]lue - 8\n";
        menuspaces += 3;
    } if (score >= 16) {
        for (int i = 0; i < menuspaces; i++) { cout << " "; }
        cout << "- [G]reen - 16\n";
        menuspaces += 3;
    } if (score >= 32) {
        for (int i = 0; i < menuspaces; i++) { cout << " "; }
        cout << "- b[L]ack - 32\n";
        menuspaces += 3;
    } if (cancel) {
        for (int i = 0; i < menuspaces; i++) { cout << " "; }
        cout << "\n- pl[A]y turn\n";
    }
    for (int i = 0; i < 3; i++) { cout << " "; }
    cout << "\n- [*]exit to menu\n";
    cout << "\n";
}

/*
    show how player could play their hand
*/
void showAvailableMoves(vector<char> &player, vector<char>& dealer, vector<char>&deck, vector<char>&discard) {
    cout << "play your hand ->\n";
    int menuspaces = 3;
    if (true) {
        for (int i = 0; i < menuspaces; i++) { cout << " "; }
        cout << "- [S]tand\n";
        menuspaces += 3;
    } if (!(deck.empty() && discard.empty())) {
        for (int i = 0; i < menuspaces; i++) { cout << " "; }
        cout << "- [H]it\n";
        menuspaces += 3;
    } else {
        for (int i = 0; i < menuspaces; i++) { cout << " "; }
        cout << "- {deck and discard empty, no hit or double down}\n";
        menuspaces += 3;
    } if (!(deck.empty() && discard.empty())) {
        for (int i = 0; i < menuspaces; i++) { cout << " "; }
        cout << "- [D]ouble down\n";
        menuspaces += 3;
    } if (true) {
        for (int i = 0; i < menuspaces; i++) { cout << " "; }
        cout << "- s[U]rrender\n";
        menuspaces += 3;
    }
    cout << "\n";
}

/*
    push a card from deck and put it into hand, pop from deck
*/
void drawCard(vector<char>& deck, vector<char>& hand, vector<char>& discard) {
    //first check if deck is empty, put discard into it and shuffle
    if (deck.empty() && !discard.empty()) {
        PutDiscardIntoDeck(deck, discard);
        shuffleDeck(deck);
        cout << "deck shuffled\n\n\n";
    }
    else if (deck.empty() && discard.empty()) {
        cout << "main deck and discard are empty, cannot draw more cards\n\n\n";
    }
    else {
        hand.push_back(deck.back());
        deck.pop_back();
    }
}

/*
    checks a hand's value, returns result
*/
int checkValue(vector<char>& hand) {
    int value = 0;
    int aceChecks = 0;
    bool hasJoker = false;
    for (char x : hand) {
        switch (x) {
        case 'A':
            aceChecks++; //check for what value aces should be after all other cards
            break;
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            value += x - '0'; //add value of number in char
            break;
        case '0':
        case 'J':
        case 'Q':
        case 'K':
            value += 10;
            break;
        case 'Z':
            if (!hasJoker) {
                value -= 3;
            }
            hasJoker = true; //only one Joker will have this effect
            break;
        }

    }
    //check what aces should count as now
    for (int x = 0; x < aceChecks; x++) {
        if (value + 11 > 21) {
            value += 1;
        }
        else {
            value += 11;
        }
    }
    //value should never display as 0 or below
    if (value <= 0) {
        value = 1;
    }
    return value;
}

/*
    returns whether the hand is a blackjack
*/
bool checkBlackjack(vector<char>& hand) {
    bool hasAce = false;
    bool hasTen = false;
    if (hand.size() != 2) {
        return false;
    }
    for (char x : hand) {
        switch (x) {
        case 'A':
            hasAce = true;
            break;
        case '0':
        case 'J':
        case 'Q':
        case 'K':
            hasTen = true;
            break;
        default:
            return false;
        }
    }
    if (hasAce && hasTen) {
        return true;
    }
    else {
        return false;
    }
}

/*
    main gameplay view
*/
void displayHands(vector<char>& playerHand, vector<char>& dealerHand, bool showDealer) {
    int i = 0;
    cout << "dealer:\t";
    for (char x : dealerHand) {
        if (!showDealer && i == 0) {
            i++;
            cout << "~ "; //don't show dealer's first card
        }
        else {
            cout << x << " ";
        }
    }
    cout << "\n\n\n";
    cout << "player:\t";
    for (char x : playerHand) {
        cout << x << " ";
    }
    cout << "\n\n\n";
}

/*
    Display main menu, returns input of whatever player chose
*/
int mainMenu(vector<char>& deck) {
    char input;
    int i = 0;

    //status message first if there was any from another function

    //display sorted deck
    sort(deck.begin(), deck.end());
    cout << "current deck:\n\n";
    for (char x : deck) {
        cout << x << " ";
        i++;
        if (i > 25) { cout << "\n"; i = 0; }
    }
    cout << "\n\n";

    //display menu options
    cout << "select an option ->\n"
        << "   - [S]tart game\n"
        << "      - [C]ustomize deck\n"
        << "         - [V]iew readme\n\n";

    while (true) {
        cin >> input;

        switch (toupper(input))
        {
        case 'S':
            return 1;
            break;
        case 'C':
            return 3;
            break;
        case 'V':
            return 2;
            break;
        default:
            break; //force correct input
        }
    }
}

/*
    Explains how to make a custom deck in a .txt file, then asks if the user wants to proceed
*/
int confirmCustomDeckLoad() {
    
    cout << "here you can create a deck from the characters of any .txt file. when a valid file is given\n"
        << "each character is read and put into the deck according to this chart:\n\n"
        << "\tA = Ace, 2-0 = 2 through 10, J = Jack, Q = Queen, K = King\n\n"
        << "any character that is not listed is converted to a Joker (denoted with a Z).\n"
        << "having at least one of these special cards subtracts your hand's total by 3.\n"
        << "the dealer will never accept these cards, and will instead discard them\n\n\n";

    cout << "would you like to ->\n"
        << "   - [L]oad a txt file to use as a custom deck\n"
        << "      - [R]eset deck to standard\n"
        << "         - [*]cancel\n\n";
    char confirm;
    cin >> confirm;
    
    switch (toupper(confirm)) {
    case 'L':
        return 1;
        break;
    case 'R':
        return 2;
        break;
    default:
        return 0;
        break;
    }
}

/*
    load default deck
*/
void LoadDeckFromDefault(vector<char>& deck) {
    deck.clear();

    for (int i = 0; i < sizeof(DECKDEFAULT); i++) {
        deck.push_back(DECKDEFAULT[i]);
    }
}

/*
    load contents of a .txt file into the deck
*/
void LoadDeckFromCustom(vector<char>& deck) {
    string filepath;
    string cancelCheck;
    ifstream file;
    char card;

    cout << "enter the full path of the .txt file to load it, or type 'cancel' or 'C' to return\n\n";
    cin >> filepath;
    nextScreen();

    cancelCheck = filepath; //check for cancel input first and foremost
    for (auto& c : cancelCheck) c = toupper(c);
    if (cancelCheck.compare("CANCEL") == 0 || cancelCheck.compare("C") == 0) {
        return;
    }

    file.open(filepath);
    if (!file) {
        cout << "error reading file\n\n\n";
        return;
    }
    deck.clear(); //assuming valid file at this point
    while (file >> card) {
        switch (card) {
        case 'A':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '0':
        case 'J':
        case 'Q':
        case 'K':
            deck.push_back(toupper(card)); //if expected value, push actual char
            break;
        default:
            deck.push_back('Z'); //if anything else push Z for joker card
            break;
        }
        
    }
    file.close();

    cout << "custom deck successfully loaded\n\n";
    
    return;
}

/*
    put all cards from the discard pile into the deck
*/
void PutDiscardIntoDeck(vector<char>& deck, vector<char>& discard) {
    if (!discard.empty()) {
        for (char x : discard) {
            deck.push_back(discard.back());
            discard.pop_back();
        }
    }
}

/*
    put hand into the shadow real-- I mean, discard pile
*/
void discardHand(vector<char>& hand, vector<char>& discard) {
    for (char x : hand) {
        discard.push_back(hand.back());
        hand.pop_back();
    }
}

/*
    Load a whole bunch of text explaining the program
*/
void loadReadme() {
    string blank;

    cout << "coming soon lol";

    cout << "\n\n";
    cin >> blank;
    return;
}

/*
    shuffles the deck
*/
void shuffleDeck(vector<char>& deck) {
    shuffle(deck.begin(), deck.end(), rng);
}

/*
    Very basic screen refresh implementation
*/
void nextScreen() { cout << string(100, '\n'); }


// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
