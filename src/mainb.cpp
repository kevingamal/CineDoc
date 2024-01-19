#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <iostream>
#include <fstream>
#include <vector>

class Scene
{
public:
    int id;
    int number;
    int parentId;
    int locationId;
    int type;
    int time;
    std::string plain_text;
    int position;

    Scene() {}

    Scene(int id, int parentId, std::string plain_text, int position)
        : id(id), parentId(parentId), plain_text(plain_text), position(position) {}

    Scene(int id, int number, int parentId, int locationId, int type, int time, std::string plain_text, int position)
        : id(id), number(number), parentId(parentId), locationId(locationId), type(type), time(time), plain_text(plain_text), position(position) {}

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & id & number & parentId & locationId & type & time & plain_text & position;
    }
};

class Take
{
public:
    int id;
    int parentId;
    int number;
    int shot_size;
    int movement;
    int mount;
    int camera;
    int lens;
    int sound;
    int length;
    std::string description;
    std::string image;
    std::string floor_plan;
    int position;

    Take() {}

    Take(int id, int parentId, std::string description, int position)
        : id(id), parentId(parentId), description(description), position(position) {}

    Take(int id, int parentId, int number, int shot_size, int movement, int mount, int camera, int lens, int sound, int length, std::string description, std::string image, std::string floor_plan, int position)
        : id(id), parentId(parentId), number(number), shot_size(shot_size), movement(movement), mount(mount), camera(camera), lens(lens), sound(sound), length(length), description(description), image(image), floor_plan(floor_plan), position(position) {}

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & id & parentId & number & shot_size & movement & mount & camera & lens & sound & length & description & image & floor_plan & position;
    }
};

int main()
{
    // Algunas instancias de Escenas y Tomas
    std::vector<Scene> scenes =
        {
            Scene(1, 1, 1, 1, 0, 0, "Escena1 - Guion1", 1),
            Scene(2, 2, 1, 1, 0, 0, "Escena2 - Guion1", 1),
            Scene(3, 3, 2, 1, 0, 0, "Escena1 - Guion2", 1),
            Scene(4, 4, 3, 1, 0, 0, "Escena2 - Guion2", 1)

        };

    std::vector<Take> takes =
        {
            Take(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, "Descripcion1", "imagen1", "plano1", 1),
            Take(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, "Descripcion2", "imagen2", "plano2", 1)

        };

    // Serializamos las instancias a un archivo
    std::ofstream out_fs("datos.bin");
    boost::archive::text_oarchive oa(out_fs);
    oa << scenes;
    oa << takes;
    out_fs.close();

    // Creamos nuevos vectores para recibir los datos desde el archivo
    std::vector<Take> loadedTakes;
    std::vector<Scene> loadedScenes;

    // Deserializamos las instancias desde el archivo
    std::ifstream in_fs("datos.bin");
    boost::archive::text_iarchive ia(in_fs);
    ia >> loadedScenes;
    ia >> loadedTakes;

    // Imprimirmos los datos de los nuevos vectores para verificar
    for (const auto &scene : loadedScenes)
    {
        std::cout << " Padre: " << scene.parentId << " Escena ID: " << scene.id << " Texto: " << scene.plain_text << " Posicion: " << scene.position << std::endl;
    }

    for (const auto &take : loadedTakes)
    {
        std::cout << " Padre: " << take.parentId << " Toma ID: " << take.id << " Texto: " << take.description << " Posicion: " << take.position << std::endl;
    }

    return 0;
}
