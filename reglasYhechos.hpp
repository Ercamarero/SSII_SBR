#ifndef REGLASYHECHOS_HPP
#define REGLASYHECHOS_HPP

#include <iostream>
#include <list>
#include <string>
#include <Map>

//tipos
#define Y 1
#define O 0

using namespace std;

//Habilita la salida por fichero log
void logEnabled();

//Escribe en el log
void writeLog(string s);

//Deshabilita la salida por fichero log
void closeLog();

class hecho;

class regla
{
private:
    string nombre; //id de la regla (debe ser único)
    list<hecho*> hechos; //lista de los hechos que necesita la regla
    double factor; //Factor de certeza de la regla
    bool tipob; //Si es nodo Y o no
    double(*tipo)(double, double); //La operación que debe aplicar ante los FC de los hechos

public:
    regla(string nombre, double factor, bool tipo);
    regla(string nombre,double factor,bool tipo, list<hecho*> hechos);
    ~regla();
    bool operator< (const regla otro) const; //Funcion no necesaria para algoritmo: Necesario para las colecciones
    bool operator== (const regla otro) const; //Funcion no necesaria para algoritmo: Necesario para las colecciones
    void addHecho(hecho* Hecho); //Funcion de añadir un hecho a la lista
    double resolver(map <hecho*, double> hechosBase, string nombreHecho); //SBR-FC para las reglas
    void imprimirHechos();
    string getNombre(){return this->nombre;}
};


class hecho
{
private:
    string nombre; //id del hecho (debe ser único)
    list<regla*> productoras; //Lista de reglas productoras (Que producen este hecho)
public:
    hecho(string nombre);
    hecho(string nombre, list<regla*> productoras);
    ~hecho();
    bool operator< (const hecho otro) const; //Funcion no necesaria para algoritmo: Necesario para las colecciones
    bool operator== (const hecho otro) const; //Funcion no necesaria para algoritmo: Necesario para las colecciones
    void addProductora(regla* regla); //Añadir una regla productora a su lista
    double resolver(map <hecho*, double> hechosBase); //SBR-FC para los hechos
    void imprimirProductoras(); //Funcion no necesaria (Ha sido usada para debug)
    string getNombre(){return this->nombre;}
};

#endif