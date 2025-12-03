#include <Application.h>
#include <FL/Enumerations.H>
#include <FL/Fl_Scroll.H>
#include <bobcat_ui/bobcat_ui.h>
#include <bobcat_ui/button.h>
#include <bobcat_ui/dropdown.h>
#include <bobcat_ui/textbox.h>
#include <bobcat_ui/window.h>
#include <string>

using namespace bobcat;
using namespace std;

Application::Application() {
    // App's constructor
    initData();
    initInterface();
}

void Application::handleClick(bobcat::Widget *sender) {

    results->clear();
    window->redraw();

    int startIndex = start->value();
    int destIndex = dest->value();

    Waypoint *path = g.ucs(cities[startIndex], cities[destIndex]);

    system("clear");

    if (path) {
        cout << "We found a path" << endl;
        Waypoint *temp = path;
        int y = results->y() + 10;
        while (temp != nullptr) {
            results->add(new TextBox(40, y, 300, 25, temp->vertex->data));
            y += 40;
            if (temp->parent != nullptr) {
                results->add(new TextBox(
                    40, y, 300, 25,
                    "    Flight time: " + to_string(temp->weight) + " hours"));
                y += 40;
            }
            cout << temp->vertex->data << " " << temp->partialCost << endl;
            temp = temp->parent;

            window->redraw();
        }
    } else {
        cout << "There is no path" << endl;
    }
}

void Application::initData() {
    cities.append(new Vertex("San Francisco"));  // 0
    cities.append(new Vertex("New York"));       // 1
    cities.append(new Vertex("Rio De Janeiro")); // 2
    cities.append(new Vertex("Paris"));          // 3
    cities.append(new Vertex("Johannesburg"));   // 4
    cities.append(new Vertex("Moscow"));         // 5
    cities.append(new Vertex("Sydney"));         // 6
    cities.append(new Vertex("Tokyo"));          // 7
    cities.append(new Vertex("Beijing"));        // 8

    for (int i = 0; i < cities.size(); i++) {
        g.addVertex(cities[i]);
    }

    g.addEdge(cities[0], cities[1], 6);
    g.addEdge(cities[1], cities[2], 13);
    g.addEdge(cities[1], cities[3], 7);
    g.addEdge(cities[1], cities[4], 14);
    g.addEdge(cities[1], cities[5], 15);
    g.addEdge(cities[1], cities[6], 40);
    g.addEdge(cities[2], cities[3], 11);
    g.addEdge(cities[2], cities[8], 18);
    g.addEdge(cities[3], cities[6], 17);
    g.addEdge(cities[4], cities[7], 16);
    g.addEdge(cities[4], cities[6], 11);
    g.addEdge(cities[6], cities[7], 10);
    g.addEdge(cities[6], cities[8], 3);
    g.addEdge(cities[8], cities[5], 8);
}

void Application::initInterface() {
    window = new Window(100, 100, 400, 400, "Flight Planner");

    start = new Dropdown(20, 40, 360, 25, "Starting Point");
    dest = new Dropdown(20, 100, 360, 25, "Destination");

    for (int i = 0; i < cities.size(); i++) {
        start->add(cities[i]->data);
        dest->add(cities[i]->data);
    }

    search = new Button(20, 150, 360, 25, "Search");
    ON_CLICK(search, Application::handleClick);

    results = new Fl_Scroll(20, 200, 360, 180, "Results");
    results->align(FL_ALIGN_TOP_LEFT);
    results->box(FL_THIN_UP_BOX);

    window->show();
}
