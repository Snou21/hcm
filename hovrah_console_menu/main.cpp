#include <iostream>
#include <string>
#include <vector>
#include <termios.h>
#include <unistd.h>
#include <functional>
#include "colors.h"

using namespace std;

enum Key{
    UP,
    DOWN,
    ESC,
    SELECT,
    BACKSPACE,
    UNKNOWN
};


struct Widget {
    virtual void Show(bool selected = false) = 0;
    virtual void OnSelect() {}; 
    virtual ~Widget() = default;
};

class Button : public Widget {
private:
    string text;
    function<void()> f;
public:
    Button(string text, function<void()> f){
        this->text = text;
        this->f = f;
    }

    void Show(bool selected = false) override {
        if (selected)
            cout << YELLOW << text << RED << " <--- " << endl;
        else
            cout << WHITE << text << "   " << endl;
    }

    void OnSelect() override{
        f();
    }
};

class Spacer: public Widget {
private:
    char text;
    int spacer_size;
public:
    Spacer(char text, int spacer_size){
        this->text = text;
        this->spacer_size = spacer_size;
    }

    string getSpacer(){
        string result = "";
        for (int i = 0; i < spacer_size; i++){
            result += text;
        }
        return result;
    }

    void Show(bool selected = false) override {
        if (selected)
            cout << BLACK << getSpacer() << BLACK << " <--- " << endl;
        else
            cout << BLACK << getSpacer() << "   " << endl;
    }

};

class Label : public Widget {
    private:
        string text;
    public:
        Label(string text){
            this->text = text;
        }
    
        void Show(bool selected = false) override {
            if (selected)
                cout << BLACK << text << BLACK << " <--- " << endl;
            else
                cout << BLACK << text << "   " << endl;
        }
    
    };

class List : public Widget{
private:
    string text;
    bool opened;
    vector<Widget*> w;
    int selectedIndex = 0;
public:
    List(string text){
        this->text = text;
        opened = false;
        w.push_back(new Button("(X) Close list", [=](){opened = false;}));
    }

    void AddItem(Widget* item) {
        w.push_back(item);
    }

    void Show(bool selected = false) override {
        cout << BLACK << (opened ? "v " : "> ");

        if (selected)
            cout << GREEN << text << RED << " <--- " << endl;
        else
            cout << WHITE << text << "   " << endl;

        if (opened) {
            for (size_t i = 0; i < w.size(); i++) {
                cout << BLACK << "|- ";
                w[i]->Show(i == selectedIndex); 
            }
        }
        
    }

    void OnSelect() override{
        if (opened) {
            if (!w.empty()) {
                w[selectedIndex]->OnSelect();
            }
        } else {
            opened = true;
        }
    }

    void Input(Key key) {
        if (opened) {
            switch (key) {
                case Key::UP:
                    if (selectedIndex > 0) selectedIndex--;
                    break;
                case Key::DOWN:
                    if (selectedIndex < w.size() - 1) selectedIndex++;
                    break;
                case Key::SELECT:
                    w[selectedIndex]->OnSelect();
                    break;
                case Key::ESC:
                    opened = false; // Закрываем список при нажатии ESC
                    break;
                default:
                    break;
            }
        } else {
            if (key == Key::SELECT) {
                opened = true; // Открываем список, если он был закрыт
            }
        }
    }

    bool IsOpened() {
        return opened;
    }

    ~List() { 
        for (auto item : w) delete item; 
    }
    
};

class Input : public Widget{
private:
    string text;
    string comment;
public:
    Input(string placeholder, string comment){
        text = placeholder;
        this->comment = comment;
    }
    void Show(bool selected = false) override {
        if (selected)
            cout<< BLACK << "~ " << BLUE << text << RED << " <--- " << endl;
        else
            cout<< BLACK << "~ " << WHITE << text << "   " << endl;
    }
    
    void OnSelect() override{
        cout << endl << RED <<comment << BLUE;
        cin.ignore(); // Очистка ввода (важно для корректного считоывания)
        //cin >> text;
        getline(cin, text); // аналог cin >> text;
    }
};

class Menu{
private:
    string header;
public:
    vector<Widget*> widgets; 
    int selectedIndex = 0;

    Menu(string header){
        this->header = header;
    }
    
    void Show(){// метод вывода кнопок
        CLEAR_SCREEN(); // Очистка консоли
        cout << GREEN << header << WHITE << endl << endl;
        for (int i = 0; i < widgets.size(); i++) {
            widgets[i]->Show(i==selectedIndex);
        }
    }

    void Input(Key key) {
        if (widgets[selectedIndex] != nullptr) {
            auto* list = dynamic_cast<List*>(widgets[selectedIndex]); // ???
            if (list && list->IsOpened()) { 
                list->Input(key); // Делегируем управление внутрь списка
                return;
            }
        }
    
        switch (key) {
            case Key::UP:
                if (selectedIndex > 0) selectedIndex--;
                break;
            case Key::DOWN:
                if (selectedIndex < widgets.size() - 1) selectedIndex++;
                break;
            case Key::SELECT:
                widgets[selectedIndex]->OnSelect();
                break;
            case Key::ESC:
                exit(0);
        }
    }
    ~Menu() { // деструктор
        for (auto widget : widgets) delete widget;
    }
};

Key getch() {
    struct termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt); // Получаем текущие настройки терминала
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); // Выключаем буферизированный ввод и эхо
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // Применяем новые настройки
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Восстанавливаем старые настройки
    
    switch(ch){
        case 'k' : return Key::UP;
        case 'j' : return Key::DOWN;
        case '\n' : return Key::SELECT;
        case 'x' : return Key::ESC;
        default : return Key::UNKNOWN;
    }
}

void Execute() {
    cout << BLACK << "some button pressed" << BLACK << endl;
    sleep(2); // Имитация загрузки
}


void ShowCurrentMenu(Menu &m){
    m.Show();
}

int main(){
    Menu root("MENU");  
    Menu help("HELP MENU");

    Menu* currentMenu = &root;

    root.widgets.push_back(new Button("Press for help", [&](){currentMenu = &help;}));
    root.widgets.push_back(new Spacer('~', 10));
    root.widgets.push_back(new Input("Input 3D model`s path", "Enter path..."));
    root.widgets.push_back(new Button("Start Engine", Execute));
    
    List* lst = new List("List");
    List* sublst = new List("Sub List");

    lst->AddItem(new Label("1 item"));
    lst->AddItem(new Button("2 item", Execute));
    lst->AddItem(new Label("3 item"));
    lst->AddItem(sublst);

    root.widgets.push_back(lst);
    root.widgets.push_back(new Spacer('~', 10));
    root.widgets.push_back(new Button("Exit", [](){exit(0); cout << " Exit" << endl;}));
    help.widgets.push_back(new Label("Move up and down with 'k' and 'j' \nSelect with 'Enter' \nBack with 'x'"));
    help.widgets.push_back(new Spacer('~', 10));
    help.widgets.push_back(new Button("Back", [&](){currentMenu = &root;}));


    while (true)
    {
        ShowCurrentMenu(*currentMenu);
        Key key = getch();
        currentMenu->Input(key);

    }
    return 0;
}