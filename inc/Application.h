#ifndef APPLICATION_H
#define APPLICATION_H

#include <bobcat_ui/bobcat_ui.h>
#include <bobcat_ui/button.h>
#include <bobcat_ui/dropdown.h>
#include <bobcat_ui/window.h>
#include <Graph.h>
#include <FL/Fl_Scroll.H>


class Application : public bobcat::Application_ {
    bobcat::Window *window;
    bobcat::Dropdown *start;
    bobcat::Dropdown *dest;

    bobcat::Button *search;

    Fl_Scroll *results;

    ArrayList<Vertex*> cities;
    Graph g;

    void initData();
    void initInterface();

    void handleClick(bobcat::Widget *sender);
public:
    Application(); // Constructor for the app
};

#endif