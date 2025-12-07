#ifndef APPLICATION_H
#define APPLICATION_H

#include <bobcat_ui/bobcat_ui.h>
#include <bobcat_ui/button.h>
#include <bobcat_ui/dropdown.h>
#include <bobcat_ui/window.h>
#include <bobcat_ui/textbox.h>
#include <Graph.h>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <vector>
#include <map>

//
// ─── HELPER FOR COORDINATES ──────────────────────────────────────────
//
struct Point { int x, y; };

//
//
// ─── CUSTOM WIDGET FOR VISUALIZATION ─────────────────────────────────
//
class GraphDisplay : public Fl_Box {
    Graph* graphRef;
    std::vector<std::string> currentPath; 
    
    // Adjusted Coordinates to spread cities out better
    Point getCoords(std::string city) {
        // X, Y coordinates relative to the box (0,0 is top-left of box)
        if (city == "San Francisco") return {40, 220}; 
        if (city == "New York")      return {150, 140};
        if (city == "Rio De Janeiro")return {140, 380};
        if (city == "Paris")         return {280, 120};
        if (city == "London")        return {270, 80};  // Moved up
        if (city == "Johannesburg")  return {290, 420};
        if (city == "Moscow")        return {380, 70};
        if (city == "Dubai")         return {340, 220};
        if (city == "Beijing")       return {350, 150}; 
        if (city == "Tokyo")         return {400, 180};
        if (city == "Sydney")        return {390, 400};
        return {200, 200}; // Default
    }

public:
    GraphDisplay(int x, int y, int w, int h, Graph* g) : Fl_Box(x, y, w, h, ""), graphRef(g) {
        box(FL_BORDER_BOX); 
        color(FL_WHITE);  
    }

    void setPath(std::vector<std::string> path) {
        currentPath = path;
        redraw(); // Triggers the draw() function below
    }

    void draw() override {
        // 1. THE FIX: Manually wipe the background white
        // This erases the previous red lines.
        fl_color(FL_WHITE);
        fl_rectf(x(), y(), w(), h());
        
        // Draw the black border around the box
        fl_color(FL_BLACK);
        fl_rect(x(), y(), w(), h());

        if (!graphRef) return;

        // Widget Position offsets
        int X = x(); 
        int Y = y(); 

        // 2. Draw ALL Edges (Gray Lines)
        fl_color(FL_LIGHT2); 
        fl_line_style(FL_SOLID, 1);

        for (int i = 0; i < graphRef->vertices.size(); i++) {
            Vertex* v = graphRef->vertices[i];
            Point p1 = getCoords(v->data);

            for (int j = 0; j < v->edgeList.size(); j++) {
                Edge* e = v->edgeList[j];
                Point p2 = getCoords(e->to->data);
                
                fl_line(X + p1.x, Y + p1.y, X + p2.x, Y + p2.y);
            }
        }

        // 3. Draw Highlighted Path (Red Thick Lines)
        if (currentPath.size() > 1) {
            fl_color(FL_RED);
            fl_line_style(FL_SOLID, 3); 

            for (size_t i = 0; i < currentPath.size() - 1; i++) {
                Point p1 = getCoords(currentPath[i]);
                Point p2 = getCoords(currentPath[i+1]);
                fl_line(X + p1.x, Y + p1.y, X + p2.x, Y + p2.y);
            }
        }

        // 4. Draw City Nodes (Circles + Text)
        fl_font(FL_HELVETICA, 10);
        
        for (int i = 0; i < graphRef->vertices.size(); i++) {
            std::string name = graphRef->vertices[i]->data;
            Point p = getCoords(name);

            bool inPath = false;
            for(const auto& s : currentPath) if(s == name) inPath = true;

            if(inPath) fl_color(FL_RED);
            else       fl_color(FL_BLUE);

            // Draw Dot
            fl_pie(X + p.x - 4, Y + p.y - 4, 8, 8, 0, 360);

            // Draw Text Label (Offset slightly for readability)
            fl_color(FL_BLACK);
            fl_draw(name.c_str(), X + p.x + 8, Y + p.y + 4);
        }
        
        fl_line_style(0); // Reset style
    }
};

class Application : public bobcat::Application_ {
    // UI Components
    bobcat::Window* window;
    bobcat::Dropdown* start;
    bobcat::Dropdown* dest;
    bobcat::Dropdown* mode;     
    bobcat::Button* search;
    Fl_Scroll* results;
    
    // NEW: The Visualizer Widget
    GraphDisplay* mapDisplay;

    // Data
    ArrayList<Vertex*> cities;
    Graph g;

    // Internal helpers
    void initData();
    void initInterface();
    void loadAirports(const std::string& filename);
    void loadEdges(const std::string& filename);
    
    // Note: displayRoute removed/merged into handleClick logic
    void handleClick(bobcat::Widget* sender);

public:
    Application(); 
    ~Application(); 
};

#endif