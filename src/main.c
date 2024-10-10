#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

struct item
{
    int amount;
    int category;
    int item;
    float price;
};

struct offer {
    int intention;
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
    int max_visitors;
    int frequency;
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

void fetch_names();
void interaction(float* pBal, int inv[4][4], struct visitors* vTT);
void remove_visitor(struct visitor *array[10], int index, int array_length);
void upgrades(float *bal, struct visitors* vTT);
void sort_visitors(struct visitors* v);
char get_save();
int upgrade(float *bal, float price);
int generate_visitor_data(struct visitors* v);
int get_number_input(int low, int high);
int display_offer(struct visitor v);
int load_save(char sel, float* pBal, int inv[4][4], struct visitors* vTT);
int save(char sel, float* pBal, int inv[4][4], struct visitors* vTT);

int main(void)
{
    srand(time(NULL));
    const char selectadSave = get_save();
    char move;
    struct visitors visitors =
    {
        .max_visitors = 4,
        .frequency = 60,
        .amount = 0,
    };

    struct visitors *vTT = malloc(sizeof(*vTT));
    *vTT = visitors;

    for (int i = 0; i < vTT->max_visitors; i++)
    {
        struct visitor vis = {0};
        visitors.visitor[i] = vis;
    }

    fetch_names();
    
    float balance = 100;
    int inventory[4][4] =
    {
        {0, 0, 0, 0},   // Farming
        {0, 0, 0, 0},   // Lumber
        {0, 0, 0, 0},   // Fishing
        {0, 0, 0, 0}    // Butcher
    };

    load_save(selectadSave, &balance, inventory, vTT);

    pthread_t visitorT;
    pthread_create(&visitorT, NULL, visitor, vTT);

    while (1)
    {
        move = getchar();
        switch(move)
        {
            case 'Q':
            case 'q':
                printf("Saving and quitting...\n");
                save(selectadSave, &balance, inventory, vTT);
                return 0;

            case 'S':
            case 's':
                printf("Saving...\n");
                save(selectadSave, &balance, inventory, vTT);
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

            case 'U':
            case 'u':
                upgrades(&balance, vTT);
                break;

        }
    }

    free(vTT);
    return 0;
}

char get_save()
{
    int x;
    printf("Select a save (0-9)\n");
    do x = getchar();
    while (x < 48 || x > 57);
    return x;
}

void fetch_names()
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

int upgrade(float *bal, float price)
{
    int selection = get_number_input(48, 49);
    if (selection == 0)
    {
        return 1;
    }
    
    if (*bal < price)
    {
        printf("You can't afford this upgrade!\n");
        return 1;
    }

    *bal -= price;
    return 0;
}

void sort_visitors(struct visitors* v)
{
    int x = 0;
    for (int i = 0; i < v->max_visitors; i++)
    {
        if (v->visitor[i].active == 0) x++;
        if (x > 0) v->visitor[i] = v->visitor[i + 1];
    }
    return;
}

void upgrades(float *bal, struct visitors* vTT)
{
    float price;
    printf("--- <  UPGRADES  > ---\n\n[0] Exit\n[1] Visitor Frequency\n[2] Max Visitors\n");
    int selection = get_number_input(48, 50);
    
    switch (selection)
    {
        case 0:
            return;

        case 1:
            price = (float) 15 * (61 - vTT->frequency);
            printf("Visitor Frequency (%is)\n-1s for $%.2f\n[0] Deny\n[1] Accept\n", vTT->frequency, price);
            if (upgrade(bal, price) == 0) vTT->frequency--;
            break;
        
        case 2:
            price = (float) 250 * (vTT->max_visitors - 3);
            printf("Max visitors (%i)\n+1 for $%.2f\n[0] Deny\n[1] Accept\n", vTT->max_visitors, price);
            if (upgrade(bal, price) == 0) vTT->max_visitors++;
            break;
    }
    
}

int load_save(char sel, float* pBal, int inv[4][4], struct visitors* vTT)
{
    char file_name [13] = "saves/save_#\0";
    file_name [11] = sel;
    FILE *saveFile;
    saveFile = fopen(file_name, "r");
    if (saveFile == NULL)
    {
        return 1;
    }
    *pBal = (float) getw(saveFile) / 100;
    vTT->frequency = getw(saveFile);
    vTT->max_visitors = getw(saveFile);
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

int save(char sel, float* pBal, int inv[4][4], struct visitors* vTT)
{
    char file_name [13] = "saves/save_#\0";
    file_name [11] = sel;
    FILE *saveFile;
    saveFile = fopen(file_name, "w");
    int nBal = (int) (*pBal * 100);
    putw(nBal, saveFile);
    putw(vTT->frequency, saveFile);
    putw(vTT->max_visitors, saveFile);
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
    sort_visitors(vTT);
    struct visitor empty_vis = {0};
    
    char intent [2] = "BS";
    for (int i = 0; i < vTT->amount; i++)
    {
        if (vTT->visitor[i].active == 0) printf("[%i]\n", i + 1);
        else printf("[%i] [%s]\t[%c]  %s\n", i + 1, vTT->visitor[i].category, intent[vTT->visitor[i].offer.intention], vTT->visitor[i].name);
    }

    int select_visitor = get_number_input(49, 49 + vTT->amount);
    
    if (vTT->visitor[select_visitor].active == 0)
    {
        return;
    }

    display_offer(vTT->visitor[select_visitor]);

    struct offer offer = vTT->visitor[select_visitor].offer;

    int tradeNum = get_number_input(48, 48 + offer.amount);

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

int get_number_input(int low, int high)
{
    char s = 10;
    while (s == 10) s = getchar();
    
    if (s < low || s > high) return 10;

    return s - low;
}

int display_offer(struct visitor v)
{
    char int_opts[2][5] = {"buy\0", "sell\0"};
    printf("%s would like to %s:\n", v.name, int_opts[v.offer.intention]);
    printf("[0]\tLeave\n");
    for (int i = 0; i < v.offer.amount; i++)
    {
        struct item item = v.offer.items[i];
        printf("[%i]\t$", i + 1);
        for (float j = 1000; j > item.price && j > 1; j /= 10){printf("");}
        printf("%.2f  %ix  %s\n", item.price, item.amount, item_list[item.category][item.item]);
    }

    return 0;
}

int generate_visitor_data(struct visitors* v)
{
    int free_slot;
    for (int i = 0; i < v->max_visitors; i++)
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

    for (int i = 0; i < v->max_visitors; i++)
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
        sleep(visitors->frequency);
        if (visitors->amount < visitors->max_visitors)
        {
            slot = generate_visitor_data(v);
            visitors->amount++;
            printf("%s has arrived! (%i/%i)\n", visitors->visitor[slot].name, visitors->amount, visitors->max_visitors);
        }
    }
    return NULL;
}