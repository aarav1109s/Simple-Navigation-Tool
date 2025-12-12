#ifndef APPLICATION_H
#define APPLICATION_H

#include <bobcat_ui/bobcat_ui.h>
#include <bobcat_ui/button.h>
#include <bobcat_ui/dropdown.h>
#include <bobcat_ui/textbox.h>
#include <bobcat_ui/window.h>
#include <cmath>


#include <FL/Fl_Scroll.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>

#include <Graph.h>
#include <string>

// ------------------------------------------------------------
// Helper struct for drawing positions
// ------------------------------------------------------------
struct Point { int x, y; };

// ------------------------------------------------------------
// GraphDisplay — draws graph dynamically from CSV input
// ------------------------------------------------------------
class GraphDisplay : public Fl_Box {
    Graph* graphRef;
    std::vector<std::string> path;

public:
    GraphDisplay(int X, int Y, int W, int H, Graph* g)
        : Fl_Box(X, Y, W, H, ""), graphRef(g)
    {
        box(FL_BORDER_BOX);
        color(FL_WHITE);
    }

    // Set a new path to highlight
    void setPath(const std::vector<std::string>& p) {
        path = p;
        redraw();
    }

private:
    // Circular positioning for ANY number of airports
    Point getCoords(int index, int total) {
        double angle = (2 * M_PI * index) / total;
        int radius = (w() - 80) / 2;

        int cx = x() + w()/2;
        int cy = y() + h()/2;

        return {
            cx + (int)(cos(angle) * radius),
            cy + (int)(sin(angle) * radius)
        };
    }

    int indexOf(const std::string& name) {
        for (int i = 0; i < graphRef->vertices.size(); i++)
            if (graphRef->vertices[i]->data == name)
                return i;
        return -1;
    }

public:
    // ------------------------------------------------------------
    // DRAWING ROUTINE — called automatically
    // ------------------------------------------------------------
    void draw() override {
        fl_color(FL_WHITE);
        fl_rectf(x(), y(), w(), h());

        fl_color(FL_BLACK);
        fl_rect(x(), y(), w(), h());

        if (!graphRef) return;

        int N = graphRef->vertices.size();

        // ---------------- Draw edges (gray) ----------------
        fl_color(FL_GRAY);
        fl_line_style(FL_SOLID, 1);

        for (int i = 0; i < N; i++) {
            Vertex* v = graphRef->vertices[i];
            Point p1 = getCoords(i, N);

            for (int j = 0; j < v->edgeList.size(); j++) {
                Vertex* t = v->edgeList[j]->to;
                int idx = indexOf(t->data);
                if (idx < 0) continue;

                Point p2 = getCoords(idx, N);
                fl_line(p1.x, p1.y, p2.x, p2.y);
            }
        }

        // ---------------- Highlight path (red) ----------------
        if (path.size() > 1) {
            fl_color(FL_RED);
            fl_line_style(FL_SOLID, 3);

            for (int i = 0; i < path.size() - 1; i++) {
                int a = indexOf(path[i]);
                int b = indexOf(path[i + 1]);
                if (a < 0 || b < 0) continue;

                Point p1 = getCoords(a, N);
                Point p2 = getCoords(b, N);
                fl_line(p1.x, p1.y, p2.x, p2.y);
            }
        }

        // ---------------- Draw nodes ----------------
        fl_font(FL_HELVETICA, 12);

        for (int i = 0; i < N; i++) {
            Vertex* v = graphRef->vertices[i];
            Point p = getCoords(i, N);

            // Is node in highlighted path?
            bool highlight = false;
            for (auto& s : path)
                if (s == v->data) highlight = true;

            fl_color(highlight ? FL_RED : FL_BLUE);
            fl_pie(p.x - 5, p.y - 5, 10, 10, 0, 360);

            fl_color(FL_BLACK);
            fl_draw(v->data.c_str(), p.x + 8, p.y + 8);
        }

        fl_line_style(0);
    }
};

// ------------------------------------------------------------
// APPLICATION CLASS
// ------------------------------------------------------------
class Application : public bobcat::Application_ {

    // UI
    bobcat::Window*   window;
    bobcat::Dropdown* start;
    bobcat::Dropdown* dest;
    bobcat::Dropdown* mode;
    bobcat::Button*   search;
    Fl_Scroll*        results;

    GraphDisplay*     map;   // Visualization

    // Data
    ArrayList<Vertex*> cities;
    Graph g;

    // Helpers
    void initData();
    void initInterface();
    void loadAirports(const std::string& file);
    void loadEdges(const std::string& file);

    void handleClick(bobcat::Widget* sender);

public:
    Application();
    ~Application();
};

#endif
