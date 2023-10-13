#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/sizer.h>
#include <wx/wxprec.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/stattext.h>
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
std::vector<int> positionsContainer;
// numeros.push_back(x); // Añade el numero x al final
// int primerNumero = numeros[0]; // Obtenemos el numero en la primera posicion
// size_t cantidad = numeros.size(); // Obtenemos la cantidad de items en el vector
// std::sort(numeros.begin(), numeros.end()); // Ordena ascendentemente (se puede usar al revés)

// Posiciones de la lista
int indexPosition;
int panelPosition;

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
        wxButton *upButton = new wxButton(this, wxID_ANY, "<", wxDefaultPosition, wxSize(25, 25), wxBORDER_NONE);
        titleSizer->Add(upButton, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
        upButton->Bind(wxEVT_BUTTON, &TitledTextBox::OnupButtonClick, this);

        // Botón de abajo
        wxButton *downButton = new wxButton(this, wxID_ANY, ">", wxDefaultPosition, wxSize(25, 25), wxBORDER_NONE);
        titleSizer->Add(downButton, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
        downButton->Bind(wxEVT_BUTTON, &TitledTextBox::OndownButtonClick, this);

        // BARRA DE TITULO
        wxString title = wxString::Format("Fragmento %d", index);
        wxStaticText *titleLabel = new wxStaticText(this, wxID_ANY, title);
        titleSizer->Add(titleLabel, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);

        // Boton de edicion >
        wxButton *editButton = new wxButton(this, wxID_ANY, ">", wxDefaultPosition, wxSize(25, 25), wxBORDER_NONE);
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
        panelPosition = GetItemPositionInSizer(parentSizer, this);
    }

    void OndownButtonClick(wxCommandEvent &event)
    {
        desiredPosition = GetItemPositionInSizer(parentSizer, this) + 1;
        MoveToDesiredPosition();
        panelPosition = GetItemPositionInSizer(parentSizer, this);
    }

    void OneditButtonClick(wxCommandEvent &event)
    {
        // Implementa la lógica para este botón aquí
    }

    void OndeleteButtonClick(wxCommandEvent &event)
    {
        deleteVectorItem(positionsContainer, itemPosition);
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
        // panelPosition = GetItemPositionInSizer(parentSizer, this);
        // indexPosition = itemPosition;

        // panelPosition se averigua cada vez que se selecciona el panel, para saber donde está (ya que puede cambiarse)
        // es inamovible e unico, sirve para saber su id en la BD

        // wxCommandEvent evt(wxEVT_UPDATE_POSITION_EVENT);
        // evt.SetInt(panelPosition);
        // wxPostEvent(GetParent(), evt);
        // event.Skip();
    }

    void OnMouseUp(wxMouseEvent &event)
    {
        // MoveToDesiredPosition();
        // wxLogMessage(wxT("OnMouseUp llamado"));
        // panelPosition = GetItemPositionInSizer(parentSizer, this);

        // if (HasCapture())
        //     ReleaseMouse();

        // wxCommandEvent evt(wxEVT_UPDATE_POSITION_EVENT);
        // evt.SetInt(panelPosition);
        // wxPostEvent(GetParent(), evt);
        //  event.Skip();
    }

    void OnMouseMove(wxMouseEvent &event)
    {
        panelPosition = GetItemPositionInSizer(parentSizer, this) + 1;
        indexPosition = itemPosition;

        wxCommandEvent evta(wxEVT_UPDATE_POSITION_EVENT);
        evta.SetInt(panelPosition);
        wxPostEvent(GetParent(), evta);

        wxCommandEvent evtb(wxEVT_UPDATE_INDEX_EVENT);
        evtb.SetInt(indexPosition);
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
        // menuProject->AppendSeparator();
        menuProject->Append(wxID_NEW, "&Nuevo...", "Nuevo proyecto");
        menuProject->Append(wxID_OPEN, "&Abrir...", "Abrir proyecto");
        menuProject->Append(wxID_SAVE, "&Guardar...", "Guardar proyecto");
        menuProject->Append(wxID_CLOSE, "&Cerrar...", "Cerrar proyecto");
        menuProject->Append(wxID_EXIT, "&Salir...", "Salir de CineDoc");

        // MENU GUION
        wxMenu *menuScript = new wxMenu;
        // menuScript->Append(ID_Hello, "&Hello...", "Test");
        menuScript->Append(ID_SCRIPT_NEW, "&Nuevo...\tCtrl-L", "Nuevo guión");
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
        // menuBar->Append(menuCharacter, "&Personaje");
        // menuBar->Append(menuActor, "&Actor");
        // menuBar->Append(menuLocation, "&Locacion");
        // menuBar->Append(menuObject, "&Objeto");
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
        wxBoxSizer *rightSizer = new wxBoxSizer(wxVERTICAL);

        wxString choices[] = {wxT("Opción 1"), wxT("Opción 2"), wxT("Opción 3")}; // wxT("cadena") forza a tomar como unicode el string cadena

        // itemSelector = new wxComboBox(this, wxID_ANY, "Opción 1", wxDefaultPosition, wxDefaultSize,
        // 3, choices, wxCB_DROPDOWN | wxCB_READONLY, wxDefaultValidator, "itemSelector");
        // En este ejemplo "Opción 1" será la opción seleccionada inicialmente en el wxComboBox.

        // itemSelector->SetValue(wxT("Opción 1")); // Para cambiar posteriormente la opcion marcada por default

        itemSelector = new wxComboBox(this, wxID_ANY, wxT("Opción 1"), wxDefaultPosition, wxDefaultSize,
                                      3, choices, wxCB_DROPDOWN | wxCB_READONLY,
                                      wxDefaultValidator, "itemSelector");

        // itemSelector->SetValue(wxT("Opción 3"));

        leftSizer->Add(itemSelector, 1, wxEXPAND | wxALL, 5);

        leftTextBox = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
                                     wxDefaultPosition, wxSize(400, 600),
                                     wxTE_MULTILINE, wxDefaultValidator, "leftTextBox");
        leftSizer->Add(leftTextBox, 1, wxEXPAND | wxALL, 5);

        wxButton *backButton = new wxButton(this, wxID_ANY, "Volver");
        backButton->Bind(wxEVT_BUTTON, &MyFrame::OnAddButtonClicked, this); // Dejamos asi por ahora, hasta crear el evento
        buttonSizer->Add(backButton, 1, wxEXPAND | wxRIGHT, 5);

        wxButton *addButton = new wxButton(this, wxID_ANY, "Agregar");
        addButton->Bind(wxEVT_BUTTON, &MyFrame::OnAddButtonClicked, this);
        buttonSizer->Add(addButton, 2, wxEXPAND, 0);

        leftSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 5);
        mainSizer->Add(leftSizer, 0, wxEXPAND | wxALL, 5);

        containerPanel = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN, "containerPanel");
        containerPanel->SetScrollRate(0, 10); // 0 en la dirección x (horizontal) y 10 en la dirección y (vertical).

        containerSizer = new wxBoxSizer(wxVERTICAL);
        containerPanel->SetSizer(containerSizer);
        mainSizer->Add(containerPanel, 1, wxEXPAND | wxALL, 5);

        // Crear un wxStaticBox con el título "Propiedades"
        wxStaticBox *propertiesBox = new wxStaticBox(this, wxID_ANY, "Propiedades");
        // Crear un sizer vertical para los controles dentro del wxStaticBox
        wxStaticBoxSizer *propertiesSizer = new wxStaticBoxSizer(propertiesBox, wxVERTICAL);

        // Crear itemIndexTextBox y añadirlo al propertiesSizer
        itemIndexTextBox = new wxTextCtrl(propertiesBox, wxID_ANY, wxString::Format(wxT("%d"), indexPosition),
                                          wxDefaultPosition, wxSize(70, -1), 0,
                                          wxDefaultValidator, "itemIndexTextBox");
        // itemIndexTextBox->SetValue(wxString::Format(wxT("%d"), indexPosition));
        propertiesSizer->Add(itemIndexTextBox, 0, wxALL, 5);

        // Crear itemPositionTextBox y añadirlo al propertiesSizer
        itemPositionTextBox = new wxTextCtrl(propertiesBox, wxID_ANY, wxString::Format(wxT("%d"), panelPosition),
                                             wxDefaultPosition, wxSize(70, -1), 0);
        // itemPositionTextBox->SetValue(wxString::Format(wxT("%d"), panelPosition));
        propertiesSizer->Add(itemPositionTextBox, 0, wxALL, 5);

        mainSizer->Add(propertiesSizer, 0, wxEXPAND | wxALL, 5);

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

        nextNumber = firstEmpty(positionsContainer);

        TitledTextBox *newTitledTextBox = new TitledTextBox(containerPanel, containerSizer, nextNumber, selectedText);

        positionsContainer.push_back(nextNumber);

        containerSizer->Add(newTitledTextBox, 0, wxEXPAND | wxALL, 5);

        containerSizer->Layout();
        containerPanel->Layout();
        // containerPanel->SetVirtualSize(containerSizer->GetMinSize());
        containerPanel->FitInside(); // Esta funcion reemplaza a la linea de arriba
    }

    void OnUpdatePositionEvent(wxCommandEvent &event);
    void OnUpdateIndexEvent(wxCommandEvent &event);

private:
    wxComboBox *itemSelector;
    wxTextCtrl *leftTextBox;
    wxScrolledWindow *containerPanel;
    wxBoxSizer *containerSizer;
    wxTextCtrl *itemIndexTextBox;
    wxTextCtrl *itemPositionTextBox;
    int nextNumber;
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
    wxMessageBox("Welcome to CineDoc",                   // CONTENIDO VENTANA POP UP
                 "Hi there", wxOK | wxICON_INFORMATION); // TITULO VENTANA POP UP

    // wxLogMessage("Hello world from wxWidgets!"); // VENTANA CON TITULO GENERICO "MAIN INFORMATION"
}

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