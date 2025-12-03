#include <Application.h>


int main(){

    Application app;

    return app.run();

    // Vertex* sfo = new Vertex("San Francisco");
    // Vertex* nyc = new Vertex("New York");
    // Vertex* rio = new Vertex("Rio De Janeiro");
    // Vertex* paris = new Vertex("Paris");
    // Vertex* joburg = new Vertex("Johannesburg");
    // Vertex* moscow = new Vertex("Moscow");
    // Vertex* sydney = new Vertex("Sydney");
    // Vertex* tokyo = new Vertex("Tokyo");
    // Vertex* beijing = new Vertex("Beijing");
    // // Vertex* london = new Vertex("London");

    // Graph g;
    // g.addVertex(sfo);
    // g.addVertex(nyc);
    // g.addVertex(rio);
    // g.addVertex(paris);
    // g.addVertex(joburg);
    // g.addVertex(moscow);
    // g.addVertex(sydney);
    // g.addVertex(tokyo);
    // g.addVertex(beijing);

    // g.addEdge(sfo, nyc, 6);
    // g.addEdge(nyc, rio, 13);
    // g.addEdge(nyc, joburg, 14);
    // g.addEdge(nyc, paris, 7);
    // g.addEdge(nyc, moscow, 15);
    // // g.addEdge(nyc, london, 6);
    // // g.addEdge(london, moscow, 6);
    // g.addEdge(nyc, sydney, 40);
    // g.addEdge(rio, paris, 11);
    // g.addEdge(rio, beijing, 18);
    // g.addEdge(paris, sydney, 17);
    // g.addEdge(joburg, tokyo, 16);
    // g.addEdge(joburg, sydney, 11);
    // g.addEdge(sydney, tokyo, 10);
    // g.addEdge(tokyo, beijing, 3);
    // g.addEdge(beijing, moscow, 8);


    // Waypoint* path = g.ucs(sfo, beijing);

    // if (path){
    //     cout << "We found a path" << endl;
    //     Waypoint* temp = path;
    //     while (temp != nullptr){
    //         cout << temp->vertex->data << " " << temp->partialCost << endl;
    //         temp = temp->parent;
    //     }
    // }
    // else{
    //     cout << "There is no path" << endl;
    // }

    
    // return 0;
}