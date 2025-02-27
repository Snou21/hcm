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
            cout << BLUE << text << RED << " <--- " << endl;
        else
            cout << BLUE << text << "   " << endl;
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

    void Input(Key key){
        switch (key) {
            case Key::UP: // Вверх
                if (selectedIndex > 0) selectedIndex--;
                break;
            case Key::DOWN: // Вниз
                if (selectedIndex < widgets.size() - 1) selectedIndex++;
                break;
            case Key::SELECT: // Enter (выбор)
               widgets[selectedIndex]->OnSelect();
               break;
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
        case 27 : return Key::ESC;
        case 127 : return Key::BACKSPACE;
        default : return Key::UNKNOWN;
    }
}

void StartGame() {
    cout << BLACK << "Start Pressed" << BLACK << endl;
    sleep(2); // Имитация загрузки
}


void ShowCurrentMenu(Menu &m){
    m.Show();
}

int main(){
    Menu root("MENU");  
    Menu extra("EXTRA");

    Menu* currentMenu = &root;

    //root.widgets.push_back(new Label("Welcome!"));
    root.widgets.push_back(new Button("Start", StartGame));
    root.widgets.push_back(new Input("Input..", "Enter some text..."));
    root.widgets.push_back(new Button("Extra menu", [&](){currentMenu = &extra;}));
    root.widgets.push_back(new Button("Exit", [](){exit(0); cout << " Exit" << endl;}));

    extra.widgets.push_back(new Button("Back", [&](){currentMenu = &root;}));
    extra.widgets.push_back(new Label("test label"));


    while (true)
    {
        ShowCurrentMenu(*currentMenu);
        Key key = getch();
        currentMenu->Input(key);

    }
    return 0;
}