#include <iostream>
#include <fstream>
#include <limits>
#include "time.h"
#include "LEDA/graph/graph.h"
#include "LEDA/graph/edge_array.h"
#include "LEDA/graph/node_array.h"
#include "LEDA/graph/max_flow.h"

using namespace std;
using namespace leda;

int main(int argc, char **argv) {

    int i, j;
    int in;
    int max_cap;
    int flow_value;
    int inEdges, outEdges;
    bool is_maximum_flow;
    node s, t, u, v, w;
    edge e;
    graph G;
    node_array<int> Node(G);

    /* Check if network file is given */
    if(argc < 2) {
        cout << "You didn't give network file." << endl;
        exit(1);
    }

    /* Create graph -> .txt file */
    ifstream file;
    file.open (argv[1]);
    if (!file.is_open()) {
        cout << "File not found." << endl;
        exit(1);
    }

    while(file >> i) {

        in = 0;
        forall_edges(e, G) {
            if(G.source(e)->id() == i) {
                u = G.source(e);
                ++in;
            }
            else if(G.target(e)->id() == i) {
                u = G.target(e);
                ++in;
            }
        }

        if(in == 0) {
            u = G.new_node();
            Node.init(G);
            Node[u] = i;
        }

        file >> j;

        in = 0;
        forall_edges(e, G) {
            if(G.source(e)->id() == j) {
                v = G.source(e);
                ++in;
            }
            else if(G.target(e)->id() == j) {
                v = G.target(e);
                ++in;
            }
        }

        if(in == 0) {
            v = G.new_node();
            Node.init(G);
            Node[v] = j;
        }

        e = G.new_edge(u, v);
    }

    file.close();

    cout << endl;
    cout << "* MAX_FLOW() - LEDA *" << endl;

    /* Give capacity range */
    cout << "  Give capacity range: 1 to ";
    cin >> max_cap;
    cout << endl;

    /*  Initialize the capacity of each edge  [1, max_cap]  */
    srand ( (unsigned)time(NULL));

    edge_array<int> cap(G);
    forall_edges(e, G) {
        cap[e] = rand() % ((max_cap - 1) + 1) + 1;
    }

    forall_edges(e, G) {
        cout << "cap(" << G.source(e)->id() << "," << G.target(e)->id() << ") = " << cap[e] <<endl;
    }

    /* s -> source node, t -> sink node */
    forall_nodes(u, G) {

        inEdges = 0;
        forall_edges(e, G) {
            if(G.target(e) == u) {
                ++inEdges;
                break;
            }
        }

        if(inEdges == 0) {
            s = u;
        }

        outEdges = 0;
        forall_edges(e, G) {
            if(G.source(e) == u) {
                ++outEdges;
                break;
            }
        }

        if(outEdges == 0) {
            t = u;
        }
    }

    edge_array<int> flow(G);

    /* Start timer */
    timespec mTime;
    mTime.tv_nsec = 0;
    clock_settime(CLOCK_PROCESS_CPUTIME_ID, &mTime);

    /* MAX_FLOW() */
    flow_value = MAX_FLOW(G, s, t, cap, flow);

    /* Stop timer */
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &mTime);
    cout << "Time spent in MAX_FLOW(): " << mTime.tv_nsec * 0.000000001 << " seconds"<< endl;

    /* Check the result of the computation */
    is_maximum_flow = CHECK_MAX_FLOW(G,s,t,cap,flow);

    if (is_maximum_flow) {
        cout << "The maximum flow has value: " << flow_value << endl;
    }
    else {
        cout << "Error: MAX_FLOW() " <<  "did not compute a maximum flow!" << endl;
    }

    return 0;
}
