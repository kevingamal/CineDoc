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
#include <wx/datectrl.h>
#include <wx/dateevt.h>
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
    ID_CHARACTER_NEW = 7,
    ID_CHARACTER_EDIT = 8,
    ID_CHARACTER_DEL = 9,
    ID_ACTOR_NEW = 10,
    ID_ACTOR_EDIT = 11,
    ID_ACTOR_DEL = 12,
    ID_ADDRESS_NEW = 13,
    ID_ADDRESS_EDIT = 14,
    ID_ADDRESS_DEL = 15,
    ID_Hello = 16,
    // ID_ABOUT = 11,
    ID_HELP = 17
};

std::vector<int> tree; // Vector para almacenar la sucesion de parentId actual
// (Siempre en el ultimo elemento) <- Se añade al bajar, se borra al subir y se edita al cambiar

// Para crear un nuevo guion se le pregunta firstEmpty a scriptsArray y se reemplaza el elemento en la posicion 0 con ese numero
std::vector<int> scriptsArray;  // Vector para almacenar temporalmente los scrips (guiones) y reciclar los antiguos
std::vector<int> scenesArray;   // Vector para almacenar temporalmente las scenes (escenas) y reciclar los antiguos
std::vector<int> takesArray;    // Vector para almacenar temporalmente los takes (tomas) y reciclar los antiguos
std::vector<int> useCasesArray; // Vector para almacenar temporalmente los useCases (casos de uso) y reciclar los antiguos
std::vector<int> eventsArray;   // Vector para almacenar temporalmente los events (acciones) y reciclar los antiguos
std::vector<int> techUsesArray; // Vector para almacenar temporalmente los techUses (usos técnicos) y reciclar los antiguos

std::vector<int> charactersArray;
std::vector<int> actorsArray;
std::vector<int> locationsArray;
std::vector<int> objectsArray;

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

// Función para convertir vector<int> a wxString
wxString VectorToString(const std::vector<int> &vec)
{
    wxString result;
    for (int val : vec)
    {
        result += wxString::Format("%d ", val);
    }
    return result;
}

// Función para convertir wxArrayString a wxString
wxString ArrayStringToString(const wxArrayString &arr)
{
    wxString result;
    for (const wxString &val : arr)
    {
        result += val + " ";
    }
    return result;
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

    Script() {}

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

    Scene() {}

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

    Take() {}

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

    Use_case() {}

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

    Tech_use() {}

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

    Event() {}

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

    Character() {}

    Character(int id, std::string first_name, std::string last_name, std::string surrname)
        : id(id), first_name(first_name), last_name(last_name), surrname(surrname) {}

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
    std::string passport_id;
    std::string first_name;
    std::string last_name;
    std::string surrname;
    std::string birthdate;

    Actor() {}

    Actor(int id, int parentId, std::string passport_id, std::string first_name, std::string last_name, std::string surrname, std::string birthdate)
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
    std::string address;
    std::string hospital;
    std::string phone;
    std::string parking;

    Location() {}

    Location(int id, std::string name, std::string address, std::string hospital, std::string phone, std::string parking)
        : id(id), name(name), address(address), hospital(hospital), phone(phone), parking(parking) {}

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & id & name & address & hospital & phone & parking;
    }
};

class Object
{
public:
    int id;
    std::string name;
    std::string description;
    std::string type;

    Object() {}

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

// ARRAY LOADER
void loadIDs(
    const std::vector<Script> &scripts,
    const std::vector<Scene> &scenes,
    const std::vector<Take> &takes,
    const std::vector<Use_case> &use_cases,
    const std::vector<Event> &events,
    const std::vector<Tech_use> tech_uses,
    std::vector<int> &scriptsArray,
    std::vector<int> &scenesArray,
    std::vector<int> &takesArray,
    std::vector<int> &useCasesArray,
    std::vector<int> &eventsArray,
    std::vector<int> &techUsesArray)
{
    scriptsArray.clear();
    scenesArray.clear();
    takesArray.clear();
    useCasesArray.clear();
    eventsArray.clear();
    techUsesArray.clear();

    if (scripts.empty())
        return; // Verificar que haya al menos un script

    // Cargar todos los IDs de scripts
    for (const auto &script : scripts)
    {
        if (!script.id.empty())
        {
            scriptsArray.push_back(script.id[0]); // Cargar el ID de cada script
        }
    }

    // Primer script ID
    int firstScriptID = scripts[0].id[0];
    size_t firstSceneIndex = 0;
    bool foundScene = false;

    // Buscar y cargar todos los scenes que pertenecen al primer script
    for (size_t i = 0; i < scenes.size(); ++i)
    {
        const auto &scene = scenes[i];
        if (!scene.id.empty() && scene.id[0] == firstScriptID)
        {
            scenesArray.push_back(scene.id[1]);
            if (!foundScene)
            {
                firstSceneIndex = i; // Guardar el índice del primer scene válido
                foundScene = true;
            }
        }
    }

    if (!foundScene)
        return; // Si no se encontró ningún scene, terminar

    // Primer scene ID
    int firstSceneID = scenes[firstSceneIndex].id[1];
    size_t firstTakeIndex = 0;
    bool foundTake = false;

    // Buscar y cargar todos los takes que pertenecen al primer scene
    for (size_t i = 0; i < takes.size(); ++i)
    {
        const auto &take = takes[i];
        if (!take.id.empty() && take.id[0] == firstScriptID && take.id[1] == firstSceneID)
        {
            takesArray.push_back(take.id[2]);
            if (!foundTake)
            {
                firstTakeIndex = i; // Guardar el índice del primer take válido
                foundTake = true;
            }
        }
    }

    if (!foundTake)
        return; // Si no se encontró ningún take, terminar

    // Primer take ID
    int firstTakeID = takes[firstTakeIndex].id[2];

    // Buscar y cargar todos los events que pertenecen al primer take
    for (const auto &event : events)
    {
        if (!event.id.empty() && event.id[0] == firstScriptID && event.id[1] == firstSceneID && event.id[2] == firstTakeID)
        {
            eventsArray.push_back(event.id[3]);
        }
    }
}

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

void updateScript(std::vector<Script> &scripts, const Script &updatedScript, bool updateTitle = true, bool updatePlainText = true)
{
    // Buscar el Script que tiene el mismo primer valor en id
    auto it = std::find_if(scripts.begin(), scripts.end(), [&](const Script &script)
                           { return !script.id.empty() && script.id[0] == updatedScript.id[0]; });

    if (it != scripts.end())
    {
        // Actualizar solo si updateTitle es verdadero
        if (updateTitle)
        {
            it->title = updatedScript.title;
        }

        // Actualizar solo si updatePlainText es verdadero
        if (updatePlainText)
        {
            it->plain_text = updatedScript.plain_text;
        }

        std::cout << "Script actualizado correctamente." << std::endl;
    }
    else
    {
        std::cout << "No se encontró un Script con el ID especificado." << std::endl;
    }
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

// CHARACTERS
bool checkCharacterExists(const std::vector<Character> &characters, const std::string &first_name, const std::string &last_name, const std::string &surrname)
{
    // Recorrer el vector de personajes y comparar cada título con el título dado
    for (const auto &character : characters)
    {
        if (character.first_name == first_name && character.last_name == last_name && character.surrname == surrname)
        {
            return true; // El nombre ya existe
        }
    }
    return false; // El nombre no existe
}

void updateCharacter(std::vector<Character> &characters, const Character &updatedCharacter, bool updateFirstName = true, bool updateLastName = true, bool updateSurrname = true)
{
    // Buscar el Character que tiene el mismo id
    auto it = std::find_if(characters.begin(), characters.end(), [&](const Character &character)
                           { return character.id == updatedCharacter.id; });

    if (it != characters.end())
    {
        // Actualizar solo si updateFirstName es verdadero
        if (updateFirstName)
        {
            it->first_name = updatedCharacter.first_name;
        }

        // Actualizar solo si updateLastName es verdadero
        if (updateLastName)
        {
            it->last_name = updatedCharacter.last_name;
        }

        // Actualizar solo si updateSurrname es verdadero
        if (updateSurrname)
        {
            it->surrname = updatedCharacter.surrname;
        }

        std::cout << "Character actualizado correctamente." << std::endl;
    }
    else
    {
        std::cout << "No se encontró un Character con el ID especificado." << std::endl;
    }
}

// ACTORS
bool checkPassportExists(const std::vector<Actor> &actors, const std::string &passport_id, int excludeId)
{
    // Recorrer el vector de actores y comparar cada pasaporte, excepto el que tiene el id definido
    for (const auto &actor : actors)
    {
        // Omitir la comparación si el id del actor coincide con el id a excluir
        if (actor.id == excludeId)
        {
            continue;
        }

        // Verificar si el pasaporte coincide
        if (actor.passport_id == passport_id)
        {
            return true; // El pasaporte ya existe
        }
    }
    return false; // El pasaporte no existe
}

void updateActor(std::vector<Actor> &actors, const Actor &updatedActor, bool updateParentId = true, bool updatePassportId = true, bool updateFirstName = true, bool updateLastName = true, bool updateSurrname = true, bool updateBirthdate = true)
{
    // Buscar el Actor que tiene el mismo id
    auto it = std::find_if(actors.begin(), actors.end(), [&](const Actor &actor)
                           { return actor.id == updatedActor.id; });

    if (it != actors.end())
    {
        // Actualizar solo si los booleanos son verdaderos
        if (updateParentId)
        {
            it->parentId = updatedActor.parentId;
        }

        if (updatePassportId)
        {
            it->passport_id = updatedActor.passport_id;
        }

        if (updateFirstName)
        {
            it->first_name = updatedActor.first_name;
        }

        if (updateLastName)
        {
            it->last_name = updatedActor.last_name;
        }

        if (updateSurrname)
        {
            it->surrname = updatedActor.surrname;
        }

        if (updateBirthdate)
        {
            it->birthdate = updatedActor.birthdate;
        }

        std::cout << "Actor actualizado correctamente." << std::endl;
    }
    else
    {
        std::cout << "No se encontró un Actor con el ID especificado." << std::endl;
    }
}

// LOCATIONS
bool checkAddressExists(const std::vector<Location> &locations, const std::string &address, int excludeId)
{
    // Recorrer el vector de locaciones y comparar cada dirección, excepto la que tiene el id definido
    for (const auto &location : locations)
    {
        // Omitir la comparación si el id de la locación coincide con el id a excluir
        if (location.id == excludeId)
        {
            continue;
        }

        // Verificar si la dirección coincide
        if (location.address == address)
        {
            return true; // La dirección ya existe
        }
    }
    return false; // La dirección no existe
}

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
        // SetBackgroundColour(wxColour(255, 255, 255));

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
        menuCharacter->Append(ID_CHARACTER_NEW, "&Nuevo...", "Nuevo personaje");
        menuCharacter->Append(ID_CHARACTER_EDIT, "&Editar...", "Editar personaje");
        menuCharacter->Append(ID_CHARACTER_DEL, "&Eliminar...", "Eliminar personaje");

        // MENU ACTOR
        wxMenu *menuActor = new wxMenu;
        menuActor->Append(ID_ACTOR_NEW, "&Nuevo...", "Nuevo actor");
        menuActor->Append(ID_ACTOR_EDIT, "&Editar...", "Editar actor");
        menuActor->Append(ID_ACTOR_DEL, "&Eliminar...", "Eliminar actor");

        // MENU LOCACION
        wxMenu *menuLocation = new wxMenu;
        menuLocation->Append(ID_ADDRESS_NEW, "&Nuevo...", "Nueva locación");
        menuLocation->Append(ID_ADDRESS_EDIT, "&Editar...", "Editar locación");
        menuLocation->Append(ID_ADDRESS_DEL, "&Eliminar...", "Eliminar locación");

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

        leftTextBox->Enable(true);

        // leftTextBox->Clear();
        // wxString allText = leftTextBox->GetValue();
        // wxString selectedText = leftTextBox->GetStringSelection();
        // leftTextBox->SetValue("Este es el texto inicial");

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
        // wxString allText = leftTextBox->GetValue();
        wxString selectedText = leftTextBox->GetStringSelection();

        nextNumber = firstEmpty(textBoxsContainer); // DE LA LISTA DE IDS, EL SIGUIENTE DISPONIBLE (EL ULTIMO O EL PRIMER HUECO VACIO)
        lastNumber = lastEmpty(textBoxsContainer);  // LA POSICION DENTRO DEL PANEL MAS ALLA DE SU ID (SIEMPRE SE AGREGA AL FINAL)
        textBoxsContainer.push_back(nextNumber);

        TitledTextBox *newTitledTextBox = new TitledTextBox(containerPanel, containerSizer, nextNumber, selectedText);
        containerSizer->Add(newTitledTextBox, 0, wxEXPAND | wxALL, 5);

        containerSizer->Layout();
        containerPanel->Layout();
        // containerPanel->SetVirtualSize(containerSizer->GetMinSize());
        containerPanel->FitInside(); // Esta funcion reemplaza a la linea de arriba

        // nextNumber es la posicion dentro del arreglo (el que va a ser su id); tree es el parentId
        // lastNumber es la posicion dentro del contenedor, al final (por que last se inserta siempre al final)
        Scene newScene(nextNumber, tree, selectedText.ToStdString(), lastNumber);
        scenes.push_back(newScene);

        mod = true;
    }

    void OnBackButtonClicked(wxCommandEvent &event)
    {
        // wxString allText = leftTextBox->GetValue();
        wxString selectedText = leftTextBox->GetStringSelection();

        nextNumber = firstEmpty(itemsListContainer); // DE LA LISTA DE IDS, EL SIGUIENTE DISPONIBLE (EL ULTIMO O EL PRIMER HUECO VACIO)
        lastNumber = lastEmpty(itemsListContainer);  // LA POSICION DENTRO DEL PANEL MAS ALLA DE SU ID (SIEMPRE SE AGREGA AL FINAL)
        itemsListContainer.push_back(nextNumber);

        ItemTextList *newItemTextList = new ItemTextList(itemsPanel, itemsSizer, nextNumber, selectedText);
        itemsSizer->Add(newItemTextList, 0, wxEXPAND | wxALL, 5);

        itemsSizer->Layout();
        itemsPanel->Layout();
        // containerPanel->SetVirtualSize(containerSizer->GetMinSize());
        itemsPanel->FitInside(); // Esta funcion reemplaza a la linea de arriba

        // nextNumber es la posicion dentro del arreglo (el que va a ser su id); tree es el parentId
        // lastNumber es la posicion dentro del contenedor, al final (por que last se inserta siempre al final)
        Take newTake(nextNumber, tree, selectedText.ToStdString(), lastNumber);
        takes.push_back(newTake);

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
    void readFile(const wxString &filename);
    void OnExit(wxCommandEvent &event);

    // Script Menu Events
    void OnNewScript(wxCommandEvent &event);
    void OnScriptEdit(wxCommandEvent &event);
    void OnScriptDel(wxCommandEvent &event);

    // Character Menu Events
    void OnNewCharacter(wxCommandEvent &event);
    void OnCharacterEdit(wxCommandEvent &event);
    void OnCharacterDel(wxCommandEvent &event);

    // Actor Menu Events
    void OnNewActor(wxCommandEvent &event);
    void OnActorEdit(wxCommandEvent &event);
    void OnActorDel(wxCommandEvent &event);

    // Address Menu Events
    void OnNewAddress(wxCommandEvent &event);
    void OnAddressEdit(wxCommandEvent &event);
    void OnAddressDel(wxCommandEvent &event);

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

                                        EVT_MENU(ID_CHARACTER_NEW, MainWindow::OnNewCharacter)
                                            EVT_MENU(ID_CHARACTER_EDIT, MainWindow::OnCharacterEdit)
                                                EVT_MENU(ID_CHARACTER_DEL, MainWindow::OnCharacterDel)

                                                    EVT_MENU(ID_ACTOR_NEW, MainWindow::OnNewActor)
                                                        EVT_MENU(ID_ACTOR_EDIT, MainWindow::OnActorEdit)
                                                            EVT_MENU(ID_ACTOR_DEL, MainWindow::OnActorDel)

                                                                EVT_MENU(ID_ADDRESS_NEW, MainWindow::OnNewAddress)
                                                                    EVT_MENU(ID_ADDRESS_EDIT, MainWindow::OnAddressEdit)
                                                                        EVT_MENU(ID_ADDRESS_DEL, MainWindow::OnAddressDel)

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

    readFile(filename);

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
        int response = wxMessageBox(wxT("Hay cambios sin guardar en el proyecto!"), "Guardar cambios", wxYES_NO | wxICON_QUESTION);

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

    // DATA KILLER

    // Ids Arrays
    scriptsArray.clear();
    scenesArray.clear();
    takesArray.clear();
    useCasesArray.clear();
    techUsesArray.clear();
    eventsArray.clear();

    charactersArray.clear();
    actorsArray.clear();
    locationsArray.clear();
    objectsArray.clear();

    // GUI Arrays
    textBoxsContainer.clear();
    itemsListContainer.clear();

    // Class Arrays
    scripts.clear();
    scenes.clear();
    takes.clear();
    use_cases.clear();
    tech_uses.clear();
    events.clear();

    characters.clear();
    actors.clear();
    locations.clear();
    objects.clear();
}

void MainWindow::OnExit(wxCommandEvent &event)
{
    if (mod)
    {
        int response = wxMessageBox(wxT("¿Deseas guardar los cambios?"), "Guardar cambios", wxYES_NO | wxICON_QUESTION);

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

void MainWindow::readFile(const wxString &filename)
{
    std::ifstream in_fs(filename.ToStdString()); // Convierte wxString a std::string
    boost::archive::text_iarchive inArchive(in_fs);

    inArchive >> scripts;
    inArchive >> scenes;
    inArchive >> takes;

    inArchive >> use_cases;
    inArchive >> tech_uses;
    inArchive >> events;

    inArchive >> characters;
    inArchive >> actors;
    inArchive >> locations;
    inArchive >> objects;
}

void MainWindow::writeFile(const wxString &filename)
{
    std::ofstream out_fs(filename.ToStdString()); // Convierte wxString a std::string
    boost::archive::text_oarchive outArchive(out_fs);

    outArchive << scripts;
    outArchive << scenes;
    outArchive << takes;

    outArchive << use_cases;
    outArchive << tech_uses;
    outArchive << events;

    outArchive << characters;
    outArchive << actors;
    outArchive << locations;
    outArchive << objects;

    out_fs.close();
}

//// UDMR EVENTS ////

// SCRIPT MENU
void MainWindow::OnNewScript(wxCommandEvent &event)
{
    // wxTextEntryDialog dialog(NULL, wxT("Ingrese el título del guión:"), wxT("Nuevo Guión")); // Prompt / Titulo Ventana
    wxDialog dialog(NULL, wxID_ANY, wxT("Nuevo Guión"), wxDefaultPosition, wxDefaultSize);

    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);

    // Texto descriptivo
    wxStaticText *label = new wxStaticText(&dialog, wxID_ANY, wxT("Ingrese el título:"), wxDefaultPosition, wxDefaultSize);
    vbox->Add(label, 0, wxTOP, 5); // Espacio arriba

    // Campo de texto
    wxTextCtrl *textCtrl = new wxTextCtrl(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
    vbox->Add(textCtrl, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

    vbox->Add(dialog.CreateButtonSizer(wxOK | wxCANCEL), 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 10);

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(vbox, 1, wxLEFT | wxRIGHT | wxEXPAND, 10); // Aplica margen general de 10 a izquierda y derecha

    dialog.SetSizer(mainSizer);
    dialog.Fit();

    // Boton para cambiar el estado de Ok
    wxButton *okButton = dynamic_cast<wxButton *>(dialog.FindWindow(wxID_OK));
    okButton->Disable();

    // Captura okButton, textCtrl y comboBox para deshabilitar "okButton" si esta vacio
    textCtrl->Bind(wxEVT_TEXT, [okButton, textCtrl](wxCommandEvent &event)
                   {
                        wxString title = textCtrl->GetValue();

                        if (title.IsEmpty())
                        {
                            okButton->Disable();
                        }
                        else
                        {
                            okButton->Enable();
                        } });

    // Mostrar el cuadro de diálogo y obtener el resultado
    if (dialog.ShowModal() == wxID_OK)
    {
        wxString title = textCtrl->GetValue();

        if (!title.IsEmpty())
        {
            if (!checkTitleExists(scripts, title.ToStdString()))
            {
                nextNumber = firstEmpty(scriptsArray);
                scriptsArray.push_back(nextNumber);

                Script newScript({nextNumber}, title.ToStdString(), title.ToStdString());
                scripts.push_back(newScript);

                // wxMessageBox(wxString::Format(wxT("Crear guión Id Nº: %d"),), "Ok", wxOK | wxICON_INFORMATION);
                mod = true;
            }

            else
            {
                wxMessageBox(wxT("Ese nombre ya existe"),   // CONTENIDO VENTANA POP UP
                             "Error", wxOK | wxICON_ERROR); // TITULO VENTANA POP UP
            }
        }

        else
        {
            wxMessageBox(wxT("No puede estar vacío!"), "Error", wxOK | wxICON_ERROR);
        }
    }
}

void MainWindow::OnScriptEdit(wxCommandEvent &event)
{
    if (!scripts.empty()) // Si hay algo que editar (si no está vacío), editar
    {
        std::vector<int> ScriptIds;
        wxArrayString scriptTitles;

        for (const auto &script : scripts)
        {
            scriptTitles.Add(script.title);
            ScriptIds.push_back(script.id[0]); // El id dentro de cada script es un arreglo estatico, por eso el [0]
        }

        wxDialog dialog(NULL, wxID_ANY, wxT("Editar Guión"), wxDefaultPosition, wxDefaultSize);

        wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);

        // Texto descriptivo
        wxStaticText *labelSel = new wxStaticText(&dialog, wxID_ANY, wxT("Seleccione el guión:"), wxDefaultPosition, wxDefaultSize);
        vbox->Add(labelSel, 0, wxTOP, 5); // Espacio arriba

        // Selector
        wxComboBox *comboBox = new wxComboBox(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, scriptTitles, wxCB_READONLY);
        vbox->Add(comboBox, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

        // Texto descriptivo
        wxStaticText *labelDesc = new wxStaticText(&dialog, wxID_ANY, wxT("Ingrese el nuevo título:"), wxDefaultPosition, wxDefaultSize);
        vbox->Add(labelDesc, 0, wxTOP, 10); // Espacio arriba

        // Campo de texto
        wxTextCtrl *textCtrl = new wxTextCtrl(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
        vbox->Add(textCtrl, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

        vbox->Add(dialog.CreateButtonSizer(wxOK | wxCANCEL), 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 10);

        wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
        mainSizer->Add(vbox, 1, wxLEFT | wxRIGHT | wxEXPAND, 10); // Aplica margen general de 10 a izquierda y derecha

        dialog.SetSizer(mainSizer);
        dialog.Fit();

        comboBox->SetSelection(0);           // Seleccionar el primer elemento
        textCtrl->SetValue(scriptTitles[0]); // Escribir el primer elemento en el cuadro de texto

        // Evento para actualizar el cuadro de texto cuando se cambia la selección en el wxComboBox
        // comboBox->Bind(wxEVT_COMBOBOX, [&](wxCommandEvent &event)
        comboBox->Bind(wxEVT_COMBOBOX, [comboBox, textCtrl, scriptTitles, ScriptIds](wxCommandEvent &event)

                       {
                           textCtrl->SetValue(comboBox->GetStringSelection());
                           // textCtrl->SetValue(wxString::Format(wxT("Editar guión Id Nº: %d"), comboBox->GetSelection()));
                           // textCtrl->SetValue(scriptTitles[comboBox->GetSelection()]);
                           // textCtrl->SetValue(wxString::Format(wxT("Editar guión Id Nº: %d"), ScriptIds[comboBox->GetSelection()]));
                           // wxMessageBox(VectorToString(ScriptIds), "Contenido de ScriptIds", wxOK | wxICON_INFORMATION);
                           // wxMessageBox(ArrayStringToString(scriptTitles), "Contenido de scriptTitles", wxOK | wxICON_INFORMATION);
                       }

        );

        // Boton para cambiar el estado de Ok
        wxButton *okButton = dynamic_cast<wxButton *>(dialog.FindWindow(wxID_OK));
        okButton->Disable();

        // Captura okButton, textCtrl y comboBox para deshabilitar "okButton" si esta vacio o es el mismo
        textCtrl->Bind(wxEVT_TEXT, [okButton, textCtrl, comboBox](wxCommandEvent &event)
                       {
                        wxString editedTitle = textCtrl->GetValue();

                        if (editedTitle.IsEmpty() || comboBox->GetStringSelection() == editedTitle)
                        {
                            okButton->Disable();
                        }
                        else
                        {
                            okButton->Enable();
                        } });

        // Mostrar el cuadro de diálogo y obtener el resultado
        if (dialog.ShowModal() == wxID_OK)
        {
            wxString editedTitle = textCtrl->GetValue();

            if (!editedTitle.IsEmpty())
            {
                // selectedTitle = comboBox->GetStringSelection(); // Si no cambiamos y dejamos el primero

                if (comboBox->GetStringSelection() == editedTitle)
                {
                    wxMessageBox(wxT("No puede tener el mismo nombre!"), "Error", wxOK | wxICON_ERROR);
                }

                else
                {
                    if (!checkTitleExists(scripts, editedTitle.ToStdString()))
                    {
                        Script updatedScript({ScriptIds[comboBox->GetSelection()]}, editedTitle.ToStdString(), "");
                        updateScript(scripts, updatedScript, true, false); // (Arreglo, elementoActualizado, titulo, texto)

                        // wxMessageBox(wxString::Format(wxT("Editar guión Id Nº: %d"), ScriptIds[comboBox->GetSelection()]), "Ok", wxOK | wxICON_INFORMATION);
                        mod = true;
                    }

                    else
                    {
                        wxMessageBox(wxT("Ese nombre ya existe"),   // CONTENIDO VENTANA POP UP
                                     "Error", wxOK | wxICON_ERROR); // TITULO VENTANA POP UP
                    }
                }
            }

            else
            {
                wxMessageBox(wxT("No puede estar vacío!"), "Error", wxOK | wxICON_ERROR);
            }
        }
    }

    else // Si esta vacio (no hay nada que editar, error!)
    {
        wxMessageBox(wxT("Primero crea un guión!!"), "Error", wxOK | wxICON_ERROR);
    }
}

void MainWindow::OnScriptDel(wxCommandEvent &event)
{
    if (!scripts.empty()) // Si hay algo que borrar (si no está vacío), abrir ventana
    {
        std::vector<int> ScriptIds;
        wxArrayString scriptTitles;

        for (const auto &script : scripts)
        {
            scriptTitles.Add(script.title);
            ScriptIds.push_back(script.id[0]); // El id dentro de cada script es un arreglo estatico, por eso el [0]
        }

        wxDialog dialog(NULL, wxID_ANY, wxT("Eliminar Guión"), wxDefaultPosition, wxDefaultSize);

        wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);

        // Texto descriptivo
        wxStaticText *labelSel = new wxStaticText(&dialog, wxID_ANY, wxT("Seleccione el guión:"), wxDefaultPosition, wxDefaultSize);
        vbox->Add(labelSel, 0, wxTOP, 5); // Espacio arriba

        // Selector
        wxComboBox *comboBox = new wxComboBox(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, scriptTitles, wxCB_READONLY);
        vbox->Add(comboBox, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

        vbox->Add(dialog.CreateButtonSizer(wxOK | wxCANCEL), 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 10);

        wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
        mainSizer->Add(vbox, 1, wxLEFT | wxRIGHT | wxEXPAND, 10); // Aplica margen general de 10 a izquierda y derecha

        dialog.SetSizer(mainSizer);
        dialog.Fit();

        comboBox->SetSelection(0); // Seleccionar el primer elemento

        // Mostrar el cuadro de diálogo y obtener el resultado
        if (dialog.ShowModal() == wxID_OK)
        {
            wxMessageBox(wxString::Format(wxT("Eliminar guión Id Nº: %d"), ScriptIds[comboBox->GetSelection()]), "Ok", wxOK | wxICON_INFORMATION);
        }
    }

    else // Si esta vacio (no hay nada que borrar, error!)
    {
        wxMessageBox(wxT("Primero crea un guión!!"), "Error", wxOK | wxICON_ERROR);
    }
}

// CHARACTER MENU
void MainWindow::OnNewCharacter(wxCommandEvent &event)
{
    // wxTextEntryDialog dialog(NULL, wxT("Ingrese el nombre del personaje:"), wxT("Nuevo Personaje")); // Prompt / Titulo Ventana
    wxDialog dialog(NULL, wxID_ANY, wxT("Nuevo Personaje"), wxDefaultPosition, wxDefaultSize);

    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);

    // Texto descriptivo
    wxStaticText *labelFirst = new wxStaticText(&dialog, wxID_ANY, wxT("Ingrese el primer nombre:"), wxDefaultPosition, wxDefaultSize);
    vbox->Add(labelFirst, 0, wxTOP, 5); // Espacio arriba
    // Campo de texto
    wxTextCtrl *textCtrlF = new wxTextCtrl(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
    vbox->Add(textCtrlF, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

    // Texto descriptivo
    wxStaticText *labelLast = new wxStaticText(&dialog, wxID_ANY, wxT("Ingrese el segundo nombre:"), wxDefaultPosition, wxDefaultSize);
    vbox->Add(labelLast, 0, wxTOP, 10); // Espacio arriba
    // Campo de texto
    wxTextCtrl *textCtrlL = new wxTextCtrl(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
    vbox->Add(textCtrlL, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

    // Texto descriptivo
    wxStaticText *labelSurr = new wxStaticText(&dialog, wxID_ANY, wxT("Ingrese el apellido:"), wxDefaultPosition, wxDefaultSize);
    vbox->Add(labelSurr, 0, wxTOP, 10); // Espacio arriba
    // Campo de texto
    wxTextCtrl *textCtrlS = new wxTextCtrl(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
    vbox->Add(textCtrlS, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

    vbox->Add(dialog.CreateButtonSizer(wxOK | wxCANCEL), 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 10);

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(vbox, 1, wxLEFT | wxRIGHT | wxEXPAND, 10); // Aplica margen general de 10 a izquierda y derecha

    dialog.SetSizer(mainSizer);
    dialog.Fit();

    // Boton para cambiar el estado de Ok
    wxButton *okButton = dynamic_cast<wxButton *>(dialog.FindWindow(wxID_OK));
    okButton->Disable();

    // Captura okButton, textCtrl y comboBox para deshabilitar "okButton" si esta vacio
    auto validateTextFields = [okButton, textCtrlF, textCtrlS](wxCommandEvent &event)
    {
        wxString first_name = textCtrlF->GetValue();
        wxString surrname = textCtrlS->GetValue();

        // Habilitar el botón solo si todos los campos no están vacíos
        if (!first_name.IsEmpty() && !surrname.IsEmpty())
        {
            okButton->Enable();
        }
        else
        {
            okButton->Disable();
        }
    };

    // Vinculamos el evento a cualquier cambio en cualquiera de los 2 controles
    textCtrlF->Bind(wxEVT_TEXT, validateTextFields);
    textCtrlS->Bind(wxEVT_TEXT, validateTextFields);

    // Mostrar el cuadro de diálogo y obtener el resultado
    if (dialog.ShowModal() == wxID_OK)
    {
        wxString first_name = textCtrlF->GetValue();
        wxString last_name = textCtrlL->GetValue();
        wxString surrname = textCtrlS->GetValue();

        if (!first_name.IsEmpty() && !surrname.IsEmpty())
        {
            if (!checkCharacterExists(characters, first_name.ToStdString(), last_name.ToStdString(), surrname.ToStdString()))
            {
                nextNumber = firstEmpty(charactersArray);
                charactersArray.push_back(nextNumber);

                Character newCharacter(nextNumber, first_name.ToStdString(), last_name.ToStdString(), surrname.ToStdString());
                characters.push_back(newCharacter);

                // wxMessageBox(wxString::Format(wxT("Crear personaje Id Nº: %d"),), "Ok", wxOK | wxICON_INFORMATION);
                mod = true;
            }

            else
            {
                wxMessageBox(wxT("Ese nombre ya existe"),   // CONTENIDO VENTANA POP UP
                             "Error", wxOK | wxICON_ERROR); // TITULO VENTANA POP UP
            }
        }

        else
        {
            wxMessageBox(wxT("No puede estar vacío!"), "Error", wxOK | wxICON_ERROR);
        }
    }
}

void MainWindow::OnCharacterEdit(wxCommandEvent &event)
{
    if (!characters.empty()) // Si hay algo que editar (si no está vacío), editar
    {
        std::vector<int> CharactersIds;
        wxArrayString firstNames;
        wxArrayString lastNames;
        wxArrayString surrnames;
        wxArrayString fullnames;

        for (const auto &character : characters)
        {
            firstNames.Add(character.first_name);
            lastNames.Add(character.last_name);
            surrnames.Add(character.surrname);

            fullnames.Add(character.first_name + " " + character.last_name + " " + character.surrname);
            CharactersIds.push_back(character.id);
        }

        wxDialog dialog(NULL, wxID_ANY, wxT("Editar Personaje"), wxDefaultPosition, wxDefaultSize);

        wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);

        // Texto descriptivo
        wxStaticText *labelSel = new wxStaticText(&dialog, wxID_ANY, wxT("Seleccione el personaje:"), wxDefaultPosition, wxDefaultSize);
        vbox->Add(labelSel, 0, wxTOP, 5); // Espacio arriba

        // Selector
        wxComboBox *comboBox = new wxComboBox(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, fullnames, wxCB_READONLY);
        vbox->Add(comboBox, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

        // Texto descriptivo
        wxStaticText *labelFirst = new wxStaticText(&dialog, wxID_ANY, wxT("Ingrese el primer nombre:"), wxDefaultPosition, wxDefaultSize);
        vbox->Add(labelFirst, 0, wxTOP, 10); // Espacio arriba

        // Campo de texto
        wxTextCtrl *textCtrlF = new wxTextCtrl(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
        vbox->Add(textCtrlF, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

        // Texto descriptivo
        wxStaticText *labelLast = new wxStaticText(&dialog, wxID_ANY, wxT("Ingrese el segundo nombre:"), wxDefaultPosition, wxDefaultSize);
        vbox->Add(labelLast, 0, wxTOP, 10); // Espacio arriba

        // Campo de texto
        wxTextCtrl *textCtrlL = new wxTextCtrl(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
        vbox->Add(textCtrlL, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

        // Texto descriptivo
        wxStaticText *labelSurr = new wxStaticText(&dialog, wxID_ANY, wxT("Ingrese el apellido:"), wxDefaultPosition, wxDefaultSize);
        vbox->Add(labelSurr, 0, wxTOP, 10); // Espacio arriba

        // Campo de texto
        wxTextCtrl *textCtrlS = new wxTextCtrl(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
        vbox->Add(textCtrlS, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

        vbox->Add(dialog.CreateButtonSizer(wxOK | wxCANCEL), 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 10);

        wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
        mainSizer->Add(vbox, 1, wxLEFT | wxRIGHT | wxEXPAND, 10); // Aplica margen general de 10 a izquierda y derecha

        dialog.SetSizer(mainSizer);
        dialog.Fit();

        comboBox->SetSelection(0);          // Seleccionar el primer elemento
        textCtrlF->SetValue(firstNames[0]); // Escribir el primer elemento en el cuadro de texto
        textCtrlL->SetValue(lastNames[0]);  // Escribir el primer elemento en el cuadro de texto
        textCtrlS->SetValue(surrnames[0]);  // Escribir el primer elemento en el cuadro de texto

        // Boton para cambiar el estado de Ok
        wxButton *okButton = dynamic_cast<wxButton *>(dialog.FindWindow(wxID_OK));
        okButton->Disable();

        // Evento para actualizar el cuadro de texto cuando se cambia la selección en el wxComboBox
        comboBox->Bind(wxEVT_COMBOBOX, [comboBox, okButton, textCtrlF, textCtrlL, textCtrlS, CharactersIds, firstNames, lastNames, surrnames](wxCommandEvent &event)
                       {
                           // textCtrl->SetValue(comboBox->GetStringSelection());
                           // textCtrl->SetValue(wxString::Format(wxT("Editar personaje Id Nº: %d"), comboBox->GetSelection()));
                           textCtrlF->SetValue(firstNames[comboBox->GetSelection()]);
                           textCtrlL->SetValue(lastNames[comboBox->GetSelection()]);
                           textCtrlS->SetValue(surrnames[comboBox->GetSelection()]);
                           // textCtrl->SetValue(wxString::Format(wxT("Editar personaje Id Nº: %d"), CharactersIds[comboBox->GetSelection()]));
                           // wxMessageBox(VectorToString(CharactersIds), "Contenido de CharactersIds", wxOK | wxICON_INFORMATION);
                           // wxMessageBox(ArrayStringToString(firstNames), "Contenido de firstNames", wxOK | wxICON_INFORMATION);
                           // wxMessageBox(ArrayStringToString(lastNames), "Contenido de lastNames", wxOK | wxICON_INFORMATION);
                           // wxMessageBox(ArrayStringToString(surrnames), "Contenido de surrnames", wxOK | wxICON_INFORMATION);
                           okButton->Disable(); }

        );

        // Captura okButton, textCtrl y comboBox para deshabilitar ok si esta vacio o es el mismo
        auto validateTextFields = [okButton, comboBox, textCtrlF, textCtrlL, textCtrlS, firstNames, lastNames, surrnames](wxCommandEvent &event)
        {
            wxString first_name = textCtrlF->GetValue();
            wxString last_name = textCtrlL->GetValue();
            wxString surrname = textCtrlS->GetValue();

            // Si: (el primero no esta vacio Y el segundo no esta vacio) Y (el primero O el segundo O el tercero es diferente)
            if ((!first_name.IsEmpty() && !surrname.IsEmpty()) && (firstNames[comboBox->GetSelection()] != first_name || lastNames[comboBox->GetSelection()] != last_name || surrnames[comboBox->GetSelection()] != surrname))
            {
                okButton->Enable();
            }
            else
            {
                okButton->Disable();
            }
        };

        // Vinculamos el evento a cualquier cambio en cualquiera de los 3 controles
        textCtrlF->Bind(wxEVT_TEXT, validateTextFields);
        textCtrlL->Bind(wxEVT_TEXT, validateTextFields);
        textCtrlS->Bind(wxEVT_TEXT, validateTextFields);

        // Mostrar el cuadro de diálogo y obtener el resultado
        if (dialog.ShowModal() == wxID_OK)
        {
            wxString editedFirst_name = textCtrlF->GetValue();
            wxString editedLast_name = textCtrlL->GetValue();
            wxString editedSurrname = textCtrlS->GetValue();

            if (!editedFirst_name.IsEmpty() && !editedSurrname.IsEmpty())
            {
                // selectedTitle = comboBox->GetStringSelection(); // Si no cambiamos y dejamos el primero

                if (firstNames[comboBox->GetSelection()] == editedFirst_name && lastNames[comboBox->GetSelection()] == editedLast_name && surrnames[comboBox->GetSelection()] == editedSurrname)
                {
                    wxMessageBox(wxT("No puede tener el mismo nombre!"), "Error", wxOK | wxICON_ERROR);
                }

                else
                {
                    if (!checkCharacterExists(characters, editedFirst_name.ToStdString(), editedLast_name.ToStdString(), editedSurrname.ToStdString()))
                    {
                        Character updatedCharacter(CharactersIds[comboBox->GetSelection()], editedFirst_name.ToStdString(), editedLast_name.ToStdString(), editedSurrname.ToStdString());
                        updateCharacter(characters, updatedCharacter, true, true, true); // (Arreglo, elementoActualizado, pNombre, sNombre, apellido)

                        // wxMessageBox(wxString::Format(wxT("Editar personaje Id Nº: %d"), CharactersIds[comboBox->GetSelection()]), "Ok", wxOK | wxICON_INFORMATION);
                        mod = true;
                    }

                    else
                    {
                        wxMessageBox(wxT("Ese nombre ya existe"),   // CONTENIDO VENTANA POP UP
                                     "Error", wxOK | wxICON_ERROR); // TITULO VENTANA POP UP
                    }
                }
            }

            else
            {
                wxMessageBox(wxT("No puede estar vacío!"), "Error", wxOK | wxICON_ERROR);
            }
        }
    }

    else // Si esta vacio (no hay nada que editar, error!)
    {
        wxMessageBox(wxT("Primero crea un personaje!!"), "Error", wxOK | wxICON_ERROR);
    }
}

void MainWindow::OnCharacterDel(wxCommandEvent &event)
{
    if (!characters.empty()) // Si hay algo que editar (si no está vacío), editar
    {
        std::vector<int> CharactersIds;
        wxArrayString fullnames;

        for (const auto &character : characters)
        {
            fullnames.Add(character.first_name + " " + character.last_name + " " + character.surrname);
            CharactersIds.push_back(character.id);
        }

        wxDialog dialog(NULL, wxID_ANY, wxT("Eliminar Personaje"), wxDefaultPosition, wxDefaultSize);

        wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);

        // Texto descriptivo
        wxStaticText *labelSel = new wxStaticText(&dialog, wxID_ANY, wxT("Seleccione el personaje:"), wxDefaultPosition, wxDefaultSize);
        vbox->Add(labelSel, 0, wxTOP, 5); // Espacio arriba

        // Selector
        wxComboBox *comboBox = new wxComboBox(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, fullnames, wxCB_READONLY);
        vbox->Add(comboBox, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

        vbox->Add(dialog.CreateButtonSizer(wxOK | wxCANCEL), 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 10);

        wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
        mainSizer->Add(vbox, 1, wxLEFT | wxRIGHT | wxEXPAND, 10); // Aplica margen general de 10 a izquierda y derecha

        dialog.SetSizer(mainSizer);
        dialog.Fit();

        comboBox->SetSelection(0); // Seleccionar el primer elemento

        // Mostrar el cuadro de diálogo y obtener el resultado
        if (dialog.ShowModal() == wxID_OK)
        {
            wxMessageBox(wxString::Format(wxT("Eliminar personaje Id Nº: %d"), CharactersIds[comboBox->GetSelection()]), "Ok", wxOK | wxICON_INFORMATION);
        }
    }

    else // Si esta vacio (no hay nada que borrar, error!)
    {
        wxMessageBox(wxT("Primero crea un personaje!!"), "Error", wxOK | wxICON_ERROR);
    }
}

// ACTOR MENU
void MainWindow::OnNewActor(wxCommandEvent &event)
{
    if (!characters.empty()) // Si hay algo que asignar (si no está vacío), crear
    {
        std::vector<int> CharactersIds;
        wxArrayString fullnames;

        for (const auto &character : characters)
        {
            fullnames.Add(character.first_name + " " + character.last_name + " " + character.surrname);
            CharactersIds.push_back(character.id);
        }

        wxDialog dialog(NULL, wxID_ANY, wxT("Nuevo Actor"), wxDefaultPosition, wxDefaultSize);

        wxBoxSizer *vboxa = new wxBoxSizer(wxVERTICAL);
        wxBoxSizer *vboxb = new wxBoxSizer(wxVERTICAL);
        wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
        wxBoxSizer *mbox = new wxBoxSizer(wxVERTICAL);

        // Texto descriptivo
        wxStaticText *labelFirst = new wxStaticText(&dialog, wxID_ANY, wxT("Ingrese el primer nombre:"), wxDefaultPosition, wxDefaultSize);
        vboxa->Add(labelFirst, 0, wxTOP, 5); // Espacio arriba

        // Campo de texto
        wxTextCtrl *textCtrlF = new wxTextCtrl(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
        vboxa->Add(textCtrlF, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

        // Texto descriptivo
        wxStaticText *labelLast = new wxStaticText(&dialog, wxID_ANY, wxT("Ingrese el segundo nombre:"), wxDefaultPosition, wxDefaultSize);
        vboxa->Add(labelLast, 0, wxTOP, 10); // Espacio arriba

        // Campo de texto
        wxTextCtrl *textCtrlL = new wxTextCtrl(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
        vboxa->Add(textCtrlL, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

        // Texto descriptivo
        wxStaticText *labelSurr = new wxStaticText(&dialog, wxID_ANY, wxT("Ingrese el apellido:"), wxDefaultPosition, wxDefaultSize);
        vboxa->Add(labelSurr, 0, wxTOP, 10); // Espacio arriba

        // Campo de texto
        wxTextCtrl *textCtrlS = new wxTextCtrl(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
        vboxa->Add(textCtrlS, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

        // Texto descriptivo
        wxStaticText *labelPass = new wxStaticText(&dialog, wxID_ANY, wxT("Ingrese el ID / Nº de Pasaporte:"), wxDefaultPosition, wxDefaultSize);
        vboxb->Add(labelPass, 0, wxTOP, 5); // Espacio arriba

        // Campo de texto
        wxTextCtrl *textCtrlP = new wxTextCtrl(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
        vboxb->Add(textCtrlP, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

        // Texto descriptivo
        wxStaticText *labelDate = new wxStaticText(&dialog, wxID_ANY, wxT("Ingrese la fecha de nacimiento:"), wxDefaultPosition, wxDefaultSize);
        vboxb->Add(labelDate, 0, wxTOP, 10); // Espacio arriba

        // Campo de fecha
        wxDatePickerCtrl *textCtrlD = new wxDatePickerCtrl(&dialog, wxID_ANY, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DEFAULT);
        vboxb->Add(textCtrlD, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

        // Texto descriptivo
        wxStaticText *labelRol = new wxStaticText(&dialog, wxID_ANY, wxT("Ingrese el papel:"), wxDefaultPosition, wxDefaultSize);
        vboxb->Add(labelRol, 0, wxTOP, 10); // Espacio arriba

        // Selector de personaje
        wxComboBox *comboBoxRol = new wxComboBox(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, fullnames, wxCB_READONLY);
        vboxb->Add(comboBoxRol, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

        hbox->Add(vboxa, 1, wxLEFT | wxRIGHT | wxEXPAND, 5); // vboxa ocupa parte de hbox
        hbox->Add(vboxb, 1, wxLEFT | wxRIGHT | wxEXPAND, 5); // vboxb ocupa parte de hbox
        mbox->Add(hbox, 1, wxLEFT | wxRIGHT | wxEXPAND, 10);

        mbox->Add(dialog.CreateButtonSizer(wxOK | wxCANCEL), 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 10);

        dialog.SetSizer(mbox);
        dialog.Fit();

        comboBoxRol->SetSelection(0);

        // Boton para cambiar el estado de Ok
        wxButton *okButton = dynamic_cast<wxButton *>(dialog.FindWindow(wxID_OK));
        okButton->Disable();

        // Captura okButton, textCtrl y comboBox para deshabilitar "okButton" si esta vacio
        auto validateTextFields = [okButton, textCtrlP, textCtrlF, textCtrlS](wxCommandEvent &event)
        {
            wxString passport = textCtrlP->GetValue();
            wxString firstName = textCtrlF->GetValue();
            wxString surrname = textCtrlS->GetValue();

            // Habilitar el botón solo si todos los campos no están vacíos
            if (!passport.IsEmpty() && !firstName.IsEmpty() && !surrname.IsEmpty())
            {
                okButton->Enable();
            }
            else
            {
                okButton->Disable();
            }
        };

        // Vinculamos el evento a cualquier cambio en cualquiera de los 3 controles
        textCtrlP->Bind(wxEVT_TEXT, validateTextFields);
        textCtrlF->Bind(wxEVT_TEXT, validateTextFields);
        textCtrlS->Bind(wxEVT_TEXT, validateTextFields);

        // Mostrar el cuadro de diálogo y obtener el resultado
        if (dialog.ShowModal() == wxID_OK)
        {
            int parent_id = CharactersIds[comboBoxRol->GetSelection()];
            wxString passport = textCtrlP->GetValue();
            wxString first_name = textCtrlF->GetValue();
            wxString last_name = textCtrlL->GetValue();
            wxString surrname = textCtrlS->GetValue();
            wxString birth_date = textCtrlD->GetValue().FormatISODate();
            nextNumber = firstEmpty(actorsArray);

            if (!passport.IsEmpty() && !first_name.IsEmpty() && !surrname.IsEmpty())
            {
                if (!checkPassportExists(actors, passport.ToStdString(), nextNumber))
                {
                    actorsArray.push_back(nextNumber);

                    Actor newActor(nextNumber, parent_id, passport.ToStdString(), first_name.ToStdString(), last_name.ToStdString(), surrname.ToStdString(), birth_date.ToStdString());
                    actors.push_back(newActor);
                    // wxMessageBox(wxString::Format(wxT("Crear actor Id Nº: %d"),), "Ok", wxOK | wxICON_INFORMATION);
                }

                else
                {
                    wxMessageBox(wxT("Ese pasaporte ya existe"), // CONTENIDO VENTANA POP UP
                                 "Error", wxOK | wxICON_ERROR);  // TITULO VENTANA POP UP
                }
            }

            else
            {
                wxMessageBox(wxT("No puede estar vacío!"), "Error", wxOK | wxICON_ERROR);
            }
        }
    }

    else // Si esta vacio (no hay nada que asignar, error!)
    {
        wxMessageBox(wxT("Primero crea un personaje!!"), "Error", wxOK | wxICON_ERROR);
    }
}

void MainWindow::OnActorEdit(wxCommandEvent &event)
{
    if (!actors.empty()) // Si hay algo que editar (si no está vacío), editar
    {
        std::vector<int> CharactersIds;
        wxArrayString charactersFullNames;

        std::vector<int> ActorsIds;
        std::vector<int> ActorsParentsIds;
        std::vector<int> ActorsParentsIdsIndexes;
        wxArrayString passports;
        wxArrayString firstNames;
        wxArrayString lastNames;
        wxArrayString surrnames;
        wxArrayString birthDates;
        wxArrayString fullnames;

        for (const auto &character : characters)
        {
            charactersFullNames.Add(character.first_name + " " + character.last_name + " " + character.surrname);
            CharactersIds.push_back(character.id);
        }

        for (const auto &actor : actors)
        {
            passports.Add(actor.passport_id);
            firstNames.Add(actor.first_name);
            lastNames.Add(actor.last_name);
            surrnames.Add(actor.surrname);
            birthDates.Add(actor.birthdate);

            fullnames.Add(actor.first_name + " " + actor.last_name + " " + actor.surrname);
            auto it = std::find(CharactersIds.begin(), CharactersIds.end(), actor.parentId);

            ActorsIds.push_back(actor.id);
            ActorsParentsIds.push_back(actor.parentId);
            ActorsParentsIdsIndexes.push_back(std::distance(CharactersIds.begin(), it));
        }

        wxDialog dialog(NULL, wxID_ANY, wxT("Editar Actor"), wxDefaultPosition, wxDefaultSize);

        wxBoxSizer *vboxtop = new wxBoxSizer(wxVERTICAL);
        wxBoxSizer *vboxa = new wxBoxSizer(wxVERTICAL);
        wxBoxSizer *vboxb = new wxBoxSizer(wxVERTICAL);
        wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
        wxBoxSizer *mbox = new wxBoxSizer(wxVERTICAL);

        // Texto descriptivo
        wxStaticText *labelSel = new wxStaticText(&dialog, wxID_ANY, wxT("Seleccione el actor:"), wxDefaultPosition, wxDefaultSize);
        vboxtop->Add(labelSel, 0, wxTOP, 5); // Espacio arriba

        // Selector
        wxComboBox *comboBox = new wxComboBox(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, fullnames, wxCB_READONLY);
        vboxtop->Add(comboBox, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

        mbox->Add(vboxtop, 0, wxLEFT | wxRIGHT | wxEXPAND, 15); // Espacio arriba

        // Texto descriptivo
        wxStaticText *labelFirst = new wxStaticText(&dialog, wxID_ANY, wxT("Ingrese el primer nombre:"), wxDefaultPosition, wxDefaultSize);
        vboxa->Add(labelFirst, 0, wxTOP, 10); // Espacio arriba

        // Campo de texto
        wxTextCtrl *textCtrlF = new wxTextCtrl(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
        vboxa->Add(textCtrlF, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

        // Texto descriptivo
        wxStaticText *labelLast = new wxStaticText(&dialog, wxID_ANY, wxT("Ingrese el segundo nombre:"), wxDefaultPosition, wxDefaultSize);
        vboxa->Add(labelLast, 0, wxTOP, 10); // Espacio arriba

        // Campo de texto
        wxTextCtrl *textCtrlL = new wxTextCtrl(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
        vboxa->Add(textCtrlL, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

        // Texto descriptivo
        wxStaticText *labelSurr = new wxStaticText(&dialog, wxID_ANY, wxT("Ingrese el apellido:"), wxDefaultPosition, wxDefaultSize);
        vboxa->Add(labelSurr, 0, wxTOP, 10); // Espacio arriba

        // Campo de texto
        wxTextCtrl *textCtrlS = new wxTextCtrl(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
        vboxa->Add(textCtrlS, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

        // Texto descriptivo
        wxStaticText *labelPass = new wxStaticText(&dialog, wxID_ANY, wxT("Ingrese el ID / Nº de Pasaporte:"), wxDefaultPosition, wxDefaultSize);
        vboxb->Add(labelPass, 0, wxTOP, 10); // Espacio arriba

        // Campo de texto
        wxTextCtrl *textCtrlP = new wxTextCtrl(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
        vboxb->Add(textCtrlP, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

        // Texto descriptivo
        wxStaticText *labelDate = new wxStaticText(&dialog, wxID_ANY, wxT("Ingrese la fecha de nacimiento:"), wxDefaultPosition, wxDefaultSize);
        vboxb->Add(labelDate, 0, wxTOP, 10); // Espacio arriba

        // Campo de fecha
        wxDatePickerCtrl *textCtrlD = new wxDatePickerCtrl(&dialog, wxID_ANY, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DEFAULT);
        vboxb->Add(textCtrlD, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

        // Texto descriptivo
        wxStaticText *labelRol = new wxStaticText(&dialog, wxID_ANY, wxT("Ingrese el papel:"), wxDefaultPosition, wxDefaultSize);
        vboxb->Add(labelRol, 0, wxTOP, 10); // Espacio arriba

        // Selector de personaje
        wxComboBox *comboBoxRol = new wxComboBox(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, charactersFullNames, wxCB_READONLY);
        vboxb->Add(comboBoxRol, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

        hbox->Add(vboxa, 1, wxLEFT | wxRIGHT | wxEXPAND, 5); // vboxa ocupa parte de hbox
        hbox->Add(vboxb, 1, wxLEFT | wxRIGHT | wxEXPAND, 5); // vboxb ocupa parte de hbox
        mbox->Add(hbox, 1, wxLEFT | wxRIGHT | wxEXPAND, 10);

        mbox->Add(dialog.CreateButtonSizer(wxOK | wxCANCEL), 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 10);

        dialog.SetSizer(mbox);
        dialog.Fit();

        comboBox->SetSelection(0);          // Seleccionar el primer elemento
        textCtrlP->SetValue(passports[0]);  // Escribir el primer elemento en el cuadro de texto
        textCtrlF->SetValue(firstNames[0]); // Escribir el primer elemento en el cuadro de texto
        textCtrlL->SetValue(lastNames[0]);  // Escribir el primer elemento en el cuadro de texto
        textCtrlS->SetValue(surrnames[0]);  // Escribir el primer elemento en el cuadro de texto

        wxDateTime tempDate;
        if (tempDate.ParseISODate(birthDates[0]))
        {
            textCtrlD->SetValue(tempDate);
        }
        else
        {
            textCtrlD->SetValue(wxDefaultDateTime);
        }

        comboBoxRol->SetSelection(ActorsParentsIdsIndexes[0]); // Seleccionar el primer elemento

        // Boton para cambiar el estado de Ok
        wxButton *okButton = dynamic_cast<wxButton *>(dialog.FindWindow(wxID_OK));
        okButton->Disable();

        // Evento para actualizar el cuadro de texto cuando se cambia la selección en el wxComboBox
        comboBox->Bind(wxEVT_COMBOBOX, [comboBox, comboBoxRol, okButton, textCtrlP, textCtrlF, textCtrlL, textCtrlS, textCtrlD, ActorsIds, passports, firstNames, lastNames, surrnames, birthDates, ActorsParentsIdsIndexes](wxCommandEvent &event)
                       {
                           // textCtrl->SetValue(comboBox->GetStringSelection());
                           // textCtrl->SetValue(wxString::Format(wxT("Editar actor Id Nº: %d"), comboBox->GetSelection()));
                           textCtrlP->SetValue(passports[comboBox->GetSelection()]);
                           textCtrlF->SetValue(firstNames[comboBox->GetSelection()]);
                           textCtrlL->SetValue(lastNames[comboBox->GetSelection()]);
                           textCtrlS->SetValue(surrnames[comboBox->GetSelection()]);

                           wxDateTime tempDate;
                           if (tempDate.ParseISODate(birthDates[comboBox->GetSelection()]))
                           {
                               textCtrlD->SetValue(tempDate);
                           }
                           else
                           {
                               textCtrlD->SetValue(wxDefaultDateTime);
                           }

                           comboBoxRol->SetSelection(ActorsParentsIdsIndexes[comboBox->GetSelection()]);
                           //  textCtrl->SetValue(wxString::Format(wxT("Editar actor Id Nº: %d"), ActorsIds[comboBox->GetSelection()]));
                           //  wxMessageBox(VectorToString(ActorsIds), "Contenido de ActorsIds", wxOK | wxICON_INFORMATION);
                           //  wxMessageBox(ArrayStringToString(fullnames), "Contenido de fullnames", wxOK | wxICON_INFORMATION);
                           okButton->Disable(); }

        );

        // Captura okButton, textCtrl y comboBox para deshabilitar ok si esta vacio o es el mismo
        auto validateTextFields = [okButton, comboBox, comboBoxRol, textCtrlP, textCtrlF, textCtrlL, textCtrlS, textCtrlD, passports, firstNames, lastNames, surrnames, birthDates, CharactersIds, ActorsParentsIdsIndexes](wxCommandEvent &event)
        {
            wxString passport = textCtrlP->GetValue();
            wxString first_name = textCtrlF->GetValue();
            wxString last_name = textCtrlL->GetValue();
            wxString surrname = textCtrlS->GetValue();

            wxDateTime birth_date;
            birth_date.ParseISODate(birthDates[comboBox->GetSelection()]);
            bool dateChanged = textCtrlD->GetValue() != birth_date;

            //                                                                            Devuelve donde esta el numero x en el otro areglo
            bool rolChanged = CharactersIds[comboBoxRol->GetSelection()] != CharactersIds[ActorsParentsIdsIndexes[comboBox->GetSelection()]];

            // Si: (el primero no esta vacio Y el segundo no esta vacio Y el tercero no esta vacio) Y (el primero O el segundo O el tercero es diferente)
            if ((!passport.IsEmpty() && !first_name.IsEmpty() && !surrname.IsEmpty()) && (passports[comboBox->GetSelection()] != passport || firstNames[comboBox->GetSelection()] != first_name || lastNames[comboBox->GetSelection()] != last_name || surrnames[comboBox->GetSelection()] != surrname || dateChanged || rolChanged))
            {
                okButton->Enable();
            }
            else
            {
                okButton->Disable();
            }
        };

        // Vinculamos el evento a cualquier cambio en cualquiera de los controles
        textCtrlP->Bind(wxEVT_TEXT, validateTextFields);
        textCtrlF->Bind(wxEVT_TEXT, validateTextFields);
        textCtrlL->Bind(wxEVT_TEXT, validateTextFields);
        textCtrlS->Bind(wxEVT_TEXT, validateTextFields);
        textCtrlD->Bind(wxEVT_DATE_CHANGED, validateTextFields);
        comboBoxRol->Bind(wxEVT_COMBOBOX, validateTextFields);

        // Mostrar el cuadro de diálogo y obtener el resultado
        if (dialog.ShowModal() == wxID_OK)
        {
            wxString editedPassport = textCtrlP->GetValue();
            wxString editedFirst_name = textCtrlF->GetValue();
            wxString editedLast_name = textCtrlL->GetValue();
            wxString editedSurrname = textCtrlS->GetValue();
            wxString editedDate = textCtrlD->GetValue().FormatISODate();
            int editedParent_id = CharactersIds[comboBoxRol->GetSelection()];

            if (!editedFirst_name.IsEmpty() && !editedSurrname.IsEmpty() && !editedPassport.IsEmpty())
            {
                if (passports[comboBox->GetSelection()] == editedPassport && firstNames[comboBox->GetSelection()] == editedFirst_name && lastNames[comboBox->GetSelection()] == editedLast_name && surrnames[comboBox->GetSelection()] == editedSurrname && birthDates[comboBox->GetSelection()] == editedDate && ActorsParentsIds[comboBox->GetSelection()] == editedParent_id)
                {
                    wxMessageBox(wxT("No puede tener los mismos datos!"), "Error", wxOK | wxICON_ERROR);
                }

                else
                {
                    if (!checkPassportExists(actors, editedPassport.ToStdString(), ActorsIds[comboBox->GetSelection()]))
                    {
                        Actor updatedActor(ActorsIds[comboBox->GetSelection()], editedParent_id, editedPassport.ToStdString(), editedFirst_name.ToStdString(), editedLast_name.ToStdString(), editedSurrname.ToStdString(), editedDate.ToStdString());
                        updateActor(actors, updatedActor, true, true, true, true, true, true); // (Arreglo, elementoActualizado, parentId, pasaporte, pNombre, sNombre, apellido, fecha)

                        // wxMessageBox(wxString::Format(wxT("Editar actor Id Nº: %d"), ActorsIds[comboBox->GetSelection()]), "Ok", wxOK | wxICON_INFORMATION);
                        mod = true;
                    }

                    else
                    {
                        wxMessageBox(wxT("Ese pasaporte ya existe"), // CONTENIDO VENTANA POP UP
                                     "Error", wxOK | wxICON_ERROR);  // TITULO VENTANA POP UP
                    }
                }
            }

            else
            {
                wxMessageBox(wxT("No puede estar vacío!"), "Error", wxOK | wxICON_ERROR);
            }
        }
    }

    else // Si esta vacio (no hay nada que editar, error!)
    {
        wxMessageBox(wxT("Primero crea un actor!!"), "Error", wxOK | wxICON_ERROR);
    }
}

void MainWindow::OnActorDel(wxCommandEvent &event)
{
}

// ADDRESS MENU
void MainWindow::OnNewAddress(wxCommandEvent &event)
{
    wxDialog dialog(NULL, wxID_ANY, wxT("Nueva Locación"), wxDefaultPosition, wxDefaultSize);

    wxBoxSizer *vboxtop = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *vboxa = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *vboxb = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *mbox = new wxBoxSizer(wxVERTICAL);

    // Texto descriptivo
    wxStaticText *labelName = new wxStaticText(&dialog, wxID_ANY, wxT("Ingrese el nombre:"), wxDefaultPosition, wxDefaultSize);
    vboxtop->Add(labelName, 0, wxTOP, 5); // Espacio arriba

    // Campo de texto
    wxTextCtrl *textCtrlN = new wxTextCtrl(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
    vboxtop->Add(textCtrlN, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

    mbox->Add(vboxtop, 0, wxLEFT | wxRIGHT | wxEXPAND, 15); // Espacio arriba

    // Texto descriptivo
    wxStaticText *labelAdd = new wxStaticText(&dialog, wxID_ANY, wxT("Ingrese la dirección:"), wxDefaultPosition, wxDefaultSize);
    vboxa->Add(labelAdd, 0, wxTOP, 10); // Espacio arriba

    // Campo de texto
    wxTextCtrl *textCtrlA = new wxTextCtrl(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
    vboxa->Add(textCtrlA, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

    // Texto descriptivo
    wxStaticText *labelHosp = new wxStaticText(&dialog, wxID_ANY, wxT("Ingrese el hospital cercano:"), wxDefaultPosition, wxDefaultSize);
    vboxa->Add(labelHosp, 0, wxTOP, 10); // Espacio arriba

    // Campo de texto
    wxTextCtrl *textCtrlH = new wxTextCtrl(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
    vboxa->Add(textCtrlH, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

    // Texto descriptivo
    wxStaticText *labelTel = new wxStaticText(&dialog, wxID_ANY, wxT("Ingrese el teléfono:"), wxDefaultPosition, wxDefaultSize);
    vboxb->Add(labelTel, 0, wxTOP, 10); // Espacio arriba

    // Campo de texto
    wxTextCtrl *textCtrlT = new wxTextCtrl(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
    vboxb->Add(textCtrlT, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

    // Texto descriptivo
    wxStaticText *labelPark = new wxStaticText(&dialog, wxID_ANY, wxT("Ingrese el estacionamiento:"), wxDefaultPosition, wxDefaultSize);
    vboxb->Add(labelPark, 0, wxTOP, 10); // Espacio arriba

    // Campo de texto
    wxTextCtrl *textCtrlP = new wxTextCtrl(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
    vboxb->Add(textCtrlP, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

    hbox->Add(vboxa, 1, wxLEFT | wxRIGHT | wxEXPAND, 5); // vboxa ocupa parte de hbox
    hbox->Add(vboxb, 1, wxLEFT | wxRIGHT | wxEXPAND, 5); // vboxb ocupa parte de hbox
    mbox->Add(hbox, 1, wxLEFT | wxRIGHT | wxEXPAND, 10);

    mbox->Add(dialog.CreateButtonSizer(wxOK | wxCANCEL), 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 10);

    dialog.SetSizer(mbox);
    dialog.Fit();

    // Boton para cambiar el estado de Ok
    wxButton *okButton = dynamic_cast<wxButton *>(dialog.FindWindow(wxID_OK));
    okButton->Disable();

    // Captura okButton, textCtrl y comboBox para deshabilitar "okButton" si esta vacio
    auto validateTextFields = [okButton, textCtrlN, textCtrlA](wxCommandEvent &event)
    {
        wxString name = textCtrlN->GetValue();
        wxString address = textCtrlA->GetValue();

        // Habilitar el botón solo si todos los campos no están vacíos
        if (!name.IsEmpty() && !address.IsEmpty())
        {
            okButton->Enable();
        }
        else
        {
            okButton->Disable();
        }
    };

    // Vinculamos el evento a cualquier cambio en cualquiera de los 3 controles
    textCtrlN->Bind(wxEVT_TEXT, validateTextFields);
    textCtrlA->Bind(wxEVT_TEXT, validateTextFields);

    // Mostrar el cuadro de diálogo y obtener el resultado
    if (dialog.ShowModal() == wxID_OK)
    {
        wxString name = textCtrlN->GetValue();
        wxString address = textCtrlA->GetValue();
        wxString hospital = textCtrlH->GetValue();
        wxString telephone = textCtrlT->GetValue();
        wxString parking = textCtrlP->GetValue();
        nextNumber = firstEmpty(locationsArray);

        if (!name.IsEmpty() && !address.IsEmpty())
        {
            if (!checkAddressExists(locations, address.ToStdString(), nextNumber))
            {
                locationsArray.push_back(nextNumber);

                Location newLocation(nextNumber, name.ToStdString(), address.ToStdString(), telephone.ToStdString(), hospital.ToStdString(), parking.ToStdString());
                locations.push_back(newLocation);
                // wxMessageBox(wxString::Format(wxT("Crear locacion Id Nº: %d"),), "Ok", wxOK | wxICON_INFORMATION);
            }

            else
            {
                wxMessageBox(wxT("Esa dirección ya existe"), // CONTENIDO VENTANA POP UP
                             "Error", wxOK | wxICON_ERROR);  // TITULO VENTANA POP UP
            }
        }

        else
        {
            wxMessageBox(wxT("No puede estar vacío!"), "Error", wxOK | wxICON_ERROR);
        }
    }
}

void MainWindow::OnAddressEdit(wxCommandEvent &event)
{
    if (!locations.empty()) // Si hay algo que editar (si no está vacío), editar
    {
        std::vector<int> LocationsIds;
        wxArrayString LocationsNames;
        wxArrayString LocationsAddress;
        wxArrayString LocationsPhones;
        wxArrayString LocationsHospitals;
        wxArrayString LocationsParkings;
        wxArrayString fullnames;

        for (const auto &location : locations)
        {
            LocationsIds.push_back(location.id);
            LocationsNames.Add(location.name);
            LocationsAddress.Add(location.address);
            LocationsHospitals.Add(location.hospital);
            LocationsPhones.Add(location.phone);
            LocationsParkings.Add(location.parking);

            fullnames.Add(location.name + " (" + location.address + ")");
        }

        wxDialog dialog(NULL, wxID_ANY, wxT("Editar Locación"), wxDefaultPosition, wxDefaultSize);

        wxBoxSizer *vboxtop = new wxBoxSizer(wxVERTICAL);
        wxBoxSizer *vboxa = new wxBoxSizer(wxVERTICAL);
        wxBoxSizer *vboxb = new wxBoxSizer(wxVERTICAL);
        wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
        wxBoxSizer *mbox = new wxBoxSizer(wxVERTICAL);

        // Texto descriptivo
        wxStaticText *labelSel = new wxStaticText(&dialog, wxID_ANY, wxT("Seleccione la locación:"), wxDefaultPosition, wxDefaultSize);
        vboxtop->Add(labelSel, 0, wxTOP, 5); // Espacio arriba

        // Selector
        wxComboBox *comboBox = new wxComboBox(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, fullnames, wxCB_READONLY);
        vboxtop->Add(comboBox, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

        // Texto descriptivo
        wxStaticText *labelName = new wxStaticText(&dialog, wxID_ANY, wxT("Ingrese el nombre:"), wxDefaultPosition, wxDefaultSize);
        vboxtop->Add(labelName, 0, wxTOP, 5); // Espacio arriba

        // Campo de texto
        wxTextCtrl *textCtrlN = new wxTextCtrl(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
        vboxtop->Add(textCtrlN, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

        mbox->Add(vboxtop, 0, wxLEFT | wxRIGHT | wxEXPAND, 15); // Espacio arriba

        // Texto descriptivo
        wxStaticText *labelAdd = new wxStaticText(&dialog, wxID_ANY, wxT("Ingrese la dirección:"), wxDefaultPosition, wxDefaultSize);
        vboxa->Add(labelAdd, 0, wxTOP, 10); // Espacio arriba

        // Campo de texto
        wxTextCtrl *textCtrlA = new wxTextCtrl(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
        vboxa->Add(textCtrlA, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

        // Texto descriptivo
        wxStaticText *labelHosp = new wxStaticText(&dialog, wxID_ANY, wxT("Ingrese el hospital cercano:"), wxDefaultPosition, wxDefaultSize);
        vboxa->Add(labelHosp, 0, wxTOP, 10); // Espacio arriba

        // Campo de texto
        wxTextCtrl *textCtrlH = new wxTextCtrl(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
        vboxa->Add(textCtrlH, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

        // Texto descriptivo
        wxStaticText *labelTel = new wxStaticText(&dialog, wxID_ANY, wxT("Ingrese el teléfono:"), wxDefaultPosition, wxDefaultSize);
        vboxb->Add(labelTel, 0, wxTOP, 10); // Espacio arriba

        // Campo de texto
        wxTextCtrl *textCtrlT = new wxTextCtrl(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
        vboxb->Add(textCtrlT, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

        // Texto descriptivo
        wxStaticText *labelPark = new wxStaticText(&dialog, wxID_ANY, wxT("Ingrese el estacionamiento:"), wxDefaultPosition, wxDefaultSize);
        vboxb->Add(labelPark, 0, wxTOP, 10); // Espacio arriba

        // Campo de texto
        wxTextCtrl *textCtrlP = new wxTextCtrl(&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
        vboxb->Add(textCtrlP, 0, wxTOP | wxEXPAND, 5); // Espacio arriba

        hbox->Add(vboxa, 1, wxLEFT | wxRIGHT | wxEXPAND, 5); // vboxa ocupa parte de hbox
        hbox->Add(vboxb, 1, wxLEFT | wxRIGHT | wxEXPAND, 5); // vboxb ocupa parte de hbox
        mbox->Add(hbox, 1, wxLEFT | wxRIGHT | wxEXPAND, 10);

        mbox->Add(dialog.CreateButtonSizer(wxOK | wxCANCEL), 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 10);

        dialog.SetSizer(mbox);
        dialog.Fit();

        comboBox->SetSelection(0);                  // Seleccionar el primer elemento
        textCtrlN->SetValue(LocationsNames[0]);     // Escribir el primer elemento en el cuadro de texto
        textCtrlA->SetValue(LocationsAddress[0]);   // Escribir el primer elemento en el cuadro de texto
        textCtrlH->SetValue(LocationsHospitals[0]); // Escribir el primer elemento en el cuadro de texto
        textCtrlT->SetValue(LocationsPhones[0]);    // Escribir el primer elemento en el cuadro de texto
        textCtrlP->SetValue(LocationsParkings[0]);  // Escribir el primer elemento en el cuadro de texto

        // Boton para cambiar el estado de Ok
        wxButton *okButton = dynamic_cast<wxButton *>(dialog.FindWindow(wxID_OK));
        okButton->Disable();

        // Evento para actualizar el cuadro de texto cuando se cambia la selección en el wxComboBox
        comboBox->Bind(wxEVT_COMBOBOX, [comboBox, okButton, textCtrlN, textCtrlA, textCtrlH, textCtrlT, textCtrlP, LocationsIds, LocationsNames, LocationsAddress, LocationsHospitals, LocationsPhones, LocationsParkings](wxCommandEvent &event)
                       {
                           // textCtrl->SetValue(comboBox->GetStringSelection());
                           // textCtrl->SetValue(wxString::Format(wxT("Editar locación Id Nº: %d"), comboBox->GetSelection()));
                           textCtrlN->SetValue(LocationsNames[comboBox->GetSelection()]);
                           textCtrlA->SetValue(LocationsAddress[comboBox->GetSelection()]);
                           textCtrlH->SetValue(LocationsHospitals[comboBox->GetSelection()]);
                           textCtrlT->SetValue(LocationsPhones[comboBox->GetSelection()]);
                           textCtrlP->SetValue(LocationsParkings[comboBox->GetSelection()]);

                           //  textCtrl->SetValue(wxString::Format(wxT("Editar locación Id Nº: %d"), LocationsIds[comboBox->GetSelection()]));
                           //  wxMessageBox(VectorToString(LocationsIds), "Contenido de LocationsIds", wxOK | wxICON_INFORMATION);
                           //  wxMessageBox(ArrayStringToString(fullnames), "Contenido de fullnames", wxOK | wxICON_INFORMATION);
                           okButton->Disable(); }

        );

        // Captura okButton, textCtrl y comboBox para deshabilitar ok si esta vacio o es el mismo
        auto validateTextFields = [okButton, comboBox, textCtrlN, textCtrlA, textCtrlH, textCtrlT, textCtrlP, LocationsNames, LocationsAddress, LocationsHospitals, LocationsPhones, LocationsParkings](wxCommandEvent &event)
        {
            wxString name = textCtrlN->GetValue();
            wxString address = textCtrlA->GetValue();
            wxString hospital = textCtrlH->GetValue();
            wxString telephone = textCtrlT->GetValue();
            wxString parking = textCtrlP->GetValue();

            // Si: (el primero no esta vacio Y el segundo no esta vacio) Y (el primero O el segundo O el tercero es diferente)
            if ((!name.IsEmpty() && !address.IsEmpty()) && (LocationsNames[comboBox->GetSelection()] != name || LocationsAddress[comboBox->GetSelection()] != address || LocationsHospitals[comboBox->GetSelection()] != hospital || LocationsPhones[comboBox->GetSelection()] != telephone || LocationsParkings[comboBox->GetSelection()] != parking))
            {
                okButton->Enable();
            }
            else
            {
                okButton->Disable();
            }
        };

        // Vinculamos el evento a cualquier cambio en cualquiera de los controles
        textCtrlN->Bind(wxEVT_TEXT, validateTextFields);
        textCtrlA->Bind(wxEVT_TEXT, validateTextFields);
        textCtrlH->Bind(wxEVT_TEXT, validateTextFields);
        textCtrlT->Bind(wxEVT_TEXT, validateTextFields);
        textCtrlP->Bind(wxEVT_TEXT, validateTextFields);

        // Mostrar el cuadro de diálogo y obtener el resultado
        if (dialog.ShowModal() == wxID_OK)
        {
            wxString editedName = textCtrlN->GetValue();
            wxString editedAddress = textCtrlA->GetValue();
            wxString editedHospital = textCtrlH->GetValue();
            wxString editedPhone = textCtrlT->GetValue();
            wxString editedParking = textCtrlP->GetValue();

            if (!editedName.IsEmpty() && !editedAddress.IsEmpty())
            {
                if (LocationsNames[comboBox->GetSelection()] == editedName && LocationsAddress[comboBox->GetSelection()] == editedAddress && LocationsHospitals[comboBox->GetSelection()] == editedHospital && LocationsPhones[comboBox->GetSelection()] == editedPhone && LocationsParkings[comboBox->GetSelection()] == editedParking)
                {
                    wxMessageBox(wxT("No puede tener los mismos datos!"), "Error", wxOK | wxICON_ERROR);
                }

                else
                {
                    if (!checkAddressExists(locations, editedAddress.ToStdString(), LocationsIds[comboBox->GetSelection()]))
                    {
                        Location updatedLocation(LocationsIds[comboBox->GetSelection()], editedName.ToStdString(), editedAddress.ToStdString(), editedHospital.ToStdString(), editedPhone.ToStdString(), editedParking.ToStdString());
                        // updateActor(actors, updatedActor, true, true, true, true, true, true); // (Arreglo, elementoActualizado, parentId, pasaporte, pNombre, sNombre, apellido, fecha)

                        // wxMessageBox(wxString::Format(wxT("Editar locacion Id Nº: %d"), LocationsIds[comboBox->GetSelection()]), "Ok", wxOK | wxICON_INFORMATION);
                        mod = true;
                    }

                    else
                    {
                        wxMessageBox(wxT("Esa dirección ya existe"), // CONTENIDO VENTANA POP UP
                                     "Error", wxOK | wxICON_ERROR);  // TITULO VENTANA POP UP
                    }
                }
            }

            else
            {
                wxMessageBox(wxT("No puede estar vacío!"), "Error", wxOK | wxICON_ERROR);
            }
        }
    }

    else // Si esta vacio (no hay nada que editar, error!)
    {
        wxMessageBox(wxT("Primero crea una locación!!"), "Error", wxOK | wxICON_ERROR);
    }
}

void MainWindow::OnAddressDel(wxCommandEvent &event)
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