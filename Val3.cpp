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
    bool muerto=false; // esto lo podriamos borrar, realmente no se usa en la implementacion actual
};

class Torre {
public:
    int floors = 0;
    int enemies = 0;
    int *floorEnemies = nullptr; 
    Numori *floorMap = nullptr;
    
    // DEBUG
    void print() {
        cout << "Torre: " << floors << " pisos" << endl;
        cout << "Enemigos en la torre (ID): "; 
        for (int i = 0; i < enemies; i++) {
            cout << floorMap[i].name << ", ";
        }
        cout << endl;
    }

    void CopyTower(Torre old_tower) {
        this->enemies = old_tower.enemies;
        this->floors = old_tower.floors;
        this->floorEnemies = old_tower.floorEnemies;
        this->floorMap = new Numori[old_tower.enemies];
    }

    void printFloorEnemies() {
        cout << "Enemigos por piso: ";
        for (int i = 0; i < floors; i++) {
            cout << floorEnemies[i] << " ";
        }
        cout << endl;
    }
};

//Globales //////////////////////////////////////////////////////////
int n_numoris = 0;
int encontrado = false;
bool hasFindedSol = false;
Numori *solucion = new Numori[6];
// Fin de globales /////////////////////////////////////////////////

// Prototipos de funciones
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
    Numori* numoris = new Numori[n_numoris]; // recuerda liberar memoria cuando ya no sea necesario

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
        return Torre(); // probablemente haya que hacer algo aca
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
    while (getline(file, line)) {
        for (int j = 0; j < line.length(); j++) {
            if (line[j] == ' ') {
                continue;
            } else {
                floorEnemies++;
            }
        }
        TorreInstance.floorEnemies[i] = floorEnemies;
        i++; floorEnemies = 0;
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
// HAY QUE TESTEAR, deberia estar bien.
int getFloorEnd(Torre numoriTower, int piso) {
    return numoriTower.floorEnemies[piso] + getFloorInit(numoriTower, piso);
}

bool Combat(Numori *numoriUser, Torre numoriTower, int piso, bool userAlreadyHaveTurn = false, int SelectedUser = 0, int SelectedRival = 0) 
{
    SelectedUser = SearchNumoriAliveAndReturnArrPositon(numoriUser, 6);
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

// solo pasale el numori y la torre, no hace falta el piso ni victorias
// esto tambien es recursivo. No se si era 100% necesario que fuera asi, pero lo vi mas sencillo hacerlo asi que de otra manera
// te va a retornar true si la torre fue limpiada (ganamos todos los combates), false si perdemos uno solo.
bool TorreLimpiada(Numori* numoris, Torre TorreInstance, int piso = 0, int victorias = 0) {
    if(victorias == TorreInstance.floors) { // ganamos cuando el numero de victorias es igual al numero de pisos
        if(DEBUG) cout << "Torre Limpiada!" << endl; // DEBUG
        return true;
    }

    if (DEBUG) cout << "=======================\n" << "PISO " << piso << endl << "=======================\n";

    if(Combat(numoris, TorreInstance, piso)) {
        victorias++;
        piso++;
    } else {
        return false;
    }

    return TorreLimpiada(numoris, TorreInstance, piso, victorias);
}

bool alternativaValida(Numori* conjunto_solucion=nullptr,int paso=0, Numori alternativa=Numori()) {
    for (int i = 0; i < n_numoris; i++) {
        if (conjunto_solucion[i].id == alternativa.id) {
            return false;
        }
    }
    return paso<n_numoris? true : false;

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
        copyArr(conjunto_solucion, solucion, 6);
    } 

    int sumaSolGlobal=0, sumaSolActual=0;
    for(int i = 0; i<6; i++){
        sumaSolGlobal= sumaSolGlobal + solucion[i].id;
        sumaSolActual= sumaSolActual + conjunto_solucion[i].id;
    }
    if(sumaSolActual<sumaSolGlobal){
        copyArr(conjunto_solucion, solucion, 6);
    } else if (sumaSolActual == sumaSolGlobal) {
        string numeroCompletoSolActual, numeroCompletoMejorSol;
        for (int i = 0; i < 6; i++) {
            numeroCompletoSolActual += to_string(conjunto_solucion[i].id);//sugeri el *10 pero jose se metio el deo
            numeroCompletoMejorSol += to_string(solucion[i].id);
        }

        if (stoi(numeroCompletoSolActual) < stoi(numeroCompletoMejorSol)) {
            copyArr(conjunto_solucion, solucion, 6);
        }
    }
}

int backtracking(int paso=0, Numori* numoris=nullptr, Torre TorreInstance=Torre(),Numori* conjunto_solucion=nullptr) {
    int i = 0;
    Torre TempTorre;
    TempTorre.CopyTower(TorreInstance);
    copyArr(TorreInstance.floorMap, TempTorre.floorMap, TorreInstance.enemies);
    while (i < n_numoris)
    {
        if (alternativaValida(conjunto_solucion ,paso,numoris[i]))
        {
            aplicarAlternativa(conjunto_solucion,numoris[i], paso);
            if (paso == n_numoris - 1 && TorreLimpiada(conjunto_solucion, TempTorre)) {
                esMejorSol(conjunto_solucion);
            } else {
                backtracking(paso + 1, numoris, TorreInstance, conjunto_solucion);
            }
            deshacerAlternativa(conjunto_solucion,paso);
        }
        i++;
    }
    return 0;
}

int main() {
    string towerNumber;
    cin >> towerNumber;
    string NumorisData = "NumorisDB.in";
    string torre = "Torre" + towerNumber + ".in";
    Numori* numoris = ReadNumoris(NumorisData);
    Torre TorreInstance = ReadTower(torre, numoris);
    Numori conjunto_solucion[6];

    //TorreInstance.printFloorEnemies();
    backtracking(0, numoris, TorreInstance, conjunto_solucion);

    for (int i = 0; i < 6; i++) {
        cout << solucion[i].id << " ";
    }
    
    return 0;
}