#include <linux/limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


const int visitor_frequency = 5; // A new visitor will appear every x seconds.

struct item
{
    int amount;
    int category;
    int item;
    float price;
};

struct offer {
    int intention;          // 0 = Buy, 1 = Sell
    int amount;
    struct item items[4];
};

struct visitor {
    int active;
    char name[16];
    char category[16];
    struct offer offer;
};

struct visitors {
    int amount;
    struct visitor visitor[8];
};

char item_list[4][4][16] =
{
    {"Oats", "Wheat", "Egg", "Milk"},
    {"Fire Wood", "Birch Log", "Oak Log", "Spruce Log"},
    {"Cod", "Carp", "Trout", "Salmon"},
    {"Chicken Breast", "Beef", "Porkchop", "Turkey"}
};

char names [4] [8] [16];
char categories [4] [16] = {"Farming", "Lumber", "Fishing", "Butchery"};


void* visitor(void *v);

void fetchNames();
void interaction(float* pBal, int inv[4][4], struct visitors* vTT);
void remove_visitor(struct visitor *array[10], int index, int array_length);
int generate_visitor_data(struct visitors* v);
int getNumberInput(int lowerAs, int largerAs);
int display_offer(struct visitor v);
int loadSave(float* pBal, int inv[4][4]);
int save(float* pBal, int inv[4][4]);

int main(void)
{
    struct visitors visitors =
    {
        .amount = 0,
    };

    for (int i = 0; i < 8; i++)
    {
        struct visitor vis = {0};
        visitors.visitor[i] = vis;
    }

    fetchNames();

    srand(time(NULL));
    
    float balance = 100;
    int inventory[4][4] =
    {
        {0, 0, 0, 0},   // Farming
        {0, 0, 0, 0},   // Lumber
        {0, 0, 0, 0},   // Fishing
        {0, 0, 0, 0}    // Butcher
    };

    loadSave(&balance, inventory);

    
    struct visitors *vTT = malloc(sizeof(*vTT));
    *vTT = visitors;
    // Starts visitor arrival loop
    pthread_t visitorT;
    pthread_create(&visitorT, NULL, visitor, vTT);

    char move;
    //Game Loop
    while (1)
    {
        move = getchar();
        switch(move)
        {
            case 'Q':
            case 'q':
                printf("Saving and quitting...\n");
                save(&balance, inventory);
                return 0;

            case 'S':
            case 's':
                printf("Saving...\n");
                save(&balance, inventory);
                break;
            
            case 'C':
            case 'c':
                if (vTT->amount < 1)
                {
                    printf("There are currently no visitors!\n");
                    break;
                }
                interaction(&balance, inventory, vTT);
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
                        printf("%ix %s", inventory[category][item], item_list[category][item]);
                        for (int length = strlen(item_list[category][item]); length < 14; length++)
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

void fetchNames()
{
    FILE *nameFile;
    nameFile = fopen("names.txt", "r");
    if (nameFile == NULL)
    {
        return;
    }
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            fgets(names[i][j], 16, nameFile);
        }
    }
    fclose(nameFile);
}

int loadSave(float* pBal, int inv[4][4])
{
    FILE *saveFile;
    saveFile = fopen("save", "r");
    if (saveFile == NULL)
    {
        return 0;
    }
    *pBal = (float) getw(saveFile) / 100;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            inv[i][j] = getw(saveFile);
        }
    }
    fclose(saveFile);

    return 0;
}

int save(float* pBal, int inv[4][4])
{
    FILE *saveFile;
    saveFile = fopen("save", "w");
    int nBal = (int) (*pBal * 100);
    putw(nBal, saveFile);
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            putw(inv[i][j], saveFile);
        }
    }
    
    fclose(saveFile);
    return 0;
}

void interaction(float* pBal, int inv[4][4], struct visitors* vTT)
{
    float bal = *pBal;
    
    struct visitor empty_vis = {0};
    
    char intent [2] = "BS";
    for (int i = 0; i < 8; i++)
    {
        if (vTT->visitor[i].active == 0) printf("[%i]\n", i + 1);
        else printf("[%i] [%s]\t[%c]  %s\n", i + 1, vTT->visitor[i].category, intent[vTT->visitor[i].offer.intention], vTT->visitor[i].name);
    }

    int select_visitor = getNumberInput(49, 56);
    
    if (vTT->visitor[select_visitor].active == 0)
    {
        return;
    }

    display_offer(vTT->visitor[select_visitor]);

    struct offer offer = vTT->visitor[select_visitor].offer;

    int tradeNum = getNumberInput(48, 48 + offer.amount);

    if (tradeNum == 0)
    {
        printf("%s has left.\n", vTT->visitor[select_visitor].name);
        vTT->amount--;
        vTT->visitor[select_visitor] = empty_vis;
        return;
    }

    if (tradeNum == 10)
    {
        return;
    }

    tradeNum--;

    struct item sel_item = offer.items[tradeNum];

    if (offer.intention == 1)
    {
        if (bal < sel_item.price)
        {
            printf("You can't afford this purchase!\n");
            return;
        }
        bal -= sel_item.price;
        inv[sel_item.category][sel_item.item] += sel_item.amount;
        printf("You have purchased %ix %s for $%.2f!\nVisitor leaves happy.\n", sel_item.amount, item_list[sel_item.category][sel_item.item], sel_item.price);
    }
    else
    {
        if (inv[sel_item.category][sel_item.item] < sel_item.amount)
        {
            printf("You do not have enough items for this sale!\n");
            return;
        }
        bal += sel_item.price;
        inv[sel_item.category][sel_item.item] -= sel_item.amount;
        printf("You have sold %ix %s for $%.2f!\nVisitor leaves happy.\n", sel_item.amount, item_list[sel_item.category][sel_item.item], sel_item.price);
    }

    vTT->amount--;
    vTT->visitor[select_visitor] = empty_vis;
    *pBal = bal;

    return;
}

int getNumberInput(int lowerAs, int largerAs)
{
    char s = 10;
    while (s == 10) s = getchar();
    
    if (s < lowerAs || s > largerAs) return 10;

    return s - lowerAs;
}

//    if (s < 48 || s > 49 + items - 1)

int display_offer(struct visitor v)
{
    char int_opts[2][5] = {"buy\0", "sell\0"};
    printf("%s would like to %s:\n", v.name, int_opts[v.offer.intention]);
    printf("[0]\tLeave\n");
    for (int i = 0; i < v.offer.amount; i++)
    {
        struct item item = v.offer.items[i];
        printf("[%i]\t$", i + 1);
        if (item.price < 1000){printf(" ");}
        if (item.price < 100){printf(" ");}
        if (item.price < 10){printf(" ");}
        printf("%.2f  %ix  %s\n", item.price, item.amount, item_list[item.category][item.item]);
    }

    return 0;
}

int generate_visitor_data(struct visitors* v)
{
    int free_slot;
    for (int i = 0; i < 8; i++)
    {
        if (v->visitor[i].active == 0)
        {
            free_slot = i;
            break;
        }
    }

    int cat = rand() % 4, items = rand() % 4 + 1;

    struct offer offer =
    {
        .intention = rand() % 2,
        .amount = items,
    };
    
    for (int i = 0; i < items; i++)
    {
        struct item item =
        {
            .amount = rand() % 9 + 1,
            .category = cat,
            .item = rand() % 4,
            .price = (float) item.amount * (float) (item.item + 1) * (0.75 + (float) (rand() % 50) / 100.0)
        };

        offer.items[i] = item;
    }

    struct visitor visitor =
    {
        .active = 1,
        .offer = offer,
    };
    int getPerson = rand() % 8;
    for (int i = 0; i < 16; i++)
    {
        visitor.category [i] = categories [cat] [i];
        if (names [cat] [getPerson] [i] == '\n')
        {
            continue;
        }
        visitor.name [i] = names [cat] [getPerson] [i];
    }

    int first_free_slot = 10; 

    for (int i = 0; i < 8; i++)
    {
        if (v->visitor[i].active == 0)
        {
            first_free_slot = i;
            break;
        }
    }
    
    v->visitor[first_free_slot] = visitor;

    return first_free_slot;
}

void* visitor(void *v)
{
    int slot;
    while (1)
    {
        struct visitors *visitors = v;
        sleep(visitor_frequency);
        if (visitors->amount < 8)
        {
            slot = generate_visitor_data(v);
            printf("%s has arrived! (%i/8)\n", visitors->visitor[slot].name, visitors->amount + 1);
            visitors->amount++;
        }
    }
    return NULL;
}