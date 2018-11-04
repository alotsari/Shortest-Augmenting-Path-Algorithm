#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <time.h>
#include <limits>
#include "boost/random/mersenne_twister.hpp"
#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/random.hpp"
#include "boost/graph/graphviz.hpp"
#include "boost/graph/make_connected.hpp"
#include "boost/graph/graph_utility.hpp"
#include "boost/graph/transpose_graph.hpp"
#include "boost/graph/visitors.hpp"
#include "boost/graph/breadth_first_search.hpp"
#include "boost/property_map/property_map.hpp"

using namespace std;
using namespace boost;

/* Struct for edge properties */
struct EdgeProperties {
    int cap;                //capacity
};

typedef adjacency_list<vecS, vecS, directedS, no_property, EdgeProperties> Graph;
typedef graph_traits<Graph>::vertex_descriptor Vertex;
typedef graph_traits<Graph>::edge_descriptor Edge;
typedef graph_traits<Graph>::vertex_iterator Vertex_iter;
typedef graph_traits<Graph>::edge_iterator Edge_iter;
typedef property_map<Graph, vertex_index_t>::type vertexIndexMap;
typedef vector<int> distVector;
typedef iterator_property_map<distVector::iterator, vertexIndexMap> distMap;
typedef vector<Vertex> predVector;
typedef iterator_property_map<predVector::iterator, vertexIndexMap> predMap;
typedef list<Edge> list_of_edges;
typedef vector<list_of_edges> aVector;
typedef iterator_property_map<aVector::iterator, vertexIndexMap> Map_t;

/*  Class for bfs visitors  */
class bfs_discovery_visitor : public bfs_visitor <> {
    public:
        bfs_discovery_visitor(distMap distance) : distance(distance) {}

        void tree_edge(Edge e, const Graph& g) {
            size_t new_distance = get(distance, source(e, g)) + 1;
            put(distance, target(e, g), new_distance);
        }

    private:
        distMap distance;
};

/*  Function declaration */
Vertex advance(Vertex i, Vertex j, Edge e, predMap& pred);
void augment(Graph& g, Vertex s, Vertex t, predMap& pred, int max_cap, Map_t& A_Map);
Vertex retreat(Vertex i, Vertex s, Map_t& A_Map, predMap& pred, distMap& dist, Graph& g, int n);

int flow_value = 0;

/*** Main ***/
int main(int argc, char **argv) {

    /* Initialization */
    int n;                  //Number of nodes
    int max_cap;
    int max_dist;
    int inEdges, outEdges;
    Vertex s, t, i, j, nil;
    Edge e;
    Edge_iter ei, ei_end;
    Graph g;

    nil = INT_MAX;

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
        file >> j;
        add_edge(i, j, g);
    }

    file.close();

    n = num_vertices(g);

    cout << endl;
    cout << "* SHORTEST AUGMENTING PATH ALGORITHM *" << endl;

    /* Give capacity range */
    cout << "  Give capacity range: 1 to ";
    cin >> max_cap;
    cout << endl;

    /*  Initialize the capacity of each edge  [1, max_cap]  */
    srand ( (unsigned)time(NULL));

    for(tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
        g[*ei].cap = rand() % ((max_cap - 1) + 1) + 1;
    }

    #ifdef DEBUG
    for(tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
        cout << source(*ei, g) << " -> " << target(*ei, g) << "  cap:" << g[*ei].cap << endl;
    }
    #endif

    /* Create distMap to handle dist */
    distVector mDistVector(n, 0);
    vertexIndexMap mVertexIndexMap = get(vertex_index, g);
    distMap dist = make_iterator_property_map(mDistVector.begin(), mVertexIndexMap);

    /*  Create predMap to handle pred  */
    predVector mPredVector(n, nil);
    vertexIndexMap m_VertexIndexMap = get(vertex_index, g);
    predMap pred = make_iterator_property_map(mPredVector.begin(), m_VertexIndexMap);

    /* Create A_Map to handle A(i) lists */
    aVector mAVector(n);
    vertexIndexMap myVertexIndexMap = get(vertex_index, g);
    Map_t A_Map = make_iterator_property_map(mAVector.begin(), myVertexIndexMap);

    /* s -> source node, t -> sink node */
    for(pair<Vertex_iter, Vertex_iter> vIter = vertices(g); vIter.first != vIter.second; ++vIter.first) {
        inEdges = 0;
        outEdges = out_degree(*vIter.first, g);

        for(tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
            if(*vIter.first == target(*ei, g)) {
                ++inEdges;
            }
        }

        if(inEdges == 0) {
            s = *vIter.first;
        }
        else if(outEdges == 0) {
            t = *vIter.first;
        }
    }

    /* Start timer */
    timespec mTime;
    mTime.tv_nsec = 0;
    clock_settime(CLOCK_PROCESS_CPUTIME_ID, &mTime);


    /* BFS in g -> Distance Map (start vertex s) */
    bfs_discovery_visitor vis(dist);
    breadth_first_search(g, s, visitor(vis));

    /* Obtain the exact distance labels dist[] (start vertex t) */
    max_dist = dist[t];
    for(pair<Vertex_iter, Vertex_iter> vIter = vertices(g); vIter.first != vIter.second; ++vIter.first) {
        dist[*vIter.first] = max_dist - dist[*vIter.first];
    }

    #ifdef DEBUG
        for(pair<Vertex_iter, Vertex_iter> vIter = vertices(g); vIter.first != vIter.second; ++vIter.first) {
            cout << "d(" << *vIter.first << ") = " << dist[*vIter.first] << endl;
        }
    #endif

    /* Initialize lists A(i) with all the arcs emanating from node i */
    for(pair<Vertex_iter, Vertex_iter> vIter = vertices(g); vIter.first != vIter.second; ++vIter.first) {
        for(tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
            if(source(*ei, g) == *vIter.first) {
                A_Map[*vIter.first].push_back(*ei);
            }
        }
    }

    /* Main body */
    i = s;

    while(dist[s] < n) {

        outEdges = 0;
        for(tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {

            if((source(*ei, g) == i) && (g[*ei].cap > 0)) {

                j = target(*ei, g);
                ++outEdges;

                if(dist[j] < dist[i]) {
                    break;
                }
            }
        }

        /* If i has an admissible arc */
        if((outEdges > 0) && (dist[j] < dist[i])) {
            i = advance(i, j, *ei, pred);

            if(i == t) {
                augment(g, s, t, pred, max_cap, A_Map);
                i = s;
            }
        }
        else {
            i = retreat(i, s, A_Map, pred, dist, g, n);
        }
    }

    /* Elapsed time */
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &mTime);
    cout << "Time spent: " << mTime.tv_nsec * 0.000000001 << " seconds"<< endl;

    #ifdef DEBUG
        /* Create file */
        ofstream mFile;
        mFile.open("file.txt");
        mFile << "digraph G{" << endl;

        for(tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
            mFile << source(*ei, g) << " -> " << target(*ei, g) << ";" << endl;
        }
        mFile << "}" << endl;
        mFile.close();
    #endif

    cout << "The maximum flow has value: " << flow_value << endl;

    return 0;
}


/*** advance() function ***/
Vertex advance(Vertex i, Vertex j, Edge e, predMap& pred) {

    #ifdef DEBUG
        cout << "advance(" << i << "," << j << ")"<<endl;
    #endif

    pred[j] = i;
    i = j;

    return i;
}


/*** augment() funtion ***/
void augment(Graph& g, Vertex s, Vertex t, predMap& pred, int max_cap, Map_t& A_Map) {

    #ifdef DEBUG   
        cout << "augment" << endl;
    #endif

    int delta;
    Vertex j;
    Edge e;
    Edge_iter ei, ei_end;

    list<Vertex> P;

    /* Using the pred indices identify an augmenting path P from s to t*/
    j = t;

    P.push_front(t);
    while(pred[j] != s) {
        P.push_front(pred[j]);
        j = pred[j];
    }
    P.push_front(s);

    #ifdef DEBUG
        for(list<Vertex>::iterator itr = P.begin(); itr != P.end(); ++itr) {
            cout << *itr << " ";
        }
        cout << endl;
    #endif

    /* δ : = min{r:(i, j)∈ P} */
    delta = max_cap;

    list<Vertex>::iterator itr = P.begin();

    for(tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {

        if((source(*ei,g) == *itr) && (target(*ei, g) == *next(itr, 1))) {
            if(g[*ei].cap < delta) {
                delta = g[*ei].cap;
            }
            ++itr;
        }

        if(itr == P.end()) {
            break;
        }

    }

    flow_value = flow_value + delta;

    #ifdef DEBUG
        cout << "delta = " << delta << endl;
    #endif

    /* Augment δ units of flow along path P */
    itr = P.begin();

    for(tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {

        if((source(*ei,g) == *itr) && (target(*ei, g) == *next(itr, 1))) {

            g[*ei].cap = g[*ei].cap - delta;

            /* Create the arc of the residual network if it doesn't exist */
            if(!edge(*(next(itr, 1)), *itr, g).second) {
                add_edge(*(next(itr, 1)), *itr, g);
                e = edge(*(next(itr, 1)), *itr, g).first;
                g[e].cap = delta;

                A_Map[*next(itr, 1)].push_back(e);
            }
            else {
                e = edge(*(next(itr, 1)), *itr, g).first;
                g[e].cap = g[e].cap + delta;
            }
            ++itr;
        }

        if(itr == P.end()) {
            break;
        }
    }
}


/*** retreat() function ***/
Vertex retreat(Vertex i, Vertex s, Map_t& A_Map, predMap& pred, distMap& dist, Graph& g, int n) {

    #ifdef DEBUG
        cout << "retreat" << endl;
    #endif

    int min;
    Vertex j;

    min = n;

    for(list<Edge>::iterator itr_a = A_Map[i].begin() ; itr_a != A_Map[i].end(); ++itr_a) {
       if((dist[target(*itr_a, g)] < min) && (g[*itr_a].cap > 0)) {
            min = dist[target(*itr_a, g)];
        }
    }

    dist[i] = min + 1;

    #ifdef DEBUG
        cout << "--New label--" << endl;
        cout << "d(" << i << ") = " << dist[i] << endl;
    #endif

    if(i != s) {
        i = pred[i];
    }

    return i;
}



