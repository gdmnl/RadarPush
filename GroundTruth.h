//
// Created by lostrong on 11/4/19.
//

#ifndef RADARPUSHIMPL_GROUNDTRUTH_H
#define RADARPUSHIMPL_GROUNDTRUTH_H
#include <chrono>
#include <random>
#include "graph.h"

class groundtruth {
public:
    Graph* graph;
    double alpha;
    int K;
    groundtruth(Graph& graph_v, double alpha_v, int K_v){
        graph=&graph_v;
        alpha = alpha_v;
        K = K_v;
    }
    void compute() {
        std::default_random_engine generator;
        std::geometric_distribution<int> distribution(alpha);
        vector<int> lengths;
        int total_walks =  K*graph->n;
        for(int i=0;i<total_walks;i++){
            int walk_length = distribution(generator);
            lengths.push_back(walk_length);
        }
        int ind = 0;
        vector<int> end_counts;
        for(int i=0;i<graph->n;i++){
            end_counts.push_back(0);
        }
        for (int i = 0;i < graph->n;i++) {
            cout << i << endl;
            for (int j = 0; j < K; j++) {
                int walk = lengths[ind++];
                int current_node = i;
                for (int z = 0; z < walk; z++) {
                    int neighbor_cnt = ((graph->g)[current_node]).size();
                    int next_node = graph->g[current_node][rand() % neighbor_cnt];
                    current_node = next_node;
                    if(z==walk-1){
                        end_counts[current_node]++;
                    }
                }
            }
        }

        ofstream out("groundtruth.txt");
        if (!out){
            cout<<"file cannot open"<<endl;
        }
        for(int i=0;i<graph->n;i++){
            out<<i<<"\t"<<1.0*end_counts[i]/total_walks<<endl;
        }
        out.close();
    }

    void power_iteration(){
        vector<double>residue(graph->n, 1.0/graph->n);
        vector<double>reserve(graph->n, 0);
        for(int round = 0; round < 1000; round++){
            INFO(round);
           for(int u = 0; u < graph->n; u++){
               double increment = (1 - alpha) * residue[u] / graph->g[u].size();
               reserve[u] += alpha * residue[u];
               residue[u] = 0;
               for(auto v : graph->g[u]){
                   residue[v] += increment;
               }
           }
        }

        ofstream out("groundtruth.txt");
        if (!out){
            cout<<"file cannot open"<<endl;
        }
        for(int i=0;i<graph->n;i++){
            out<<i<<"\t"<<reserve[i]<<endl;
        }
        out.close();
    }
};

#endif //RADARPUSHIMPL_GROUNDTRUTH_H
