#include "reglasYhechos.hpp"

#define LOGNAME "log.txt"
#define log stdin

bool fLog= false;
FILE * loge = stdout;

//Habilitar la escritura en log
void enable_log()
{
    fLog = true;
    loge = fopen(LOGNAME, "w");
}

//Escribir en el log de forma externa
void write_in_log(string s)
{
    fprintf(loge,"%s\n",s.c_str());
}

//Cerrar la escritura en log
void disable_log()
{
    if(fLog)
    {
        fLog = false;
        fclose(log);
        loge = stdout;
    }
}

//Inicio de funciones auxiliares
double absol(double a)
{
    return ((a<0) ? a*-1 : a);
}

double max(double a, double b)
{
    return ((a < b) ? b : a);
}

double min(double a, double b)
{
    return ((a > b) ? b : a);
}
//Fin de funciones auxiliares

//Caso 2 si a y b son positivos
double gt0(double a, double b)
{
    return a+b*(1-a);
}

//Caso 2 si a y b son negativos
double lt0(double a, double b)
{
    return a+b*(1+a);
}

//Caso 2 si a y b tienen distinto signo
double other(double a, double b)
{
    return (a+b)/(1-min(absol(a),absol(b)));
}

regla::regla(string nombre, double factor, bool tipo)
{
    this->nombre = nombre;
    this->factor = factor;
    if(tipo)
        this->tipo = min;
    else
        this->tipo=max;
    this->tipob=tipo;
}

regla::regla(string nombre, double factor, bool tipo, list<hecho*> hechos)
{
    this->nombre = nombre;
    this->factor = factor;
    this->hechos = hechos;
    if(tipo)
        this->tipo = min;
    else
        this->tipo=max;
    this->tipob=tipo;
}

regla::~regla()
{

}

bool regla::operator<(const regla otro) const
{
    return this->nombre<otro.nombre;
}

bool regla::operator==(const regla otro) const
{
    return this->nombre == otro.nombre;
}

void regla::addHecho(hecho* hecho)
{
    this->hechos.push_front(hecho);
}

//Algoritmo SBR-FC (En reglas)
double regla::resolver(map<hecho*, double> mapa, string nombreHecho)
{
    //Se establece un iterador para la lista de hechos para recorrerla
    list<hecho*>::iterator it = this->hechos.begin();
    //Variable que almacena el fc del hecho que necesite
    double res;
    //Variable auxiliar
    double aux;
    //Factor de certeza que se acaba retornando
    double fc;
    //Variable que almacena el número de hechos que requiere esta regla(Solo nos interesa si es mayor que 1 para escribir en el log)
    int numHechos =0;
    //Nombre del hecho resultante antes de entrar en la regla(la conjunción o disyunción de los hechos)
    string nomHechos = "";
    //Caracter de disyuncion o conjunción
    char caracter;
    /*Si la variable tipob (tipoBoolean) muestra que es un nodo Y, la operación que se realiza entre los hechos es la conjunción
    (min) entonces se le asigna el caracter de la conjunción y a res un valor mayor al máximo, si no fuera nodo Y se le asignaria 
    el caracter de la disyuncion y un valor menor al mínimo puesto que se aplica la disyunción (maximo). La asignación inicial 
    de la variable res se hace para mayor simplicidad a la hora de hacer el for*/
    if(this->tipob)
    {
        res=1.1;
        caracter = '^';
    }
    else
    {
        caracter ='v';
        res=-1.1;
    }
    //Mientras haya hechos en la lista
    while(it!=this->hechos.end())
    {
        //Si es un hecho base que está en el mapa se obtiene su FC
        if(mapa.count(*it))
            aux = mapa.at(*it);
        //Si no es un hecho base que está en el mapa se resuelve
        else
            aux=(*it)->resolver(mapa);  
        //Se almacena en resel resultado de la conjunción/disyunción
        res = this->tipo(res,aux);
        //Suma 1 a el número de hechos
        numHechos++;
        //Se compone el nombre del hecho resultante de la disyunción/conjunción
        nomHechos+=(*it)->getNombre()+caracter;
        //Pasamos al siguiente hecho en la lista
        it++;
    }
    //Se elimina el último carácter del nombre del hecho resultante (es un simbolo de conjunción o disyunción)
    nomHechos[nomHechos.size()-1] = '\0';
    //Se escribe por log y por pantalla que se ha activado la regla
    cout<<"Regla activada "<<this->nombre<<endl;
    fprintf(loge,"Regla activada %s\n",this->nombre.c_str());
    if(numHechos>1)
    {
        //Si el número de hechos es mayor que 1 se ha dado el caso 1 y lo mostramos por pantall y por log
        cout<<"\tCaso 1: "<<nomHechos<<", "<<res<<endl;
        fprintf(loge,"\tCaso 1: %s, %f\n",nomHechos.c_str(), aux);
    }
    //Se obtiene el factor de certeza del hecho que obtiene la regla
    fc = max(0,res)*this->factor;
    //Se muestra por pantalla y log el caso 3
    cout<<"\tCaso 3: "<<nombreHecho<<", "<<fc<<endl;
    fprintf(loge,"\tCaso 3: %s, %f\n",nombreHecho.c_str(),fc);
    return fc;
}


hecho::hecho(string nombre)
{
    this->nombre = nombre;
}

hecho::hecho(string nombre, list<regla*> productoras)
{
    this->nombre = nombre;
    this->productoras = productoras;
}

hecho::~hecho()
{

}

bool hecho::operator<(const hecho otro) const
{
    return this->nombre < otro.nombre;
}

bool hecho::operator==(const hecho otro) const
{
    return this->nombre == otro.nombre;
}

void hecho::addProductora(regla* regla)
{
    this->productoras.push_front(regla);
}

// SBR-FC (En hechos)
double hecho::resolver(map<hecho*, double> mapa)
{
    list<regla*> productoras = this->productoras;
    double fc=0;
    //Si se ha querido resolver un hecho que no tiene productoras devolvemos 0 (No tenemos información suya)
    if(productoras.size()==0) return 0.0;
    //Si tiene solo una regla productora resolvemos por esa regla y retornamos el factor que devuelva
    if(productoras.size()==1)
    {
        fc = productoras.front()->resolver(mapa, this->nombre);
        return fc;
    }
    //Si tiene más de una regla se aplica el caso 2, en cuyo caso se itera sobre la lista de reglas productoras
    list<regla*>::iterator it = this->productoras.begin();
    //Se resuleve una primera regla
    fc=(*it)->resolver(mapa,this->nombre);
    //Variable b, almacena el factor de certeza de la regla que acabamos de resolver dentro del bucle
    double b;
    //Función a aplicar
    double (*funRet)(double, double);
    //Pasamos a la siguiente regla
    it++;
    //Mientras queden reglas
    for(;it!=this->productoras.end();it++)
    {
        //Se resuleve la regla y se guarda en b
        b=(*it)->resolver(mapa,this->nombre);
        //Dependiendo de los valores de los fcs obtenidos seleccionamos la función correcta a aplicar
        if(fc>=0 && b>=0) funRet=gt0;
        else if(fc<=0 && b<=0) funRet=lt0; 
        else funRet=other;
        //Se realiza
        fc = funRet(fc,b);
        //Se itera
    }
    //Se escribe en el log la resolución de este caso 2
    cout<<"Caso 2: "<<this->nombre<<", "<<fc<<endl;
    fprintf(loge,"Caso 2: %s, %f\n",this->nombre.c_str(),fc);
    //Se retorna el valor obtenido
    return fc;
}

void regla::imprimirHechos()
{
    list<hecho*>::iterator it;
    for(it=this->hechos.begin();it!=this->hechos.end();it++)
        cout<<(*it)->getNombre()<<" ";
    cout<<endl;
}

void hecho::imprimirProductoras()
{
    list<regla*>::iterator it;
    for(it=this->productoras.begin();it!=this->productoras.end();it++)
    {
        cout<<(*it)->getNombre()<<":"<<endl;
        (*it)->imprimirHechos();
    }
    cout<<this->productoras.size()<<endl;
}