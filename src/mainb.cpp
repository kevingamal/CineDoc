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

    std::vector<Proyecto> proyectos = {
        Proyecto(1, "Proyecto 1", "row_1"),
        Proyecto(2, "Proyecto 2", "row_2")};

    std::vector<Guion> guiones = {
        Guion(1, 1, "texto_plano_1"),
        Guion(2, 2, "texto_plano_2")};
}

std::vector<Escena> escenas = {
    Escena(1, "Escena 1", 1, 1, "tipo_1", "tiempo_1"),
    Escena(2, "Escena 2", 2, 2, "tipo_2", "tiempo_2")};
}

std::vector<Toma> tomas = {
    Toma(1, "Toma 1", 1, "encuadre_1"),
    Toma(2, "Toma 2", 2, "encuadre_2")};
}

std::vector<Actuacion> actuaciones = {
    Actuacion(1, 1, 1),
    Actuacion(2, 2, 2)};

std::vector<Locacion> locaciones = {
    Locacion(1, "Locacion 1", "direccion_1", "telefono_1", "hospital_cer_1", "estacionamiento_1"),
    Locacion(2, "Locacion 2", "direccion_2", "telefono_2", "hospital_cer_2", "estacionamiento_2")};

std::vector<Objeto> objetos = {
    Objeto(1, "Objeto 1", 1, "descripcion_1", "tipo_1"),
    Objeto(2, "Objeto 2", 2, "descripcion_2", "tipo_2")};
}

std::vector<Uso_Ficcional> usos_ficcionales = {
    Uso_Ficcional(1, 1, 1),
    Uso_Ficcional(2, 2, 2)};

std::vector<Uso_Tecnico> usos_tecnicos = {
    Uso_Tecnico(1, 1, 1),
    Uso_Tecnico(2, 2, 2)};
}

// Serializamos las instancias a un archivo
std::ofstream ofs("datos.bin");
boost::archive::text_oarchive oa(ofs);
oa << actores;
oa << personajes;
oa << guions;
oa << escenas;
oa << tomas;
oa << actuaciones;
oa << locaciones;
oa << objetos;
oa << usos_ficcionales;
oa << usos_tecnicos;
ofs.close();

// Creamos nuevos vectores para recibir los datos desde el archivo
std::vector<Actor> actoresCargados;
std::vector<Personaje> personajesCargados;
std::vector<Guion> guionesCargados;
std::vector<Escena> escenasCargadas;
std::vector<Toma> tomasCargadas;
std::vector<Actuacion> actuacionesCargadas;
std::vector<Locacion> locacionesCargadas;
std::vector<Objeto> objetosCargados;
std::vector<Uso_Ficcional> usos_ficcionalesCargados;
std::vector<Uso_Tecnico> usos_tecnicosCargados;

// Deserializamos las instancias desde el archivo
std::ifstream ifs("datos.bin");
boost::archive::text_iarchive ia(ifs);
ia >> actoresCargados;
ia >> personajesCargados;
ia >> guionesCargados;
ia >> escenasCargadas;
ia >> tomasCargadas;
ia >> actuacionesCargadas;
ia >> locacionesCargadas;
ia >> objetosCargados;
ia >> usos_ficcionalesCargados;
ia >> usos_tecnicosCargados;

// Imprimirmos los datos de los nuevos vectores para verificar
for (const auto &actor : actoresCargados)
{
    std::cout << "Actor: " << actor.nombre << " " << actor.apellido << std::endl;
}

for (const auto &personaje : personajesCargados)
{
    std::cout << "Personaje: " << personaje.nombre << " en " << personaje.serie << std::endl;
}

for (const auto &guion : guionesCargados)
{
    std::cout << "Guion: " << guion.texto_plano << std::endl;
}

for (const auto &escena : escenasCargadas)
{
    std::cout << "Escena: " << escena.numero << " en " << escena.tiempo << std::endl;
}

for (const auto &toma : tomasCargadas)
{
    std::cout << "Toma: " << toma.numero << " en " << toma.encuadre << std::endl;
}

for (const auto &actuacion : actuacionesCargadas)
{
    std::cout << "Actuacion: " << actuacion.id << " en " << actuacion.escenaId << std::endl;
}

for (const auto &locacion : locacionesCargadas)
{
    std::cout << "Locacion: " << locacion.nombre << " en " << locacion.direccion << std::endl;
}

for (const auto &objeto : objetosCargados)
{
    std::cout << "Objeto: " << objeto.nombre << " en " << objeto.descripcion << std::endl;
}

for (const auto &uso_ficcional : usos_ficcionalesCargados)
{
    std::cout << "Uso Ficcional: " << uso_ficcional.id << " en " << uso_ficcional.objetoId << std::endl;
}

return 0;
}
