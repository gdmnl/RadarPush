#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <vector>
#include <algorithm>
#include <assert.h>
#include "util.h"
using namespace std;
class Graph {
public:

    vector<vector<int>> g;
    vector<vector<int>> gr;
    string data_folder;

    //vector<double> global_ppr;

    // node rank[100] = 0, means node 100 has first rank
    vector<int> node_rank;
    // node_score[0]
    vector<double> node_score;

    //node order 0 = [100, 34.5], most important node is 100 with score 34.5
    vector<pair<int, double>> node_order;
    vector<int> loc;



    static bool cmp(const pair<int, double> &t1, const pair<int, double> &t2) {
        return t1.second > t2.second;
    }

    int n;
    long long m;

    Graph(string data_folder) {
        this->data_folder = data_folder;
        init_graph();
        init_nm();
        cout << "init graph n: " << this->n << " m: " << this->m << endl;
    }


    void init_nm() {
        string attribute_file = data_folder + "/attribute.txt";
        assert_file_exist("attribute file", attribute_file);
        ifstream attr(attribute_file);
        string line1, line2;
        char c;
        while (true) {
            attr >> c;
            if (c == '=') break;
        }
        attr >> n;
        while (true) {
            attr >> c;
            if (c == '=') break;
        }
        attr >> m;
    }

    void unique_vector(vector<int>& v){
        std::sort(v.begin(), v.end()); // 1 1 2 2 3 3 3 4 4 5 5 6 7
        auto last = std::unique(v.begin(), v.end());
        // v now holds {1 2 3 4 5 6 7 x x x x x x}, where 'x' is indeterminate
        v.erase(last, v.end());
    }
    void init_graph() {
        init_nm();
        g = vector<vector<int>>(n, vector<int>());
        gr = vector<vector<int>>(n, vector<int>());
        string graph_file = data_folder + "/graph.txt";
        if (!exists_test(graph_file)) {return;}
        assert_file_exist("graph file", graph_file);
        FILE *fin = fopen(graph_file.c_str(), "r");
        int t1, t2;
        while (fscanf(fin, "%d%d", &t1, &t2) != EOF) {
            assert(t1 < n);
            assert(t2 < n);
            if(t1 == t2) continue;
            g[t1].push_back(t2);
            g[t2].push_back(t1);
            gr[t2].push_back(t1);
            gr[t1].push_back(t2);
        }
        for(int i=0;i<n;i++){
            unique_vector(g[i]);
            unique_vector(gr[i]);

        }
    }


    double get_avg_degree() const {
        return double(m) / double(n);
    }


};





#endif
