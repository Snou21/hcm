#include <ncurses.h>
#include <string>
#include <vector>
#include <functional>

using namespace std;

enum Key { UP, DOWN, ESC, SELECT, UNKNOWN };

struct Widget {
    virtual void Show(bool selected = false, int y = 0) = 0;
    virtual void OnSelect() {};
    virtual ~Widget() = default;
};

class Button : public Widget {
private:
    string text;
    function<void()> f;
    int colorPair;
public:
    Button(string text, function<void()> f, int colorPair) : text(text), f(f), colorPair(colorPair) {}

    void Show(bool selected = false, int y = 0) override {
        if (selected) attron(COLOR_PAIR(colorPair)); // Включаем цветовую пару
        mvprintw(y, 5, text.c_str()); // Выводим текст
        if (selected) attroff(COLOR_PAIR(colorPair)); // Выключаем цветовую пару
    }
    void OnSelect() override {
        f();
    }
};

class Label : public Widget {
private:
    string text;
    int colorPair;
public:
    Label(string text, int colorPair) : text(text), colorPair(colorPair) {}

    void SetText(string newText) {
        text = newText;
    }

    void Show(bool selected = false, int y = 0) override {
        if (selected) attron(COLOR_PAIR(colorPair)); // Включаем цветовую пару
        mvprintw(y, 5, text.c_str()); // Выводим текст
        if (selected) attroff(COLOR_PAIR(colorPair)); // Выключаем цветовую пару
    }
};

class List : public Widget {
private:
    bool opened;
    vector<Widget*> w;
    int selectedIndex = 0;
    int colorPair;
public:

    string text;

    List(string text, int colorPair) : text(text), opened(false), colorPair(colorPair) {
        w.push_back(new Button("(X) Close list", [=]() { opened = false; }, 6));
    }

    void AddItem(Widget* item) {
        w.push_back(item);
    }

    int GetSize() const {
        return w.size();
    }

    void Show(bool selected = false, int y = 0) override {
        mvprintw(y, 3, opened ? "v " : "> ");
        if (selected) attron(COLOR_PAIR(colorPair)); // Включаем цветовую пару
        mvprintw(y, 5, text.c_str());
        if (selected) attroff(COLOR_PAIR(colorPair)); // Выключаем цветовую пару

        if (opened) {
            for (size_t i = 0; i < w.size(); i++) {
                w[i]->Show(i == selectedIndex, y + i + 1);
            }
        }
    }

    bool IsOpened() const {
        return opened;
    }

    void OnSelect() override {
        opened = !opened;
    }

    void Input(Key key) {
        if (opened) {
            switch (key) {
                case UP: if (selectedIndex > 0) selectedIndex--; break;
                case DOWN: if (selectedIndex < w.size() - 1) selectedIndex++; break;
                case SELECT: w[selectedIndex]->OnSelect(); break;
                case ESC: opened = false; break;
                default: break;
            }
        } else {
            if (key == SELECT) opened = true;
        }
    }
};

class Input : public Widget {
private:
    string comment;
    int colorPair;
public:
    string text;

    Input(string placeholder, string comment, int colorPair) : text(placeholder), comment(comment), colorPair(colorPair) {}

    void Show(bool selected = false, int y = 0) override {
        if (selected) attron(COLOR_PAIR(colorPair)); // Включаем цветовую пару
        mvprintw(y, 5, text.c_str());
        if (selected) attroff(COLOR_PAIR(colorPair)); // Выключаем цветовую пару
    }

    void OnSelect() override {
        printw("\n%s", comment.c_str());
        echo();
        char buffer[256];
        getstr(buffer);
        text = string(buffer);
        noecho();
    }
};

class Menu {
private:
    string header;
public:
    vector<Widget*> widgets;
    int selectedIndex = 0;

    Menu(string header) : header(header) {}

    void Show() {
        clear();
        attron(COLOR_PAIR(1)); // Включаем цветовую пару 1 для заголовка
        mvprintw(1, 5, header.c_str());
        attroff(COLOR_PAIR(1)); // Выключаем цветовую пару 1

        int y = 3;
        for (size_t i = 0; i < widgets.size(); i++) {
            widgets[i]->Show(i == selectedIndex, y);
            y++;
            if (dynamic_cast<List*>(widgets[i]) && dynamic_cast<List*>(widgets[i])->IsOpened()) {
                y += dynamic_cast<List*>(widgets[i])->GetSize();
            }
        }
        refresh();
    }

    void Input(Key key) {
        if (widgets[selectedIndex] != nullptr) {
            auto* list = dynamic_cast<List*>(widgets[selectedIndex]);
            if (list && list->IsOpened()) {
                list->Input(key);
                return;
            }
        }

        switch (key) {
            case UP: if (selectedIndex > 0) selectedIndex--; break;
            case DOWN: if (selectedIndex < widgets.size() - 1) selectedIndex++; break;
            case SELECT: widgets[selectedIndex]->OnSelect(); break;
            case ESC: endwin(); exit(0);
        }
    }

    ~Menu() {
        for (auto widget : widgets) delete widget;
    }
};