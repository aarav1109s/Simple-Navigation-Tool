#include <Application.h>
#include <bobcat_ui/bobcat_ui.h>

#include <FL/fl_draw.H>
#include <fstream>
#include <sstream>

using namespace std;
using namespace bobcat;

//
// ─────────────────────────────────────────────────────────────
//  CONSTRUCTOR + DESTRUCTOR
// ─────────────────────────────────────────────────────────────
//
Application::Application() {
    initData();
    initInterface();
}

Application::~Application() {
    delete map;
    delete results;
    delete search;
    delete mode;
    delete dest;
    delete start;
    delete window;
}

//
// ─────────────────────────────────────────────────────────────
//  LOAD AIRPORTS
// ─────────────────────────────────────────────────────────────
//
void Application::loadAirports(const std::string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "ERROR: Cannot open vertices CSV: " << filename << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        if (line.size() == 0) continue;

        Vertex* v = new Vertex(line);
        cities.append(v);
        g.addVertex(v);
    }

    file.close();
}

//
// ─────────────────────────────────────────────────────────────
//  LOAD EDGES
// ─────────────────────────────────────────────────────────────
//
void Application::loadEdges(const std::string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "ERROR: Cannot open edges CSV: " << filename << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        if (line.size() == 0) continue;

        string a, b, c, d;
        stringstream ss(line);

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
// ─────────────────────────────────────────────────────────────
//  INIT DATA
// ─────────────────────────────────────────────────────────────
//
void Application::initData() {
    loadAirports("assets/vertices.csv");
    loadEdges("assets/edges.csv");
}

//
// ─────────────────────────────────────────────────────────────
//  INIT INTERFACE
// ─────────────────────────────────────────────────────────────
//
void Application::initInterface() {
    window = new Window(100, 100, 900, 550, "Flight Planner");

    // Dropdowns
    start = new Dropdown(20, 40, 350, 25, "Starting Airport");
    dest  = new Dropdown(20, 90, 350, 25, "Destination Airport");

    for (int i = 0; i < cities.size(); i++) {
        start->add(cities[i]->data);
        dest->add(cities[i]->data);
    }

    // Search type select
    mode = new Dropdown(20, 140, 350, 25, "Search Mode");
    mode->add("Cheapest Price");
    mode->add("Shortest Time");
    mode->add("Fewest Stops");

    // Search button
    search = new Button(20, 180, 350, 30, "Search");
    ON_CLICK(search, Application::handleClick);

    // Results panel
    results = new Fl_Scroll(20, 230, 350, 280, "Results");
    results->align(FL_ALIGN_TOP_LEFT);
    results->box(FL_THIN_UP_BOX);
    results->end();  // critical (do not remove)

    // Visualization panel
    map = new GraphDisplay(400, 20, 480, 500, &g);

    window->show();
}

//
// ─────────────────────────────────────────────────────────────
//  HANDLE SEARCH BUTTON CLICK
// ─────────────────────────────────────────────────────────────
//
void Application::handleClick(bobcat::Widget* sender) {
    results->clear();

    int sIndex = start->value();
    int dIndex = dest->value();
    int modeIndex = mode->value();

    Vertex* S = cities[sIndex];
    Vertex* D = cities[dIndex];

    SearchResult result;

    if (modeIndex == 0)
        result = g.ucs(S, D, USE_PRICE);
    else if (modeIndex == 1)
        result = g.ucs(S, D, USE_TIME);
    else
        result = g.bfs(S, D);

    Waypoint* goal = result.goal;
    Waypoint* root = result.root;

    // No path found
    if (!goal) {
        results->add(new TextBox(40, 260, 280, 30, "No route found."));
        map->setPath(vector<string>());  // clear map
        deleteWaypointTree(root);
        window->redraw();
        return;
    }

    // Extract path by walking backwards
    vector<Waypoint*> rev;
    Waypoint* temp = goal;
    while (temp) {
        rev.push_back(temp);
        temp = temp->parent;
    }

    // Reverse order → start → destination
    for (int i = 0; i < rev.size() / 2; i++) {
        Waypoint* t = rev[i];
        rev[i] = rev[rev.size() - 1 - i];
        rev[rev.size() - 1 - i] = t;
    }

    // Convert to string list for visualization
    vector<string> names;
    for (int i = 0; i < rev.size(); i++)
        names.push_back(rev[i]->vertex->data);

    map->setPath(names);

    // ---------------- print RESULTS ----------------
    int ry = results->y() + 10;

    int totalPrice = 0;
    int totalTime  = 0;

    for (int i = 0; i < rev.size() - 1; i++) {
        Vertex* A = rev[i]->vertex;
        Vertex* B = rev[i + 1]->vertex;

        results->add(new TextBox(40, ry, 260, 25, A->data));
        ry += 25;

        for (int j = 0; j < A->edgeList.size(); j++) {
            Edge* e = A->edgeList[j];
            if (e->to == B) {
                totalPrice += e->price;
                totalTime  += e->time;

                string info = "Price: $" + to_string(e->price)
                            + ", Time: " + to_string(e->time) + " min";

                results->add(new TextBox(60, ry, 240, 25, info));
                ry += 25;
                break;
            }
        }
    }

    // destination display
    results->add(new TextBox(40, ry, 260, 25,
                             rev.back()->vertex->data));
    ry += 30;

    // summary
    results->add(new TextBox(40, ry, 260, 25, "=========="));
    ry += 25;
    results->add(new TextBox(40, ry, 260, 25,
                             "Total Price: $" + to_string(totalPrice)));
    ry += 25;
    results->add(new TextBox(40, ry, 260, 25,
                             "Total Time: " + to_string(totalTime) + " min"));
    ry += 25;
    results->add(new TextBox(40, ry, 260, 25,
                             "Stops: " + to_string((int)rev.size() - 2)));

    // Cleanup whole tree
    deleteWaypointTree(root);

    window->redraw();
}
