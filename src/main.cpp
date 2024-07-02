#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

// #include <wx/wxprec.h>
#include <wx/filedlg.h>

// BOOST INCLUDES
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>

// Resto de los includes
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/stattext.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

// FILE DATA
bool opf = false;
bool mod = false;

wxDECLARE_EVENT(wxEVT_UPDATE_POSITION_EVENT, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_UPDATE_INDEX_EVENT, wxCommandEvent);

enum
{
    // ID_NEW = 1, (PARECE QUE ANDA IGUAL AUNQUE ID_SAVE_AS EMPIECE EN 4)
    // ID_OPEN = 2,
    // ID_SAVE = 3,
    ID_SAVE_AS = 1,
    //  ID_CLOSE = 5,
    ID_EXIT = 2,
    ID_SCRIPT_NEW = 3,
    ID_SCRIPT_EDIT = 4,
    ID_SCRIPT_DEL = 5,
    ID_Hello = 6,
    // ID_ABOUT = 11,
    ID_HELP = 7
};

std::vector<int> scriptsArray; // Vector para almacenar temporalmente los scrips (guiones) y reciclar los antiguos
std::vector<int> tree;         // Vector para almacenar la sucesion de parentId actual
// (Siempre en el ultimo elemento) <- Se añade al bajar, se borra al subir y se edita al cambiar
// Para crear un nuevo guion se le pregunta firstEmpty a scriptsArray y se reemplaza el elemento en la posicion 0 con ese numero

// VECTOR (solo para almacenar temporalmente los indices y posiciones y reciclar los antiguos)
std::vector<int> textBoxsContainer;
std::vector<int> itemsListContainer;

// FUNCION PARA ENCONTRAR POTENCIALES LUGARES LIBRES EN UN VECTOR (SI SE BORRARON Y QUEDARON HUECOS)
int firstEmpty(std::vector<int> vector)
{
    // Si el vector está vacío, el primer número faltante es 1
    if (vector.empty())
    {
        return 1;
    }

    // Ordena el vector en orden ascendente
    std::sort(vector.begin(), vector.end());

    // Si el primer número no es 1, entonces 1 es el número faltante
    if (vector[0] != 1)
    {
        return 1;
    }

    // Recorre el vector y busca el primer número faltante
    for (size_t i = 0; i < vector.size() - 1; ++i)
    {
        if (vector[i + 1] - vector[i] > 1)
        {
            return vector[i] + 1;
        }
    }

    // Si no encontraste ningún número faltante, retorna el último número + 1
    return vector.back() + 1;
}
// LA PROXIMA POSICION LIBRE (SIEMPRE SERA LA ULTIMA YA QUE EL VECTOR SE COLAPSA SOBRE SI MISMO AL ELIMINAR ALGO DE EL)
int lastEmpty(const std::vector<int> vector)
{
    return vector.size();
}

void deleteVectorItem(std::vector<int> &vector, int item)
{
    for (size_t i = 0; i < vector.size();)
    {
        if (vector[i] == item)
        {
            vector.erase(vector.begin() + i);
            break;
        }
        else
        {
            ++i;
        }
    }
}

void updateElementPosition(std::vector<int> &vector, int id, int newPosition)
{
    auto it = std::find(vector.begin(), vector.end(), id);

    if (it == vector.end())
    {
        // El ID no se encuentra en el vector, manejar este caso según sea necesario
        return;
    }

    int oldPosition = std::distance(vector.begin(), it);

    // Verificar que newPosition esté dentro de los límites válidos del vector
    if (newPosition < 0 || newPosition >= vector.size())
    {
        // Manejar el caso en que newPosition es inválido
        return;
    }

    // Eliminar el elemento de su posición actual
    vector.erase(it);

    // Insertar el elemento en la nueva posición
    vector.insert(vector.begin() + newPosition, id);
}

// numeros.push_back(x); // Añade el numero x al final
// int primerNumero = numeros[0]; // Obtenemos el numero en la primera posicion
// size_t cantidad = numeros.size(); // Obtenemos la cantidad de items en el vector
// std::sort(numeros.begin(), numeros.end()); // Ordena ascendentemente (se puede usar al revés)

// Posiciones de la lista
//  t/i   [indx/panl]pos
int TextBoxIndexPosition;  // TitledTextBox (used in move (evta))
int TextBoxPanelPosition;  // TitledTextBox (used in up, down, move (evtb))
int TextListIndexPosition; // ItemTextList  (used in move (evta))
int TextListPanelPosition; // ItemTextList  (used in up, down, move (evtb))

// DATA CLASSES:

class Script
{
public:
    std::vector<int> id; // No importa que sea vector por que se basa siempre en scriptsArray
    std::string title;
    std::string plain_text;

    Script(std::vector<int> id, std::string title, std::string plain_text)
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
    std::vector<int> id;
    std::vector<int> parentId;
    int locationId;
    int type;
    int time;
    std::string plain_text;
    int position;

    Scene(int idTail, std::vector<int> parentId, std::string plain_text, int position)
        : parentId(parentId), plain_text(plain_text), position(position)
    {
        id = parentId;
        id.push_back(idTail);
    }

    Scene(std::vector<int> id, std::vector<int> parentId, int locationId, int type, int time, std::string plain_text, int position)
        : id(id), parentId(parentId), locationId(locationId), type(type), time(time), plain_text(plain_text), position(position) {}

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & id & parentId & locationId & type & time & plain_text & position;
    }
};

class Take
{
public:
    std::vector<int> id;
    std::vector<int> parentId;
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

    Take(int idTail, std::vector<int> parentId, std::string description, int position)
        : parentId(parentId), description(description), position(position)
    {
        id = parentId;
        id.push_back(idTail);
    }

    Take(std::vector<int> id, std::vector<int> parentId, int number, int shot_size, int movement, int mount, int camera, int lens, int sound, int length, std::string description, std::string image, std::string floor_plan, int position)
        : id(id), parentId(parentId), number(number), shot_size(shot_size), movement(movement), mount(mount), camera(camera), lens(lens), sound(sound), length(length), description(description), image(image), floor_plan(floor_plan), position(position) {}

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & id & parentId & number & shot_size & movement & mount & camera & lens & sound & length & description & image & floor_plan & position;
    }
};

class Use_case
{
public:
    std::vector<int> id;
    std::vector<int> parentId; // Relaciona actores y objetos con escenas, si es un objeto tendra un id en ojbectId, si es actor tendra un id en actorId
    int actorId;
    int objectId;
    int position;

    Use_case(int idTail, std::vector<int> parentId, int actorId, int objectId, int position)
        : parentId(parentId), actorId(actorId), objectId(objectId), position(position)
    {
        id = parentId;
        id.push_back(idTail);
    }

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & id & parentId & actorId & objectId & position;
    }
};

class Tech_use
{
public:
    std::vector<int> id;
    std::vector<int> parentId; // Relaciona objetos con tomas
    int objectId;
    int position;

    Tech_use(int idTail, std::vector<int> parentId, int objectId, int position)
        : parentId(parentId), objectId(objectId), position(position)
    {
        id = parentId;
        id.push_back(idTail);
    }
    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {

        ar & id & parentId & objectId & position;
    }
};

class Event // Contiene acciones y dialogos. Cual de los 2 es se define en type
{
public:
    std::vector<int> id;
    std::vector<int> parentId;
    int type;
    int position;

    Event(int idTail, std::vector<int> parentId, int type, int position)
        : parentId(parentId), type(type), position(position)
    {
        id = parentId;
        id.push_back(idTail);
    }

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & id & parentId & type & position;
    }
};

class Character
{
public:
    int id;
    std::string first_name;
    std::string last_name;
    std::string surrname;

    Character(std::string first_name, std::string last_name, std::string surrname)
        : first_name(first_name), last_name(last_name), surrname(surrname) {}

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & id & first_name & last_name & surrname;
    }
};

class Actor
{
public:
    int id;
    int parentId;
    int passport_id;
    std::string first_name;
    std::string last_name;
    std::string surrname;
    std::string birthdate;

    Actor(int parentId, int passport_id, std::string first_name, std::string last_name, std::string surrname, std::string birthdate)
        : id(id), parentId(parentId), passport_id(passport_id), first_name(first_name), last_name(last_name), surrname(surrname), birthdate(birthdate) {}

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & id & parentId & passport_id & first_name & last_name & surrname & birthdate;
    }
};

class Location
{
public:
    int id;
    std::string name;
    std::string adress;
    std::string phone;
    std::string hospital;
    std::string parking;

    Location(std::string name, std::string adress)
        : name(name), adress(adress) {}

    Location(std::string name, std::string adress, std::string phone, std::string hospital, std::string parking)
        : name(name), adress(adress), phone(phone), hospital(hospital), parking(parking) {}

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & id & name & adress & phone & hospital & parking;
    }
};

class Object
{
public:
    int id;
    std::string name;
    std::string description;
    std::string type;

    Object(std::string name, std::string description, std::string type)
        : name(name), description(description), type(type) {}

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {

        ar & id & name & description & type;
    }
};

// DATA ARRAYS
std::vector<Script> scripts = {};
std::vector<Scene> scenes = {};
std::vector<Take> takes = {};
std::vector<Use_case> use_cases = {};
std::vector<Tech_use> tech_uses = {};
std::vector<Event> events = {};

std::vector<Character> characters = {};
std::vector<Actor> actors = {};
std::vector<Location> locations = {};
std::vector<Object> objects = {};

// TEMP GUI DATA ARRAYS
std::vector<Scene> scenesTemp = {};
std::vector<Take> takesTemp = {};
std::vector<Use_case> use_casesTemp = {};
std::vector<Tech_use> tech_usesTemp = {};
std::vector<Event> eventsTemp = {};

// ARRAYS FUNCTIONS

// SCRIPTS
bool checkTitleExists(const std::vector<Script> &scripts, const std::string &title)
{
    // Recorrer el vector de scripts y comparar cada título con el título dado
    for (const auto &script : scripts)
    {
        if (script.title == title)
        {
            return true; // El título ya existe
        }
    }
    return false; // El título no existe
}

// SCENES
void transferScenes(std::vector<Scene> &source, std::vector<Scene> &destination, std::vector<int> specificParentId) // Transfiere todos los de un padre -> TEMP
{
    // Paso 1: Limpia el vector de destino antes de transferir los nuevos elementos
    destination.clear();

    // Paso 2: Ordenar el vector 'source' por la variable 'position' de menor a mayor
    std::sort(source.begin(), source.end(), [](const Scene &a, const Scene &b)
              { return a.position < b.position; });

    // Paso 3: Transferir todos los elementos de 'source' a 'destination'
    for (const auto &scene : source)
    {
        if (scene.parentId == specificParentId)
        {
            destination.push_back(scene);
        }
    }
}

void updateScenes(std::vector<Scene> &source, std::vector<Scene> &destination, std::vector<int> specificParentId) // Transfiere todos los de un padre -> MAIN
{
    // Paso 1: Eliminar todos los elementos con el parentId específico de 'destination'
    destination.erase(std::remove_if(destination.begin(), destination.end(),
                                     [specificParentId](const Scene &scene)
                                     {
                                         return scene.parentId == specificParentId;
                                     }),
                      destination.end());

    // Paso 2: Ordenar el vector 'source' por la variable 'position' de menor a mayor
    std::sort(source.begin(), source.end(), [](const Scene &a, const Scene &b)
              { return a.position < b.position; });

    // Paso 3: Transferir todos los elementos de 'source' a 'destination'
    destination.insert(destination.end(), source.begin(), source.end());

    // Paso 4: Limpiar 'source'
    source.clear();
}

void updateScenePosition(std::vector<Scene> &source, std::vector<int> specificParentId, int idTail, int newPosition) // Usar SOLO EN TEMP!!
{
    // Construir specificId concatenando specificParentId e idTail
    std::vector<int> specificId = specificParentId;
    specificId.push_back(idTail);

    for (auto &scene : source)
    {
        if (scene.parentId == specificParentId && scene.id == specificId)
        {
            scene.position = newPosition;
            // break; // Salir del bucle una vez que se actualice el elemento
        }

        // Paso 2: Ordenar el vector 'source' por la variable 'position' de menor a mayor
        for (size_t i = 0; i < source.size(); ++i)
        {
            for (size_t j = 0; j < source.size() - i - 1; ++j)
            {
                if (source[j].position > source[j + 1].position)
                {
                    // Intercambiar elementos
                    std::swap(source[j], source[j + 1]);
                }
            }
        }
    }
}

void removeScene(std::vector<Scene> &source, std::vector<int> specificParentId, int idTail) // Usar SOLO EN TEMP!!
{
    // Construir specificId concatenando specificParentId e idTail
    std::vector<int> specificId = specificParentId;
    specificId.push_back(idTail);

    // Utilizamos un iterador y std::remove_if para encontrar y eliminar el elemento
    source.erase(std::remove_if(source.begin(), source.end(),
                                [specificParentId, specificId](const Scene &scene)
                                {
                                    return scene.parentId == specificParentId && scene.id == specificId;
                                }),
                 source.end());
}

// TAKES
void transferTakes(std::vector<Take> &source, std::vector<Take> &destination, std::vector<int> specificParentId) // Transfiere todos los de un padre -> TEMP
{
    // Paso 1: Limpia el vector de destino antes de transferir los nuevos elementos
    destination.clear();

    // Paso 2: Ordenar el vector 'source' por la variable 'position' de menor a mayor
    std::sort(source.begin(), source.end(), [](const Take &a, const Take &b)
              { return a.position < b.position; });

    // Paso 3: Transferir todos los elementos de 'source' a 'destination'
    for (const auto &take : source)
    {
        if (take.parentId == specificParentId)
        {
            destination.push_back(take);
        }
    }
}

void updateTakes(std::vector<Take> &source, std::vector<Take> &destination, std::vector<int> specificParentId) // Transfiere todos los de un padre -> MAIN
{
    // Paso 1: Eliminar todos los elementos con el parentId específico de 'destination'
    destination.erase(std::remove_if(destination.begin(), destination.end(),
                                     [specificParentId](const Take &take)
                                     {
                                         return take.parentId == specificParentId;
                                     }),
                      destination.end());

    // Paso 2: Ordenar el vector 'source' por la variable 'position' de menor a mayor
    std::sort(source.begin(), source.end(), [](const Take &a, const Take &b)
              { return a.position < b.position; });

    // Paso 3: Transferir todos los elementos de 'source' a 'destination'
    destination.insert(destination.end(), source.begin(), source.end());

    // Paso 4: Limpiar 'source'
    source.clear();
}

void updateTakePosition(std::vector<Take> &source, std::vector<int> specificParentId, int idTail, int newPosition) // Usar SOLO EN TEMP!!
{
    // Construir specificId concatenando specificParentId e idTail
    std::vector<int> specificId = specificParentId;
    specificId.push_back(idTail);

    for (auto &take : source)
    {
        if (take.parentId == specificParentId && take.id == specificId)
        {
            take.position = newPosition;
            // break; // Salir del bucle una vez que se actualice el elemento
        }
    }

    // Paso 2: Ordenar el vector 'source' por la variable 'position' de menor a mayor
    for (size_t i = 0; i < source.size(); ++i)
    {
        for (size_t j = 0; j < source.size() - i - 1; ++j)
        {
            if (source[j].position > source[j + 1].position)
            {
                // Intercambiar elementos
                std::swap(source[j], source[j + 1]);
            }
        }
    }
}

void removeTake(std::vector<Take> &source, std::vector<int> specificParentId, int idTail) // Usar SOLO EN TEMP!!
{
    // Construir specificId concatenando specificParentId e idTail
    std::vector<int> specificId = specificParentId;
    specificId.push_back(idTail);

    // Utilizamos un iterador y std::remove_if para encontrar y eliminar el elemento
    source.erase(std::remove_if(source.begin(), source.end(),
                                [specificParentId, specificId](const Take &take)
                                {
                                    return take.parentId == specificParentId && take.id == specificId;
                                }),
                 source.end());
}

// USE CASES (ACTING AND OBJECT)
void transferUseCase(std::vector<Use_case> &source, std::vector<Use_case> &destination, std::vector<int> specificParentId) // Transfiere todos los de un padre -> TEMP
{
    // Paso 1: Limpia el vector de destino antes de transferir los nuevos elementos
    destination.clear();

    // Paso 2: Ordenar el vector 'source' por la variable 'position' de menor a mayor
    std::sort(source.begin(), source.end(), [](const Use_case &a, const Use_case &b)
              { return a.position < b.position; });

    // Paso 3: Transferir todos los elementos de 'source' a 'destination'
    for (const auto &use_case : source)
    {
        if (use_case.parentId == specificParentId)
        {
            destination.push_back(use_case);
        }
    }
}

void updateUse_Case(std::vector<Use_case> &source, std::vector<Use_case> &destination, std::vector<int> specificParentId) // Transfiere todos los de un padre -> MAIN
{
    // Paso 1: Eliminar todos los elementos con el parentId específico de 'destination'
    destination.erase(std::remove_if(destination.begin(), destination.end(),
                                     [specificParentId](const Use_case &use_case)
                                     {
                                         return use_case.parentId == specificParentId;
                                     }),
                      destination.end());

    // Paso 2: Ordenar el vector 'source' por la variable 'position' de menor a mayor
    std::sort(source.begin(), source.end(), [](const Use_case &a, const Use_case &b)
              { return a.position < b.position; });

    // Paso 3: Transferir todos los elementos de 'source' a 'destination'
    destination.insert(destination.end(), source.begin(), source.end());

    // Paso 4: Limpiar 'source'
    source.clear();
}

void updateUse_CasePosition(std::vector<Use_case> &source, std::vector<int> specificParentId, int idTail, int newPosition) // Usar SOLO EN TEMP!!
{
    // Construir specificId concatenando specificParentId e idTail
    std::vector<int> specificId = specificParentId;
    specificId.push_back(idTail);

    for (auto &use_case : source)
    {
        if (use_case.parentId == specificParentId && use_case.id == specificId)
        {
            use_case.position = newPosition;
            // break; // Salir del bucle una vez que se actualice el elemento
        }
    }

    // Paso 2: Ordenar el vector 'source' por la variable 'position' de menor a mayor
    for (size_t i = 0; i < source.size(); ++i)
    {
        for (size_t j = 0; j < source.size() - i - 1; ++j)
        {
            if (source[j].position > source[j + 1].position)
            {
                // Intercambiar elementos
                std::swap(source[j], source[j + 1]);
            }
        }
    }
}

void removeUse_Case(std::vector<Use_case> &source, std::vector<int> specificParentId, int idTail) // Usar SOLO EN TEMP!!
{
    // Construir specificId concatenando specificParentId e idTail
    std::vector<int> specificId = specificParentId;
    specificId.push_back(idTail);

    // Utilizamos un iterador y std::remove_if para encontrar y eliminar el elemento
    source.erase(std::remove_if(source.begin(), source.end(),
                                [specificParentId, specificId](const Use_case &use_case)
                                {
                                    return use_case.parentId == specificParentId && use_case.id == specificId;
                                }),
                 source.end());
}

// TECH USE
void transferTech_use(std::vector<Tech_use> &source, std::vector<Tech_use> &destination, std::vector<int> specificParentId) // Transfiere todos los de un padre -> TEMP
{
    // Paso 1: Limpia el vector de destino antes de transferir los nuevos elementos
    destination.clear();

    // Paso 2: Ordenar el vector 'source' por la variable 'position' de menor a mayor
    std::sort(source.begin(), source.end(), [](const Tech_use &a, const Tech_use &b)
              { return a.position < b.position; });

    // Paso 3: Transferir todos los elementos de 'source' a 'destination'
    for (const auto &tech_use : source)
    {
        if (tech_use.parentId == specificParentId)
        {
            destination.push_back(tech_use);
        }
    }
}

void updateTech_use(std::vector<Tech_use> &source, std::vector<Tech_use> &destination, std::vector<int> specificParentId) // Transfiere todos los de un padre -> MAIN
{
    // Paso 1: Eliminar todos los elementos con el parentId específico de 'destination'
    destination.erase(std::remove_if(destination.begin(), destination.end(),
                                     [specificParentId](const Tech_use &tech_use)
                                     {
                                         return tech_use.parentId == specificParentId;
                                     }),
                      destination.end());

    // Paso 2: Ordenar el vector 'source' por la variable 'position' de menor a mayor
    std::sort(source.begin(), source.end(), [](const Tech_use &a, const Tech_use &b)
              { return a.position < b.position; });

    // Paso 3: Transferir todos los elementos de 'source' a 'destination'
    destination.insert(destination.end(), source.begin(), source.end());

    // Paso 4: Limpiar 'source'
    source.clear();
}

void updateTech_usePosition(std::vector<Tech_use> &source, std::vector<int> specificParentId, int idTail, int newPosition) // Usar SOLO EN TEMP!!
{
    // Construir specificId concatenando specificParentId e idTail
    std::vector<int> specificId = specificParentId;
    specificId.push_back(idTail);

    for (auto &tech_use : source)
    {
        if (tech_use.parentId == specificParentId && tech_use.id == specificId)
        {
            tech_use.position = newPosition;
            // break; // Salir del bucle una vez que se actualice el elemento
        }
    }

    // Paso 2: Ordenar el vector 'source' por la variable 'position' de menor a mayor
    for (size_t i = 0; i < source.size(); ++i)
    {
        for (size_t j = 0; j < source.size() - i - 1; ++j)
        {
            if (source[j].position > source[j + 1].position)
            {
                // Intercambiar elementos
                std::swap(source[j], source[j + 1]);
            }
        }
    }
}

void removeTech_use(std::vector<Tech_use> &source, std::vector<int> specificParentId, int idTail) // Usar SOLO EN TEMP!!
{
    // Construir specificId concatenando specificParentId e idTail
    std::vector<int> specificId = specificParentId;
    specificId.push_back(idTail);

    // Utilizamos un iterador y std::remove_if para encontrar y eliminar el elemento
    source.erase(std::remove_if(source.begin(), source.end(),
                                [specificParentId, specificId](const Tech_use &tech_use)
                                {
                                    return tech_use.parentId == specificParentId && tech_use.id == specificId;
                                }),
                 source.end());
}

// EVENTS
void transferEvents(std::vector<Event> &source, std::vector<Event> &destination, std::vector<int> specificParentId) // Transfiere todos los de un padre -> TEMP
{
    // Paso 1: Limpia el vector de destino antes de transferir los nuevos elementos
    destination.clear();

    // Paso 2: Ordenar el vector 'source' por la variable 'position' de menor a mayor
    std::sort(source.begin(), source.end(), [](const Event &a, const Event &b)
              { return a.position < b.position; });

    // Paso 3: Transferir todos los elementos de 'source' a 'destination'
    for (const auto &event : source)
    {
        if (event.parentId == specificParentId)
        {
            destination.push_back(event);
        }
    }
}
// transferEvents(events, eventsTemp, specificParentId);

void updateEvents(std::vector<Event> &source, std::vector<Event> &destination, std::vector<int> specificParentId) // Transfiere todos los de un padre -> MAIN
{
    // Paso 1: Eliminar todos los elementos con el parentId específico de 'destination'
    destination.erase(std::remove_if(destination.begin(), destination.end(),
                                     [specificParentId](const Event &event)
                                     {
                                         return event.parentId == specificParentId;
                                     }),
                      destination.end());

    // Paso 2: Ordenar el vector 'source' por la variable 'position' de menor a mayor
    std::sort(source.begin(), source.end(), [](const Event &a, const Event &b)
              { return a.position < b.position; });

    // Paso 3: Transferir todos los elementos de 'source' a 'destination'
    destination.insert(destination.end(), source.begin(), source.end());

    // Paso 4: Limpiar 'source'
    source.clear();
}
// updateEvents(eventsTemp, events, specificParentId);

void updateEventPosition(std::vector<Event> &source, std::vector<int> specificParentId, int idTail, int newPosition) // Usar SOLO EN TEMP!!
{
    // Construir specificId concatenando specificParentId e idTail
    std::vector<int> specificId = specificParentId;
    specificId.push_back(idTail);

    for (auto &event : source)
    {
        if (event.parentId == specificParentId && event.id == specificId)
        {
            event.position = newPosition;
            // break; // Salir del bucle una vez que se actualice el elemento
        }
    }

    // Paso 2: Ordenar el vector 'source' por la variable 'position' de menor a mayor
    for (size_t i = 0; i < source.size(); ++i)
    {
        for (size_t j = 0; j < source.size() - i - 1; ++j)
        {
            if (source[j].position > source[j + 1].position)
            {
                // Intercambiar elementos
                std::swap(source[j], source[j + 1]);
            }
        }
    }
}
// updateEventsPosition(eventsTemp, specificParentId, specificId, newPosition);

void removeEvent(std::vector<Event> &source, std::vector<int> specificParentId, int idTail) // Usar SOLO EN TEMP!!
{
    // Construir specificId concatenando specificParentId e idTail
    std::vector<int> specificId = specificParentId;
    specificId.push_back(idTail);

    // Utilizamos un iterador y std::remove_if para encontrar y eliminar el elemento
    source.erase(std::remove_if(source.begin(), source.end(),
                                [specificParentId, specificId](const Event &event)
                                {
                                    return event.parentId == specificParentId && event.id == specificId;
                                }),
                 source.end());
}
// removeEvent(eventsTemp, specificParentId, specificId);

// CONTROLS CLASSES

class TitledTextBox : public wxPanel
{
public:
    TitledTextBox(wxWindow *parent, wxBoxSizer *sizer, int index, const wxString &text)
        : wxPanel(parent, wxID_ANY), textBox(nullptr), parentSizer(sizer), indexPosition(index)
    {
        wxStaticBox *staticBox = new wxStaticBox(this, wxID_ANY, "");
        wxBoxSizer *sizerLocal = new wxStaticBoxSizer(staticBox, wxVERTICAL);

        // Sizer horizontal para el título y los botones
        wxBoxSizer *titleSizer = new wxBoxSizer(wxHORIZONTAL);

        // Botón de colapsar
        wxButton *collapseButton = new wxButton(this, wxID_ANY, "-", wxDefaultPosition, wxSize(25, 25), wxBORDER_NONE); // L"\uf4f4"
        titleSizer->Add(collapseButton, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
        collapseButton->Bind(wxEVT_BUTTON, &TitledTextBox::OncollapseButtonClick, this);

        // Botón de arriba
        wxButton *upButton = new wxButton(this, wxID_ANY, L"\u2191", wxDefaultPosition, wxSize(25, 25), wxBORDER_NONE); // u2191 (flecha) // 06f8 (Arabe) // uf0aa // L"\uebff"
        titleSizer->Add(upButton, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
        upButton->Bind(wxEVT_BUTTON, &TitledTextBox::OnupButtonClick, this);

        // Botón de abajo
        wxButton *downButton = new wxButton(this, wxID_ANY, L"\u2193", wxDefaultPosition, wxSize(25, 25), wxBORDER_NONE); // u2193 (flecha) // u06f7 (Arabe) // uf0ab // L"\uebfc"
        titleSizer->Add(downButton, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
        downButton->Bind(wxEVT_BUTTON, &TitledTextBox::OndownButtonClick, this);

        // BARRA DE TITULO
        wxString title = wxString::Format("Fragmento %d", index);
        wxStaticText *titleLabel = new wxStaticText(this, wxID_ANY, title);
        titleSizer->Add(titleLabel, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);

        // Boton de edicion >
        wxButton *editButton = new wxButton(this, wxID_ANY, L"\u2192", wxDefaultPosition, wxSize(25, 25), wxBORDER_NONE); // u270F (Lapiz) // u2192 (flecha) // L"\uf044"
        titleSizer->Add(editButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
        editButton->Bind(wxEVT_BUTTON, &TitledTextBox::OneditButtonClick, this);

        // Boton de eliminar
        wxButton *deleteButton = new wxButton(this, wxID_ANY, "x", wxDefaultPosition, wxSize(25, 25), wxBORDER_NONE); // L"\uf52f"
        titleSizer->Add(deleteButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
        deleteButton->Bind(wxEVT_BUTTON, &TitledTextBox::OndeleteButtonClick, this);

        sizerLocal->Add(titleSizer, 0, wxEXPAND | wxBOTTOM, 7);

        // Crear un objeto wxFont con la fuente deseada
        // wxFont font(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "Symbols Nerd Font Mono");

        // Aplicar la fuente al botón
        // collapseButton->SetFont(font);
        // upButton->SetFont(font);
        // downButton->SetFont(font);
        // editButton->SetFont(font);
        // deleteButton->SetFont(font);

        // CUADRO DE TEXTO
        textBox = new wxTextCtrl(this, wxID_ANY, text,
                                 wxDefaultPosition, wxDefaultSize,
                                 wxTE_MULTILINE | wxTE_READONLY);

        // Establecer el cursor a la flecha estándar
        textBox->SetCursor(wxCURSOR_ARROW);
        // Bloquea la edicion (ya estaba bloqueada, es para que ande el scroll)
        textBox->SetEditable(false);
        // Manda todos los eventos de la rueda al controlador padre
        textBox->Bind(wxEVT_MOUSEWHEEL, &TitledTextBox::OnMouseWheel, this);

        sizerLocal->Add(textBox, 1, wxEXPAND | wxALL, 5);

        SetSizer(sizerLocal);

        // Añade los eventos de ratón
        Bind(wxEVT_MOTION, &TitledTextBox::OnMouseMove, this);
    }

    // wxTextCtrl *GetTextBox() const { return textBox; } /////////////////// NO IDEA !!!!!!!! ///////////

    void OncollapseButtonClick(wxCommandEvent &event)
    {
        textBox->Show(!textBox->IsShown());
        wxButton *btn = dynamic_cast<wxButton *>(event.GetEventObject());
        if (btn)
        {
            // Si el textBox ahora está mostrándose, cambia el símbolo a "-"
            // De lo contrario, cambia el símbolo a "+"
            //                                      -    :    +
            btn->SetLabel(textBox->IsShown() ? "-" : "+"); // L"\uf4f4" : L"\uf501"
        }
        Layout();
        dynamic_cast<wxScrolledWindow *>(GetParent())->FitInside();
    }

    void OnupButtonClick(wxCommandEvent &event)
    {
        desiredPosition = GetItemPositionInSizer(parentSizer, this) - 1;
        MoveToDesiredPosition();
        TextBoxPanelPosition = GetItemPositionInSizer(parentSizer, this);
    }

    void OndownButtonClick(wxCommandEvent &event)
    {
        desiredPosition = GetItemPositionInSizer(parentSizer, this) + 1;
        MoveToDesiredPosition();
        TextBoxPanelPosition = GetItemPositionInSizer(parentSizer, this);
    }

    void OneditButtonClick(wxCommandEvent &event)
    {
        // Implementa la lógica para este botón aquí
    }

    void OndeleteButtonClick(wxCommandEvent &event)
    {
        deleteVectorItem(textBoxsContainer, indexPosition);
        removeScene(scenes, tree, indexPosition);
        wxWindow *parentWindow = GetParent(); // Guardar referencia al padre antes de destruir
        Destroy();
        parentWindow->Layout(); // Llamar al Layout del padre
        if (wxDynamicCast(parentWindow, wxScrolledWindow))
        {
            dynamic_cast<wxScrolledWindow *>(parentWindow)->FitInside();
        }

        // ACTUALIZA POSICIONES DEL VECTOR A LOS ELEMENTOS DE LA CLASE
        for (int i = 0; i < textBoxsContainer.size(); ++i)
        {
            int specificId = textBoxsContainer[i]; // El id de cada elemento se recupera de textBoxsContainer uno por uno.
            int newPosition = i;                   // La posición en textBoxsContainer es la nueva posición (el i va iterando).

            // Actualizar la posición de la escena con specificId en el vector scenes
            updateScenePosition(scenes, tree, specificId, newPosition);
        }

        mod = true;
    }

    void OnMouseWheel(wxMouseEvent &event)
    {
        wxScrolledWindow *scrollingParent = wxDynamicCast(GetParent(), wxScrolledWindow);
        if (scrollingParent)
        {
            // Determina la dirección del desplazamiento
            int rotation = event.GetWheelRotation();
            int amount = event.GetLinesPerAction();

            if (rotation < 0)
            {
                scrollingParent->ScrollLines(amount); // Desplazar hacia abajo
            }
            else if (rotation > 0)
            {
                scrollingParent->ScrollLines(-amount); // Desplazar hacia arriba
            }
        }
    }

    void OnMouseMove(wxMouseEvent &event)
    {
        TextBoxPanelPosition = GetItemPositionInSizer(parentSizer, this) + 1;
        TextBoxIndexPosition = indexPosition;

        wxCommandEvent evta(wxEVT_UPDATE_INDEX_EVENT);
        evta.SetInt(TextBoxIndexPosition);
        wxPostEvent(GetParent(), evta);

        wxCommandEvent evtb(wxEVT_UPDATE_POSITION_EVENT);
        evtb.SetInt(TextBoxPanelPosition);
        wxPostEvent(GetParent(), evtb);

        if (event.Dragging() && event.LeftIsDown())
        {
            wxPoint currentPosition = GetParent()->ScreenToClient(wxGetMousePosition());

            for (size_t i = 0; i < parentSizer->GetItemCount(); i++)
            {
                wxRect childRect = parentSizer->GetItem(i)->GetWindow()->GetRect(); // Funciona sin getwindow ???

                // Detectar si estamos por encima o por debajo del elemento actual.
                if (currentPosition.y < childRect.y + childRect.height / 2)
                {
                    desiredPosition = i;
                    thresholdTop = childRect.y - childRect.height / 3;    // 33% de altura arriba.
                    thresholdBottom = childRect.y + childRect.height / 3; // 33% de altura abajo.

                    // Si la posición actual del ratón está por encima o por debajo de los umbrales, realiza el cambio.
                    if (currentPosition.y < thresholdTop || currentPosition.y > thresholdBottom) // FUNCIONA CON LIN Y MAC; DESHABILITAR CON WIN
                    {
                        MoveToDesiredPosition();
                    }
                    break;
                }
            }
        }
    }

    void MoveToDesiredPosition()
    {
        int count = parentSizer->GetItemCount();

        if (desiredPosition != -1 && desiredPosition < count)
        {
            int currentPos = GetItemPositionInSizer(parentSizer, this);

            if (desiredPosition != currentPos)
            {
                parentSizer->Detach(this);
                parentSizer->Insert(desiredPosition, this, 0, wxEXPAND | wxALL, 5);
                parentSizer->Layout();
                dynamic_cast<wxScrolledWindow *>(GetParent())->FitInside();
                // (indexPosition es en realidad el ID, desiredPosition es la posicion en relacion a los demás)
                updateElementPosition(textBoxsContainer, indexPosition, desiredPosition);
                // Actualiza la posicion de si mismo y de los demas dentro del vector

                mod = true;
            }
            desiredPosition = -1; // Resetea la posición deseada después de procesarla.
        }

        // ACTUALIZA POSICIONES DEL VECTOR A LOS ELEMENTOS DE LA CLASE
        for (int i = 0; i < textBoxsContainer.size(); ++i)
        {
            int specificId = textBoxsContainer[i]; // El id de cada elemento se recupera de textBoxsContainer uno por uno.
            int newPosition = i;                   // La posición en textBoxsContainer es la nueva posición (el i va iterando).

            // Actualizar la posición de la escena con specificId en el vector scenes
            updateScenePosition(scenes, tree, specificId, newPosition);
        }
    }

    int GetItemPositionInSizer(wxBoxSizer *sizer, wxWindow *window)
    {
        for (size_t i = 0; i < sizer->GetItemCount(); ++i)
        {
            if (sizer->GetItem(i)->GetWindow() == window)
            {
                return i;
            }
        }
        return -1; // No encontrado
    }

private:
    wxTextCtrl *textBox;
    wxBoxSizer *parentSizer;
    int desiredPosition = -1;
    int thresholdTop = -1;
    int thresholdBottom = -1;
    int indexPosition;
};

class ItemTextList : public wxPanel
{
public:
    ItemTextList(wxWindow *parent, wxBoxSizer *sizer, int index, const wxString &text)
        : wxPanel(parent, wxID_ANY), parentSizer(sizer), indexPosition(index)
    {
        wxBoxSizer *sizerLocal = new wxBoxSizer(wxVERTICAL);

        // Sizer horizontal para el título y los botones
        wxBoxSizer *titleSizer = new wxBoxSizer(wxHORIZONTAL);

        // Botón de arriba
        wxButton *upButton = new wxButton(this, wxID_ANY, L"\u2191", wxDefaultPosition, wxSize(25, 25), wxBORDER_NONE);
        titleSizer->Add(upButton, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
        upButton->Bind(wxEVT_BUTTON, &ItemTextList::OnupButtonClick, this);

        // Botón de abajo
        wxButton *downButton = new wxButton(this, wxID_ANY, L"\u2193", wxDefaultPosition, wxSize(25, 25), wxBORDER_NONE);
        titleSizer->Add(downButton, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
        downButton->Bind(wxEVT_BUTTON, &ItemTextList::OndownButtonClick, this);

        // BARRA DE TITULO
        // wxString title = wxString::Format("Fragmento %d", text);
        // wxString title = wxString::Format("Fragmento %s", text.c_str());
        wxString title = text;
        wxStaticText *titleLabel = new wxStaticText(this, wxID_ANY, title);
        titleSizer->Add(titleLabel, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);

        // Boton de eliminar
        wxButton *deleteButton = new wxButton(this, wxID_ANY, "x", wxDefaultPosition, wxSize(25, 25), wxBORDER_NONE);
        titleSizer->Add(deleteButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
        deleteButton->Bind(wxEVT_BUTTON, &ItemTextList::OndeleteButtonClick, this);

        sizerLocal->Add(titleSizer, 0, wxEXPAND);

        SetSizer(sizerLocal);

        // Añade los eventos de ratón
        Bind(wxEVT_MOTION, &ItemTextList::OnMouseMove, this);
    }

    void OnupButtonClick(wxCommandEvent &event)
    {
        desiredPosition = GetItemPositionInSizer(parentSizer, this) - 1;
        MoveToDesiredPosition();
        TextListPanelPosition = GetItemPositionInSizer(parentSizer, this);
    }

    void OndownButtonClick(wxCommandEvent &event)
    {
        desiredPosition = GetItemPositionInSizer(parentSizer, this) + 1;
        MoveToDesiredPosition();
        TextListPanelPosition = GetItemPositionInSizer(parentSizer, this);
    }

    void OndeleteButtonClick(wxCommandEvent &event)
    {
        deleteVectorItem(itemsListContainer, indexPosition);
        removeTake(takes, tree, indexPosition);
        wxWindow *parentWindow = GetParent(); // Guardar referencia al padre antes de destruir
        Destroy();
        parentWindow->Layout(); // Llamar al Layout del padre
        if (wxDynamicCast(parentWindow, wxScrolledWindow))
        {
            dynamic_cast<wxScrolledWindow *>(parentWindow)->FitInside();
        }

        // ACTUALIZA POSICIONES DEL VECTOR A LOS ELEMENTOS DE LA CLASE
        for (int i = 0; i < itemsListContainer.size(); ++i)
        {
            int specificId = itemsListContainer[i]; // El id de cada elemento se recupera de textBoxsContainer uno por uno.
            int newPosition = i;                    // La posición en textBoxsContainer es la nueva posición (el i va iterando).

            // Actualizar la posición de la escena con specificId en el vector takes
            updateTakePosition(takes, tree, specificId, newPosition);
        }

        mod = true;
    }

    void OnMouseMove(wxMouseEvent &event)
    {
        TextListPanelPosition = GetItemPositionInSizer(parentSizer, this) + 1;
        TextListIndexPosition = indexPosition;

        wxCommandEvent evta(wxEVT_UPDATE_INDEX_EVENT);
        evta.SetInt(TextListIndexPosition);
        wxPostEvent(GetParent(), evta);

        wxCommandEvent evtb(wxEVT_UPDATE_POSITION_EVENT);
        evtb.SetInt(TextListPanelPosition);
        wxPostEvent(GetParent(), evtb);

        if (event.Dragging() && event.LeftIsDown())
        {
            wxPoint currentPosition = GetParent()->ScreenToClient(wxGetMousePosition());

            for (size_t i = 0; i < parentSizer->GetItemCount(); i++)
            {
                wxRect childRect = parentSizer->GetItem(i)->GetWindow()->GetRect(); // Funciona sin getwindow ???

                // Detectar si estamos por encima o por debajo del elemento actual.
                if (currentPosition.y < childRect.y + childRect.height / 2)
                {
                    desiredPosition = i;
                    thresholdTop = childRect.y - childRect.height / 3;    // 33% de altura arriba.
                    thresholdBottom = childRect.y + childRect.height / 3; // 33% de altura abajo.

                    // Si la posición actual del ratón está por encima o por debajo de los umbrales, realiza el cambio.
                    if (currentPosition.y < thresholdTop || currentPosition.y > thresholdBottom) // FUNCIONA CON LIN Y MAC; DESHABILITAR CON WIN
                    {
                        MoveToDesiredPosition();
                    }
                    break;
                }
            }
        }
    }

    void MoveToDesiredPosition()
    {
        int count = parentSizer->GetItemCount();

        if (desiredPosition != -1 && desiredPosition < count)
        {
            int currentPos = GetItemPositionInSizer(parentSizer, this);

            if (desiredPosition != currentPos)
            {
                parentSizer->Detach(this);
                parentSizer->Insert(desiredPosition, this, 0, wxEXPAND | wxALL, 5);
                parentSizer->Layout();
                dynamic_cast<wxScrolledWindow *>(GetParent())->FitInside();
                // (indexPosition es en realidad el ID, desiredPosition es la posicion en relacion a los demás)
                updateElementPosition(itemsListContainer, indexPosition, desiredPosition);
                // Actualiza la posicion de si mismo y de los demas dentro del vector

                mod = true;
            }
            desiredPosition = -1; // Resetea la posición deseada después de procesarla.
        }

        // ACTUALIZA POSICIONES DEL VECTOR A LOS ELEMENTOS DE LA CLASE
        for (int i = 0; i < itemsListContainer.size(); ++i)
        {
            int specificId = itemsListContainer[i]; // El id de cada elemento se recupera de textBoxsContainer uno por uno.
            int newPosition = i;                    // La posición en textBoxsContainer es la nueva posición (el i va iterando).

            // Actualizar la posición de la escena con specificId en el vector takes
            updateTakePosition(takes, tree, specificId, newPosition);
        }
    }

    int GetItemPositionInSizer(wxBoxSizer *sizer, wxWindow *window)
    {
        for (size_t i = 0; i < sizer->GetItemCount(); ++i)
        {
            if (sizer->GetItem(i)->GetWindow() == window)
            {
                return i;
            }
        }
        return -1; // No encontrado
    }

private:
    wxBoxSizer *parentSizer;
    int desiredPosition = -1;
    int thresholdTop = -1;
    int thresholdBottom = -1;
    int indexPosition;
};

class MainWindow : public wxFrame
{
public:
    MainWindow(const wxString &title, const wxPoint &pos, const wxSize &size)
        : wxFrame(NULL, wxID_ANY, title, pos, size), leftTextBox(nullptr)
    {
        SetBackgroundColour(wxColour(255, 255, 255));
        // MENU PROYECTO
        wxMenu *menuProject = new wxMenu;
        // nombreMenu->añadir(EVENTO, "nombreItem\KeyShortcut", "mssg to statusbar")//
        // menuProject->Append(ID_Hello, "&Hello...\tCtrl-H", "Hello mssg");
        // El comentario en la statusbar "Hello mssg" obedece a la primra definicion del evento
        // (si el mismo evento "ID_Hello" esta varias veces aparece el primer comentario que lo define y no obedece a los siguientes)
        // menuProject->AppendSeparator();
        menuProject->Append(wxID_NEW, "&Nuevo...", "Nuevo proyecto");
        menuProject->Append(wxID_OPEN, "&Abrir...", "Abrir proyecto");
        menuProject->Append(wxID_SAVE, "&Guardar...", "Guardar proyecto");
        menuProject->Append(ID_SAVE_AS, "&Guardar como...\tCtrl-K", "Guardar proyecto como"); /// FUNCION TEMPORAL!!!
        menuProject->Append(wxID_CLOSE, "&Cerrar...", "Cerrar proyecto");
        menuProject->Append(ID_EXIT, "&Salir...\tCtrl-Q", "Salir de CineDoc");

        // MENU GUION
        wxMenu *menuScript = new wxMenu;
        menuScript->Append(ID_SCRIPT_NEW, "&Nuevo...", "Nuevo guión");
        menuScript->Append(ID_SCRIPT_EDIT, "&Editar...", "Editar guión");
        menuScript->Append(ID_SCRIPT_DEL, "&Eliminar...", "Eliminar guión");

        // MENU PERSONAJE
        wxMenu *menuCharacter = new wxMenu;
        menuCharacter->Append(ID_SCRIPT_NEW, "&Nuevo...", "Nuevo personaje");
        menuCharacter->Append(ID_SCRIPT_EDIT, "&Editar...", "Editar personaje");
        menuCharacter->Append(ID_SCRIPT_DEL, "&Eliminar...", "Eliminar personaje");

        // MENU ACTOR
        wxMenu *menuActor = new wxMenu;
        menuActor->Append(ID_SCRIPT_NEW, "&Nuevo...", "Nuevo actor");
        menuActor->Append(ID_SCRIPT_EDIT, "&Editar...", "Editar actor");
        menuActor->Append(ID_SCRIPT_DEL, "&Eliminar...", "Eliminar actor");

        // MENU LOCACION
        wxMenu *menuLocation = new wxMenu;
        menuLocation->Append(ID_SCRIPT_NEW, "&Nuevo...", "Nueva locación");
        menuLocation->Append(ID_SCRIPT_EDIT, "&Editar...", "Editar locación");
        menuLocation->Append(ID_SCRIPT_DEL, "&Eliminar...", "Eliminar locación");

        // MENU OBJETO
        wxMenu *menuObject = new wxMenu;
        menuObject->Append(ID_SCRIPT_NEW, "&Nuevo...", "Nuevo objeto");
        menuObject->Append(ID_SCRIPT_EDIT, "&Editar...", "Editar objeto");
        menuObject->Append(ID_SCRIPT_DEL, "&Eliminar...", "Eliminar objeto");

        // MENU AYUDA
        wxMenu *menuHelp = new wxMenu;
        menuHelp->Append(ID_HELP, "&Ayuda...\tCtrl-H", "Buscar ayuda");
        menuHelp->Append(wxID_ABOUT, "Acerca de...", "Acerca de CineDoc");

        // CARGA MENUES
        wxMenuBar *menuBar = new wxMenuBar;
        menuBar->Append(menuProject, "&Proyecto");
        menuBar->Append(menuScript, "&Guion");
        menuBar->Append(menuCharacter, "&Personaje");
        menuBar->Append(menuActor, "&Actor");
        menuBar->Append(menuLocation, "&Locacion");
        menuBar->Append(menuObject, "&Objeto");
        menuBar->Append(menuHelp, "&Ayuda");
        SetMenuBar(menuBar);

        CreateStatusBar();
        SetStatusText("Bienvenido a CineDoc!");

        // Principal (HORIZONTAL)
        wxBoxSizer *mainSizer = new wxBoxSizer(wxHORIZONTAL);

        ///////////////////////// PARTE IZQUIERDA /////////////////////////
        wxBoxSizer *leftSizer = new wxBoxSizer(wxVERTICAL);
        wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);

        wxString choices[] = {wxT("Opción 1"), wxT("Opción 2"), wxT("Opción 3")}; // wxT("cadena") forza a tomar como unicode el string cadena

        // itemSelector = new wxComboBox(this, wxID_ANY, "Opción 1", wxDefaultPosition, wxDefaultSize,
        // 3, choices, wxCB_DROPDOWN | wxCB_READONLY, wxDefaultValidator, "itemSelector");
        // En este ejemplo "Opción 1" será la opción seleccionada inicialmente en el wxComboBox.

        // itemSelector->SetValue(wxT("Opción 1")); // Para cambiar posteriormente la opcion marcada por default

        itemSelector = new wxComboBox(this, wxID_ANY, wxT("Opción 1"), wxDefaultPosition, wxDefaultSize,
                                      3, choices, wxCB_DROPDOWN | wxCB_READONLY,
                                      wxDefaultValidator, "itemSelector");

        // itemSelector->SetValue(wxT("Opción 3"));

        leftSizer->Add(itemSelector, 0, wxEXPAND | wxALL, 5);

        leftTextBox = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
                                     wxDefaultPosition, wxDefaultSize,
                                     wxTE_MULTILINE, wxDefaultValidator, "leftTextBox");
        leftSizer->Add(leftTextBox, 1, wxEXPAND | wxALL, 5);

        wxButton *backButton = new wxButton(this, wxID_ANY, "Volver");
        backButton->Bind(wxEVT_BUTTON, &MainWindow::OnBackButtonClicked, this);
        buttonSizer->Add(backButton, 1, wxEXPAND | wxRIGHT, 5);

        wxButton *addButton = new wxButton(this, wxID_ANY, "Agregar");
        addButton->Bind(wxEVT_BUTTON, &MainWindow::OnAddButtonClicked, this);
        buttonSizer->Add(addButton, 2, wxEXPAND, 0);

        leftSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 5);
        mainSizer->Add(leftSizer, 1, wxEXPAND | wxALL, 5);

        ///////////////////////// PARTE CENTRAL /////////////////////////

        // Crear un wxStaticBox con el título "Escenas"
        wxStaticBox *bigBox = new wxStaticBox(this, wxID_ANY, "Escenas");
        // bigBox->SetSize(100, 800);

        // Crear un sizer vertical para los controles dentro del wxStaticBox
        wxStaticBoxSizer *bigSizer = new wxStaticBoxSizer(bigBox, wxVERTICAL);

        /// PANEL CONTENEDOR
        containerPanel = new wxScrolledWindow(bigBox, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE, "containerPanel");
        containerPanel->SetScrollRate(0, 10); // 0 en la dirección x (horizontal) y 10 en la dirección y (vertical).

        containerSizer = new wxBoxSizer(wxVERTICAL);
        containerPanel->SetSizer(containerSizer);

        bigSizer->Add(containerPanel, 1, wxEXPAND | wxALL, 5);
        mainSizer->Add(bigSizer, 1, wxEXPAND | wxALL, 5); ////////////////////////////////////

        ///////////////////////// PARTE DERECHA /////////////////////////

        // Crear un wxStaticBox con el título "Propiedades"
        wxStaticBox *propertiesBox = new wxStaticBox(this, wxID_ANY, "Propiedades");
        // propertiesBox->SetSize(100, 500);

        // Crear un sizer vertical para los controles dentro del wxStaticBox
        wxStaticBoxSizer *propertiesSizer = new wxStaticBoxSizer(propertiesBox, wxVERTICAL);

        // Crear itemIndexTextBox y añadirlo al propertiesSizer
        itemIndexTextBox = new wxTextCtrl(propertiesBox, wxID_ANY, wxString::Format(wxT("%d"), 0), // 0 Es el valor inicial!!!
                                          wxDefaultPosition, wxSize(200, -1), 0);

        propertiesSizer->Add(itemIndexTextBox, 0, wxEXPAND | wxALL, 5);

        // Crear itemPositionTextBox y añadirlo al propertiesSizer
        itemPositionTextBox = new wxTextCtrl(propertiesBox, wxID_ANY, wxString::Format(wxT("%d"), 0), // 0 Es el valor inicial!!!
                                             wxDefaultPosition, wxSize(200, -1), 0);

        propertiesSizer->Add(itemPositionTextBox, 0, wxEXPAND | wxALL, 5);

        // TIPO ESCENA (INT /EXT)
        wxString types[] = {wxT("EXT"), wxT("INT")}; // wxT("cadena") forza a tomar como unicode el string cadena
        typeSelector = new wxComboBox(propertiesBox, wxID_ANY, wxT("EXT"), wxDefaultPosition, wxDefaultSize,
                                      2, types, wxCB_DROPDOWN | wxCB_READONLY,
                                      wxDefaultValidator, "typeSelector");
        propertiesSizer->Add(typeSelector, 0, wxEXPAND | wxALL, 5);

        // TIEMPO ESCENA (DAY /NIGHT)
        wxString times[] = {wxT("DAY"), wxT("NIGHT")}; // wxT("cadena") forza a tomar como unicode el string cadena
        timeSelector = new wxComboBox(propertiesBox, wxID_ANY, wxT("DAY"), wxDefaultPosition, wxDefaultSize,
                                      2, times, wxCB_DROPDOWN | wxCB_READONLY,
                                      wxDefaultValidator, "timeSelector");
        propertiesSizer->Add(timeSelector, 0, wxEXPAND | wxALL, 5);

        // LOCACIONES
        wxString locations[] = {wxT("HOME"), wxT("CAR")}; // wxT("cadena") forza a tomar como unicode el string cadena
        locationSelector = new wxComboBox(propertiesBox, wxID_ANY, wxT("HOME"), wxDefaultPosition, wxDefaultSize,
                                          2, locations, wxCB_DROPDOWN | wxCB_READONLY,
                                          wxDefaultValidator, "timeSelector");
        propertiesSizer->Add(locationSelector, 0, wxEXPAND | wxALL, 5);

        // CAJA ELEMENTOS
        wxStaticBox *miniBox = new wxStaticBox(propertiesBox, wxID_ANY, "Items");
        // Crear un sizer vertical para los controles dentro del wxStaticBox
        wxStaticBoxSizer *miniSizer = new wxStaticBoxSizer(miniBox, wxVERTICAL);

        /// PANEL CONTENEDOR
        itemsPanel = new wxScrolledWindow(miniBox, wxID_ANY, wxDefaultPosition, wxSize(190, 200), wxBORDER_NONE, "itemsPanel");
        itemsPanel->SetScrollRate(0, 10); // 0 en la dirección x (horizontal) y 10 en la dirección y (vertical).

        itemsSizer = new wxBoxSizer(wxVERTICAL);
        itemsPanel->SetSizer(itemsSizer);

        miniSizer->Add(itemsPanel, 1, wxALL, 5);
        propertiesSizer->Add(miniSizer, 1, wxALL, 5);

        mainSizer->Add(propertiesSizer, 0, wxALL | wxEXPAND, 5); ////////////////////////////////////

        SetSizer(mainSizer);

        Bind(wxEVT_UPDATE_POSITION_EVENT, &MainWindow::OnUpdatePositionEvent, this);
        Bind(wxEVT_UPDATE_INDEX_EVENT, &MainWindow::OnUpdateIndexEvent, this);
    }

    void OnAddButtonClicked(wxCommandEvent &event)
    {
        if (!leftTextBox)
        {
            wxMessageBox("No se pudo obtener el cuadro de texto a la izquierda.", "Error", wxOK | wxICON_ERROR);
            return;
        }

        wxString selectedText = leftTextBox->GetStringSelection();

        if (!containerPanel)
        {
            wxMessageBox("No se pudo obtener el contenedor de los cuadros de texto.", "Error", wxOK | wxICON_ERROR);
            return;
        }

        if (!containerSizer)
        {
            wxMessageBox("El contenedor no tiene un sizer asociado.", "Error", wxOK | wxICON_ERROR);
            return;
        }

        nextNumber = firstEmpty(textBoxsContainer); // DE LA LISTA DE IDS, EL SIGUIENTE DISPONIBLE (EL ULTIMO O EL PRIMER HUECO VACIO)
        lastNumber = lastEmpty(textBoxsContainer);  // LA POSICION DENTRO DEL PANEL MAS ALLA DE SU ID (SIEMPRE SE AGREGA AL FINAL)

        TitledTextBox *newTitledTextBox = new TitledTextBox(containerPanel, containerSizer, nextNumber, selectedText);
        textBoxsContainer.push_back(nextNumber);
        containerSizer->Add(newTitledTextBox, 0, wxEXPAND | wxALL, 5);

        // nextNumber es la posicion dentro del arreglo (el que va a ser su id); tree es el parentId
        // lastNumber es la posicion dentro del contenedor, al final (por que last se inserta siempre al final)
        Scene newScene(nextNumber, tree, selectedText.ToStdString(), lastNumber);
        scenes.push_back(newScene);

        containerSizer->Layout();
        containerPanel->Layout();
        // containerPanel->SetVirtualSize(containerSizer->GetMinSize());
        containerPanel->FitInside(); // Esta funcion reemplaza a la linea de arriba

        mod = true;
    }

    void OnBackButtonClicked(wxCommandEvent &event)
    {
        if (!leftTextBox)
        {
            wxMessageBox("No se pudo obtener el cuadro de texto a la izquierda.", "Error", wxOK | wxICON_ERROR);
            return;
        }

        wxString selectedText = leftTextBox->GetStringSelection();

        if (!itemsPanel)
        {
            wxMessageBox("No se pudo obtener el contenedor de los cuadros de texto.", "Error", wxOK | wxICON_ERROR);
            return;
        }

        if (!itemsSizer)
        {
            wxMessageBox("El contenedor no tiene un sizer asociado.", "Error", wxOK | wxICON_ERROR);
            return;
        }

        nextNumber = firstEmpty(itemsListContainer); // DE LA LISTA DE IDS, EL SIGUIENTE DISPONIBLE (EL ULTIMO O EL PRIMER HUECO VACIO)
        lastNumber = lastEmpty(itemsListContainer);  // LA POSICION DENTRO DEL PANEL MAS ALLA DE SU ID (SIEMPRE SE AGREGA AL FINAL)

        ItemTextList *newItemTextList = new ItemTextList(itemsPanel, itemsSizer, nextNumber, selectedText);
        itemsListContainer.push_back(nextNumber);
        itemsSizer->Add(newItemTextList, 0, wxEXPAND | wxALL, 5);

        // nextNumber es la posicion dentro del arreglo (el que va a ser su id); tree es el parentId
        // lastNumber es la posicion dentro del contenedor, al final (por que last se inserta siempre al final)
        Take newTake(nextNumber, tree, selectedText.ToStdString(), lastNumber);
        takes.push_back(newTake);

        itemsSizer->Layout();
        itemsPanel->Layout();
        // containerPanel->SetVirtualSize(containerSizer->GetMinSize());
        itemsPanel->FitInside(); // Esta funcion reemplaza a la linea de arriba

        mod = true;
    }

    void OnUpdatePositionEvent(wxCommandEvent &event);
    void OnUpdateIndexEvent(wxCommandEvent &event);

private:
    wxString filename;
    wxComboBox *itemSelector;
    wxComboBox *typeSelector;
    wxComboBox *timeSelector;
    wxComboBox *locationSelector;
    wxTextCtrl *leftTextBox;
    wxScrolledWindow *containerPanel;
    wxScrolledWindow *itemsPanel;
    wxBoxSizer *containerSizer;
    wxBoxSizer *itemsSizer;
    wxTextCtrl *itemIndexTextBox;
    wxTextCtrl *itemPositionTextBox;
    int nextNumber;
    int lastNumber;

    // File menu Events
    void OnNewFile(wxCommandEvent &event);
    void OnOpenFile(wxCommandEvent &event);
    void OnSaveFile(wxCommandEvent &event);
    void OnSaveAsFile(wxCommandEvent &event);
    void OnCloseFile(wxCommandEvent &event);
    void writeFile(const wxString &filename);
    void OnExit(wxCommandEvent &event);

    // Script Menu Events
    void OnNewScript(wxCommandEvent &event);
    void OnScriptEdit(wxCommandEvent &event);
    void OnScriptDel(wxCommandEvent &event);

    //
    void OnAbout(wxCommandEvent &event);
    wxDECLARE_EVENT_TABLE();
};

class MyApp : public wxApp
{
public:
    virtual bool OnInit()
    {
        // Crear ventana proncipal "frame"
        MainWindow *frame = new MainWindow("CineDoc", wxPoint(50, 50), wxSize(1000, 700));

        // Establecer el tamaño mínimo de la ventana
        frame->SetMinSize(wxSize(1000, 700));

        frame->Show(true);
        return true;
    }
};

wxDEFINE_EVENT(wxEVT_UPDATE_POSITION_EVENT, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_UPDATE_INDEX_EVENT, wxCommandEvent);

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)

    EVT_MENU(wxID_NEW, MainWindow::OnNewFile)
        EVT_MENU(wxID_OPEN, MainWindow::OnOpenFile)
            EVT_MENU(wxID_SAVE, MainWindow::OnSaveFile)
                EVT_MENU(ID_SAVE_AS, MainWindow::OnSaveAsFile)
                    EVT_MENU(wxID_CLOSE, MainWindow::OnCloseFile)
                        EVT_MENU(ID_EXIT, MainWindow::OnExit)

                            EVT_MENU(ID_SCRIPT_NEW, MainWindow::OnNewScript)
                                EVT_MENU(ID_SCRIPT_EDIT, MainWindow::OnScriptEdit)
                                    EVT_MENU(ID_SCRIPT_DEL, MainWindow::OnScriptDel)

                                        EVT_MENU(ID_HELP, MainWindow::OnAbout)
                                            EVT_MENU(wxID_ABOUT, MainWindow::OnAbout)
                                                wxEND_EVENT_TABLE()
                                                    wxIMPLEMENT_APP(MyApp);

// FILE MENU
void MainWindow::OnNewFile(wxCommandEvent &event)
{
    // GUARDAR PRIMERO SI ES QUE HABIA ALGO
    wxCommandEvent closeEvent;
    OnCloseFile(closeEvent);

    mod = true;
    opf = false;
}

void MainWindow::OnOpenFile(wxCommandEvent &event)
{
    // GUARDAR PRIMERO SI ES QUE HABIA ALGO
    wxCommandEvent closeEvent;
    OnCloseFile(closeEvent);

    wxString defaultDir = wxEmptyString; // Directorio inicial (puede ser wxEmptyString para usar el directorio actual)
    wxString wildcard = "Archivos de CineDoc (*.cdc)|*.cdc|Todos los archivos (*.*)|*.*";

    wxFileDialog openFileDialog(this, "Abrir archivo", defaultDir, wxEmptyString, wildcard, wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (openFileDialog.ShowModal() == wxID_CANCEL)
    {
        // El usuario canceló la operación, no se seleccionó ningún archivo
        return;
    }

    // El usuario seleccionó un archivo, actualiza la variable filename
    filename = openFileDialog.GetPath();

    // Se llama a la futura funcion openFile();

    mod = false;
    opf = true;
}

void MainWindow::OnSaveFile(wxCommandEvent &event)
{
    if (!opf) // Si NO hay un achivo abierto, preguntar el nombre
    {
        wxString defaultDir = wxEmptyString; // Directorio inicial (puede ser wxEmptyString para usar el directorio actual)
        wxString wildcard = "Archivos de CineDoc (*.cdc)|*.cdc|Todos los archivos (*.*)|*.*";

        wxFileDialog saveFileDialog(this, "Guardar archivo", defaultDir, wxEmptyString, wildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

        if (saveFileDialog.ShowModal() == wxID_CANCEL)
        {
            // El usuario canceló la operación, no se guardará ningún archivo
            return;
        }

        // El usuario seleccionó un archivo, actualiza la variable filename
        filename = saveFileDialog.GetPath();

        writeFile(filename);

        mod = false;
        opf = true;
    }

    if (opf) // Si SI hay un achivo abierto, escribir directamente
    {
        writeFile(filename);

        mod = false;
    }
}

void MainWindow::OnSaveAsFile(wxCommandEvent &event)
{

    wxString defaultDir = wxEmptyString; // Directorio inicial (puede ser wxEmptyString para usar el directorio actual)
    wxString wildcard = "Archivos de CineDoc (*.cdc)|*.cdc|Todos los archivos (*.*)|*.*";

    wxFileDialog saveFileDialog(this, "Guardar archivo", defaultDir, wxEmptyString, wildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (saveFileDialog.ShowModal() == wxID_CANCEL)
    {
        // El usuario canceló la operación, no se guardará ningún archivo
        return;
    }

    // El usuario seleccionó un archivo, actualiza la variable filename
    filename = saveFileDialog.GetPath();

    writeFile(filename);

    mod = false;
    opf = true;
}

void MainWindow::OnCloseFile(wxCommandEvent &event)
{
    if (mod)
    {
        int response = wxMessageBox("Hay cambios sin guardar en el proyecto!", "Guardar cambios", wxYES_NO | wxICON_QUESTION);

        if (response == wxYES)
        {
            if (filename.IsEmpty()) // SI NO SE GUARDO ANTES PREGUNTAR EL NOMBRE
            {
                wxString defaultDir = wxEmptyString; // Directorio inicial (puede ser wxEmptyString para usar el directorio actual)
                wxString wildcard = "Archivos de CineDoc (*.cdc)|*.cdc|Todos los archivos (*.*)|*.*";

                wxFileDialog saveFileDialog(this, "Guardar archivo", defaultDir, wxEmptyString, wildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

                if (saveFileDialog.ShowModal() == wxID_CANCEL)
                {
                    // El usuario canceló la operación, no se guardará ningún archivo, volver antes
                    return;
                }

                filename = saveFileDialog.GetPath();
            }

            // CON EL NUEVO NOMBRE O EL QUE YA TENIA ESCRIBIR
            writeFile(filename); // Se guarda el archivo

            // Se resetean las variables (YA SE GUARDO)
            mod = false;
            opf = false;
            filename = "";
        }

        else if (response == wxNO)
        {
            // Se resetean las variables (SE DESCARTAN LOS CAMBIOS)
            mod = false;
            opf = false;
            filename = "";
        }
        // Si el usuario cierra la ventana de mensaje, no se realiza ninguna acción adicional
    }
}

void MainWindow::OnExit(wxCommandEvent &event)
{
    if (mod)
    {
        int response = wxMessageBox("¿Deseas guardar los cambios?", "Guardar cambios", wxYES_NO | wxICON_QUESTION);

        if (response == wxYES)
        {
            if (filename.IsEmpty()) // SI NO SE GUARDO ANTES PREGUNTAR EL NOMBRE
            {
                wxString defaultDir = wxEmptyString; // Directorio inicial (puede ser wxEmptyString para usar el directorio actual)
                wxString wildcard = "Archivos de CineDoc (*.cdc)|*.cdc|Todos los archivos (*.*)|*.*";

                wxFileDialog saveFileDialog(this, "Guardar archivo", defaultDir, wxEmptyString, wildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

                if (saveFileDialog.ShowModal() == wxID_CANCEL)
                {
                    // El usuario canceló la operación, no se guardará ningún archivo, volver antes
                    return;
                }

                filename = saveFileDialog.GetPath();
            }

            // CON EL NUEVO NOMBRE O EL QUE YA TENIA ESCRIBIR
            writeFile(filename); // Se guarda el archivo
            Close(true);
        }

        else if (response == wxNO)
        {
            // No se desea guardar los cambios, se cierra
            Close(true);
        }
    }

    else
    {
        Close(true);
    }

    // wxCommandEvent closeEvent;
    // OnCloseFile(closeEvent);

    // Close(true);
}

void MainWindow::writeFile(const wxString &filename)
{
    std::ofstream out_fs(filename.ToStdString()); // Convierte wxString a std::string
    boost::archive::text_oarchive outArchive(out_fs);
    outArchive << scripts;
    outArchive << scenes;
    outArchive << takes;
    out_fs.close();
}

//// UDMR EVENTS ////
// SCRIPT MENU
void MainWindow::OnNewScript(wxCommandEvent &event)
{
    wxTextEntryDialog dialog(NULL, wxT("Ingrese el título del guión:"), wxT("Nuevo Guión")); // Prompt / Titulo Ventana

    // Mostrar el cuadro de diálogo y obtener el resultado
    if (dialog.ShowModal() == wxID_OK)
    {
        wxString title = dialog.GetValue();

        if (!checkTitleExists(scripts, title.ToStdString()))
        {
            nextNumber = firstEmpty(scriptsArray);
            Script newScript({nextNumber}, title.ToStdString(), title.ToStdString());
            scripts.push_back(newScript);
        }

        else
        {
            wxMessageBox("Ese nombre ya existe",              // CONTENIDO VENTANA POP UP
                         "Error", wxOK | wxICON_INFORMATION); // TITULO VENTANA POP UP
        }

        // Aquí puedes realizar las verificaciones adicionales con el título ingresado
        // ...
    }

    /*     wxMessageBox("Test",                                          // CONTENIDO VENTANA POP UP
                     "Crear nuevo guion", wxOK | wxICON_INFORMATION); // TITULO VENTANA POP UP
        SetStatusText("StatusBar overide");
        // wxLogMessage("Hello world from wxWidgets!"); // VENTANA CON TITULO GENERICO "MAIN INFORMATION" */
}

void MainWindow::OnScriptEdit(wxCommandEvent &event)
{
}

void MainWindow::OnScriptDel(wxCommandEvent &event)
{
}

//// CONTROL MSSG EVENTS ////

void MainWindow::OnUpdatePositionEvent(wxCommandEvent &event)
{
    // wxLogMessage(wxT("Evento Position corriendo"));
    int receivedNumber = event.GetInt();
    //(wxString::Format(wxT("Posición recibida: %d"), receivedNumber));
    itemPositionTextBox->SetValue(wxString::Format(wxT("%d"), receivedNumber));
    // itemPositionTextBox->Refresh();
    // itemPositionTextBox->Update();
}

void MainWindow::OnUpdateIndexEvent(wxCommandEvent &event)
{
    // wxLogMessage(wxT("Evento Position corriendo"));
    int receivedNumber = event.GetInt();
    //(wxString::Format(wxT("Posición recibida: %d"), receivedNumber));
    itemIndexTextBox->SetValue(wxString::Format(wxT("%d"), receivedNumber));
    // itemIndexTextBox->Refresh();
    // itemIndexTextBox->Update();
}

// HELP MENU

void MainWindow::OnAbout(wxCommandEvent &event)
{
    wxMessageBox("This is CineDoc: An C++ and wxWidgets multiplattform App", // CONTENIDO VENTANA POP UP
                 "About CineDoc", wxOK | wxICON_INFORMATION);                // TITULO VENTANA POP UP
}