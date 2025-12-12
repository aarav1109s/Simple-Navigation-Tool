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
// This is the constructor - it gets called when we create a new Application object
Application::Application() {
    initData();       // Load the airport and edge data from CSV files
    initInterface();  // Set up all the UI stuff like buttons and dropdowns
}

// This is the destructor - it gets called when the Application object is destroyed
// We need to clean up all the memory we allocated with 'new'
Application::~Application(){
    // Delete all the UI widgets we created
    // I'm not 100% sure if FLTK/bobcat widgets delete their children automatically,
    // but to be safe I'm deleting them explicitly here
    delete mapDisplay;  // The graph visualization widget
    delete results;     // The scroll box that shows search results
    delete search;      // The search button
    delete mode;        // The dropdown for search type
    delete dest;        // The destination airport dropdown
    delete start;       // The starting airport dropdown
    delete window;      // The main window
    
    // The Graph and cities don't need to be deleted here because they're not pointers
    // They're just regular member variables, so C++ will automatically call their
    // destructors when this Application object is destroyed
    // The Graph destructor will delete all the Vertex objects
    // And each Vertex destructor will delete all its Edge objects
    // So we don't have to worry about memory leaks for those!
}

//
// ─── LOADING DATA ──────────────────────────────────
//
// This function reads the airport names from a CSV file
// Each line in the file is the name of an airport
void Application::loadAirports(const std::string &filename) {
    ifstream file(filename);  // Open the file for reading
    if (!file.is_open()) {    // Check if the file opened successfully
        cerr << "Error: Cannot open " << filename << endl;
        return;  // If we can't open it, just give up
    }
    string line;
    // Read each line from the file
    while (getline(file, line)) {
        if (line.size() == 0) continue;  // Skip empty lines
        // Create a new Vertex object for this airport
        Vertex *v = new Vertex(line);
        // Add it to our cities list and to the graph
        cities.append(v);
        g.addVertex(v);
    }
    file.close();  // Don't forget to close the file!
}

// This function reads the flight connections (edges) from a CSV file
// Each line has: from_index, to_index, price, time (all separated by commas)
void Application::loadEdges(const std::string &filename) {
    ifstream file(filename);  // Open the file
    if (!file.is_open()) {
        cerr << "Error: Cannot open " << filename << endl;
        return;
    }
    string line;
    while (getline(file, line)) {
        if (line.size() == 0) continue;  // Skip empty lines
        // Parse the CSV line - split by commas
        stringstream ss(line);
        string a, b, c, d;  // These will hold the 4 values as strings
        getline(ss, a, ',');  // Get the first value (from index)
        getline(ss, b, ',');  // Get the second value (to index)
        getline(ss, c, ',');  // Get the third value (price)
        getline(ss, d, ',');  // Get the fourth value (time)
        // Convert strings to integers
        int from = stoi(a);   // Starting airport index
        int to   = stoi(b);   // Destination airport index
        int price = stoi(c);  // Price of the flight
        int time  = stoi(d);  // Time it takes (in minutes I think?)
        // Add the edge to the graph
        g.addEdge(cities[from], cities[to], price, time);
    }
    file.close();
}

// This just calls the two load functions to read all the data
void Application::initData() {
    loadAirports("assets/vertices.csv");  // Load all the airports
    loadEdges("assets/edges.csv");        // Load all the flight connections
}

//
// ─── INIT USER INTERFACE ──────────────────
//
// This function creates all the UI elements and sets up the window layout
void Application::initInterface() {
    // Create the main window - it's wide enough to fit both the controls and the map
    window = new Window(100, 100, 850, 500, "Flight Planner - Visualized");

    // === LEFT COLUMN: CONTROLS ===
    // Create dropdowns for selecting starting and destination airports
    start = new Dropdown(20, 40, 360, 25, "Starting Airport");
    dest  = new Dropdown(20, 90, 360, 25, "Destination Airport");

    // Add all the cities to both dropdowns so the user can pick any airport
    for (int i = 0; i < cities.size(); i++) {
        start->add(cities[i]->data);  // Add to starting airport dropdown
        dest->add(cities[i]->data);    // Add to destination dropdown
    }

    // Create dropdown for search type (cheapest, fastest, or fewest stops)
    mode = new Dropdown(20, 140, 360, 25, "Search Type");
    mode->add("Cheapest Price");   // Option 0
    mode->add("Shortest Time");    // Option 1
    mode->add("Fewest Stops");     // Option 2

    // Create the search button
    search = new Button(20, 190, 360, 30, "Search");
    // Make the button call handleClick when clicked
    ON_CLICK(search, Application::handleClick);

    // === LEFT COLUMN: RESULTS ===
    // Create a scrollable area to show the search results
    results = new Fl_Scroll(20, 240, 360, 230, "Results");
    results->align(FL_ALIGN_TOP_LEFT);
    results->box(FL_THIN_UP_BOX);
    
    // IMPORTANT: We have to call end() here to close the results group
    // If we don't, the map widget will get added INSIDE the results box instead of next to it
    // This was causing a bug before, so I'm keeping this comment to remember why
    results->end(); 


    // === RIGHT COLUMN: VISUALIZATION ===
    // Create the graph display widget on the right side of the window
    // The x position is 400 so it's to the right of the controls
    mapDisplay = new GraphDisplay(400, 20, 430, 460, &g);
    mapDisplay->box(FL_BORDER_BOX);  // Give it a border
    mapDisplay->color(FL_WHITE);     // Make the background white

    // Show the window to the user
    window->show();
}

//
// ─── HANDLE SEARCH BUTTON CLICK ──────────────────────
//
// This function gets called when the user clicks the search button
// It runs the pathfinding algorithm and displays the results
void Application::handleClick(bobcat::Widget *sender) {
    // Clear any previous results
    results->clear();
    
    // Get the selected values from the dropdowns
    int startIndex = start->value();  // Which airport the user selected for start
    int destIndex  = dest->value();   // Which airport the user selected for destination
    int modeIndex  = mode->value();   // Which search mode (0=cheapest, 1=fastest, 2=fewest stops)

    // Get the actual Vertex objects for the selected airports
    Vertex *s = cities[startIndex];  // Starting vertex
    Vertex *d = cities[destIndex];   // Destination vertex

    Waypoint *path = nullptr;  // This will hold the result path (or nullptr if no path found)

    // Run the appropriate algorithm based on the search mode
    if (modeIndex == 0) {
        // Cheapest price - use UCS with price as the weight
        path = g.ucs(s, d, USE_PRICE);
    } else if (modeIndex == 1) {
        // Shortest time - use UCS with time as the weight
        path = g.ucs(s, d, USE_TIME);
    } else {
        // Fewest stops - use BFS (breadth-first search)
        path = g.bfs(s, d);
    }

    // Check if we found a path
    if (!path) {
        // No path was found, so show an error message
        results->add(new TextBox(30, 260, 320, 30, "No route found."));
        vector<string> empty;  // Empty path for the map
        mapDisplay->setPath(empty);  // Clear the map (remove any red lines)
        window->redraw();  // Update the display
        return;  // Exit early since there's nothing to show
    }

    // We found a path! Now we need to collect all the waypoints
    // The path is stored backwards (from destination to start), so we need to reverse it
    vector<Waypoint*> reversePath;
    Waypoint *temp = path;  // Start from the destination
    // Walk backwards through the parent pointers to get the whole path
    while (temp != nullptr) {
        reversePath.push_back(temp);
        temp = temp->parent;  // Move to the parent (previous city in the path)
    }
    // Reverse the vector so it goes from start to destination
    std::reverse(reversePath.begin(), reversePath.end());

    // 1. EXTRACT STRINGS FOR VISUALIZATION
    // We need to convert the Waypoint objects to just city names (strings)
    // so we can pass them to the map display widget
    vector<string> visualPath;
    for(Waypoint* wp : reversePath) {
        visualPath.push_back(wp->vertex->data);  // Get the city name from each waypoint
    }
    // Update the map to show the path in red
    mapDisplay->setPath(visualPath);


    // 2. DISPLAY TEXT RESULTS
    // Now we'll show the path as text in the results box
    int y = results->y() + 10;  // Starting Y position for the first result

    // Loop through each city in the path
    for (int i = 0; i < reversePath.size(); i++) {
        Waypoint *wp = reversePath[i];
        // Display the city name
        results->add(new TextBox(40, y, 300, 25, wp->vertex->data));
        y += 30;  // Move down for the next line

        // If this isn't the last city, show the cost/time info for the edge
        if (wp->parent != nullptr) {
            string info;
            // Show different info depending on the search mode
            if (modeIndex == 0) {
                info = "Price: $" + to_string(wp->edgeCost);  // Show price
            } else if (modeIndex == 1) {
                info = "Time: " + to_string(wp->edgeCost/60) + " hours";  // Show time (convert minutes to hours)
            } else {
                info = "Stop " + to_string(i);  // Show stop number
            }

            results->add(new TextBox(60, y, 280, 25, info));
            y += 30;  // Move down again
        }
    }

    // Add a summary at the bottom showing the total cost/time/stops
    // We need to calculate the actual totals by looking at the edges in the path
    y += 10;  // Add some spacing
    results->add(new TextBox(40, y, 300, 25, "======================="));
    y += 30;

    // Calculate total price and total time by traversing the path
    int totalPrice = 0;
    int totalTime = 0;
    for (int i = 0; i < reversePath.size() - 1; i++) {
        Vertex *from = reversePath[i]->vertex;
        Vertex *to = reversePath[i + 1]->vertex;
        // Find the edge between these two vertices
        for (int j = 0; j < from->edgeList.size(); j++) {
            Edge *e = from->edgeList[j];
            if (e->to == to) {
                totalPrice += e->price;
                totalTime += e->time;
                break;
            }
        }
    }

    // Show all three totals regardless of search mode
    // The route was chosen based on user preference, but we show all the info
    results->add(new TextBox(40, y, 300, 25, "Total Price: $" + to_string(totalPrice)));
    y += 30;
    results->add(new TextBox(40, y, 300, 25, "Total Time: " + to_string(totalTime/60) + " hours"));
    y += 30;
    // Total stops (subtract 2 because start and destination aren't stops)
    results->add(new TextBox(40, y, 300, 25, "Total Stops: " + to_string((int)reversePath.size() - 2)));

    // Clean up memory
    // The search algorithm creates a tree of Waypoint objects, and we need to delete them
    // to avoid memory leaks. The Waypoint destructor will recursively delete all children,
    // so we just need to find the root (the starting waypoint) and delete it.
    Waypoint * root = path;  // Start from the destination
    // Walk backwards to find the root (where parent is nullptr)
    while (root != nullptr && root->parent != nullptr) {
        root = root->parent;
    }
    if (root != nullptr) {
        // Delete the root - this will call the destructor which deletes all children recursively
        delete root;
    }
    
    // Force the window to redraw so the user sees the updated results
    window->redraw();
}