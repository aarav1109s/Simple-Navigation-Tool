#ifndef GRAPH_H
#define GRAPH_H

#include "LinkedList.h"
#include <ArrayList.h>
#include <HashTable.h>
#include <Queue.h>
#include <Stack.h>
#include <string>
#include <ostream>

//
// ─── EDGE STRUCT ─────────────────────────────────────────────────────────
//
struct Vertex;

struct Edge {
    Vertex* from;
    Vertex* to;
    int price;
    int time;

    Edge(Vertex* f, Vertex* t, int p, int tm)
        : from(f), to(t), price(p), time(tm) {}
};

//
// ─── VERTEX STRUCT ─────────────────────────────────────────────────────
//
struct Vertex {
    std::string data;
    ArrayList<Edge*> edgeList;

    Vertex(std::string name) : data(name) {}

    ~Vertex() {
        for (int i = 0; i < edgeList.size(); i++)
            delete edgeList[i];
    }
};

//
// ─── WEIGHT MODE ───────────────────────────────────────────────────────
//
enum WeightMode { USE_PRICE, USE_TIME };

//
// ─── WAYPOINT STRUCT (NO DESTRUCTOR!) ──────────────────────────────────
//
struct Waypoint {
    Waypoint* parent;
    Vertex* vertex;
    ArrayList<Waypoint*> children;

    int partialCost;
    int edgeCost;
    WeightMode mode;

    Waypoint(Vertex* v, WeightMode m)
        : parent(nullptr),
          vertex(v),
          children(),
          partialCost(0),
          edgeCost(0),
          mode(m)
    {}

    void expand() {
        for (int i = 0; i < vertex->edgeList.size(); i++) {
            Edge* e = vertex->edgeList[i];

            Waypoint* child = new Waypoint(e->to, mode);
            child->parent = this;

            child->edgeCost =
                (mode == USE_PRICE ? e->price : e->time);

            child->partialCost =
                partialCost + child->edgeCost;

            children.append(child);
        }
    }
};

//
// ─── SAFE CLEANUP FUNCTION FOR WAYPOINT TREES ─────────────────────────
//
inline void deleteWaypointTree(Waypoint* wp) {
    if (!wp) return;

    for (int i = 0; i < wp->children.size(); i++)
        deleteWaypointTree(wp->children[i]);

    delete wp;
}

//
// ─── SEARCH RESULT WRAPPER ────────────────────────────────────────────
//
struct SearchResult {
    Waypoint* root;
    Waypoint* goal;
};

//
// ─── GRAPH CLASS ───────────────────────────────────────────────────────
//
struct Graph {
    ArrayList<Vertex*> vertices;

    ~Graph() {
        for (int i = 0; i < vertices.size(); i++)
            delete vertices[i];
    }

    void addVertex(Vertex* v) { vertices.append(v); }

    void addEdge(Vertex* a, Vertex* b, int price, int time) {
        a->edgeList.append(new Edge(a, b, price, time));
        b->edgeList.append(new Edge(b, a, price, time));
    }

    //
    // ─── BFS FOR FEWEST STOPS ──────────────────────────────────────────
    //
    SearchResult bfs(Vertex* start, Vertex* dest) {
        Waypoint* root = new Waypoint(start, USE_PRICE);

        Queue<Waypoint*> q;
        HashTable<std::string> seen;
        q.enqueue(root);
        seen.insert(start->data);

        while (!q.isEmpty()) {
            Waypoint* n = q.dequeue();
            if (n->vertex == dest)
                return { root, n };

            n->expand();
            for (int i = 0; i < n->children.size(); i++) {
                Waypoint* c = n->children[i];
                if (!seen.search(c->vertex->data)) {
                    q.enqueue(c);
                    seen.insert(c->vertex->data);
                }
            }
        }
        return { root, nullptr };
    }

    //
    // ─── UCS (DIJKSTRA) ────────────────────────────────────────────────
    //
    SearchResult ucs(Vertex* start, Vertex* dest, WeightMode mode) {
        Waypoint* root = new Waypoint(start, mode);

        ArrayList<Waypoint*> frontier;
        HashTable<std::string> visited;

        frontier.append(root);

        while (frontier.size() > 0) {

            // pop smallest cost
            Waypoint* node = frontier[0];
            frontier.removeFirst();

            if (node->vertex == dest)
                return { root, node };

            visited.insert(node->vertex->data);

            node->expand();

            for (int i = 0; i < node->children.size(); i++) {
                Waypoint* c = node->children[i];

                if (visited.search(c->vertex->data))
                    continue;

                frontier.append(c);
            }

            // sort frontier by cost ascending
            for (int i = 1; i < frontier.size(); i++) {
                int j = i;
                while (j > 0 &&
                       frontier[j]->partialCost < frontier[j - 1]->partialCost) {
                    Waypoint* temp = frontier[j];
                    frontier[j] = frontier[j - 1];
                    frontier[j - 1] = temp;
                    j--;
                }
            }
        }
        return { root, nullptr };
    }
};

#endif
