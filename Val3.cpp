#include <iostream>
#include <fstream>
#include <string>

using namespace std;

bool DEBUG = false; // Hay que borrar todo lo que sea debug al final.

class Numori {
public:
    int id = 0;
    string name = "";
    string type = "";
    float attack = 0.0;
    float life = 0.0;
};

class Torre {
public:
    int floors = 0;
    int enemies = 0;
    int *floorEnemies = nullptr; 
    Numori *floorMap = nullptr;

    void CopyTower(Torre old_tower) {
        this->enemies = old_tower.enemies;
        this->floors = old_tower.floors;
        this->floorEnemies = old_tower.floorEnemies;
        this->floorMap = new Numori[old_tower.enemies];
    }

    void FreeMemory() {
        delete[] floorMap;
    }
};

//Globales //////////////////////////////////////////////////////////
const int numorisMaximos=6;
int n_numoris = 0;
int encontrado = false;
bool hasFindedSol = false;
Numori *solucion = new Numori[numorisMaximos];
float totalDamage = 0.0;
float current_damage = 0.0;
int deaths = 0;
int current_deaths = 0;
// Fin de globales /////////////////////////////////////////////////

Numori* ReadNumoris(string filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error opening file: " << filename << endl;
        return nullptr;
    }

    string line;

    // tomo el valor de cuantos numoris hay
    file >> line;
    n_numoris = stoi(line);
    Numori* numoris = new Numori[n_numoris];

    // relleno los datos de los numoris
    int i = 0, j = 0;
    while (file >> line) {
        if (i >= 5) { // 5 es el número de parámetros constante de un numori
            i = 0;
            j++;
        }
        switch (i) {
            case 0:
                numoris[j].id = stoi(line);
                break;
            case 1:
                numoris[j].name = line;
                break;
            case 2:
                numoris[j].type = line;
                break;
            case 3:
                numoris[j].attack = stoi(line);
                break;
            case 4:
                numoris[j].life = stoi(line);
                break;
        }
        i++;
    }

    file.close();
    return numoris;
}

Numori SearchNumoriByID(Numori* numorisDB, int id) {
    for (int i = 0; i < n_numoris; i++) {
        if (numorisDB[i].id == id) {
            return numorisDB[i];
        }
    }
    return Numori();
}

int SearchNumoriByIDAndReturnArrPosition(Numori* numorisDB, int id) {
    for (int i = 0; i < n_numoris; i++) {
        if (numorisDB[i].id == id) {
            return id;
        }
    }
    return -1;
}

int SearchNumoriAliveAndReturnArrPositon(Numori* numorisDB, int arrSize, int i = 0){
    while (i < arrSize) {
        if (numorisDB[i].life > 0) {
            return i;
        }
        i++;
    }
    return -1;
}

Torre ReadTower(string filename, Numori* numorisDB) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error opening file: " << filename << endl;
        return Torre(); 
    }
    
    string line;
    file >> line;
    Torre TorreInstance; // asigna el número de pisos a la torre
    TorreInstance.floors = stoi(line);

    // relleno los datos de la torre
    int i = 0;
    while (file >> line) {
        i++; 
    }

    TorreInstance.floorMap = new Numori[i];
    i = 0;

    file.close();
    file.open(filename);
    file >> line; // saltamos primera linea

    while (file >> line) {
        TorreInstance.floorMap[i] = SearchNumoriByID(numorisDB, stoi(line));
        TorreInstance.enemies++;
        i++;
    }

    
    // Ahora guardamos la cantidad de enemigos por piso
    TorreInstance.floorEnemies = new int[TorreInstance.floors];
    file.close(); 
    file.open(filename);
    file >> line; getline(file, line); // saltamos primera linea y vamos directamente a los pisos.

    i = 0; int floorEnemies = 0;

    bool inNumber = false;

    while (getline(file, line)) {
        for (int j = 0; j < line.length(); j++) {
            if (line[j] == ' ') {
                if(inNumber) {
                    inNumber = false;
                }
                continue;
            } else if(!inNumber) {
                floorEnemies++;
                inNumber = true;
            }
        }
        TorreInstance.floorEnemies[i] = floorEnemies;
        i++; floorEnemies = 0; inNumber = false;
    }

    file.close();

    return TorreInstance;
}

void copyArr(Numori *old_arr, Numori *new_arr, int size) {
    for (int i = 0; i < size; i++) {
        new_arr[i] = old_arr[i];
    }
}

float AttackRival(Numori *numoriUser, Numori *numoriRival, int user, int rival) {
    // Tipo Agua
    if(numoriUser[user].type == "Agua") {
        if(numoriRival[rival].type == "Fuego") {    // x2 Daño
            return numoriUser[user].attack * 2;
        } 
        if(numoriRival[rival].type == "Aire") {     // Mitad de daño
            return numoriUser[user].attack / 2;
        }
    } 

    // Tipo Fuego
    if(numoriUser[user].type == "Fuego") {
        if(numoriRival[rival].type == "Tierra") {   // x2 Daño
            return numoriUser[user].attack * 2;
        } 
        if(numoriRival[rival].type == "Agua") {     // Mitad de daño
            return numoriUser[user].attack / 2;
        }
    } 

    // Tipo Tierra
    if(numoriUser[user].type == "Tierra") {
        if(numoriRival[rival].type == "Aire") {     // x2 Daño
            return numoriUser[user].attack * 2;
        }
        if(numoriRival[rival].type == "Fuego") {    // Mitad de daño
            return numoriUser[user].attack / 2;
        }
    }

    // Tipo Aire
    if(numoriUser[user].type == "Aire") {
        if(numoriRival[rival].type == "Agua") {     // x2 Daño
            return numoriUser[user].attack * 2;
        }
        if(numoriRival[rival].type == "Tierra") {   // Mitad de daño
            return numoriUser[user].attack / 2;
        }
    }

    // Daño neutral
    return numoriUser[user].attack;
}

// Obtiene la posicion inicial del arreglo de enemigos en la torre en determinado piso.
int getFloorInit(Torre numoriTower, int piso) {
    int numEnemies = 0;

    if(piso == 0) return 0;
    
    for(int i = 0; i < piso; i++) {
        numEnemies += numoriTower.floorEnemies[i];
    }
    return numEnemies;
}

// Obtiene la posicion final del arreglo de enemigos en la torre en determinado piso.
int getFloorEnd(Torre numoriTower, int piso) {
    return numoriTower.floorEnemies[piso] + getFloorInit(numoriTower, piso);
}

bool Combat(Numori *numoriUser, Torre numoriTower, int piso, bool userAlreadyHaveTurn = false, int SelectedUser = 0, int SelectedRival = 0) 
{
    SelectedUser = SearchNumoriAliveAndReturnArrPositon(numoriUser, numorisMaximos);
    SelectedRival = SearchNumoriAliveAndReturnArrPositon(numoriTower.floorMap, getFloorEnd(numoriTower, piso), getFloorInit(numoriTower, piso)); 

    // Todo equipo rival el rival esta muerto
    if(SelectedRival == -1) {
        return true;
    }

    // Todo nuestro equipo esta muerto
    if(SelectedUser == -1) {
        if (DEBUG) cout << "MORISTE... INTENTANDO OTRA ALTERNATIVA." << endl;
        return false;
    }

    if (DEBUG) cout << "-------------" << endl;

    if(!userAlreadyHaveTurn) { // Turno del Usuario (falso en el primer turno)
        float dmg = AttackRival(numoriUser, numoriTower.floorMap, SelectedUser, SelectedRival);
        float &rivalLife = numoriTower.floorMap[SelectedRival].life;
        rivalLife -= dmg;
        if (DEBUG) {
            cout << "Turno de: USUARIO" << endl;
            cout << "Numori: " << numoriUser[SelectedUser].name << " Vida: " << numoriUser[SelectedUser].life << endl;
            cout << "Ataque: " << dmg << endl;
            cout << "Rival: " << numoriTower.floorMap[SelectedRival].name << " Vida: " << rivalLife << endl;
        }
        userAlreadyHaveTurn = true; 
    } else { // Turno del rival
        float dmg = AttackRival(numoriTower.floorMap, numoriUser, SelectedRival, SelectedUser); 
        float &userLife = numoriUser[SelectedUser].life;

        //parche para q el daño no se pase de la vida del numori
        if (dmg >= userLife) {
            current_damage += userLife;
            current_deaths++;
        } else {
            current_damage += dmg;
        }

        userLife -= dmg;
        if (DEBUG) {
            cout << "Turno de: RIVAL" << endl;
            cout << "Numori: " << numoriTower.floorMap[SelectedRival].name << " Vida: " << numoriTower.floorMap[SelectedRival].life << endl;
            cout << "Ataque: " << dmg << endl;
            cout << "Rival: " << numoriUser[SelectedUser].name << " Vida: " << userLife << endl;
        }
        userAlreadyHaveTurn = false; 
    }

    return Combat(numoriUser, numoriTower, piso, userAlreadyHaveTurn, SelectedUser, SelectedRival);
}

bool TorreLimpiada(Numori* numoris, Torre TorreInstance, int piso = 0, int victorias = 0) {
    if(victorias == TorreInstance.floors) { // ganamos cuando el numero de victorias es igual al numero de pisos
        if(DEBUG) cout << "Torre Limpiada!" << endl; // DEBUG
        return true;
    }

    if(Combat(numoris, TorreInstance, piso)) {
        victorias++;
        piso++;
    } else {
        return false;
    }

    return TorreLimpiada(numoris, TorreInstance, piso, victorias);
}


//// ++FUNCIONES BACKTRACKING +++++++++++++++++++++++++++++++++++++++++ ////


void applyGlobalsSol(Numori *old_arr, Numori *new_arr, int size) {
    copyArr(old_arr, new_arr, size);
    deaths = current_deaths;
    totalDamage = current_damage;
    /*cout << "Mejor Solucion: " << endl;
    for (int i = 0; i < numorisMaximos; i++) {
        cout << new_arr[i].id << " ";
    }
    cout << endl;
    cout << "Total Damage: " << totalDamage << endl;
    cout << "Total Deaths: " << deaths << endl;
    cout << "------------------------" << endl;*/
}
void resetGlobalsSol() {
    current_damage = 0.0;
    current_deaths = 0;
}
bool alternativaValida(Numori* conjunto_solucion=nullptr,int paso=0, Numori alternativa=Numori()) {
    for (int i = 0; i < numorisMaximos ; i++) {
        if (conjunto_solucion[i].id == alternativa.id) {
            return false;
        }
    }
    return paso<numorisMaximos? true : false;
}
void deshacerAlternativa(Numori* conjunto_solucion=nullptr ,int i=0) {
    conjunto_solucion[i] = Numori();
}
void aplicarAlternativa(Numori* conjunto_solucion=nullptr ,Numori alternativa=Numori(), int i=0) {
    conjunto_solucion[i] = alternativa;
}
void esMejorSol(Numori* conjunto_solucion){
    if(hasFindedSol == false) {
        hasFindedSol = true;
        applyGlobalsSol(conjunto_solucion, solucion, numorisMaximos);
        return;
    } 

    if(current_deaths < deaths) {
        applyGlobalsSol(conjunto_solucion, solucion, numorisMaximos);
    }
    if(current_deaths == deaths && current_damage < totalDamage) {
        applyGlobalsSol(conjunto_solucion, solucion, numorisMaximos);
    }
    if(current_damage == totalDamage && current_deaths == deaths) {
        int sumaSolGlobal=0, sumaSolActual=0;
        for(int i = 0; i<numorisMaximos; i++){
            sumaSolGlobal += solucion[i].id;
            sumaSolActual += conjunto_solucion[i].id;
        }
        
        if(sumaSolActual < sumaSolGlobal) {
            applyGlobalsSol(conjunto_solucion, solucion, numorisMaximos);
        } else if (sumaSolActual == sumaSolGlobal) {
            string numeroCompletoSolActual, numeroCompletoMejorSol;
            for (int i = 0; i < numorisMaximos; i++) {
                numeroCompletoSolActual += to_string(conjunto_solucion[i].id);
                numeroCompletoMejorSol += to_string(solucion[i].id);
            }
            if (stoull(numeroCompletoSolActual) < stoull(numeroCompletoMejorSol)) {
                applyGlobalsSol(conjunto_solucion, solucion, numorisMaximos);
            }
        }
    }
    resetGlobalsSol();
}


//// ++ FIN FUNCIONES BACKTRACKING +++++++++++++++++++++++++++++++++++++++++ ////


int backtracking(int paso=0, Numori* numoris=nullptr, Torre TorreInstance=Torre(),Numori* conjunto_solucion=nullptr) {
    Torre TempTorre;                            // optimizacion
    TempTorre.CopyTower(TorreInstance);         // optimizacion
    Numori TempNumoris[numorisMaximos];         // optimizacion

    int i = 0;
    while (i < n_numoris)
    {
        if (alternativaValida(conjunto_solucion ,paso,numoris[i]))
        {
            aplicarAlternativa(conjunto_solucion,numoris[i], paso);

            if (paso == numorisMaximos-1) {
                copyArr(TorreInstance.floorMap, TempTorre.floorMap, TorreInstance.enemies); 
                copyArr(conjunto_solucion, TempNumoris, numorisMaximos);
            }
            if (TorreLimpiada(TempNumoris, TempTorre)&&paso == numorisMaximos-1) {
                    /*for(int i = 0; i < numorisMaximos; i++) {
                        cout << TempNumoris[i].id << " "; 
                    }
                    cout << "CD:" << current_damage << " CT:" << current_deaths << endl;*/
                esMejorSol(TempNumoris);
            } else {
                backtracking(paso + 1, numoris, TorreInstance, conjunto_solucion);
            }
            deshacerAlternativa(conjunto_solucion,paso);
        }
        i++;
    }
    TempTorre.FreeMemory(); //optimizacion
    return 0;
}

int main() {
    string towerNumber;
    cin >> towerNumber;
    string NumorisData = "NumorisDB.in";
    string torre = "Torre" + towerNumber + ".in";
    Numori* numoris = ReadNumoris(NumorisData);
    Torre TorreInstance = ReadTower(torre, numoris);
    Numori conjunto_solucion[numorisMaximos];

    backtracking(0, numoris, TorreInstance, conjunto_solucion);

    for (int i = 0; i < numorisMaximos; i++) {
        cout << solucion[i].id << " ";
    }
    
    return 0;
}