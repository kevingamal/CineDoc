#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

class Actor
{
public:
    int id;
    std::string nombre;
    std::string apellido;
    std::string fechaNacimiento;

    Actor() {}

    Actor(int id, std::string nombre, std::string apellido, std::string fechaNacimiento)
        : id(id), nombre(nombre), apellido(apellido), fechaNacimiento(fechaNacimiento) {}

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & id & nombre & apellido & fechaNacimiento;
    }
};

class Toma
{
public:
    int id;
    std::string numero;
    int escenaId;
    std::string encuadre;

    Toma() {}

    Toma(int id, std::string numero, int escenaId, std::string encuadre)
        : id(id), numero(numero), escenaId(escenaId), encuadre(encuadre) {}

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & id & numero & escenaId & encuadre;
    }
};

class Escena
{
public:
    int id;
    std::string numero;
    int guionId;
    int locacionId;
    std::string tipo;
    std::string tiempo;

    Escena() {}

    Escena(int id, std::string numero, int guionId, int locacionId, std::string tipo, std::string tiempo)
        : id(id), numero(numero), guionId(guionId), locacionId(locacionId), tipo(tipo), tipo(tiempo) {}

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & id & numero & guionId & locacionId & tipo & tiempo;
    }
};

class Actuacion
{
public:
    int id;
    int actorId;
    int escenaId;

    Actuacion() {}

    Actuacion(int id, int actorId, int escenaId)
        : id(id), actorId(actorId), escenaId(escenaId) {}

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & id & actorId & escenaId;
    }
};

class Guion
{
public:
    int id;
    int proyectoId;
    std::string texto_plano;

    Guion() {}

    Guion(int id, int proyectoId, std::string texto_plano)
        : id(id), proyectoId(proyectoId), texto_plano(texto_plano) {}

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & id & proyectoId & texto_plano;
    }
};

class Locacion
{
public:
    int id;
    std::string nombre;
    std::string direccion;
    std::string telefono;
    std::string hospital_cer;
    std::string estacionamiento;

    Locacion() {}

    Locacion(int id, std::string nombre, std::string direccion, std::string telefono, std::string hospital_cer, std::string estacionamiento)
        : id(id), nombre(nombre), direccion(direccion), telefono(telefono), hospital_cer(hospital_cer), estacionamiento(estacionamiento) {}

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & id & nombre & direccion & telefono & hospital_cer & estacionamiento;
    }
};

class Uso_Ficcional
{
public:
    int id;
    int escenaId;
    int objetoId;

    Uso_Ficcional() {}

    Uso_Ficcional(int id, int actorId, int objetoId)
        : id(id), actorId(actorId), objetoId(objetoId) {}

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & id & actorId & objetoId;
    }
};

class Objeto
{
public:
    int id;
    std::string nombre;
    int proyectoId;
    std::string descripcion;
    std::string tipo;

    Objeto() {}

    Objeto(int id, std::string nombre, int actorId, std::string descripcion, std::string tipo)
        : id(id), nombre(nombre), actorId(actorId), descripcion(descripcion), tipo(tipo) {}

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {

        ar & id & nombre & actorId & descripcion & tipo;
    }
};

class Uso_Tecnico
{
public:
    int id;
    int tomaId;
    int objetoId;

    Uso_Tecnico() {}

    Uso_Tecnico(int id, int tomaId, int objetoId)
        : id(id), tomaId(tomaId), objetoId(objetoId) {}
    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {

        ar & id & tomaId & objetoId;
    }
};

class Personaje
{
public:
    int id;
    std::string nombre;
    int actorId;
    std::string serie;

    Personaje() {}

    Personaje(int id, std::string nombre, int actorId, std::string serie)
        : id(id), nombre(nombre), actorId(actorId), serie(serie) {}

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & id & nombre & actorId & serie;
    }
};

class Proyecto
{
public:
    int id;
    std::string nombre;
    std::string row_1;

    Proyecto() {}

    Proyecto(int id, std::string nombre, std::string row_1)
        : id(id), nombre(nombre), row_1(row_1) {}
    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {

        ar & id & nombre & actorId & row_1;
    }
};

int main()
{
    // Algunas instancias de Actores y Personajes
    std::vector<Actor> actores = {
        Actor(1, "John", "Doe", "01/01/1980"),
        Actor(2, "Jane", "Smith", "02/02/1990")};

    std::vector<Personaje> personajes = {
        Personaje(1, "Superhéroe", 1, "Serie A"),
        Personaje(2, "Villano", 2, "Serie B")};

    // Serializamos las instancias a un archivo
    std::ofstream ofs("datos.bin");
    boost::archive::text_oarchive oa(ofs);
    oa << actores;
    oa << personajes;
    ofs.close();

    // Creamos nuevos vectores para recibir los datos desde el archivo
    std::vector<Actor> actoresCargados;
    std::vector<Personaje> personajesCargados;

    // Deserializamos las instancias desde el archivo
    std::ifstream ifs("datos.bin");
    boost::archive::text_iarchive ia(ifs);
    ia >> actoresCargados;
    ia >> personajesCargados;

    // Imprimirmos los datos de los nuevos vectores para verificar
    for (const auto &actor : actoresCargados)
    {
        std::cout << "Actor: " << actor.nombre << " " << actor.apellido << std::endl;
    }

    for (const auto &personaje : personajesCargados)
    {
        std::cout << "Personaje: " << personaje.nombre << " en " << personaje.serie << std::endl;
    }

    return 0;
}
