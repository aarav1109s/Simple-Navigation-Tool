#ifndef APPLICATION_H
#define APPLICATION_H

#include <bobcat_ui/bobcat_ui.h>
#include <bobcat_ui/button.h>
#include <bobcat_ui/dropdown.h>
#include <bobcat_ui/window.h>
#include <bobcat_ui/textbox.h>
#include <Graph.h>
#include <FL/Fl_Scroll.H>

class Application : public bobcat::Application_ {
    // UI Components
    bobcat::Window* window;
    bobcat::Dropdown* start;
    bobcat::Dropdown* dest;
    bobcat::Dropdown* mode;     // NEW: search preference dropdown
    bobcat::Button* search;
    Fl_Scroll* results;

    // Data
    ArrayList<Vertex*> cities;
    Graph g;

    // Internal helpers
    void initData();
    void initInterface();
    void loadAirports(const std::string& filename);   // NEW
    void loadEdges(const std::string& filename);      // NEW

    void handleClick(bobcat::Widget* sender);

public:
    Application(); // constructor
};

#endif
