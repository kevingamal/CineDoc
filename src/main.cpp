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

enum
{
    ID_Hello = 1
};

class TitledTextBox : public wxPanel
{
public:
    TitledTextBox(wxWindow *parent, wxBoxSizer *sizer, int index, const wxString &text)
        : wxPanel(parent, wxID_ANY), textBox(nullptr), parentSizer(sizer), itemPosition(index)
    {
        wxBoxSizer *sizerLocal = new wxBoxSizer(wxVERTICAL);

        // Sizer horizontal para el título y los botones
        wxBoxSizer *titleSizer = new wxBoxSizer(wxHORIZONTAL);

        // Botón izquierdo
        wxButton *leftButton = new wxButton(this, wxID_ANY, "^", wxDefaultPosition, wxSize(25, 25), wxBORDER_NONE);
        titleSizer->Add(leftButton, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
        leftButton->Bind(wxEVT_BUTTON, &TitledTextBox::OnLeftButtonClick, this);

        // BARRA DE TITULO
        wxString title = wxString::Format("Fragmento %d", index);
        wxStaticText *titleLabel = new wxStaticText(this, wxID_ANY, title);
        titleSizer->Add(titleLabel, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);

        // CUADRO DE TEXTO
        textBox = new wxTextCtrl(this, wxID_ANY, text,
                                 wxDefaultPosition, wxDefaultSize,
                                 wxTE_MULTILINE | wxTE_READONLY);

        // Boton derecho >
        wxButton *middleButton = new wxButton(this, wxID_ANY, ">", wxDefaultPosition, wxSize(25, 25), wxBORDER_NONE);
        titleSizer->Add(middleButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
        middleButton->Bind(wxEVT_BUTTON, &TitledTextBox::OnMiddleButtonClick, this);

        // Boton derecho x
        wxButton *rightButton = new wxButton(this, wxID_ANY, "X", wxDefaultPosition, wxSize(25, 25), wxBORDER_NONE);
        titleSizer->Add(rightButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
        rightButton->Bind(wxEVT_BUTTON, &TitledTextBox::OnRightButtonClick, this);

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

    void OnLeftButtonClick(wxCommandEvent &event)
    {
        textBox->Show(!textBox->IsShown());
        Layout();
        dynamic_cast<wxScrolledWindow *>(GetParent())->FitInside();
    }

    void OnMiddleButtonClick(wxCommandEvent &event)
    {
        // Implementa la lógica para este botón aquí
    }

    void OnRightButtonClick(wxCommandEvent &event)
    {
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
        CaptureMouse();
        dragStartPosition = ClientToScreen(event.GetPosition());
        initialWindowPosition = GetPosition();

        // IMPRESION DEL Nº DE ITEM EN PANTALLA
        panelPosition = GetItemPositionInSizer(parentSizer, this); // ----> Mandar este dato al TextBox correspondiente
        // panelPosition se averigua cada vez que se selecciona el panel, para saber donde está (ya que puede cambiarse)
        // itemPosition ----> Mandar este dato al TextBox correspondiente
        // itemPosition es inamovible e unico, sirve para saber su id en la BD
    }

    void OnMouseUp(wxMouseEvent &event)
    {
        if (HasCapture())
            ReleaseMouse();
    }

    void OnMouseMove(wxMouseEvent &event)
    {
        if (event.Dragging() && event.LeftIsDown())
        {
            wxPoint currentPosition = GetParent()->ScreenToClient(wxGetMousePosition());

            int insertPosition = -1;

            for (size_t i = 0; i < parentSizer->GetItemCount(); i++)
            {
                wxRect childRect = parentSizer->GetItem(i)->GetWindow()->GetRect();

                if (currentPosition.y < childRect.y + childRect.height / 2)
                {
                    insertPosition = i;
                    break;
                }
            }

            // Asegúrate de que insertPosition esté en un rango válido
            int currentPos = GetItemPositionInSizer(parentSizer, this);
            if (insertPosition < 0)
                insertPosition = currentPos;
            else if (insertPosition > parentSizer->GetItemCount())
                insertPosition = parentSizer->GetItemCount();

            if (insertPosition != currentPos)
            {
                parentSizer->Detach(this);
                parentSizer->Insert(insertPosition, this, 0, wxEXPAND | wxALL, 5);
                parentSizer->Layout();
                dynamic_cast<wxScrolledWindow *>(GetParent())->FitInside();
            }
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
    wxPoint dragStartPosition;
    wxPoint initialWindowPosition;
    wxBoxSizer *parentSizer;
    int panelPosition;
    int itemPosition;
};

class MyFrame : public wxFrame
{
public:
    MyFrame(const wxString &title, const wxPoint &pos, const wxSize &size)
        : wxFrame(NULL, wxID_ANY, title, pos, size), leftTextBox(nullptr)
    {
        wxMenu *menuFile = new wxMenu;
        menuFile->Append(ID_Hello, "&Hello...\tCtrl-H",
                         "Hello mssg");
        menuFile->AppendSeparator();
        menuFile->Append(wxID_EXIT);
        wxMenu *menuHelp = new wxMenu;
        menuHelp->Append(wxID_ABOUT);
        wxMenuBar *menuBar = new wxMenuBar;
        menuBar->Append(menuFile, "&File");
        menuBar->Append(menuHelp, "&Help");
        SetMenuBar(menuBar);
        CreateStatusBar();
        SetStatusText("Welcome to CineDoc!");

        wxBoxSizer *mainSizer = new wxBoxSizer(wxHORIZONTAL);

        leftTextBox = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
                                     wxDefaultPosition, wxSize(400, 600),
                                     wxTE_MULTILINE, wxDefaultValidator, "leftTextBox");
        mainSizer->Add(leftTextBox, 1, wxEXPAND | wxALL, 5);

        containerPanel = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN, "containerPanel");
        containerPanel->SetScrollRate(0, 10); // 0 en la dirección x (horizontal) y 10 en la dirección y (vertical).

        containerSizer = new wxBoxSizer(wxVERTICAL);
        containerPanel->SetSizer(containerSizer);
        mainSizer->Add(containerPanel, 1, wxEXPAND | wxALL, 5);

        wxButton *addButton = new wxButton(this, wxID_ANY, "Agregar");
        addButton->Bind(wxEVT_BUTTON, &MyFrame::OnAddButtonClicked, this);
        mainSizer->Add(addButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

        SetSizer(mainSizer);
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

private:
    wxTextCtrl *leftTextBox;
    wxScrolledWindow *containerPanel;
    wxBoxSizer *containerSizer;
    int nextNumber;

    // VECTOR (solo para almacenar temporalmente los indices y reciclar los antiguos)
    std::vector<int> positionsContainer;
    // numeros.push_back(x); // Añade el numero x al final
    // int primerNumero = numeros[0]; // Obtenemos el numero en la primera posicion
    // size_t cantidad = numeros.size(); // Obtenemos la cantidad de items en el vector
    // std::sort(numeros.begin(), numeros.end()); // Ordena ascendentemente (se puede usar al revés)

    void OnHello(wxCommandEvent &event);
    void OnExit(wxCommandEvent &event);
    void OnAbout(wxCommandEvent &event);
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

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(ID_Hello, MyFrame::OnHello)
        EVT_MENU(wxID_EXIT, MyFrame::OnExit)
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
