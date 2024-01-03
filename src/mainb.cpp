#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <iostream>
#include <fstream>
#include <vector>

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
    std::string nombre;
    int actorId;
    std::string serie;

    Toma() {}

    Toma(int id, std::string nombre, int actorId, std::string serie)
        : id(id), nombre(nombre), actorId(actorId), serie(serie) {}

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & id & nombre & actorId & serie;
    }
};

class Escena
{
public:
    int id;
    std::string nombre;
    int actorId;
    std::string serie;

    Escena() {}

    Escena(int id, std::string nombre, int actorId, std::string serie)
        : id(id), nombre(nombre), actorId(actorId), serie(serie) {}

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & id & nombre & actorId & serie;
    }
};

class Actuacion
{
public:
    int id;
    std::string nombre;
    int actorId;
    std::string serie;

    Actuacion() {}

    Actuacion(int id, std::string nombre, int actorId, std::string serie)
        : id(id), nombre(nombre), actorId(actorId), serie(serie) {}

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & id & nombre & actorId & serie;
    }
};

class Guion
{
public:
    int id;
    std::string nombre;
    int actorId;
    std::string serie;

    Guion() {}

    Guion(int id, std::string nombre, int actorId, std::string serie)
        : id(id), nombre(nombre), actorId(actorId), serie(serie) {}

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & id & nombre & actorId & serie;
    }
};

class Locacion
{
public:
    int id;
    std::string nombre;
    int actorId;
    std::string serie;

    Locacion() {}

    Locacion(int id, std::string nombre, int actorId, std::string serie)
        : id(id), nombre(nombre), actorId(actorId), serie(serie) {}

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & id & nombre & actorId & serie;
    }
};

class Uso_Ficcional
{
public:
    int id;
    std::string nombre;
    int actorId;
    std::string serie;

    Uso_Ficcional() {}

    Uso_Ficcional(int id, std::string nombre, int actorId, std::string serie)
        : id(id), nombre(nombre), actorId(actorId), serie(serie) {}

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & id & nombre & actorId & serie;
    }
};

class Objeto
{
public:
    int id;
    std::string nombre;
    int actorId;
    std::string serie;

    Objeto() {}

    Objeto(int id, std::string nombre, int actorId, std::string serie)
        : id(id), nombre(nombre), actorId(actorId), serie(serie) {}

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {

        ar & id & nombre & actorId & serie;
    }
};

class Uso_Tecnico
{
public:
    int id;
    std::string nombre;
    int actorId;
    std::string serie;

    Uso_Tecnico() {}

    Uso_Tecnico(int id, std::string nombre, int actorId, std::string serie)
        : id(id), nombre(nombre), actorId(actorId), serie(serie) {}
    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {

        ar & id & nombre & actorId & serie;
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
    int actorId;
    std::string serie;

    Proyecto() {}

    Proyecto(int id, std::string nombre, int actorId, std::string serie)
        : id(id), nombre(nombre), actorId(actorId), serie(serie) {}
    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {

        ar & id & nombre & actorId & serie;
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
