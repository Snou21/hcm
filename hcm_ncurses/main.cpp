// g++ main.cpp -o main -lncurses -ltinfo
#include <iostream>
#include "hcm.cpp"

using namespace std;

Key getch_key() {
    int ch = getch();
    switch (ch) {
        case 'k': return Key::UP;
        case 'j': return Key::DOWN;
        case '\n': return Key::SELECT;
        case 'x': return Key::ESC;
        default: return Key::UNKNOWN;
    }
}

void Execute() {
    mvprintw(15, 5, "Some button pressed...");
    refresh();
    napms(1000);
}

int func(int a, int b) {
    return a + b;
}

int main() {
    initscr(); // Инициализация ncurses
    start_color(); // Инициализация цветов
    init_pair(1, COLOR_MAGENTA, COLOR_CYAN); // Пара для заголовка
    init_pair(2, COLOR_BLACK, COLOR_YELLOW); // Пара для выделенной кнопки
    init_pair(3, COLOR_WHITE, COLOR_CYAN); // Пара для выделенного поля ввода
    init_pair(4, COLOR_WHITE, COLOR_GREEN); // Пара для выделенного списка
    init_pair(5, COLOR_YELLOW, COLOR_BLACK); // Пара для строки
    init_pair(6, COLOR_BLACK, COLOR_RED);
    init_pair(7, COLOR_MAGENTA, COLOR_BLUE);
    init_pair(8, COLOR_BLACK, COLOR_MAGENTA);

    bkgd(COLOR_PAIR(7)); // Заливаем фон цветом 1

    noecho(); // Не показывать вводимые символы
    curs_set(0); // Не показывать курсор
    keypad(stdscr, TRUE); // Включаем режим работы с функциональными клавишами

    Menu root(" MENU ");
    Menu help("HELP MENU");
    Menu calculator("CALCULATOR");

    Menu* currentMenu = &root;

    Input* input1 = new Input("Enter 1st number", "Please enter number: ", 3);
    Input* input2 = new Input("Enter 2nd number", "Please enter number: ", 3);
    Label* resultLabel = new Label("Result: ", 5);

    Button* calculateButton = new Button("Calculate", [&]() {
        int a = stoi(input1->text); // Преобразуем строку в число
        int b = stoi(input2->text); // Преобразуем строку в число
        resultLabel->SetText("Result: " + to_string(func(a, b)));
    }, 2);


    root.widgets.push_back(new Button("Press for help", [&]() { currentMenu = &help; }, 8));
    root.widgets.push_back(new Button("Start Engine", Execute, 2));
    root.widgets.push_back(new Input("Enter something", "Please enter something: ", 3));

    List* lst = new List("List", 4);
    lst->AddItem(new Label("1 item", 5));
    lst->AddItem(new Button("2 item", Execute, 4));
    lst->AddItem(new Label("3 item", 5));

    root.widgets.push_back(lst);
    root.widgets.push_back(new Button("Calculator", [&]() { currentMenu = &calculator; }, 8));
    root.widgets.push_back(new Button("Exit", []() { endwin(); exit(0); }, 6));

    help.widgets.push_back(new Label("Use UP/DOWN to navigate", 5));
    help.widgets.push_back(new Label("Enter to select", 5));
    help.widgets.push_back(new Label("X to exit", 5));
    help.widgets.push_back(new Button("Back", [&]() { currentMenu = &root; }, 6));

    calculator.widgets.push_back(input1);
    calculator.widgets.push_back(input2);
    calculator.widgets.push_back(calculateButton);
    calculator.widgets.push_back(resultLabel);
    calculator.widgets.push_back(new Button("Back", [&]() { currentMenu = &root; }, 6));

    while (true) {
        currentMenu->Show();
        Key key = getch_key();
        currentMenu->Input(key);
    }

    endwin(); // Завершение работы с ncurses
    return 0;
}