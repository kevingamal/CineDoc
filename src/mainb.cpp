#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <iostream>
#include <fstream>
#include <vector>

class Script
{
public:
    int id;
    std::string title;
    std::string plain_text;

    Script() {}

    Script(int id, std::string plain_text)
        : id(id), plain_text(plain_text) {}

    Script(int id, std::string title, std::string plain_text)
        : id(id), title(title), plain_text(plain_text) {}

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & id & title & plain_text;
    }
};

class Scene
{
public:
    int id;
    int number;
    int scriptId;
    int locationId;
    int type;
    int time;
    std::string plain_text;
    int position;

    Scene() {}

    Scene(int id, std::string plain_text, int position)
        : id(id), plain_text(plain_text), position(position) {}

    Scene(int id, int number, int scriptId, int locationId, int type, int time, std::string plain_text, int position)
        : id(id), number(number), scriptId(scriptId), locationId(locationId), type(type), time(time), plain_text(plain_text), position(position) {}

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & id & number & scriptId & locationId & type & time & plain_text & position;
    }
};

int main()
{
    // Algunas instancias de Guiones y Escenas
    std::vector<Script> scripts = {
        Script(1, "Guion1", "ContenidoGuion1"),
        Script(2, "Guion2", "ContenidoGuion1")};

    std::vector<Scene> scenes = {
        Scene(1, 1, 1, 1, 0, 0, "Escena1Guion1", 1),
        Scene(2, 2, 1, 1, 0, 0, "Escena2Guion1", 1),
        Scene(3, 3, 2, 1, 0, 0, "Escena1Guion2", 1),
        Scene(4, 4, 3, 1, 0, 0, "Escena2Guion2", 1)};

    // Serializamos las instancias a un archivo
    std::ofstream out_fs("datos.bin");
    boost::archive::text_oarchive oa(out_fs);
    oa << scripts;
    oa << scenes;
    out_fs.close();

    // Creamos nuevos vectores para recibir los datos desde el archivo
    std::vector<Script> loadedScripts;
    std::vector<Scene> loadedScenes;

    // Deserializamos las instancias desde el archivo
    std::ifstream in_fs("datos.bin");
    boost::archive::text_iarchive ia(in_fs);
    ia >> loadedScripts;
    ia >> loadedScenes;

    // Imprimirmos los datos de los nuevos vectores para verificar
    for (const auto &script : loadedScripts)
    {
        std::cout << "Guion: " << script.id << " Titulo: " << script.title << " Texto: " << script.plain_text << std::endl;
    }

    for (const auto &scene : loadedScenes)
    {
        std::cout << "Escena: " << scene.id << " Numero: " << scene.number << " Guion: " << scene.scriptId << " Texto: " << scene.plain_text << " Posicion: " << scene.position << std::endl;
    }

    return 0;
}
