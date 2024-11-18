#include "reglasYhechos.hpp"
#include <io.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <string>
#include <set>
#define BC "Base de conocimientos"
#define BH "Base de Hechos"


set<hecho *> borrarHechos;
set<regla *> borrarReglas;

hecho inicializar(map<hecho*,double> &mapa, string bc, string bh);

void error(string fichero)
{
        cerr<<"Error en la estructura de la "<<fichero<<endl;
        exit(EXIT_FAILURE);
}

void error(string fichero, string informacionAdicional)
{
        cerr<<"Error en la estructura de la "<<fichero<<": "<<informacionAdicional<<endl;
        exit(EXIT_FAILURE);
}

int main(int argc, char * argv[])
{
    
    if(argc!=3)
    {
        cout<<"Error: uso\n SBR-FC.exe BasedeConocimientos(fichero de texto) BasedeReglas(fichero de texto)"<<endl;
        exit(EXIT_FAILURE);
    }
    map<hecho*,double> mapa = map <hecho*,double>();
    hecho objetivo = inicializar(mapa, argv[1], argv[2]);
    //Abrimos el log
    enable_log();
    //Obtenemos el fc del hecho objetivo (Ejecutamos el algoritmo)
    double fc = objetivo.resolver(mapa);
    //Mostramos por pantalla
    cout<<objetivo.getNombre()<<", "<<fc;
    string s = "";
    s+=objetivo.getNombre()+" "+to_string(fc);
    //Lo escribimos en el log
    write_in_log(s);
    //Cerramos el log
    disable_log();
    //Liberamos memoria
    for(set<hecho* >::iterator it= borrarHechos.begin();it==borrarHechos.end();it++)
        delete *it;
    for(set<regla* >::iterator it= borrarReglas.begin();it==borrarReglas.end();it++)
        delete *it;

}

hecho inicializar(map<hecho*,double> &mapa, string bc, string bh)
{
    ifstream fileBC(bc);
    ifstream fileBH(bh);
    set<string> palabrasReservadas;
    palabrasReservadas.insert("FC=");
    palabrasReservadas.insert("Objetivo");
    palabrasReservadas.insert("Entonces");
    if(!fileBC.is_open())
    {
        cerr<<"Error: no se ha podido abrir el fichero "<<bc<<endl;
        exit(1);
    }
    if(!fileBH.is_open())
    {
        cerr<<"Error: no se ha podido abrir el fichero "<<bh<<endl;
        exit(1);
    }
    //variables para BC y BH
    string linea;
    map<string, hecho*> nombres = map<string,hecho*>();
    string nomRegla; //nombre de cada regla antes de guardarla
    int numReglas; //Numero de las reglas 
    int numHechos; //Numero de hechos
    int index; //indice para cada for (itera en las posiciones de cada linea del fichero)
    int flag; // para terminar el bucle que reconoce los hechos para una regla
    int tipo; //determinar el tipo de una regla
    string nombreHecho; //nombre de cada hecho antes de guardarlo
    list<hecho*> hechosRegla;
    float factor; //para almacenar el factor de cada hecho o regla antes de guardarlo en le objeto
    bool flagH=0; //para terminar el bucle que inicializa el mapa a los valores correctos
    //Si el fichero esta vacío salimos del programa
    if(!getline(fileBC,linea))
    {
        error(BC,"El fichero esta vacio");
    }
    //En la primera linea deberia de estar el número de reglas que hay
    try{
        numReglas=stoi(linea);
        //Si es un número no válido salimos del programa
        if(numReglas<1)
        {
            error(BC, "Numero de reglas incorrecto, debe tener al menos una regla");
        }
            
    }
    catch(...)
    {
        error(BC, "Numero de reglas incorrecto");
    }
    getline(fileBC,linea);
    //bucle de porcesar el resto de lineas de BC
    while(linea.size())
    {
        //Inicializamos el nombre de la regla a vacio
        nomRegla = "";
        //Obtenemos el nombre de la regla que deberia estar hasta los dos puntos
        for(index =0;linea[index]!=':'&& index!=linea.size();++index)
            nomRegla+=linea[index];
        //Si llegamos al final es que el fichero tiene un mal formato
        if(index==linea.size())
        {
            error(BC,"Mal formato nombre de regla");
        }
        //Llegamos a la siguiente palabra
        for(index+=1;linea[index]==' ';++index);
        //Comprobamos aqui que utiliza el "Si" antes de los hechos y que el nombre de la regla no es una palabra reservada
        if(linea[index++]!='S' || linea[index++]!='i' || palabrasReservadas.count(nomRegla))
        {
            error(BC);
        }
        //Pasamos al primer hecho
        for(index+=1;linea[index]==' ';++index);
        flag = 0;
        //inicializamos tipo a -1
        tipo = -1;
        //establecemos la lista de hechos de la regla como una lista nueva
        hechosRegla = list<hecho*>();
        //Bucle donde guardamos todos los hechos
        while(!flag)
        {
            //Se inicializa a vacio el nombre del hecho
            nombreHecho="";
            for(;linea[index]!=' ';++index)
                nombreHecho+=linea[index];
            //Si se encuentra entonces antes del siguiente espacio es que ahora siguen los hechos (terminamos el bucle)
            if(nombreHecho=="Entonces") flag=1;
            //Si el hecho tiene palabra reservada error
            else if(palabrasReservadas.count(nombreHecho))
            {
                error(BC,"Uso de palabra reservada");
            }
            //En los siguientes se elige el tipo
            else if(nombreHecho=="o" && (tipo==-1 ||tipo==O))
                tipo= O;
            else if(nombreHecho=="y" && (tipo==-1 || tipo==Y))
                tipo=Y;
            //Si utiliza "y" y "o" en una misma regla es incorrecot
            else if((nombreHecho=="y" && tipo==O) || (nombreHecho=="o" && tipo==O))
            {
                error(BC,"Incoherencia (y/o incompatibles)");
            }
            //Si no ocurre nada de lo anterior se crea e inserta el hecho
            else
            {
                if(!nombres.count(nombreHecho))
                {
                    hecho * aux = new hecho(nombreHecho);
                    nombres.insert({nombreHecho, aux});
                    //Para liberar la memoria
                    borrarHechos.insert(aux);
                }
                hechosRegla.push_front(nombres.at(nombreHecho));
                
            }
            //Se continua a la siguiente palabra
            for(;linea[index]==' ';++index);
        }
        //Si el tipo de esta regla no se ha indicado se considera que es O (no es un nodo Y, es una disyunción)
        if(tipo==-1) tipo=O;
        //Se inicializa a vacio el nombre del hecho, esta vez será el hecho que porduzca la regla
        nombreHecho="";
        //Hasta encontrar la coma se guarda el nombre del hecho
        for(;linea[index]!=','&&index!=linea.size();++index)
            nombreHecho+=linea[index];
        for(;linea[index]!='=' && index!=linea.size();++index);
        if(index==linea.size())
            error(BC,"La regla "+nomRegla+" ha terminado antes de lo esperado.");
        //Lo siguiente sera el factor de la regla, y este se guarda
        try
        {
            factor = stof(linea.substr(++index));
            if(factor<-1 || factor > 1)
            {
                error(BC,"Mal factor de certeza en la regla "+ nomRegla);
            }
        }
        catch(...)
        {
            error(BC,"Mal factor de certeza en la regla "+ nomRegla);
        }
        //Se guarda el hecho
        if(!nombres.count(nombreHecho))
        {
            hecho * aux = new hecho(nombreHecho);
            nombres.insert({nombreHecho,aux});
            borrarHechos.insert(aux);
        }
        //Se crea la regla, añadiendo la lista de hechos que se ha ido creando en el bucle
        regla * reglaAux = new regla(nomRegla,factor,tipo,hechosRegla);
        //Se añade la regla como productora del hecho que hemos obtenido al final
        nombres.at(nombreHecho)->addProductora(reglaAux);
        //añadimos la regla al conjunto de liberar la memoria.
        borrarReglas.insert(reglaAux);
        hechosRegla.clear();
        getline(fileBC,linea);
        
    }
    
    //Si el fichero esta vacío salimos del programa
    if(!getline(fileBH,linea))
    {
        error(BH,"El fichero esta vacío");
    }
    try
    {
        numHechos=stoi(linea);
        if(numHechos<1)
            error(BH,"Numero de hechos incorrecto, debe tener al menos un hecho");
    }
    catch(...)
    {
        error(BH,"Numero de hechos incorrecto");
    }
    
    //Bucle donde se obtienen los hechos
    while(!flagH)
    {
        getline(fileBH,linea);
        //Inicializamos el nombre de hecho a vacio
        nombreHecho ="";
        //Y lo obtenemos
        for(index=0;linea[index]!=','&& linea[index]!='\0';++index)
            nombreHecho+=linea[index];
        if(linea[index]=='\0' || linea.size()==0)
        {
            if(nombreHecho=="Objetivo")
            flagH=1;
            else
                error(BH);
            break;
        }
        for(;linea[index]!='='; ++index);
        factor = stof(linea.substr(++index));
        if(nombres.count(nombreHecho))
        {
            mapa.insert({(nombres.at(nombreHecho)),factor});
        }
        
    }
    //Obtenemos la que deberia ser la última linea
    getline(fileBH,linea);
    //Si la línea no es el nombre un hecho que estuviera en la base de conocimientos esta mal y el programa termina
    if(!nombres.count(linea))
    {
        error(BC,"El hecho objetivo no existe");
    }
    fileBC.close();
    fileBH.close();
    return *(nombres.at(linea));
}