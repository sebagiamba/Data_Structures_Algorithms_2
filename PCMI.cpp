#include <iostream>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <set>
#include <map>
#include <list>

using namespace std;

int N, MG, MH;

vector<list<int>> G;
vector<list<int>> H;

using TColoreo = vector<int>;
using TImpacto = int;

struct TResult {
    double tiempo = 0;
    TImpacto impacto = 0;
    TColoreo coloreo = {};
};

template<typename T>
bool perteneceALista(T elemento, list<T> lista) {
    for (T i : lista)
        if (i == elemento)
            return true;

    return false;
}

int calcularImpacto(TColoreo coloreo) {
    int impacto = 0;
    vector<bool> recorridos = {};
    recorridos.assign(N, false);

    list<int> adyacentes;
    for (int i = 0; i < N; i++) {
        adyacentes = H[i];
        recorridos[i] = true;
        for (int adyacente : adyacentes)
            if (!recorridos[adyacente] && coloreo[adyacente] == coloreo[i]) {
                impacto++;
            }
    }

    return impacto;
}

bool esFactible(TColoreo coloreo) {
    list<int> adyacentes;

    for (int i = 0; i < N; i++) {
        adyacentes = G[i];
        for (int adyacente : adyacentes) {
            if (coloreo[adyacente] == coloreo[i])
                return false;
        }
    }

    return true;
}

bool colorEsFactible(TColoreo coloreo, int nodo, int color) {
    list<int> adyacentes;
    adyacentes = G[nodo];
    for (int adyacente : adyacentes) {
        if (coloreo[adyacente] == color)
            return false;
    }
    return true;
}

list<int> coloresPosibles(TColoreo coloreo) {
    list<int> colores = {};
    for (int i = 0; i < N; ++i) {
        if (!perteneceALista(coloreo[i], colores)) {
            colores.push_back(coloreo[i]);
        }
    }
    return colores;
}

list<TColoreo> obtenerVecinos(TColoreo coloreo, int nodo, list<int> gamaDeColores) {
    list<TColoreo> vecinos = {};
    TColoreo coloreoVecino = coloreo;

    for (auto color : gamaDeColores) {
        if (colorEsFactible(coloreo, nodo, color)) {
            coloreoVecino = coloreo;
            coloreoVecino[nodo] = color;
            vecinos.push_back(coloreoVecino);
        }
    }
    return vecinos;
}

pair<int, int> calcularParCambiado(TColoreo coloreo, TColoreo coloreoVecino) {
    for (int i = 0; i < N; ++i)
        if (coloreo[i] != coloreoVecino[i]) return make_pair(i, coloreo[i]);
    return make_pair(-1, -1);
}

TResult H1() {
    auto start = chrono::steady_clock::now();

    vector<int> coloreo;
    coloreo.assign(N, -1);

    list<int> coloresUsados = {};

    // Recorremos los nodos.
    for (int i = 0; i < N; i++) {
        for (int color : coloresUsados) {
            bool factible = true;

            for (int adyacente : G[i])
                if (coloreo[adyacente] == color) {
                    factible = false;
                    break;
                }

            if (factible) {
                coloreo[i] = color;
                break;
            }
        }

        if (coloreo[i] == -1) {
            coloreo[i] = i;
            coloresUsados.push_back(i);
        }
    }

    auto end = chrono::steady_clock::now();
    double total_time = chrono::duration<double, milli>(end - start).count();

    return {total_time, calcularImpacto(coloreo), coloreo};
}

TResult H2() {
    auto start = chrono::steady_clock::now();

    vector<int> coloreo;
    coloreo.assign(N, -1);

    // Recorremos los nodos.
    for (int i = 0; i < N; i++) {
        int color = i;

        if (coloreo[i] == -1)
            coloreo[i] = color;
        else {
            for (int adyacente : G[i]) {
                if (coloreo[adyacente] == coloreo[i])
                    coloreo[i] = color;
            }
        }

        // Chequeamos los adyacentes de i en H.
        if (H[i].size() != 0)
            // Coloreamos
            for (int adyacente : H[i]) {
                if (!perteneceALista(adyacente, G[i]) && coloreo[adyacente] == -1) {
                    coloreo[adyacente] = coloreo[i];
                }
            }
    }

    auto end = chrono::steady_clock::now();
    double total_time = chrono::duration<double, milli>(end - start).count();

    return {total_time, calcularImpacto(coloreo), coloreo};
}

// Tabú A
TResult TABU3(TResult MEJOR_RESULTADO, int CANT_ITERACIONES, int TAM_MEMORIA, int PORCENTAJE_VECINDAD) {
    auto start = chrono::steady_clock::now();

    list<int> gamaDeColores = coloresPosibles(MEJOR_RESULTADO.coloreo);
    list<pair<int, int>> memoria = {};
    TColoreo coloreo = MEJOR_RESULTADO.coloreo;
    int i = 0;

    while (i < CANT_ITERACIONES) {
        int mejorImpacto = 0;
        TColoreo mejorVecino;

        int cant_nodos_subvecindario = int(N * PORCENTAJE_VECINDAD / 100);
        list<int> nodos_subvecindario = {};

        for (int l = 0; l < cant_nodos_subvecindario; ++l) {
            int nodo;
            do {
                nodo = rand() % N;
            } while (perteneceALista(nodo, nodos_subvecindario));
            nodos_subvecindario.push_back(nodo);
        }

        for (int nodo : nodos_subvecindario) {
            list<int> gamaDeColoresNodo = gamaDeColores;

            for (int color : gamaDeColores)
                if (perteneceALista(make_pair(nodo, color), memoria))
                    gamaDeColoresNodo.remove(color);

            list<TColoreo> vecinos = obtenerVecinos(coloreo, nodo, gamaDeColoresNodo);

            for (auto vecino : vecinos) {
                int impactoVecino = calcularImpacto(vecino);

                if (impactoVecino >= mejorImpacto) {
                    mejorVecino = vecino;
                    mejorImpacto = impactoVecino;
                }

                if (mejorImpacto > MEJOR_RESULTADO.impacto) {
                    MEJOR_RESULTADO.impacto = mejorImpacto;
                    MEJOR_RESULTADO.coloreo = mejorVecino;
                }

                // Calculo el par <nodo,color> cambiado entre el coloreo anterior y el nuevo coloreo.
                pair<int, int> cambio = calcularParCambiado(coloreo, mejorVecino);

                if (TAM_MEMORIA > 0) {
                    if (memoria.size() == TAM_MEMORIA)
                        memoria.pop_front();

                    memoria.push_back(cambio);
                }

                coloreo = mejorVecino;
            }
        }

        i++;
    }

    auto end = chrono::steady_clock::now();
    double total_time = chrono::duration<double, milli>(end - start).count();
    MEJOR_RESULTADO.tiempo = total_time;

    clog << "Cantidad de iteraciones: " << i << endl;
    clog << "Tamanio de memoria utilizada: " << memoria.size() << endl;

    return MEJOR_RESULTADO;
}

// Tabú B
TResult TABU4(TResult MEJOR_RESULTADO, int CANT_ITERACIONES, int TAM_MEMORIA, int PORCENTAJE_VECINDAD) {
    auto start = chrono::steady_clock::now();

    list<int> gamaDeColores = coloresPosibles(MEJOR_RESULTADO.coloreo);
    list<TColoreo> memoria = {};
    TColoreo coloreo = MEJOR_RESULTADO.coloreo;
    int i = 0;
    int _tabu = 0;
    int _vecinos = 0;

    while (i < CANT_ITERACIONES) {
        int cant_nodos_subvecindario = int(N * PORCENTAJE_VECINDAD / 100);
        list<int> nodos_subvecindario = {};

        for (int l = 0; l < cant_nodos_subvecindario; ++l) {
            int nodo;

            do {
                nodo = rand() % N;
            } while (perteneceALista(nodo, nodos_subvecindario));

            nodos_subvecindario.push_back(nodo);
        }

        int mejorImpacto = 0;
        TColoreo mejorVecino;

        for (int nodo : nodos_subvecindario) {
            list<TColoreo> vecinos = obtenerVecinos(coloreo, nodo, gamaDeColores);

            for (auto vecino : vecinos) {
                _vecinos++;
                if (perteneceALista(vecino, memoria)) {
                    _tabu++;
                    continue;
                }

                int impactoVecino = calcularImpacto(vecino);

                if (impactoVecino >= mejorImpacto) {
                    mejorVecino = vecino;
                    mejorImpacto = impactoVecino;
                }

                // Si encontré un mejor global me lo guardo.
                if (mejorImpacto > MEJOR_RESULTADO.impacto) {
                    MEJOR_RESULTADO.impacto = mejorImpacto;
                    MEJOR_RESULTADO.coloreo = mejorVecino;
                }

                // Me muevo a..
                coloreo = mejorVecino;
            }
        }

        // Pusheo el mejor vecino. Si la memoria se agotó la trato como una FIFO.
        if (!perteneceALista(mejorVecino, memoria)) {
            if (TAM_MEMORIA > 0) {
                if (memoria.size() == TAM_MEMORIA)
                    memoria.pop_front();
                memoria.push_back(mejorVecino);
            }
        }

        i++;
    }

    auto end = chrono::steady_clock::now();
    double total_time = chrono::duration<double, milli>(end - start).count();
    MEJOR_RESULTADO.tiempo = total_time;

    clog << "Cantidad de iteraciones: " << i << endl;
    clog << "Tamanio de memoria utilizada: " << memoria.size() << endl;
    clog << "Soluciones Tabú: " << _tabu << endl;
    clog << "Total vecinos probados: " << _vecinos << endl;

    return MEJOR_RESULTADO;
}

int main(int argc, char **argv) {
    // Leemos el parametro que indica el algoritmo a ejecutar.
    map<string, string> algoritmos_implementados = {
            {"H1",      "Heurística 1"},
            {"H2",      "Heurística 2"},
            {"TABU3",   "Tabú A"},
            {"TABU4",   "Tabú B"},
    };

    // Verificamos que el algoritmo pedido exista.
    if (argc < 2 || algoritmos_implementados.find(argv[1]) == algoritmos_implementados.end()) {
        cerr << "Algoritmo no encontrado: " << argv[1] << endl;
        cerr << "Los algoritmos existentes son: " << endl;
        for (auto &alg_desc: algoritmos_implementados)
            cerr << "\t- " << alg_desc.first << ": " << alg_desc.second << endl;
        return 0;
    }

    string algoritmo = argv[1];

    // Verificamos que la heurística inicial exista.
    string ALGORITMO_INICIAL = argv[2];
    if (!(ALGORITMO_INICIAL == "H1" || ALGORITMO_INICIAL == "H2")) {
        cerr << "Algoritmo inicial debe ser H1 o H2, no " << argv[2] << endl;
        return 0;
    }

    int CANT_ITERACIONES = stoi(argv[3]);
    int TAM_MEMORIA = stoi(argv[4]);
    int PORCENTAJE_VECINDAD = stoi(argv[5]);

    // Leer la entrada y armar la lista de adyacencias.
    cin >> N >> MG >> MH;
    clog << N << " - " << MG << " - " << MH;

    // Grafo G
    G.assign(N, {});
    int nodo1, nodo2;
    for (int i = 0; i < MG; i++) {
        cin >> nodo1 >> nodo2;
        nodo1--;
        nodo2--;

        G[nodo1].push_back(nodo2);
        G[nodo2].push_back(nodo1);
    }

    // Grafo H
    H.assign(N, {});
    for (int i = 0; i < MH; i++) {
        cin >> nodo1 >> nodo2;
        nodo1--;
        nodo2--;

        H[nodo1].push_back(nodo2);
        H[nodo2].push_back(nodo1);
    }

    clog << "\n\nGrafo G \n";
    for (int i = 0; i < N; i++) {
        clog << "Adyacentes a " << i << ": ";
        for (auto n : G[i])
            clog << n << " ";
        clog << "\n";
    }

    clog << "\n\n";

    clog << "Grafo H \n";
    for (int i = 0; i < N; i++) {
        clog << "Adyacentes a " << i << ": ";
        for (auto n : H[i])
            clog << n << " ";
        clog << "\n";
    }

    // Ejecutamos el algoritmo y obtenemos su tiempo de ejecución.
    TResult result;
    result = {};

    if (algoritmo == "H1")
        result = H1();

    if (algoritmo == "H2")
        result = H2();

    if (algoritmo == "TABU3") {
        TResult MEJOR_RESULTADO = ALGORITMO_INICIAL == "H1" ? H1() : H2();
        result = TABU3(MEJOR_RESULTADO, CANT_ITERACIONES, TAM_MEMORIA, PORCENTAJE_VECINDAD);
    }

    if (algoritmo == "TABU4") {
        TResult MEJOR_RESULTADO = ALGORITMO_INICIAL == "H1" ? H1() : H2();
        result = TABU4(MEJOR_RESULTADO, CANT_ITERACIONES, TAM_MEMORIA, PORCENTAJE_VECINDAD);
    }

    // Imprimo datos útiles de la ejecución por clog.
    clog << "\n\nResultado!" << endl;

    if (esFactible(result.coloreo))
        clog << "es factible!" << endl;
    else
        clog << "no es factible che, rehacer!" << endl;

    clog << result.impacto << endl;

    for (auto n : result.coloreo)
        clog << n << " ";

    clog << "\n\nTiempo Utilizado: " << result.tiempo << endl;

    // Devuelvo el resultado (impacto y tiempo) por cout.
    cout << result.impacto << " " << result.tiempo;

    return 0;
}
