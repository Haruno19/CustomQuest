#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <filesystem>
#include <time.h>
#include "include/json.hpp"

using namespace std;
using json = nlohmann::json;

struct Move
{
    string name;
    int basePower, mpCost, hpCost, defBuf, atkBuf, magBuf;
};

struct Items
{
    string name, desc;
    int hpHeal, mpHeal, defBuf, atkBuf, magBuf, quantity;
};

struct Inventory
{
    Items items[4];

    void InitInventory(istream& f)
    {   
        json _inventory;
        f >> _inventory;

        for(int i=0;i<4;i++)
        {
            items[i].name=_inventory[to_string(i)]["name"];
            items[i].mpHeal=_inventory[to_string(i)]["MP Heal"];
            items[i].hpHeal=_inventory[to_string(i)]["HP Heal"];
            items[i].defBuf=_inventory[to_string(i)]["DEF Buf"];
            items[i].atkBuf=_inventory[to_string(i)]["ATK Buf"];
            items[i].magBuf=_inventory[to_string(i)]["MAG Buf"];
            items[i].desc=_inventory[to_string(i)]["desc"];
            items[i].quantity=_inventory[to_string(i)]["quantity"];
        }
    }

    void UseItem(int item);
};

struct Entity
{
    int mp, hp, atk, def, mag, maxHp, maxMp, atkBuf, defBuf, magBuf;
    Move moves[4];
    string name;

    void InitEntity(istream& f)
    {
        json _ent;
        f >> _ent;

        name = _ent["name"];
        mp = _ent["mp"];
        hp = _ent["hp"];
        maxMp = _ent["mp"];
        maxHp = _ent["hp"];
        atk = _ent["atk"];
        def = _ent["def"];
        mag = _ent["mag"];
        atkBuf=0;
        defBuf=0;
        magBuf=0;

        for(int i=0;i<4;i++)
        {
            moves[i].name=_ent["moves"][to_string(i)]["name"];
            moves[i].basePower=_ent["moves"][to_string(i)]["Base Power"];
            moves[i].mpCost=_ent["moves"][to_string(i)]["MP Cost"];
            moves[i].hpCost=_ent["moves"][to_string(i)]["HP Cost"];
            moves[i].defBuf=_ent["moves"][to_string(i)]["DEF Buf"];
            moves[i].atkBuf=_ent["moves"][to_string(i)]["ATK Buf"];
            moves[i].magBuf=_ent["moves"][to_string(i)]["MAG Buf"];
        }
    }

    void NormalAttack(Entity *target);

    void UseMove(int move, Entity *target);

    bool Critical()
    {
        return (rand()%100 < 10);
    }

    void GetDamage(int damage)
    {
        hp -= damage;
    }

    void UpdateStats(int _atk, int _def, int _mag)
    {
        /*
            st  b   _b
            4   0   8
            12  8   
            12  8   4
            4   8   4
            4   12
            16  
        */
        atk -= atkBuf;  //preventing stacking buffs from moves and items
        def -= defBuf;  //if a buffing move and a buffing item are used consequentially, the first applied
        mag -= magBuf;  //buff never gets removed
        atkBuf += _atk;
        defBuf += _def;
        magBuf += _mag;
        atk += atkBuf;
        def += defBuf;
        mag += magBuf;
    }

    bool CheckKO()
    {
        return (hp<=0);
    }

    void ResetStats()
    {
        atk -= atkBuf;
        def -= defBuf;
        mag -= magBuf;
        atkBuf=0;
        defBuf=0;
        magBuf=0;
    }

    void Heal(int _hp, int _mp)
    {
        hp+=_hp;
        if(hp > maxHp) hp = maxHp;
        mp+=_mp;
        if(mp > maxMp) mp = maxMp;
    }
};

int getch(void)
{
	struct termios oldt, newt;
	int ch;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

	return ch;
}

void InitEntities(string*);
void PrintScreen();
int ManageInput(int);
void MainLoop();
void MainMenu();
void PrintOptions(vector<string>, int);
vector<string> GetJSON(string);
void AssignOptions(int);
int NextMode(int, char);
void EnemyTurn();
void UpdateDialog(int);
void NotEnough(string);
void Winner(string);
void HomeScreen();

string screen, art;
Entity enemy, hero;
Inventory inventory;
int cursor = 1;
string dialogs[2] = {"line1", "line2"};
string options[4] = {"Fight", "Moves", "Items", "Run"};

string path = "/usr/local/share/customquest/";

int main()
{
    srand(time(NULL));

    HomeScreen();

    cout << "\033[H\033[2J\033[0m\033[?25h";
    return 0;
}

void MainMenu()
{
    char c;
    vector<string> elements;    //contains the availables enemies, heroes or items
    string types[3] = {"enemies", "heroes", "items"};

    for(int i=0;i<3;i++)    //for all the 3 categories, it makes the user select one
    {
        cursor=1;
        elements=GetJSON(types[i]); //gets the elements json files name
        do
        {   
            PrintOptions(elements, i);
            c=getch();   
            if(c == 27) //intercepts the arrow keys and moves the cursor accordingly
            {
                getch();
                switch(getch())
                {
                    case 65:
                        if (cursor-- <= 1) cursor = 1;
                        break;
                    case 66:
                        if (cursor++ >= elements.size()) cursor = elements.size();
                }
            }
        } while (c != 10);  //once the user has selected the option
        types[i] = elements[cursor-1];  //it saves the name of the selected file
    }
    InitEntities(types);
    MainLoop();
}

void PrintOptions(vector<string> elements, int type)
{
    size_t pos;
    string s;

    cout << "\033[H\033[2J" << endl << "   Select ";
    switch(type)
    {
        case 0:
            cout << "an enemy:";
            break;
        case 1:
            cout << "a hero:";
            break;
        case 2:
            cout << "an inventory:";
    }
    cout << endl << "  ╭────────────────────────────╮" << endl;
    for (int k=0; k<elements.size();k++)
    {
        s = elements[k];
        for(int i=0;s.length()<25;i++) s+= " ";
        if(s != ".DS_Store")
        {
            s+="│";
            if(k+1 == cursor)
                cout << "  │ ► " << s << endl;
            else
                cout << "  │   " << s << endl;
        }
    }  
    cout << "  ╰────────────────────────────╯";
}

vector<string> GetJSON(string type)
{
    size_t pos;
    vector<string> elements; 
    string s;
    for (const auto & entry : filesystem::directory_iterator(path+type+"/"))
    {
        s = entry.path().u8string().substr(entry.path().u8string().find(type+"/")+type.length()+1); //removing /usr/local/share/customquest/res/type/
        s = s.substr(0, s.length()-5);
        elements.push_back(s);
    }

    return elements;
}

void InitEntities(string* entities)
{
    //initiates the entities and the needed files
    ifstream f;

    f.open(path+"enemies/"+entities[0]+".json");
    enemy.InitEntity(f);
    f.close();

    f.open(path+"heroes/"+entities[1]+".json");
    hero.InitEntity(f);
    f.close();

    f.open(path+"items/"+entities[2]+".json");
    inventory.InitInventory(f);
    f.close();

    ifstream t(path+"template");
    ifstream a;
    if(filesystem::exists(path+"arts/"+entities[0]+".txt"))
        a.open(path+"arts/"+entities[0]+".txt");
    else
        a.open(path+"arts/slime.txt");
    stringstream buffer;
    buffer << t.rdbuf(); t.close();
    screen = buffer.str();
    buffer.str("");
    buffer << a.rdbuf(); a.close();
    art = buffer.str();

    cursor = 1;
}

void PrintScreen()
{
    string _art = art;
    string _screen = screen; 

    string dummy = "";
    //bar : 42 = hp : maxhp
    //bar = 42*hp / maxhp
    for(int i=0;i<47;i++)
        if(i<47*enemy.hp/enemy.maxHp)
            dummy += "▆";
        else
            dummy += " ";
    _screen.replace(_screen.find("%1"), string("%0").size(), dummy);

    _screen.replace(_screen.find("%2"), string("%0").size(), _art);

    dummy.clear();
    for(int i=0;i<18;i++)
        if(i<18*hero.hp / hero.maxHp)
            dummy += "▆";
        else
            dummy += " ";
    _screen.replace(_screen.find("%4"), string("%0").size(), dummy);

    dummy.clear();
    for(int i=0;i<18;i++)
        if(i<18*hero.mp / hero.maxMp)
            dummy += "▆";
        else
            dummy += " ";
    _screen.replace(_screen.find("%5"), string("%0").size(), dummy);

    for(int i=0;i<4;i++)  //the cursors has 4 possibile positions, 1 (top left) 2 (bottom left) 3 (top right) 4 (bottom right)
        if(cursor-1==i)
            _screen.replace(_screen.find("%"+to_string(i+6)), string("%0").size(), "►");
        else
            _screen.replace(_screen.find("%"+to_string(i+6)), string("%0").size(), " ");

    int diag1lim = (dialogs[1].find("▼")!= string::npos) ? 53 : 51; //if dialog[1] contains '▼', its length is 53
    for(int i=0;dialogs[0].length()<51;i++) dialogs[0] += " ";
    for(int i=0;dialogs[1].length()<diag1lim;i++) dialogs[1] += " ";
    _screen.replace(_screen.find("%10"), string("%10").size(), dialogs[0].substr(0, 51));
    _screen.replace(_screen.find("%11"), string("%10").size(), dialogs[1].substr(0, diag1lim));

    for(int i=0;i<4;i++)
    {
        for(int k=0; options[i].length() < 9;k++) options[i]+=" ";
        options[i] = options[i].substr(0,9);
        if(options[i][8]!=' ') options[i][8]='.';
        _screen.replace(_screen.find("%"+to_string(i+12)), string("%10").size(), options[i]);
    }

    //system("clear");
    cout << "\033[H\033[2J" << _screen;
}

void MainLoop()
{
    int mode=1; //1 main menu, 2 moves, 3 items, 4 run

    do
    {
        AssignOptions(mode);
        mode = ManageInput(mode);
    }while(mode != 4);
    dialogs[0] = hero.name+" runs away!";
    dialogs[1] = "                                                  ▼";
    PrintScreen();
    getch();
    HomeScreen();
}

int ManageInput(int mode)
{
    char c;
    do
    {   
        UpdateDialog(mode);
            
        PrintScreen();
        c=getch();   
        if(c == 27)
        {
            getch();
            switch(getch())
            {   //1 (top left) 2 (bottom left) 3 (top right) 4 (bottom right)
                case 65:
                    if(cursor == 2) cursor = 1;
                    else if(cursor == 4) cursor = 3;
                    break;
                case 66:
                    if(cursor == 1) cursor = 2;
                    else if(cursor == 3) cursor = 4;
                    break;
                case 67:
                    if(cursor == 1) cursor = 3;
                    else if(cursor == 2) cursor = 4;
                    break;
                case 68:
                    if(cursor == 3) cursor = 1;
                    else if(cursor == 4) cursor = 2;
            }
        }
    } while (c != 10 && c!='b');
    return NextMode(mode, c);
}

void UpdateDialog(int mode)
{
    if(mode==2)
    {
        if(hero.moves[cursor-1].name!="")
        {
            dialogs[0] = hero.moves[cursor-1].name;
            dialogs[1] = "MP Cost: "+to_string(hero.moves[cursor-1].mpCost)+"  HP Cost: "+to_string(hero.moves[cursor-1].hpCost);
        }else{
            dialogs[0] = "Select a move.";
            dialogs[1] = "";
        }
    }else if(mode==3){
        if(inventory.items[cursor-1].name!="")
        {
            dialogs[0] = inventory.items[cursor-1].name+" ["+to_string(inventory.items[cursor-1].quantity)+"] ";
            dialogs[1] = inventory.items[cursor-1].desc;
        }else{
            dialogs[0] = "Select an item.";
            dialogs[1] = "";
        }
    }else{
        dialogs[0] = hero.name+"'s turn!";
        dialogs[1] = "HP: "+to_string(hero.hp)+"  MP: "+to_string(hero.mp);
    }
}

void AssignOptions(int mode)
{
    switch(mode)
    {
        case 1:
            options[0] = "Fight";
            options[1] = "Moves";
            options[2] = "Items";
            options[3] = "Run";
            dialogs[0] = hero.name+"'s turn!";
            break;
        case 2:
            for(int i=0;i<4;i++)
                options[i] = hero.moves[i].name;
            dialogs[0] = "Select a move.";
            break;
        case 3:
            for(int i=0;i<4;i++)
                options[i] = inventory.items[i].name;
            dialogs[0] = "Select an item.";
            break;
    }
}

int NextMode(int mode, char c)
{
    switch(mode)
    {
        case 1: //main menu 
            if(c!='b')
            {
                if(cursor == 1) //if user selected attack (mode 1 cursor 1)
                {
                    hero.NormalAttack(&enemy); //base attack;
                    EnemyTurn();
                }
                mode = cursor;
                cursor = 1;
            }
            break;
        case 2: //moves
            if(c!='b' && hero.moves[cursor-1].name!="")  //if user choose a move
            {
                if(hero.mp >= hero.moves[cursor-1].mpCost)  //if enough mp
                {
                    if(hero.hp >= hero.moves[cursor-1].hpCost)  //if enough hp
                    {
                        hero.UseMove(cursor-1, &enemy); //performs the move
                        EnemyTurn();    //gives the turn to the enemy
                        mode = 1;       //goes back to the main fight menu
                    }else
                        NotEnough("HP");
                }else
                    NotEnough("MP");
            }else if(c=='b'){   //if the users goes back to the previous menu
                mode = 1;   //goes back to the main fight menu
                cursor=1;   //reset the cursor
            }  
            break;
        case 3: //items
            if(c!='b' && inventory.items[cursor-1].name!="")  //if user chosse an item
            {
                if(inventory.items[cursor-1].quantity > 0)
                {
                    inventory.UseItem(cursor-1);
                    EnemyTurn();
                    mode = 1;
                }else
                    NotEnough("item");
            }else if(c=='b'){
                mode = 1;
                cursor=1;
            }
    }
    return mode;
}

void NotEnough(string type)
{
    dialogs[0]="Not enough "+type+"s!";
    dialogs[1] = "                                                  ▼";
    PrintScreen();
    getch();
}

void EnemyTurn()
{
    dialogs[0] = "Enemy's turn!";
    dialogs[1] = "                                                  ▼";
    PrintScreen();
    getch();

    int prob = rand()%100, k=0;
    //lower the hp, lower the chance of using a normal attack
    //maxHp:30 = hp:chance
    if(prob < enemy.hp/30*enemy.maxHp)   //chanche of normal attack
        enemy.NormalAttack(&hero);
    else
    {
        prob = rand()%4-1;
        do
        {   
            k++;
            if(++prob > 3) prob=0;
        } while (enemy.moves[prob].name=="" && k<=4);   //tires different moves at most 4 times, then if all the moves are "" (the enemy has no moves) it performs basic attac
        if(k<=4 && enemy.mp >= enemy.moves[prob].mpCost && enemy.hp >= enemy.moves[prob].hpCost)    
            enemy.UseMove(prob, &hero); //enemy performs the move only if it has enogh mps and hps
        else    //othervies it does a normal attack
            enemy.NormalAttack(&hero);
    }
    cursor = 1;
}

void Winner(string name)
{
    dialogs[0] = name+" wins!";
    dialogs[1] = "                                                  ▼";
    PrintScreen();
    getch();
    HomeScreen();
}

void HomeScreen()
{
    cout << "\033[?25l";    //hide cursor

    ifstream h(path+"homescreen");
    stringstream buffer;
    buffer << h.rdbuf(); h.close();
    string home;
    char c;

    cursor = 1;
    do
    {   
        home = buffer.str();
        for(int i=0;i<2;i++)
            if(i==cursor-1)
                home.replace(home.find("%"+to_string(i)), string("%0").size(), "►");
            else
                home.replace(home.find("%"+to_string(i)), string("%0").size(), " ");
        cout << "\033[H\033[2J" << home;
        c=getch();   
        if(c == 27) //intercepts the arrow keys and moves the cursor accordingly
        {
            getch();
            switch(getch())
            {
                case 65:
                    if (cursor-- <= 1) cursor = 1;
                    break;
                case 66:
                    if (cursor++ >= 2) cursor = 2;
            }
        }
    } while (c != 10);  //once the user has selected the option

    if(cursor==1)
        MainMenu();
    else
        cout << "\033[H\033[2J\033[?25h";
    exit(0);
}

/******* ENTITY FUCTIONS ********/

void Entity::NormalAttack(Entity *target)
{
    int crit = (Critical()) ? 2 : 1;
    dialogs[0] = name+" attacks!";
    dialogs[1] = (crit==2) ? "A critical hit!                                   ▼" : "                                                  ▼";
    target->GetDamage((atk/3 + (atk * (atk - target->def)/200)) * crit);
    ResetStats();
    PrintScreen();
    getch();
    if(target->CheckKO())
        Winner(name);
}

void Entity::UseMove(int move, Entity *target)
{
    int crit = (Critical()) ? 2 : 1;
    dialogs[0] = name+" uses "+moves[move].name+"!";
    dialogs[1] = (crit==2) ? "A critical hit!                                   ▼" : "                                                  ▼";
    target->GetDamage((moves[move].basePower/3 + (moves[move].basePower * (mag - target->def)/200)) * crit);
    //hp-=moves[move].hpCost;
    //mp-=moves[move].mpCost;
    Heal(-moves[move].hpCost, -moves[move].mpCost); //this way, using the heal function to recalculate HPs and MPs, a move can also be curative
    UpdateStats(moves[move].atkBuf, moves[move].defBuf, moves[move].magBuf);
    
    
    ResetStats();
    PrintScreen();
    getch();
    if(CheckKO())
        Winner(target->name);
    if(target->CheckKO())
        Winner(name);
}

void Inventory::UseItem(int item)
{
    dialogs[0] = inventory.items[item].name+" used!";
    dialogs[1] = "                                                  ▼";
    inventory.items[item].quantity--;
    hero.Heal(inventory.items[item].hpHeal, inventory.items[item].mpHeal);
    hero.UpdateStats(inventory.items[item].atkBuf, inventory.items[item].defBuf,inventory.items[item].magBuf);
    PrintScreen();
    getch();
}