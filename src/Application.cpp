#include "Graph.h"
#include <Application.h>

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
// ───────────────────────── CONSTRUCTOR / DESTRUCTOR ─────────────────────────
//
Application::Application() {
    initData();
    initInterface();
}

Application::~Application() {
    delete mapDisplay;
    delete results;
    delete search;
    delete mode;
    delete dest;
    delete start;
    delete window;
}

//
// ──────────────────────────── LOAD AIRPORT DATA ─────────────────────────────
//
void Application::loadAirports(const string &filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening " << filename << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        if (line.size() == 0) continue;

        Vertex* v = new Vertex(line);
        cities.append(v);
        g.addVertex(v);
    }
}

//
// ───────────────────────────── LOAD EDGE DATA ───────────────────────────────
//
void Application::loadEdges(const string &filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening " << filename << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        if (line.size() == 0) continue;

        stringstream ss(line);
        string a, b, c, d;

        getline(ss, a, ',');
        getline(ss, b, ',');
        getline(ss, c, ',');
        getline(ss, d, ',');

        int from = stoi(a);
        int to   = stoi(b);
        int price = stoi(c);
        int time  = stoi(d);

        g.addEdge(cities[from], cities[to], price, time);
    }
}

//
// ───────────────────────────── INITIALIZE DATA ──────────────────────────────
//
void Application::initData() {
    loadAirports("assets/vertices.csv");
    loadEdges("assets/edges.csv");
}

//
// ───────────────────────────── UI INITIALIZATION ─────────────────────────────
//
void Application::initInterface() {
    window = new Window(100, 100, 850, 500, "Flight Planner - Visualized");

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

    results = new Fl_Scroll(20, 240, 360, 230, "Results");
    results->align(FL_ALIGN_TOP_LEFT);
    results->box(FL_THIN_UP_BOX);
    results->end();

    mapDisplay = new GraphDisplay(400, 20, 430, 460, &g);
    mapDisplay->box(FL_BORDER_BOX);
    mapDisplay->color(FL_WHITE);

    window->show();
}

//
// ───────────────────────────── HANDLE SEARCH CLICK ───────────────────────────
//
void Application::handleClick(bobcat::Widget *sender) {

    results->clear();

    int si = start->value();
    int di = dest->value();
    int mi = mode->value();

    Vertex* s = cities[si];
    Vertex* d = cities[di];

    // NEW SYSTEM: Graph functions return SearchResult
    SearchResult res;

    if (mi == 0)
        res = g.ucs(s, d, USE_PRICE);
    else if (mi == 1)
        res = g.ucs(s, d, USE_TIME);
    else
        res = g.bfs(s, d);

    Waypoint* goal = res.goal;

    if (!goal) {
        results->add(new TextBox(40, 260, 250, 25, "No route found."));
        mapDisplay->setPath({});
        window->redraw();
        deleteWaypointTree(res.root);
        return;
    }

    //
    // ─────────────────────────── BUILD PATH LIST ───────────────────────────────
    //
    vector<Waypoint*> pathNodes;
    Waypoint* t = goal;

    while (t != nullptr) {
        pathNodes.push_back(t);
        t = t->parent;
    }

    reverse(pathNodes.begin(), pathNodes.end());

    //
    // ─────────────────────────── UPDATE VISUALIZER ─────────────────────────────
    //
    vector<string> names;
    for (Waypoint* wp : pathNodes)
        names.push_back(wp->vertex->data);

    mapDisplay->setPath(names);

    //
    // ─────────────────────────── DISPLAY TEXT RESULTS ───────────────────────────
    //
    int y = results->y() + 10;

    for (int i = 0; i < pathNodes.size(); i++) {
        Waypoint* wp = pathNodes[i];

        results->add(new TextBox(40, y, 300, 25, wp->vertex->data));
        y += 30;

        if (wp->parent) {
            Edge* used = nullptr;
            Vertex* from = wp->parent->vertex;

            for (int j = 0; j < from->edgeList.size(); j++) {
                if (from->edgeList[j]->to == wp->vertex) {
                    used = from->edgeList[j];
                    break;
                }
            }

            if (used) {
                string info;
                if (mi == 0) info = "Price: $" + to_string(used->price);
                if (mi == 1) info = "Time: " + to_string(used->time / 60) + " hrs";
                if (mi == 2) info = "Stop " + to_string(i);

                results->add(new TextBox(60, y, 280, 25, info));
                y += 30;
            }
        }
    }

    //
    // ─────────────────────────── SHOW TOTALS ─────────────────────────────
    //
    int totalPrice = 0;
    int totalTime  = 0;

    for (int i = 0; i < pathNodes.size() - 1; i++) {
        Vertex* a = pathNodes[i]->vertex;
        Vertex* b = pathNodes[i+1]->vertex;

        for (int j = 0; j < a->edgeList.size(); j++) {
            Edge* e = a->edgeList[j];
            if (e->to == b) {
                totalPrice += e->price;
                totalTime  += e->time;
                break;
            }
        }
    }

    y += 10;
    results->add(new TextBox(40, y, 300, 25, "======================="));
    y += 30;

    results->add(new TextBox(40, y, 300, 25, "Total Price: $" + to_string(totalPrice)));
    y += 30;

    results->add(new TextBox(40, y, 300, 25, "Total Time: " + to_string(totalTime / 60) + " hours"));
    y += 30;

    results->add(new TextBox(40, y, 300, 25, "Total Stops: " +
                                to_string((int)pathNodes.size() - 2)));

    //
    // ─────────────────────────── CLEAN MEMORY SAFELY ───────────────────────────
    //
    deleteWaypointTree(res.root);

    window->redraw();
}
