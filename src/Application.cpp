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

using namespace bobcat;
using namespace std;

//
// ─── CONSTRUCTOR ───────────────────────────────────────────────
//
Application::Application() {
    initData();       // load CSVs
    initInterface();  // build UI
}

//
// ─── LOAD VERTICES FROM vertices.csv ───────────────────────────
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

//
// ─── LOAD EDGES FROM edges.csv ─────────────────────────────────
//   Format: FROM_INDEX,TO_INDEX,PRICE,TIME
//
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

    file.close();
}

//
// ─── INIT DATA ───────────────────────────────────────────────
//   Loads from CSV instead of hardcoding
//
void Application::initData() {
    loadAirports("assets/vertices.csv");
    loadEdges("assets/edges.csv");
}

//
// ─── INIT USER INTERFACE ─────────────────────────────────────
//
void Application::initInterface() {
    window = new Window(100, 100, 420, 500, "Flight Planner");

    // dropdowns
    start = new Dropdown(20, 40, 360, 25, "Starting Airport");
    dest  = new Dropdown(20, 90, 360, 25, "Destination Airport");

    for (int i = 0; i < cities.size(); i++) {
        start->add(cities[i]->data);
        dest->add(cities[i]->data);
    }

    // mode dropdown
    mode = new Dropdown(20, 140, 360, 25, "Search Type");
    mode->add("Cheapest Price");
    mode->add("Shortest Time");
    mode->add("Fewest Stops");

    search = new Button(20, 190, 360, 30, "Search");
    ON_CLICK(search, Application::handleClick);

    results = new Fl_Scroll(20, 240, 360, 230, "Results");
    results->align(FL_ALIGN_TOP_LEFT);
    results->box(FL_THIN_UP_BOX);

    window->show();
}

//
// ─── HANDLE SEARCH BUTTON CLICK ─────────────────────────────
//
void Application::handleClick(bobcat::Widget *sender) {
    results->clear();
    window->redraw();

    int startIndex = start->value();
    int destIndex  = dest->value();
    int modeIndex  = mode->value();  // 0=price,1=time,2=stops

    Vertex *s = cities[startIndex];
    Vertex *d = cities[destIndex];

    Waypoint *path = nullptr;

    if (modeIndex == 0) {                       // cheapest
        path = g.ucs(s, d, USE_PRICE);
    }
    else if (modeIndex == 1) {                  // fastest
        path = g.ucs(s, d, USE_TIME);
    }
    else {                                      // fewest stops
        path = g.bfs(s, d);
    }

    system("clear");

    if (!path) {
        results->add(new TextBox(30, 260, 320, 30, "No route found."));
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

    // reverse so path is start → end
    std::reverse(reversePath.begin(), reversePath.end());

    // Display route
    int y = results->y() + 10;

    for (int i = 0; i < reversePath.size(); i++) {
        Waypoint *wp = reversePath[i];

        results->add(new TextBox(40, y, 300, 25, wp->vertex->data));
        y += 30;

        if (wp->parent != nullptr) {


            // Show time in hours (B option)
            int hours = wp->edgeCost / 60;

            string info;
            if (modeIndex == 0) {
                info = "Price: $" + to_string(wp->edgeCost);
            }
            else if (modeIndex == 1) {
                info = "Time: " + to_string(hours) + " hours";
            }
            else {
                info = "Stop " + to_string(i);
            }

            results->add(new TextBox(60, y, 280, 25, info));
            y += 30;
        }
    }

    // Summary section
    y += 10;
    results->add(new TextBox(40, y, 300, 25, "======================="));
    y += 30;

    if (modeIndex == 0) {
        results->add(new TextBox(40, y, 300, 25,
                                 "Total Price: $" + to_string(path->partialCost)));
        y += 30;
    }
    else if (modeIndex == 1) {
        int finalHours = path->partialCost / 60;
        results->add(new TextBox(40, y, 300, 25,
                                 "Total Time: " + to_string(finalHours) + " hours"));
        y += 30;
    }
    else {
        int stops = reversePath.size() - 2;
        results->add(new TextBox(40, y, 300, 25,
                                 "Total Stops: " + to_string(stops)));
        y += 30;
    }

    window->redraw();
}
