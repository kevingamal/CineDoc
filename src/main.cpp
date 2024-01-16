#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <wx/sizer.h>
#include <wx/wxprec.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm> // para std::sort

wxDECLARE_EVENT(wxEVT_UPDATE_POSITION_EVENT, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_UPDATE_INDEX_EVENT, wxCommandEvent);

enum
{
    ID_Hello = 1,
    ID_SCRIPT_NEW = 2,
    ID_SCRIPT_EDIT = 3,
    ID_SCRIPT_DEL = 4
};

// VECTOR (solo para almacenar temporalmente los indices y reciclar los antiguos)
std::vector<int> textBoxsContainer;
std::vector<int> itemsListContainer;
// numeros.push_back(x); // Añade el numero x al final
// int primerNumero = numeros[0]; // Obtenemos el numero en la primera posicion
// size_t cantidad = numeros.size(); // Obtenemos la cantidad de items en el vector
// std::sort(numeros.begin(), numeros.end()); // Ordena ascendentemente (se puede usar al revés)

// Posiciones de la lista
int textIndexPosition;
int itemIndexPosition;
int textPanelPosition;
int itemPanelPosition;

// DATA CLASSES:

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

class Location
{
public:
    int id;
    std::string name;
    std::string adress;
    std::string phone;
    std::string hospital;
    std::string parking;

    Location() {}

    Location(int id, std::string name, std::string adress, std::string phone, std::string hospital, std::string parking)
        : id(id), name(name), adress(adress), phone(phone), hospital(hospital), parking(parking) {}

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

    Object() {}

    Object(int id, std::string name, std::string description, std::string type)
        : id(id), name(name), description(description), type(type) {}

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {

        ar & id & name & description & type;
    }
};

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

    Actor() {}

    Actor(int id, int parentId, int passport_id, std::string first_name, std::string last_name, std::string surrname, std::string birthdate)
        : id(id), parentId(parentId), passport_id(passport_id), first_name(first_name), last_name(last_name), surrname(surrname), birthdate(birthdate) {}

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & id & parentId & passport_id & first_name & last_name & surrname & birthdate;
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

    Take(int id, int parentId, int number, int shot_size, int movement, int mount, int camera, int lens, int sound, int length, std::string description, std::string image, std::string floor_plan, int position)
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
    int id;
    int parentId; // Relaciona actores y objetos con escenas, si es un objeto tendra un id en ojbectId, si es actor tendra un id en actorId
    int actorId;
    int objectId;
    int position;

    Use_case() {}

    Use_case(int id, int parentId, int actorId, int objectId, int position)
        : id(id), parentId(parentId), actorId(actorId), objectId(objectId), position(position) {}

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
    int id;
    int parentId; // Relaciona objetos con tomas
    int objectId;
    int position;

    Tech_use() {}

    Tech_use(int id, int parentId, int objectId, int position)
        : id(id), parentId(parentId), objectId(objectId), position(position) {}
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
    int id;
    int parentId;
    int type;
    int position;

    Event() {}

    Event(int id, int parentId, int type, int position)
        : id(id), parentId(parentId), type(type), position(position) {}

    // Función de serialización
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & id & parentId & type & position;
    }
};

// DATA ARRAYS
std::vector<Script> scripts = {};
std::vector<Character> characters = {};
std::vector<Location> locations = {};
std::vector<Object> objects = {};
std::vector<Scene> scenes = {};
std::vector<Actor> actors = {};
std::vector<Take> takes = {};
std::vector<Use_case> use_cases = {};
std::vector<Tech_use> tech_uses = {};
std::vector<Event> events = {};

// TEMP GUI DATA ARRAYS
std::vector<Scene> scenesTemp = {};
std::vector<Take> takesTemp = {};
std::vector<Use_case> use_casesTemp = {};
std::vector<Tech_use> tech_usesTemp = {};
std::vector<Event> eventsTemp = {};

// ARRAYS FUNCTIONS

// SCENES
void transferScenes(std::vector<Scene> &source, std::vector<Scene> &destination, int specificParentId)
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

void updateScenes(std::vector<Scene> &source, std::vector<Scene> &destination, int specificParentId)
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

void updateScenePosition(std::vector<Scene> &array, int specificParentId, int specificId, int newPosition)
{
    for (auto &scene : array)
    {
        if (scene.parentId == specificParentId && scene.id == specificId)
        {
            scene.position = newPosition;
            break; // Salir del bucle una vez que se actualice el elemento
        }
    }
}

void removeScene(std::vector<Scene> &array, int specificParentId, int specificId)
{
    // Utilizamos un iterador y std::remove_if para encontrar y eliminar el elemento
    array.erase(std::remove_if(array.begin(), array.end(),
                               [specificParentId, specificId](const Scene &scene)
                               {
                                   return scene.parentId == specificParentId && scene.id == specificId;
                               }),
                array.end());
}
// removeScene(scenesTemp, specificParentId, specificId);

// TAKES
void transferTakes(std::vector<Take> &source, std::vector<Take> &destination, int specificParentId)
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

void updateTakes(std::vector<Take> &source, std::vector<Take> &destination, int specificParentId)
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

void updateTakePosition(std::vector<Take> &array, int specificParentId, int specificId, int newPosition)
{
    for (auto &take : array)
    {
        if (take.parentId == specificParentId && take.id == specificId)
        {
            take.position = newPosition;
            break; // Salir del bucle una vez que se actualice el elemento
        }
    }
}

void removeTake(std::vector<Take> &array, int specificParentId, int specificId)
{
    // Utilizamos un iterador y std::remove_if para encontrar y eliminar el elemento
    array.erase(std::remove_if(array.begin(), array.end(),
                               [specificParentId, specificId](const Take &take)
                               {
                                   return take.parentId == specificParentId && take.id == specificId;
                               }),
                array.end());
}

// USE CASES (ACTING AND OBJECT)
void transferUseCase(std::vector<Use_case> &source, std::vector<Use_case> &destination, int specificParentId)
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

void updateUse_Case(std::vector<Use_case> &source, std::vector<Use_case> &destination, int specificParentId)
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

void updateUse_CasePosition(std::vector<Use_case> &array, int specificParentId, int specificId, int newPosition)
{
    for (auto &use_case : array)
    {
        if (use_case.parentId == specificParentId && use_case.id == specificId)
        {
            use_case.position = newPosition;
            break; // Salir del bucle una vez que se actualice el elemento
        }
    }
}

void removeUse_Case(std::vector<Use_case> &array, int specificParentId, int specificId)
{
    // Utilizamos un iterador y std::remove_if para encontrar y eliminar el elemento
    array.erase(std::remove_if(array.begin(), array.end(),
                               [specificParentId, specificId](const Use_case &use_case)
                               {
                                   return use_case.parentId == specificParentId && use_case.id == specificId;
                               }),
                array.end());
}

// TECH USE
void transferTech_use(std::vector<Tech_use> &source, std::vector<Tech_use> &destination, int specificParentId)
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

void updateTech_use(std::vector<Tech_use> &source, std::vector<Tech_use> &destination, int specificParentId)
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

void updateTech_usePosition(std::vector<Tech_use> &array, int specificParentId, int specificId, int newPosition)
{
    for (auto &tech_use : array)
    {
        if (tech_use.parentId == specificParentId && tech_use.id == specificId)
        {
            tech_use.position = newPosition;
            break; // Salir del bucle una vez que se actualice el elemento
        }
    }
}

void removeTech_use(std::vector<Tech_use> &array, int specificParentId, int specificId)
{
    // Utilizamos un iterador y std::remove_if para encontrar y eliminar el elemento
    array.erase(std::remove_if(array.begin(), array.end(),
                               [specificParentId, specificId](const Tech_use &tech_use)
                               {
                                   return tech_use.parentId == specificParentId && tech_use.id == specificId;
                               }),
                array.end());
}

// EVENTS
void transferEvents(std::vector<Event> &source, std::vector<Event> &destination, int specificParentId)
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

void updateEvents(std::vector<Event> &source, std::vector<Event> &destination, int specificParentId)
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

void updateEventPosition(std::vector<Event> &array, int specificParentId, int specificId, int newPosition)
{
    for (auto &event : array)
    {
        if (event.parentId == specificParentId && event.id == specificId)
        {
            event.position = newPosition;
            break; // Salir del bucle una vez que se actualice el elemento
        }
    }
}
// updateEventsPosition(eventsTemp, specificParentId, specificId, newPosition);

void removeEvent(std::vector<Event> &array, int specificParentId, int specificId)
{
    // Utilizamos un iterador y std::remove_if para encontrar y eliminar el elemento
    array.erase(std::remove_if(array.begin(), array.end(),
                               [specificParentId, specificId](const Event &event)
                               {
                                   return event.parentId == specificParentId && event.id == specificId;
                               }),
                array.end());
}
// removeEvent(eventsTemp, specificParentId, specificId);

// CONTROLS CLASSES

class TitledTextBox : public wxPanel
{
public:
    TitledTextBox(wxWindow *parent, wxBoxSizer *sizer, int index, const wxString &text)
        : wxPanel(parent, wxID_ANY), textBox(nullptr), parentSizer(sizer), itemPosition(index)
    {
        // this->SetFocus();
        wxBoxSizer *sizerLocal = new wxBoxSizer(wxVERTICAL);

        // Sizer horizontal para el título y los botones
        wxBoxSizer *titleSizer = new wxBoxSizer(wxHORIZONTAL);

        // Botón de colapsar
        wxButton *collapseButton = new wxButton(this, wxID_ANY, "-", wxDefaultPosition, wxSize(25, 25), wxBORDER_NONE); // Cambiado "^" por "-"
        titleSizer->Add(collapseButton, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
        collapseButton->Bind(wxEVT_BUTTON, &TitledTextBox::OncollapseButtonClick, this);

        // Botón de arriba
        wxButton *upButton = new wxButton(this, wxID_ANY, L"\u2191", wxDefaultPosition, wxSize(25, 25), wxBORDER_NONE);
        titleSizer->Add(upButton, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
        upButton->Bind(wxEVT_BUTTON, &TitledTextBox::OnupButtonClick, this);

        // Botón de abajo
        wxButton *downButton = new wxButton(this, wxID_ANY, L"\u2193", wxDefaultPosition, wxSize(25, 25), wxBORDER_NONE);
        titleSizer->Add(downButton, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
        downButton->Bind(wxEVT_BUTTON, &TitledTextBox::OndownButtonClick, this);

        // BARRA DE TITULO
        wxString title = wxString::Format("Fragmento %d", index);
        wxStaticText *titleLabel = new wxStaticText(this, wxID_ANY, title);
        titleSizer->Add(titleLabel, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);

        // Boton de edicion >
        wxButton *editButton = new wxButton(this, wxID_ANY, L"\u2192", wxDefaultPosition, wxSize(25, 25), wxBORDER_NONE); // u270F
        titleSizer->Add(editButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
        editButton->Bind(wxEVT_BUTTON, &TitledTextBox::OneditButtonClick, this);

        // Boton de eliminar
        wxButton *deleteButton = new wxButton(this, wxID_ANY, "x", wxDefaultPosition, wxSize(25, 25), wxBORDER_NONE);
        titleSizer->Add(deleteButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
        deleteButton->Bind(wxEVT_BUTTON, &TitledTextBox::OndeleteButtonClick, this);

        // CUADRO DE TEXTO
        textBox = new wxTextCtrl(this, wxID_ANY, text,
                                 wxDefaultPosition, wxDefaultSize,
                                 wxTE_MULTILINE | wxTE_READONLY);

        sizerLocal->Add(titleSizer, 0, wxEXPAND);

        // Establecer el cursor a la flecha estándar
        textBox->SetCursor(wxCURSOR_ARROW);
        // Bloquea la edicion (ya estaba bloqueada, es para que ande el scroll)
        textBox->SetEditable(false);
        // Manda todos los eventos de la rueda al controlador padre
        textBox->Bind(wxEVT_MOUSEWHEEL, &TitledTextBox::OnMouseWheel, this);

        sizerLocal->Add(textBox, 1, wxEXPAND | wxALL, 5);

        SetSizer(sizerLocal);

        // Añade los eventos de ratón
        Bind(wxEVT_LEFT_DOWN, &TitledTextBox::OnMouseDown, this);
        Bind(wxEVT_LEFT_UP, &TitledTextBox::OnMouseUp, this);
        Bind(wxEVT_MOTION, &TitledTextBox::OnMouseMove, this);
    }

    wxTextCtrl *GetTextBox() const { return textBox; }

    void OncollapseButtonClick(wxCommandEvent &event)
    {
        textBox->Show(!textBox->IsShown());
        wxButton *btn = dynamic_cast<wxButton *>(event.GetEventObject());
        if (btn)
        {
            // Si el textBox ahora está mostrándose, cambia el símbolo a "-"
            // De lo contrario, cambia el símbolo a "+"
            btn->SetLabel(textBox->IsShown() ? "-" : "+");
        }
        Layout();
        dynamic_cast<wxScrolledWindow *>(GetParent())->FitInside();
    }

    void OnupButtonClick(wxCommandEvent &event)
    {
        desiredPosition = GetItemPositionInSizer(parentSizer, this) - 1;
        MoveToDesiredPosition();
        textPanelPosition = GetItemPositionInSizer(parentSizer, this);
    }

    void OndownButtonClick(wxCommandEvent &event)
    {
        desiredPosition = GetItemPositionInSizer(parentSizer, this) + 1;
        MoveToDesiredPosition();
        textPanelPosition = GetItemPositionInSizer(parentSizer, this);
    }

    void OneditButtonClick(wxCommandEvent &event)
    {
        // Implementa la lógica para este botón aquí
    }

    void OndeleteButtonClick(wxCommandEvent &event)
    {
        deleteVectorItem(textBoxsContainer, itemPosition);
        removeScene(scenes, 1, itemPosition);
        wxWindow *parentWindow = GetParent(); // Guardar referencia al padre antes de destruir
        Destroy();
        parentWindow->Layout(); // Llamar al Layout del padre
        if (wxDynamicCast(parentWindow, wxScrolledWindow))
        {
            dynamic_cast<wxScrolledWindow *>(parentWindow)->FitInside();
        }
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

    void OnMouseDown(wxMouseEvent &event)
    {
        // wxLogMessage(wxT("OnMouseDown llamado"));
        //  CaptureMouse();
        //   dragStartPosition = ClientToScreen(event.GetPosition());
        //   initialWindowPosition = GetPosition();

        // IMPRESION DEL Nº DE ITEM EN PANTALLA
        // textPanelPosition = GetItemPositionInSizer(parentSizer, this);
        // textIndexPosition = itemPosition;

        // textPanelPosition se averigua cada vez que se selecciona el panel, para saber donde está (ya que puede cambiarse)
        // es inamovible e unico, sirve para saber su id en la BD

        // wxCommandEvent evt(wxEVT_UPDATE_POSITION_EVENT);
        // evt.SetInt(textPanelPosition);
        // wxPostEvent(GetParent(), evt);
        // event.Skip();
    }

    void OnMouseUp(wxMouseEvent &event)
    {
        // MoveToDesiredPosition();
        // wxLogMessage(wxT("OnMouseUp llamado"));
        // textPanelPosition = GetItemPositionInSizer(parentSizer, this);

        // if (HasCapture())
        //     ReleaseMouse();

        // wxCommandEvent evt(wxEVT_UPDATE_POSITION_EVENT);
        // evt.SetInt(textPanelPosition);
        // wxPostEvent(GetParent(), evt);
        //  event.Skip();
    }

    void OnMouseMove(wxMouseEvent &event)
    {
        textPanelPosition = GetItemPositionInSizer(parentSizer, this) + 1;
        textIndexPosition = itemPosition;

        wxCommandEvent evta(wxEVT_UPDATE_POSITION_EVENT);
        evta.SetInt(textPanelPosition);
        wxPostEvent(GetParent(), evta);

        wxCommandEvent evtb(wxEVT_UPDATE_INDEX_EVENT);
        evtb.SetInt(textIndexPosition);
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
            }
            desiredPosition = -1; // Resetea la posición deseada después de procesarla.
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

private:
    wxTextCtrl *textBox;
    wxBoxSizer *parentSizer;
    int desiredPosition = -1;
    int thresholdTop = -1;
    int thresholdBottom = -1;
    int itemPosition;
};

class ItemTextList : public wxPanel
{
public:
    ItemTextList(wxWindow *parent, wxBoxSizer *sizer, int index, const wxString &text)
        : wxPanel(parent, wxID_ANY), parentSizer(sizer), itemPosition(index)
    {
        // this->SetFocus();
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
        itemPanelPosition = GetItemPositionInSizer(parentSizer, this);
    }

    void OndownButtonClick(wxCommandEvent &event)
    {
        desiredPosition = GetItemPositionInSizer(parentSizer, this) + 1;
        MoveToDesiredPosition();
        itemPanelPosition = GetItemPositionInSizer(parentSizer, this);
    }

    void OndeleteButtonClick(wxCommandEvent &event)
    {
        deleteVectorItem(itemsListContainer, itemPosition);
        wxWindow *parentWindow = GetParent(); // Guardar referencia al padre antes de destruir
        Destroy();
        parentWindow->Layout(); // Llamar al Layout del padre
        if (wxDynamicCast(parentWindow, wxScrolledWindow))
        {
            dynamic_cast<wxScrolledWindow *>(parentWindow)->FitInside();
        }
    }

    void OnMouseMove(wxMouseEvent &event)
    {
        itemPanelPosition = GetItemPositionInSizer(parentSizer, this) + 1;
        itemIndexPosition = itemPosition;

        wxCommandEvent evta(wxEVT_UPDATE_POSITION_EVENT); // se define en la linea 1063, 1186
        evta.SetInt(itemPanelPosition);
        wxPostEvent(GetParent(), evta);

        wxCommandEvent evtb(wxEVT_UPDATE_INDEX_EVENT); // se define en la linea 1064, 1187
        evtb.SetInt(itemIndexPosition);
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

                // For que recorra a todos y escriba para cada uno lo que devuelva GetItemPositionInSizer() a position en el arreglo temporal
                // Dependiendo de un if que lea el nivel invocara a la funcion que corresponda
            }
            desiredPosition = -1; // Resetea la posición deseada después de procesarla.
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

private:
    wxBoxSizer *parentSizer;
    int desiredPosition = -1;
    int thresholdTop = -1;
    int thresholdBottom = -1;
    int itemPosition;
};

class MyFrame : public wxFrame
{
public:
    MyFrame(const wxString &title, const wxPoint &pos, const wxSize &size)
        : wxFrame(NULL, wxID_ANY, title, pos, size), leftTextBox(nullptr)
    {
        // MENU PROYECTO
        wxMenu *menuProject = new wxMenu;
        // nombreMenu->añadir(EVENTO, "nombreItem\KeyShortcut", "mssg to statusbar")//
        // menuProject->Append(ID_Hello, "&Hello...\tCtrl-H", "Hello mssg");
        // El comentario en la statusbar "Hello mssg" obedece a la primra definicion del evento
        // (si el mismo evento "ID_Hello" esta varias veces aparece el primer comentario que lo define y no obedece a los siguientes)
        // menuProject->AppendSeparator();
        menuProject->Append(wxID_NEW, "&Nuevo...", "Nuevo proyecto");
        menuProject->Append(wxID_OPEN, "&Abrir...", "Abrir proyecto");
        // menuProject->Append(wxID_SAVE, "&Guardar...", "Guardar proyecto");
        menuProject->Append(ID_Hello, "&Guardar...", "Guardar proyecto"); /// FUNCION TEMPORAL!!!
        menuProject->Append(wxID_CLOSE, "&Cerrar...", "Cerrar proyecto");
        menuProject->Append(wxID_EXIT, "&Salir...", "Salir de CineDoc");

        // MENU GUION
        wxMenu *menuScript = new wxMenu;
        menuScript->Append(ID_SCRIPT_NEW, "&Nuevo...", "Nuevo guión");
        menuScript->Append(ID_SCRIPT_EDIT, "&Editar...", "Editar guión");
        menuScript->Append(ID_SCRIPT_DEL, "&Eliminar...", "Eliminar guión");

        // MENU PERSONAJE
        wxMenu *menuCharacter = new wxMenu;
        menuCharacter->Append(ID_Hello, "&Nuevo...", "Nuevo personaje");
        menuCharacter->Append(ID_Hello, "&Editar...", "Editar personaje");
        menuCharacter->Append(ID_Hello, "&Eliminar...", "Eliminar personaje");

        // MENU ACTOR
        wxMenu *menuActor = new wxMenu;
        menuActor->Append(ID_Hello, "&Nuevo...", "Nuevo actor");
        menuActor->Append(ID_Hello, "&Editar...", "Editar actor");
        menuActor->Append(ID_Hello, "&Eliminar...", "Eliminar actor");

        // MENU LOCACION
        wxMenu *menuLocation = new wxMenu;
        menuLocation->Append(ID_Hello, "&Nuevo...", "Nueva locación");
        menuLocation->Append(ID_Hello, "&Editar...", "Editar locación");
        menuLocation->Append(ID_Hello, "&Eliminar...", "Eliminar locación");

        // MENU OBJETO
        wxMenu *menuObject = new wxMenu;
        menuObject->Append(ID_Hello, "&Nuevo...", "Nuevo objeto");
        menuObject->Append(ID_Hello, "&Editar...", "Editar objeto");
        menuObject->Append(ID_Hello, "&Eliminar...", "Eliminar objeto");

        // MENU AYUDA
        wxMenu *menuHelp = new wxMenu;
        menuHelp->Append(wxID_ABOUT, "&Ayuda...\tCtrl-H", "Buscar ayuda");

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

        // Izquierdo (VERTICAL)
        wxBoxSizer *leftSizer = new wxBoxSizer(wxVERTICAL);
        wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);

        // Derecho (VERTICAL)
        // wxBoxSizer *rightSizer = new wxBoxSizer(wxVERTICAL);

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
                                     wxDefaultPosition, wxSize(400, 600),
                                     wxTE_MULTILINE, wxDefaultValidator, "leftTextBox");
        leftSizer->Add(leftTextBox, 1, wxEXPAND | wxALL, 5);

        wxButton *backButton = new wxButton(this, wxID_ANY, "Volver");
        backButton->Bind(wxEVT_BUTTON, &MyFrame::OnBackButtonClicked, this);
        buttonSizer->Add(backButton, 1, wxEXPAND | wxRIGHT, 5);

        wxButton *addButton = new wxButton(this, wxID_ANY, "Agregar");
        addButton->Bind(wxEVT_BUTTON, &MyFrame::OnAddButtonClicked, this);
        buttonSizer->Add(addButton, 2, wxEXPAND, 0);

        leftSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 5);
        mainSizer->Add(leftSizer, 0, wxEXPAND | wxALL, 5);

        /// PANEL CONTENEDOR 1
        containerPanel = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN, "containerPanel");
        containerPanel->SetScrollRate(0, 10); // 0 en la dirección x (horizontal) y 10 en la dirección y (vertical).

        containerSizer = new wxBoxSizer(wxVERTICAL);
        containerPanel->SetSizer(containerSizer);
        mainSizer->Add(containerPanel, 1, wxEXPAND | wxALL, 5);

        // Crear un wxStaticBox con el título "Propiedades"
        wxStaticBox *propertiesBox = new wxStaticBox(this, wxID_ANY, "Propiedades");
        // propertiesBox->SetSize(100, 500);

        // Crear un sizer vertical para los controles dentro del wxStaticBox
        wxStaticBoxSizer *propertiesSizer = new wxStaticBoxSizer(propertiesBox, wxVERTICAL);

        // Crear itemIndexTextBox y añadirlo al propertiesSizer
        itemIndexTextBox = new wxTextCtrl(propertiesBox, wxID_ANY, wxString::Format(wxT("%d"), itemIndexPosition),
                                          wxDefaultPosition, wxSize(200, -1), 0,
                                          wxDefaultValidator, "itemIndexTextBox");
        // itemIndexTextBox->SetValue(wxString::Format(wxT("%d"), indexPosition));
        propertiesSizer->Add(itemIndexTextBox, 0, wxALL, 5);

        // Crear itemPositionTextBox y añadirlo al propertiesSizer
        itemPositionTextBox = new wxTextCtrl(propertiesBox, wxID_ANY, wxString::Format(wxT("%d"), itemPanelPosition),
                                             wxDefaultPosition, wxSize(200, -1), 0);
        // itemPositionTextBox->SetValue(wxString::Format(wxT("%d"), panelPosition));
        propertiesSizer->Add(itemPositionTextBox, 0, wxALL, 5);

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

        /// PANEL CONTENEDOR 2
        itemsPanel = new wxScrolledWindow(propertiesBox, wxID_ANY, wxDefaultPosition, wxSize(190, 200), wxBORDER_SUNKEN, "itemsPanel");
        itemsPanel->SetScrollRate(0, 10); // 0 en la dirección x (horizontal) y 10 en la dirección y (vertical).

        itemsSizer = new wxBoxSizer(wxVERTICAL);
        itemsPanel->SetSizer(itemsSizer);

        miniSizer->Add(itemsPanel, 1, wxALL, 5);
        propertiesSizer->Add(miniSizer, 1, wxALL, 5);

        mainSizer->Add(propertiesSizer, 0, wxALL | wxEXPAND, 5);

        SetSizer(mainSizer);

        Bind(wxEVT_UPDATE_POSITION_EVENT, &MyFrame::OnUpdatePositionEvent, this);
        Bind(wxEVT_UPDATE_INDEX_EVENT, &MyFrame::OnUpdateIndexEvent, this);
    }

    // ORDENAMIENTO DEL VECTOR PARA ENCONTRAR POTENCIALES LUGARES LIBRES (SI SE BORRARON Y QUEDARON HUECOS)
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

    int lastEmpty(std::vector<int> vector)
    {
        // Si el vector está vacío, el primer número faltante es 1
        if (vector.empty())
        {
            return 1;
        }

        // Ordena el vector en orden ascendente
        std::sort(vector.begin(), vector.end());

        // Retorna el último número + 1
        return vector.back() + 1;
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

        nextNumber = firstEmpty(textBoxsContainer);
        lastNumber = lastEmpty(textBoxsContainer);

        TitledTextBox *newTitledTextBox = new TitledTextBox(containerPanel, containerSizer, nextNumber, selectedText);
        textBoxsContainer.push_back(nextNumber);
        containerSizer->Add(newTitledTextBox, 0, wxEXPAND | wxALL, 5);

        // nextNumber es la posicion dentro del arreglo (id); lastNumber es la posicion dentro del contenedor
        Scene newScene(nextNumber, 1, selectedText.ToStdString(), lastNumber);
        scenes.push_back(newScene);

        containerSizer->Layout();
        containerPanel->Layout();
        // containerPanel->SetVirtualSize(containerSizer->GetMinSize());
        containerPanel->FitInside(); // Esta funcion reemplaza a la linea de arriba
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

        nextNumber = firstEmpty(itemsListContainer);
        lastNumber = lastEmpty(itemsListContainer);

        ItemTextList *newItemTextList = new ItemTextList(itemsPanel, itemsSizer, nextNumber, selectedText);
        itemsListContainer.push_back(nextNumber);
        itemsSizer->Add(newItemTextList, 0, wxEXPAND | wxALL, 5);

        itemsSizer->Layout();
        itemsPanel->Layout();
        // containerPanel->SetVirtualSize(containerSizer->GetMinSize());
        itemsPanel->FitInside(); // Esta funcion reemplaza a la linea de arriba
    }

    void OnUpdatePositionEvent(wxCommandEvent &event);
    void OnUpdateIndexEvent(wxCommandEvent &event);

private:
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
    void OnHello(wxCommandEvent &event);
    void OnExit(wxCommandEvent &event);
    void OnAbout(wxCommandEvent &event);
    void OnNewScript(wxCommandEvent &event);
    wxDECLARE_EVENT_TABLE();
};

class MyApp : public wxApp
{
public:
    virtual bool OnInit()
    {
        MyFrame *frame = new MyFrame("CineDoc", wxPoint(50, 50), wxSize(800, 600));
        frame->Show(true);
        return true;
    }
};

wxDEFINE_EVENT(wxEVT_UPDATE_POSITION_EVENT, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_UPDATE_INDEX_EVENT, wxCommandEvent);

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)

    EVT_MENU(wxID_EXIT, MyFrame::OnExit)
        EVT_MENU(ID_SCRIPT_NEW, MyFrame::OnNewScript)
            EVT_MENU(ID_SCRIPT_EDIT, MyFrame::OnHello)
                EVT_MENU(ID_SCRIPT_DEL, MyFrame::OnHello)

                    EVT_MENU(ID_Hello, MyFrame::OnHello)
                        EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
                            wxEND_EVENT_TABLE()
                                wxIMPLEMENT_APP(MyApp);

void MyFrame::OnExit(wxCommandEvent &event)
{
    Close(true);
}
void MyFrame::OnAbout(wxCommandEvent &event)
{
    wxMessageBox("This is CineDoc: An C++ and wxWidgets multiplattform App", // CONTENIDO VENTANA POP UP
                 "About CineDoc", wxOK | wxICON_INFORMATION);                // TITULO VENTANA POP UP
}
void MyFrame::OnHello(wxCommandEvent &event)
{
    // wxMessageBox("Welcome to CineDoc",                   // CONTENIDO VENTANA POP UP
    //              "Hi there", wxOK | wxICON_INFORMATION); // TITULO VENTANA POP UP

    // wxLogMessage("Hello world from wxWidgets!"); // VENTANA CON TITULO GENERICO "MAIN INFORMATION"

    // Serializamos las instancias a un archivo
    std::ofstream out_fs("datos.bin");
    boost::archive::text_oarchive oa(out_fs);
    oa << scripts;
    oa << scenes;
    out_fs.close();
}

// UDMR EVENTS

void MyFrame::OnNewScript(wxCommandEvent &event)
{
    wxMessageBox("Test",                                          // CONTENIDO VENTANA POP UP
                 "Crear nuevo guion", wxOK | wxICON_INFORMATION); // TITULO VENTANA POP UP
    SetStatusText("StatusBar overide");
    // wxLogMessage("Hello world from wxWidgets!"); // VENTANA CON TITULO GENERICO "MAIN INFORMATION"
}

void MyFrame::OnUpdatePositionEvent(wxCommandEvent &event)
{
    // wxLogMessage(wxT("Evento Position corriendo"));
    int receivedNumber = event.GetInt();
    //(wxString::Format(wxT("Posición recibida: %d"), receivedNumber));
    itemPositionTextBox->SetValue(wxString::Format(wxT("%d"), receivedNumber));
    // itemPositionTextBox->Refresh();
    // itemPositionTextBox->Update();
}

void MyFrame::OnUpdateIndexEvent(wxCommandEvent &event)
{
    // wxLogMessage(wxT("Evento Position corriendo"));
    int receivedNumber = event.GetInt();
    //(wxString::Format(wxT("Posición recibida: %d"), receivedNumber));
    itemIndexTextBox->SetValue(wxString::Format(wxT("%d"), receivedNumber));
    // itemIndexTextBox->Refresh();
    // itemIndexTextBox->Update();
}