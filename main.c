#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void* visitor();

int interaction(float* pBal, char item_list[4][4][16], int inv[4][4]);
int visitors;
int max_visitors = 10; // Max amount of visitors before more stop arriving.
int visitor_frequency = 30; // A new visitor will appear every x seconds.

struct of {
    int intention;      // 0 = Buy, 1 = Sell
    int item[3];        // Selects item {[0] = amount, [1] = category, [2] = item}
    float price;        // Offers one price for all requested items
};

int main(void)
{

    srand(time(NULL));
    
    float balance = 100;
    float* pBalance = &balance;

    char items[4][4][16] =
    {
        {"Oats", "Wheat", "Egg", "Milk Jug"},
        {"Fire Wood", "Birch Log", "Oak Log", "Spruce Log"},
        {"Cod", "Carp", "Trout", "Salmon"},
        {"Chicken Breast", "Beef", "Porkchop", "Turkey"}
    };

    int inventory[4][4] =
    {
        {0, 0, 0, 0},   // Farming
        {0, 0, 0, 0},   // Lumber
        {0, 0, 0, 0},   // Fishing
        {0, 0, 0, 0}    // Butcher
    };

    // Starts visitor arrival loop
    pthread_t visitorT;
    pthread_create(&visitorT, NULL, visitor, NULL);

    char move;
    //Game Loop
    while (1)
    {
        move = getchar();
        switch(move)
        {
            case 'Q':
            case 'q':
                printf("Quitting...\n");
                return 0;
            
            case 'C':
            case 'c':
                if (visitors < 1)
                {
                    printf("There are currently no visitors!\n");
                    break;
                }
                interaction(pBalance, items, inventory);
                break;

            case 'I':
            case 'i':
                printf("INVENTORY:\n");
                for (int item = 0; item < 4; item++)
                {
                    for (int category = 0; category < 4; category++)
                    {
                        if (inventory[category][item] < 100)
                        {
                            printf(" ");
                        }
                        if (inventory[category][item] < 10)
                        {
                            printf(" ");
                        }
                        printf("%ix %s", inventory[category][item], items[category][item]);
                        for (int length = strlen(items[category][item]); length < 14; length++)
                        {
                            printf(" ");
                        }
                        printf("\t");
                    }
                    printf("\n");
                }
                break;

            case 'B':
            case 'b':
                printf("Balance:\n$ %.2f\n", balance);
                break;

            case 'H':
            case 'h':
                printf("Every action only takes one character!\n[b] -> display balance.\n[c] -> interact with visitor.\n[h] -> display this menu.\n[i] -> display inventory.\n[q] -> quit.\n");
                break;
        }
    }

    return 0;
}

int interaction(float* pBal, char it_lst[4][4][16], int inv[4][4])
{
    float bal = *pBal;
    int item[3] = {rand() % 10 + 1, rand() % 4, rand() % 4};
    struct of offer =
    {
        .intention = rand() % 2,
        .item = {item[0], item[1], item[2]},
        .price = (float) item[0] * (item[2] + 1) * (rand() % 70 + 80) / 100
    };

    char intent_opts[2][4] = {"buy", "sell"};
    printf("Hey, I would like to %s:\n  %ix %s\t(You have %i)\nfor $%.2f\t[y/n]\n", intent_opts[offer.intention], offer.item[0], it_lst[offer.item[1]][offer.item[2]], inv[offer.item[1]][offer.item[2]],offer.price);
    char input;
    while (1)
    {
        input = getchar();
        if (input == 'N' || input == 'n')
        {
            printf("Denied the offer!\nVisitor has left.\n");
            break;
        }
        else if (input != 'Y' && input != 'y')
        {
            continue;
        }

        if (offer.intention == 1)
        {
            if (bal < offer.price)
            {
                printf("You can't afford this purchase!\nVisitor has left.\n");
                break;
            }
            bal -= offer.price;
            inv[offer.item[1]][offer.item[2]] += offer.item[0];
            printf("You have purchased %ix %s for $%.2f!\nVisitor leaves happy.\n", offer.item[0], it_lst[offer.item[1]][offer.item[2]], offer.price);
            break;
        }
        else
        {
            if (inv[offer.item[1]][offer.item[2]] < offer.item[0])
            {
                printf("You do not have enough items for this sale!\nVisitor has left.\n");
                break;
            }
            bal += offer.price;
            inv[offer.item[1]][offer.item[2]] -= offer.item[0];
            printf("You have sold %ix %s for $%.2f!\nVisitor leaves happy.\n", offer.item[0], it_lst[offer.item[1]][offer.item[2]], offer.price);
            break;
        }
    }
    visitors--;
    *pBal = bal;
    return 0;
}

void* visitor()
{
    while (1)
    {
        sleep(visitor_frequency);
        if (visitors < max_visitors)
        {
            visitors++;
            printf("A new visitor has arrived! (%i/10)\n", visitors);
        }
    }
    return NULL;
}