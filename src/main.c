#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void* visitor();

int interaction(float* pBal, char item_list[4][4][16], int inv[4][4]);
int getTradeInput(int items);


int visitors;
int max_visitors = 10; // Max amount of visitors before more stop arriving.
int visitor_frequency = 10; // A new visitor will appear every x seconds.

struct item
{
    int amount;
    int category;
    int item;
    float price;
};

struct offer {
    int intention;          // 0 = Buy, 1 = Sell
    struct item items[4];
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
    int items = rand() % 4 + 1;

    struct offer offer =
    {
        .intention = rand() % 2
    };

    char intent_opts[2][4] = {"buy", "sell"};
    printf("Hey, I would like to %s:\n", intent_opts[offer.intention]);
    
    int category = rand() % 4;
    for (int i = 0; i < items; i++)
    {
        int amount = rand() % 9 + 1, item = rand() % 4;
        struct item temp =
        {
            .amount = amount,
            .category = category,
            .item = item,
            .price = (float) amount * (float) (item + 1) * (0.75 + (float) (rand() % 50) / 100.0)
        };
        printf("[%i]  $%.2f\t%ix  %s\n", i + 1, temp.price, amount, it_lst[category][item]);
        offer.items[i] = temp;
    }

    printf("Select a trade, enter 0 to deny.\n");

    int tradeNum = getTradeInput(items);

    if (tradeNum == 0)
    {
        visitors--;
        return 0;
    }

    tradeNum--;

    struct item sel_item = offer.items[tradeNum];

    if (offer.intention == 1)
    {
        if (bal < sel_item.price)
        {
            printf("You can't afford this purchase!\nVisitor has left.\n");
            return 0;
        }
        bal -= sel_item.price;
        inv[sel_item.category][sel_item.item] += sel_item.amount;
        printf("You have purchased %ix %s for $%.2f!\nVisitor leaves happy.\n", sel_item.amount, it_lst[sel_item.category][sel_item.item], sel_item.price);
    }
    else
    {
        if (inv[sel_item.category][sel_item.item] < sel_item.amount)
        {
            printf("You do not have enough items for this sale!\nVisitor has left.\n");
            return 0;
        }
        bal += sel_item.price;
        inv[sel_item.category][sel_item.item] -= sel_item.amount;
        printf("You have sold %ix %s for $%.2f!\nVisitor leaves happy.\n", sel_item.amount, it_lst[sel_item.category][sel_item.item], sel_item.price);
    }

    visitors--;
    *pBal = bal;

    return 0;
}

int getTradeInput(int items)
{
    int tNum;

    scanf("%i", &tNum);

    if (tNum == 0)
    {
        printf("You denied the offer. The visitor left.\n");
    }
    else if (tNum > items)
    {
        tNum = getTradeInput(items);
    }

    return tNum;
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