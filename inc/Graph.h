#ifndef GRAPH_H
#define GRAPH_H

#include "LinkedList.h"
#include <ArrayList.h>
#include <HashTable.h>
#include <Queue.h>
#include <Stack.h>
#include <cstddef>
#include <ostream>
#include <string>

//
// ─── EDGE STRUCT WITH PRICE + TIME ─────────────────────────────────────
//
// Forward declaration of Vertex 
struct Vertex;

struct Edge {
    Vertex *from;
    Vertex *to;
    int price;
    int time;

    Edge(Vertex *from, Vertex *to, int price, int time)
        : from(from), to(to), price(price), time(time) {}
};


struct Vertex {
    std::string data;
    ArrayList<Edge *> edgeList;

    Vertex(std::string data) { this->data = data; }
    ~Vertex(){
        for(int i = 0; i < edgeList.size(); i++){
            delete edgeList[i];
        }
    }
};

inline std::ostream &operator<<(std::ostream &os, Vertex *v) {
    os << v->data;
    return os;
}

inline std::ostream &operator<<(std::ostream &os, Edge *e) {
    os << "(" << e->from << " -> " << e->to
       << ", price=" << e->price
       << ", time=" << e->time << ")";
    return os;
}


//
// ─── WAYPOINT WITH MODE (PRICE OR TIME) ───────────────────────────────
//

enum WeightMode { USE_PRICE, USE_TIME };

struct Waypoint {
    Waypoint *parent;
    Vertex *vertex;
    ArrayList<Waypoint *> children;

    int partialCost;    // sum of price or time
    int edgeCost;       // cost of the single step
    WeightMode mode;

    Waypoint(Vertex *v, WeightMode mode = USE_PRICE) {
        parent = nullptr;
        vertex = v;
        this->mode = mode;
        partialCost = 0;
        edgeCost = 0;
    }

    ~Waypoint() {
        for (int i = 0; i < children.size(); i++) {
            delete children[i];
        }
    }

    void expand() {
        for (int i = 0; i < vertex->edgeList.size(); i++) {
            Edge *e = vertex->edgeList[i];
            Waypoint *temp = new Waypoint(e->to, mode);

            temp->parent = this;

            // Choose the correct weight (price or time)
            temp->edgeCost = (mode == USE_PRICE ? e->price : e->time);
            temp->partialCost = partialCost + temp->edgeCost;

            children.append(temp);
        }
    }
};

inline std::ostream &operator<<(std::ostream &os, Waypoint *wp) {
    std::string p = "null";
    if (wp->parent != nullptr)
        p = wp->parent->vertex->data;

    os << p << " -> " << wp->vertex->data;
    return os;
}


//
// ─── GRAPH CLASS ──────────────────────────────────────────────────────
//

struct Graph {
    ArrayList<Vertex *> vertices;

    // Adding graph destructor for memory clean up
    ~Graph(){
        for(int i = 0; i < vertices.size(); i++){
            delete vertices[i];
        }
    }


    void addVertex(Vertex *v) { vertices.append(v); }

    void addEdge(Vertex *x, Vertex *y, int price, int time) {
        x->edgeList.append(new Edge(x, y, price, time));
        y->edgeList.append(new Edge(y, x, price, time));
    }

    void addDirectedEdge(Vertex *x, Vertex *y, int price, int time) {
        x->edgeList.append(new Edge(x, y, price, time));
    }


    //
    // BFS — used for fewest stops
    //
    Waypoint *bfs(Vertex *start, Vertex *dest) {
        Queue<Waypoint *> frontier;
        HashTable<std::string> seen;

        Waypoint *first = new Waypoint(start, USE_PRICE);
        frontier.enqueue(first);
        seen.insert(first->vertex->data);

        while (!frontier.isEmpty()) {
            Waypoint *node = frontier.dequeue();
            if (node->vertex == dest)
                return node;

            node->expand();
            for (int i = 0; i < node->children.size(); i++) {
                Vertex *v = node->children[i]->vertex;
                if (!seen.search(v->data)) {
                    frontier.enqueue(node->children[i]);
                    seen.insert(v->data);
                }
            }
        }

        return nullptr;
    }


    //
    // UCS — generic Dijkstra-like behavior depending on mode
    //
    Waypoint* ucs(Vertex* start, Vertex* dest, WeightMode mode) {
    ArrayList<Waypoint*> frontier;
    HashTable<std::string> visited;          // permanently visited
    HashTable<std::string> inFrontier;       // tracks nodes currently in frontier

    Waypoint* first = new Waypoint(start, mode);
    frontier.append(first);
    inFrontier.insert(start->data);

    while (frontier.size() > 0) {

        // REMOVE CHEAPEST ELEMENT FROM FRONT
        Waypoint* node = frontier[0];
        frontier.removeFirst();

        inFrontier = HashTable<std::string>(); // reset frontier tracker
        for (int i = 0; i < frontier.size(); i++)
            inFrontier.insert(frontier[i]->vertex->data);

        // If we've reached the destination, return the node
        if (node->vertex == dest)
            return node;

        // Mark node as permanently visited
        visited.insert(node->vertex->data);

        // Expand children
        node->expand();

        for (int i = 0; i < node->children.size(); i++) {
            Waypoint* child = node->children[i];
            Vertex* v = child->vertex;

            // Skip permanently visited nodes
            if (visited.search(v->data))
                continue;

            bool found = false;

            // If the child is already in frontier with worse cost, replace it
            for (int j = 0; j < frontier.size(); j++) {
                if (frontier[j]->vertex->data == v->data) {
                    found = true;

                    if (child->partialCost < frontier[j]->partialCost) {
                        delete frontier[j];
                        frontier[j] = child;
                    } else {
                        delete child;  // discard worse path
                    }
                    break;
                }
            }

            // If not found anywhere, add child
            if (!found) {
                frontier.append(child);
            }
        }

        // Now sort frontier by partialCost ASCENDING
        for (int a = 1; a < frontier.size(); a++) {
            int b = a;
            while (b > 0 && frontier[b]->partialCost < frontier[b - 1]->partialCost) {
                Waypoint* tmp = frontier[b];
                frontier[b] = frontier[b - 1];
                frontier[b - 1] = tmp;
                b--;
            }
        }
    }

    return nullptr;
}
};

inline std::ostream &operator<<(std::ostream &os, const Graph &g) {
    for (int i = 0; i < g.vertices.size(); i++) {
        os << g.vertices[i]->edgeList << std::endl;
    }
    return os;
}

//
// ─── OPTIONAL SELF-TEST SECTION ───────────────────────────────────────
//   Compile with:
//       g++ test.cpp -o test
//   where test.cpp contains:
//       #define GRAPH_TEST
//       #include "graph.h"
//
#ifdef GRAPH_TEST
#include <iostream>
int main() {
    Graph g;

    Vertex *A = new Vertex("A");
    Vertex *B = new Vertex("B");
    Vertex *C = new Vertex("C");

    g.addVertex(A);
    g.addVertex(B);
    g.addVertex(C);

    g.addEdge(A, B, 100, 5);
    g.addEdge(B, C, 50, 2);
    g.addEdge(A, C, 500, 1);

    std::cout << "Testing cheapest price route (A → C):\n";
    Waypoint *cheap = g.ucs(A, C, USE_PRICE);

    Waypoint *t = cheap;
    while (t) {
        std::cout << t->vertex->data
                  << " (cost=" << t->partialCost << ")\n";
        t = t->parent;
    }

    std::cout << "\nTesting fastest time route (A → C):\n";
    Waypoint *fast = g.ucs(A, C, USE_TIME);

    t = fast;
    while (t) {
        std::cout << t->vertex->data
                  << " (time=" << t->partialCost << ")\n";
        t = t->parent;
    }

    return 0;
}
#endif

#endif
