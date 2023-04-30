#include <iostream> // standard input output stream library
#include <ctime> // time related library
#include <thread> // library for working with threads
#include <mutex> // library for working with mutexes
#include <vector> // library for dynamic arrays
#include <algorithm> // used for the random shuffle of cards aka random
#include <string> // library for strings
#include <sstream> // used with formatted strings

using namespace std; // Using the standard namespace to make things easier.


class Card {
public:
    Card(int value) : value(value) {} // constructor that initializes the value member variable of the card object

    int getValue() const { // getter for the vale variable
        return value;
    }

    // function for returning a string of the card that is drawn
    string toString() const {
        const char* suits[] = { "Hearts ", "Diamonds ", "Clubs ", "Spades " }; // array for suits of cards
        const char* ranks[] = { "2", "3", "4", "5", "6", "7", "8", "9", "10", "Jack", "Queen", "King", "Ace" }; // array for the ranks of cards

        stringstream ss; // creates a stringstream to build the string of the drawn card
        ss << ranks[value % 13] << " of " << suits[value / 13];
        return ss.str(); // returns the string of the card
    }

private:
    int value; // holds value of the card
};

class Deck {
public:
    // crostucter that builds the deck of 52 cards and then shuffles it
    Deck() {
        for (int i = 0; i < 52; ++i) {
            cards.push_back(Card(i));
        }
        shuffle();
    }

    // Deal a card function from the deck by removing the last card and returning it to the deck
    Card dealCard() {
        Card card = cards.back();
        cards.pop_back();
        return card;
    }

    // Shuffle function for the deck and uses the random_shuffle function from the algorithm library.
    void shuffle() {
        srand(time(nullptr)); // seeds the random number generator with the current time.
        random_shuffle(cards.begin(), cards.end()); // vector to shuffle cards
    }

private:
    vector<Card> cards; // this vector holds the card in the deck
};

class Hand {
public:
    // function for adding a card to the 'hand'
    void addCard(const Card& card) {
        cards.push_back(card);
    }

    // this function will calculate the score total of the hand
    int getScore() const {
        int score = 0;
        int aceCount = 0;

        for (const Card& card : cards) {
            int rank = card.getValue() % 13;
            int value;

            if (rank < 9) {
                value = rank + 2;
            }
            else if (rank < 12) {
                value = 10;
            }
            else {
                value = 11;
                ++aceCount;
            }

            score += value;
        }

        while (score > 21 && aceCount > 0) {
            score -= 10;
            --aceCount;
        }

        return score;
    }

    // function to convert the hand to a string representation that is easily printable to the screen.
    string toString() const {
        stringstream ss;
        for (const Card& card : cards) {
            ss << card.toString() << " ";
        }
        return ss.str();
    }

private:
    vector<Card> cards; // this vector holds the cards in the deck
};

class Player {
public:
    // Constructor for a new empty hand each game
    Player() : hand(Hand()) {}

    // add a card to players hand
    void addToHand(const Card& card) {
        hand.addCard(card);
    }

    // gets player score
    int getScore() const {
        return hand.getScore();
    }

    // gets the string reprensentation of the players hand
    string getHand() const {
        return hand.toString();
    }

private:
    Hand hand; 
};

class Dealer : public Player {
public:
    // function for the dealers turn/thread
    void dealerTurn (Deck& deck, mutex& mtx)
    {
        while (getScore() < 17) { // if score is under 17 the dealer will hit.
            unique_lock<mutex> lock(mtx);
            addToHand(deck.dealCard());
            lock.unlock();
        }
    }
};

class Human : public Player {
public:
    void playerTurn(Dealer& house, Deck& deck, mutex& mtx) {
        char choice; // holds users choice
        bool hit = true; // boolean variable and set to true
        do {
            cout << "Dealer's Hand = " << endl;    // show dealer's hand
            cout << "** " << house.getHand().substr(house.getHand().find(" ")) << endl;    // shows the dealer's hand while hiding the first card
            cout << "Player's Hand = " << getScore() << endl;    // display the player's current total value of their hand
            cout << getHand() << endl;    // display the player's current hand/cards

            cout << "(h)it, (s)tay, or (q)uit: ";    // ask the user to hit stay or quit
            cin >> choice;    // check the user's choice from stored input
            if (choice == 'h') {    // if the user chooses to hit do this
                unique_lock<mutex> lock(mtx);    // lock the mutex
                addToHand(deck.dealCard());    // adds a new card to the player's hand
                lock.unlock();    // unlocks the mutex
            }
            else if (choice == 's') {    // if the user chooses to stay it will do the following
                hit = false;    // set the hit to false then exit the loop
            }
            else if (choice == 'q') {    // if the user chooses to quit it will do the following
                cout << "Good Luck! Quitting the game... " << endl;    // displays goodbye message
                exit(EXIT_SUCCESS);    // exit the program with success status to the log
            }
            else {    // if the user enters an invalid choice it will do the follow
                cout << "Error: Please Try Again!" << endl;    // display an error message to the screen
            }
            cout << " " << endl;    // print an extra blank line
        } while (hit && getScore() < 22);    // This will loop as long as the hit flag is true and the player doesn't bust
    }
};

int main() {
    // initialize the below variables to zero
    int handCount = 0;
    int winCount = 0;
    int loseCount = 0;
    int tieCount = 0;

    cout << "Welcome to ";
    // Prints welcome message to the screen, used https://patorjk.com/ ASCII Art Generator
    cout << R"(                                                                                  
                                                ,---._                                  
  ,----..                                     .-- -.' \                            ,-.  
 /   /   \ ,-.----.  ,-.----.                 |    |   :                       ,--/ /|  
|   :     :\    /  \ \    /  \                :    ;   |                     ,--. :/ |  
.   |  ;. /|   :    ||   :    |               :        |                     :  : ' /   
.   ; /--` |   | .\ :|   | .\ :               |    :   :  ,--.--.     ,---.  |  '  /    
;   | ;    .   : |: |.   : |: |               :          /       \   /     \ '  |  :    
|   : |    |   |  \ :|   |  \ :               |    ;   |.--.  .-. | /    / ' |  |   \   
.   | '___ |   : .  ||   : .  |           ___ l          \__\/: . ..    ' /  '  : |. \  
'   ; : .'|:     |`-':     |`-'         /    /\    J   : ," .--.; |'   ; :__ |  | ' \ \ 
'   | '/  ::   : :   :   : :           /  ../  `..-    ,/  /  ,.  |'   | '.'|'  : |--'  
|   :    / |   | :   |   | :           \    \         ;;  :   .'   \   :    :;  |,'     
 \   \ .'  `---'.|   `---'.|            \    \      ,' |  ,     .-./\   \  / '--'       
  `---`      `---`     `---`             "---....--'    `--`---'     `----'             
)" << '\n';
    cout << "This is a Blackjack game but created in C++. " << endl;
    cout << "All the standard rules apply. House stays at 17. Do not exceed 21. Cards 2-10 are worth their number vaule. " << endl;
    cout << "Face cards are worth 10. Ace is worth 1 or 11. " << endl;
    cout << " " << endl;
    // Getting the user's choice
    cout << "Please press 1 to start or 2 to exit the game: " << endl;
    int choice; // variable to store users choice
    cin >> choice; // store it to choice

    // Starts loop if 1 is pressed or exits if 2 is pressed
    switch (choice) {
    case 1:
    while (true) {
        ++handCount;
        Deck deck;
        Dealer dealer;
        Human player;
        // Deals two cards to each player(dealer and player)
        dealer.addToHand(deck.dealCard());
        player.addToHand(deck.dealCard());
        dealer.addToHand(deck.dealCard());
        player.addToHand(deck.dealCard());

        system("cls"); // clears screen

        cout << R"(                                                                                  
                                                ,---._                                  
  ,----..                                     .-- -.' \                            ,-.  
 /   /   \ ,-.----.  ,-.----.                 |    |   :                       ,--/ /|  
|   :     :\    /  \ \    /  \                :    ;   |                     ,--. :/ |  
.   |  ;. /|   :    ||   :    |               :        |                     :  : ' /   
.   ; /--` |   | .\ :|   | .\ :               |    :   :  ,--.--.     ,---.  |  '  /    
;   | ;    .   : |: |.   : |: |               :          /       \   /     \ '  |  :    
|   : |    |   |  \ :|   |  \ :               |    ;   |.--.  .-. | /    / ' |  |   \   
.   | '___ |   : .  ||   : .  |           ___ l          \__\/: . ..    ' /  '  : |. \  
'   ; : .'|:     |`-':     |`-'         /    /\    J   : ," .--.; |'   ; :__ |  | ' \ \ 
'   | '/  ::   : :   :   : :           /  ../  `..-    ,/  /  ,.  |'   | '.'|'  : |--'  
|   :    / |   | :   |   | :           \    \         ;;  :   .'   \   :    :;  |,'     
 \   \ .'  `---'.|   `---'.|            \    \      ,' |  ,     .-./\   \  / '--'       
  `---`      `---`     `---`             "---....--'    `--`---'     `----'             
)" << '\n';

        cout << "Hand #: " << handCount << endl;
        cout << "Wins: " << winCount << endl;
        cout << "Losses: " << loseCount << endl;
        cout << "Ties: " << tieCount << endl;
        cout << "---------------------------------------------------------------------------------------" << endl;
        cout << " " << endl;

        mutex mtx;
        thread dealerThread(&Dealer::dealerTurn, ref(dealer), ref(deck), ref(mtx));
        thread playerThread(&Human::playerTurn, ref(player), ref(dealer), ref(deck), ref(mtx));

        // Exception handling for the thread join operations
        try {
            playerThread.join(); // Waits for the player's turn to finish
        }
        catch (const system_error& e) {
            cerr << "Error joining playerThread: " << e.what() << endl;
            return EXIT_FAILURE; // exits program with error status code to log
        }

        try {
            dealerThread.join(); // Waits for the house's turn to finish
        }
        catch (const system_error& e) {
            cerr << "Error joining houseThread: " << e.what() << endl;
            return EXIT_FAILURE; // exits program with error status code to log
        }

        // gets the scores of the dealer and user/player
        int playerScore = player.getScore();
        int dealerScore = dealer.getScore();

        // now time to determine the winner based on the scores/hand values
        if (playerScore > 21) {
            system("cls"); // clears screen
            cout << R"(                                                                                  
  ____  U _____ u    _      _    U _____ u   ____                                    _   _    ____     _    
 |  _"\ \| ___"|/U  /"\  u |"|   \| ___"|/U |  _"\ u      __        __      ___     | \ |"|  / __"| uU|"|u  
/| | | | |  _|"   \/ _ \/U | | u  |  _|"   \| |_) |/      \"\      /"/     |_"_|   <|  \| |><\___ \/ \| |/  
U| |_| |\| |___   / ___ \ \| |/__ | |___    |  _ <        /\ \ /\ / /\      | |    U| |\  |u u___) |  |_|   
 |____/ u|_____| /_/   \_\ |_____||_____|   |_| \_\      U  \ V  V /  U   U/| |\u   |_| \_|  |____/>> (_)   
  |||_   <<   >>  \\    >> //  \\ <<   >>   //   \\_     .-,_\ /\ /_,-..-,_|___|_,-.||   \\,-.)(  (__)|||_  
 (__)_) (__) (__)(__)  (__)_")("_)__) (__) (__)  (__)     \_)-'  '-(_/  \_)-' '-(_/ (_")  (_/(__)    (__)_) 
)" << '\n';
            ++loseCount; // adds 1 to the count/score
            cout << "Dealer's Hand was: " << dealerScore << endl;
            cout << dealer.getHand() << endl;
            cout << "Player's Hand was: " << playerScore << endl;
            cout << player.getHand() << endl;
            system("pause");
        }
        else if (dealerScore > 21) {
            system("cls"); // clears screen
            cout << R"(    
  __   __   U  ___ u   _   _                                 _   _     _    
  \ \ / /    \/"_ \/U |"|u| |     __        __      ___     | \ |"|  U|"|u  
   \ V /     | | | | \| |\| |     \"\      /"/     |_"_|   <|  \| |> \| |/  
  U_|"|_u.-,_| |_| |  | |_| |     /\ \ /\ / /\      | |    U| |\  |u  |_|   
    |_|   \_)-\___/  <<\___/     U  \ V  V /  U   U/| |\u   |_| \_|   (_)   
.-,//|(_       \\   (__) )(      .-,_\ /\ /_,-..-,_|___|_,-.||   \\,-.|||_  
 \_) (__)     (__)      (__)      \_)-'  '-(_/  \_)-' '-(_/ (_")  (_/(__)_) 
)" << '\n';
            ++winCount; // adds 1 to the count/score
            cout << "Dealer's Hand was: " << dealerScore << endl;
            cout << dealer.getHand() << endl;
            cout << "Player's Hand was: " << playerScore << endl;
            cout << player.getHand() << endl;
            system("pause"); // pause until user presses a key
        }
        else if (playerScore == dealerScore) {
            system("cls"); // clears screen
            cout << R"(    
  _____             U _____ u  _    
 |_ " _|     ___    \| ___"|/U|"|u  
   | |      |_"_|    |  _|"  \| |/  
  /| |\      | |     | |___   |_|   
 u |_|U    U/| |\u   |_____|  (_)   
 _// \\_.-,_|___|_,-.<<   >>  |||_  
(__) (__)\_)-' '-(_/(__) (__)(__)_) 
)" << '\n';
            ++tieCount; // adds 1 to the count/score
            cout << "Dealer's Hand was: " << dealerScore << endl;
            cout << dealer.getHand() << endl;
            cout << "Player's Hand was: " << playerScore << endl;
            cout << player.getHand() << endl;
            system("pause");
        }
        else if (playerScore > dealerScore) {
            system("cls"); // clears screen
            cout << R"(    
  __   __   U  ___ u   _   _                                 _   _     _    
  \ \ / /    \/"_ \/U |"|u| |     __        __      ___     | \ |"|  U|"|u  
   \ V /     | | | | \| |\| |     \"\      /"/     |_"_|   <|  \| |> \| |/  
  U_|"|_u.-,_| |_| |  | |_| |     /\ \ /\ / /\      | |    U| |\  |u  |_|   
    |_|   \_)-\___/  <<\___/     U  \ V  V /  U   U/| |\u   |_| \_|   (_)   
.-,//|(_       \\   (__) )(      .-,_\ /\ /_,-..-,_|___|_,-.||   \\,-.|||_  
 \_) (__)     (__)      (__)      \_)-'  '-(_/  \_)-' '-(_/ (_")  (_/(__)_) 
)" << '\n';
            ++winCount; // adds 1 to the count/score
            cout << "Dealer's Hand was: " << dealerScore << endl;
            cout << dealer.getHand() << endl;
            cout << "Player's Hand was: " << playerScore << endl;
            cout << player.getHand() << endl;
            system("pause"); // clears screen
        }
        else {
            system("cls"); // clears screen
            cout << R"(                                                                                  
  ____  U _____ u    _      _    U _____ u   ____                                    _   _    ____     _    
 |  _"\ \| ___"|/U  /"\  u |"|   \| ___"|/U |  _"\ u      __        __      ___     | \ |"|  / __"| uU|"|u  
/| | | | |  _|"   \/ _ \/U | | u  |  _|"   \| |_) |/      \"\      /"/     |_"_|   <|  \| |><\___ \/ \| |/  
U| |_| |\| |___   / ___ \ \| |/__ | |___    |  _ <        /\ \ /\ / /\      | |    U| |\  |u u___) |  |_|   
 |____/ u|_____| /_/   \_\ |_____||_____|   |_| \_\      U  \ V  V /  U   U/| |\u   |_| \_|  |____/>> (_)   
  |||_   <<   >>  \\    >> //  \\ <<   >>   //   \\_     .-,_\ /\ /_,-..-,_|___|_,-.||   \\,-.)(  (__)|||_  
 (__)_) (__) (__)(__)  (__)_")("_)__) (__) (__)  (__)     \_)-'  '-(_/  \_)-' '-(_/ (_")  (_/(__)    (__)_) 
)" << '\n';
            ++loseCount; // adds 1 to the count/score
            cout << "Dealer's Hand was: " << dealerScore << endl;
            cout << dealer.getHand() << endl;
            cout << "Player's Hand was: " << playerScore << endl;
            cout << player.getHand() << endl;
            system("pause"); // pause until user presses a key
        }
    }
    break;
    case 2:
    return 0; // exits program status 0
    default: // if there is an invalid selection does the following
        cout << "Invalid selection. Please try again. " << endl;
        break; // end
    }
    return 0; // exits program status 0
}