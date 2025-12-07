#include "Graph.h"
#include <Application.h> // Includes our new GraphDisplay class
#include <FL/Enumerations.H>
#include <FL/Fl_Scroll.H>
#include <bobcat_ui/bobcat_ui.h>
#include <bobcat_ui/button.h>
#include <bobcat_ui/dropdown.h>
#include <bobcat_ui/textbox.h>
#include <bobcat_ui/window.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <vector>

using namespace bobcat;
using namespace std;

//
// ─── CONSTRUCTOR ───────────────────────────────────────────────
//
Application::Application() {
    initData();       
    initInterface();  
}
Application::~Application(){
    delete window;
    // Graph and UI elements are cleaned up automatically or by window
}

//
// ─── LOADING DATA (UNCHANGED) ──────────────────────────────────
//
void Application::loadAirports(const std::string &filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Cannot open " << filename << endl;
        return;
    }
    string line;
    while (getline(file, line)) {
        if (line.size() == 0) continue;
        Vertex *v = new Vertex(line);
        cities.append(v);
        g.addVertex(v);
    }
    file.close();
}

void Application::loadEdges(const std::string &filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Cannot open " << filename << endl;
        return;
    }
    string line;
    while (getline(file, line)) {
        if (line.size() == 0) continue;
        stringstream ss(line);
        string a, b, c, d;
        getline(ss, a, ','); getline(ss, b, ','); getline(ss, c, ','); getline(ss, d, ',');
        int from = stoi(a);
        int to   = stoi(b);
        int price = stoi(c);
        int time  = stoi(d);
        g.addEdge(cities[from], cities[to], price, time);
    }
    file.close();
}

void Application::initData() {
    loadAirports("assets/vertices.csv");
    loadEdges("assets/edges.csv");
}

//
// ─── INIT USER INTERFACE (UPDATED FOR LAYOUT) ──────────────────
//
void Application::initInterface() {
    // 1. Main Window (Wide enough for both columns)
    window = new Window(100, 100, 850, 500, "Flight Planner - Visualized");

    // === LEFT COLUMN: CONTROLS ===
    start = new Dropdown(20, 40, 360, 25, "Starting Airport");
    dest  = new Dropdown(20, 90, 360, 25, "Destination Airport");

    for (int i = 0; i < cities.size(); i++) {
        start->add(cities[i]->data);
        dest->add(cities[i]->data);
    }

    mode = new Dropdown(20, 140, 360, 25, "Search Type");
    mode->add("Cheapest Price");
    mode->add("Shortest Time");
    mode->add("Fewest Stops");

    search = new Button(20, 190, 360, 30, "Search");
    ON_CLICK(search, Application::handleClick);

    // === LEFT COLUMN: RESULTS ===
    results = new Fl_Scroll(20, 240, 360, 230, "Results");
    results->align(FL_ALIGN_TOP_LEFT);
    results->box(FL_THIN_UP_BOX);
    
    // *** CRITICAL FIX HERE ***
    // We must close the 'results' group, otherwise the map gets added INSIDE it.
    results->end(); 


    // === RIGHT COLUMN: VISUALIZATION ===
    // Now this will be added to the main Window (at x=400), not the scroll box.
    mapDisplay = new GraphDisplay(400, 20, 430, 460, &g);
    mapDisplay->box(FL_BORDER_BOX); 
    mapDisplay->color(FL_WHITE);

    window->show();
}

//
// ─── HANDLE SEARCH BUTTON CLICK (UPDATED) ──────────────────────
//
void Application::handleClick(bobcat::Widget *sender) {
    results->clear();
    
    int startIndex = start->value();
    int destIndex  = dest->value();
    int modeIndex  = mode->value(); 

    Vertex *s = cities[startIndex];
    Vertex *d = cities[destIndex];

    Waypoint *path = nullptr;

    // Run Algorithm
    if (modeIndex == 0)      path = g.ucs(s, d, USE_PRICE);
    else if (modeIndex == 1) path = g.ucs(s, d, USE_TIME);
    else                     path = g.bfs(s, d);

    if (!path) {
        results->add(new TextBox(30, 260, 320, 30, "No route found."));
        vector<string> empty;
        mapDisplay->setPath(empty); // Clear map lines
        window->redraw();
        return;
    }

    // Collect the path
    vector<Waypoint*> reversePath;
    Waypoint *temp = path;
    while (temp != nullptr) {
        reversePath.push_back(temp);
        temp = temp->parent;
    }
    std::reverse(reversePath.begin(), reversePath.end());

    // 1. EXTRACT STRINGS FOR VISUALIZATION
    vector<string> visualPath;
    for(Waypoint* wp : reversePath) {
        visualPath.push_back(wp->vertex->data);
    }
    // Update the Map Widget
    mapDisplay->setPath(visualPath);


    // 2. DISPLAY TEXT RESULTS
    int y = results->y() + 10;

    for (int i = 0; i < reversePath.size(); i++) {
        Waypoint *wp = reversePath[i];
        results->add(new TextBox(40, y, 300, 25, wp->vertex->data));
        y += 30;

        if (wp->parent != nullptr) {
            string info;
            if (modeIndex == 0) info = "Price: $" + to_string(wp->edgeCost);
            else if (modeIndex == 1) info = "Time: " + to_string(wp->edgeCost/60) + " hours";
            else info = "Stop " + to_string(i);

            results->add(new TextBox(60, y, 280, 25, info));
            y += 30;
        }
    }

    // Summary
    y += 10;
    results->add(new TextBox(40, y, 300, 25, "======================="));
    y += 30;

    if (modeIndex == 0) {
        results->add(new TextBox(40, y, 300, 25, "Total Price: $" + to_string(path->partialCost)));
    } else if (modeIndex == 1) {
        results->add(new TextBox(40, y, 300, 25, "Total Time: " + to_string(path->partialCost/60) + " hours"));
    } else {
        results->add(new TextBox(40, y, 300, 25, "Total Stops: " + to_string((int)reversePath.size() - 2)));
    }

    // Clean up memory
    Waypoint * t = path;
    while(t != nullptr){
        Waypoint* parent = t->parent;
        delete t;
        t = parent;
    }
    
    // Force complete redraw
    window->redraw();
}