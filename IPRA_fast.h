//
// Created by lostrong on 11/2/19.
//

#ifndef RADARPUSHIMPL_IPRA_FAST_H
#define RADARPUSHIMPL_IPRA_FAST_H
#include <unordered_map>
#include <vector>
#include <chrono>
#include <random>
#include "graph.h"
class IPRA_fast
{
public:
    unsigned long total_walks = 0;
    int lambda;
    Graph *graph;
    double epsilon;
    double alpha;
    int K;
    IPRA_fast(Graph &graph_v, double alpha_v, double epsilon_v)
    {
        cout << "construct ipra_fast" << endl;
        graph = &graph_v;
        alpha = alpha_v;
        epsilon = epsilon_v;
        lambda = floor(sqrt(log(graph->n)));

        srand(time(NULL));
        K = 256 * floor(log(graph->n));
        cout << "finish construction" << endl;
    }
    void update_bandwidth(int from_node, int to_node, int num_rounds, int message_size,
                          std::vector<std::vector<std::unordered_map<int, int>>> &bandwidth)
    {
        std::unordered_map<int, int>::const_iterator got = bandwidth[num_rounds][from_node].find(to_node);
        if (got == bandwidth[num_rounds][from_node].end())
        {
            bandwidth[num_rounds][from_node][to_node] = message_size;
        }
        else
        {
            bandwidth[num_rounds][from_node][to_node] =
                got->second + message_size;
        }
    }

    inline void update_bandwidth_arr(int from_node, int to_node, int num_rounds, int message_size,
                                     std::vector<std::vector<std::vector<int>>> &bandwidth_arr)
    {
        bandwidth_arr[num_rounds][from_node][to_node] += message_size;
    }

    void run_phase_one(int &max_bandwidth, int &num_rounds)
    {
        int K = floor(6 * log(2 * graph->n) / epsilon / epsilon / alpha);
        double ratio = 1.0 * K / floor(log(graph->n));
        cout << "ratio: " << ratio << endl;
        num_rounds = lambda + 1;
        INFO(num_rounds);
        std::vector<std::vector<std::unordered_map<int, int>>> bandwidth;
        std::vector<std::vector<std::vector<int>>> bandwidth_arr;
        for (int i = 0; i < lambda + 1; i++)
        {
            INFO(i);
            //            std::vector<std::unordered_map<int, int> > temp;
            std::vector<std::vector<int>> temp_arr;
            for (int j = 0; j < graph->n; j++)
            {
                //                std::unordered_map<int, int> temp_map;
                //                temp.push_back(temp_map);
                std::vector<int> arr(graph->n, 0);
                ;
                temp_arr.push_back(arr);
            }
            //            bandwidth.push_back(temp);
            bandwidth_arr.push_back(temp_arr);
        }
        cout << "starting phase 1" << endl;
        double para = log(graph->n) * log(graph->n) / alpha * ratio;
        INFO(para);
        for (int i = 0; i < graph->n; i++)
        {
            INFO(i);
            int neighbor_cnt = ((graph->g)[i]).size();
            if (neighbor_cnt == 0)
                continue;
            int walks = ceil(neighbor_cnt * para);
            for (int j = 0; j <= walks; j++)
            {
                int current_node = i;
                for (int z = 0; z < lambda; z++)
                {
                    int neighbor_cnt = graph->g[current_node].size();
                    int next_node = graph->g[current_node][rand() % neighbor_cnt];
                    //                        update_bandwidth(current_node, next_node, z, 3, bandwidth);
                    update_bandwidth_arr(current_node, next_node, z, 3, bandwidth_arr);
                    current_node = next_node;
                }
                if (j == walks - 1)
                {
                    //                        update_bandwidth(i, current_node, lambda, 3, bandwidth);
                    update_bandwidth_arr(i, current_node, lambda, 3, bandwidth_arr);
                }
            }
        }
        cout << "starting counting max bandwidth" << endl;
        max_bandwidth = 0;
        //        for(int i =0;i<lambda;i++){
        //            for(int j=0;j<graph->n;j++){
        //                for(std::unordered_map<int, int>::iterator iter = bandwidth[i][j].begin(); iter!=bandwidth[i][j].end();iter++ ){
        //                    if(iter->second>max_bandwidth) max_bandwidth=iter->second;
        //                }
        //            }
        //        }
        for (int i = 0; i < lambda; i++)
        {
            for (int j = 0; j < graph->n; j++)
            {
                for (int z = 0; z < graph->n; z++)
                {
                    if (bandwidth_arr[i][j][z] > max_bandwidth)
                        max_bandwidth = bandwidth_arr[i][j][z];
                }
            }
        }
        cout << "max_bandwidth_phase_1: " << max_bandwidth << endl;
    }
    void run_phase_two(int &max_bandwidth, int &num_rounds)
    {
        std::default_random_engine generator;
        std::geometric_distribution<int> distribution(alpha);
        int K = floor(6 * log(2 * graph->n) / epsilon / epsilon / alpha);
        vector<int> lengths;
        int total_walks = K * graph->n;
        int max_length = 0;
        for (int i = 0; i < total_walks; i++)
        {
            int walk_length = distribution(generator);
            lengths.push_back(walk_length);
            if (walk_length > max_length)
            {
                max_length = walk_length;
            }
        }
        cout << "max_length: " << max_length << endl;
        int max_round_second_phase = floor(max_length / lambda) + max_length % lambda;
        num_rounds += floor(max_length / lambda) + max_length % lambda + lambda;
        cout << "num_rounds: " << num_rounds << endl;
        //        std::vector<std::vector<std::unordered_map<int, int> > > bandwidth;
        //        cout<<"start running"<<endl;
        //        for(int i =0;i<max_round_second_phase;i++){
        //            std::vector<std::unordered_map<int, int> > temp;
        //            for(int j=0;j<graph->n;j++){
        //                std::unordered_map<int, int> temp_map;
        //                temp.push_back(temp_map);
        //            }
        //            bandwidth.push_back(temp);
        //        }

        std::vector<std::vector<std::vector<int>>> bandwidth_arr(max_round_second_phase, std::vector<std::vector<int>>(graph->n, std::vector<int>(graph->n, 0)));
        cout << "start running" << endl;
        /*
        for(int i =0;i<max_round_second_phase;i++){
            INFO(i);
            std::vector<std::vector<int> > temp(graph->n,std::vector<int>(graph->n,0));

            for(int j=0;j<graph->n;j++){
                std::vector<int> temp_map(graph->n,0);
                temp.push_back(temp_map);
            }
            bandwidth_arr.push_back(temp);
        }*/

        cout << "starting phase 2" << endl;
        int ind = 0;
        double total_visit_nodes = total_walks / alpha;
        vector<long> pass_counts;
        for (int i = 0; i < graph->n; i++)
        {
            pass_counts.push_back(0);
        }
        for (int i = 0; i < graph->n; i++)
        {
            cout << i << endl;
            if (((graph->g)[i]).size() == 0)
                continue;
            for (int j = 0; j < K; j++)
            {
                int walk = lengths[ind++];
                //                cout<<"walk: "<<walk<<endl;
                int current_node = i;
                //                pass_counts[current_node]++;
                //                total_visit_nodes++;
                int current_big_node = i;
                for (int z = 0; z < walk; z++)
                {
                    int neighbor_cnt = ((graph->g)[current_node]).size();
                    int next_node = graph->g[current_node][rand() % neighbor_cnt];
                    current_node = next_node;
                    pass_counts[current_node]++;
                    if (z && z % lambda == 0)
                    {
                        //                        update_bandwidth(current_big_node, current_node, z/lambda-1, 3, bandwidth);
                        update_bandwidth_arr(current_big_node, current_node, z / lambda - 1, 3, bandwidth_arr);
                        current_big_node = current_node;
                    }
                }
            }
        }
        cout << "starting counting max bandwidth" << endl;
        int max_bandwidth_2 = 0;
        //        for(int i =0;i<max_round_second_phase;i++){
        //            for(int j=0;j<graph->n;j++){
        //                for(std::unordered_map<int, int>::iterator iter = bandwidth[i][j].begin(); iter!=bandwidth[i][j].end();iter++ ){
        //                    if(iter->second>max_bandwidth_2) max_bandwidth_2=iter->second;
        //                }
        //            }
        //        }
        for (int i = 0; i < max_round_second_phase; i++)
        {
            for (int j = 0; j < graph->n; j++)
            {
                for (int z = 0; z < graph->n; z++)
                {
                    if (bandwidth_arr[i][j][z] > max_bandwidth_2)
                        max_bandwidth_2 = bandwidth_arr[i][j][z];
                }
            }
        }
        if (max_bandwidth_2 > max_bandwidth)
            max_bandwidth = max_bandwidth_2;

        ofstream out("/Users/lostrong/distributedpprtheory_correct/VLDBJ/code/IPRA.out");
        if (!out)
        {
            cout << "file cannot open" << endl;
        }
        //        out<<"bandwidth: "<<max_bandwidth<<endl;
        std::cout << "bandwidth: " << max_bandwidth << std::endl;
        for (int i = 0; i < graph->n; i++)
        {
            out << i << "\t" << 1.0 * pass_counts[i] / total_visit_nodes << endl;
        }
        out.close();
    }
    void run()
    {
        int max_bandwidth;
        int num_rounds = 0;
        run_phase_one(max_bandwidth, num_rounds);
        run_phase_two(max_bandwidth, num_rounds);
        cout << "max_bandwidth: " << max_bandwidth << endl;
        cout << "num_rounds: " << num_rounds << endl;
    }
    void run_acc()
    {
        int num_of_walks = ceil(6 * log(2 * graph->n) / epsilon / epsilon / alpha);
        vector<double> pageranks(graph->n, 0);
        std::default_random_engine generator;
        std::geometric_distribution<int> distribution(alpha);
        for (int i = 0; i < graph->n; i++)
        {
            INFO(i);
            int neighbor_cnt = ((graph->g)[i]).size();
            if (neighbor_cnt == 0)
                continue;

            for (int j = 0; j < num_of_walks * neighbor_cnt; j++)
            {
                int current_node = i;
                int lambda = distribution(generator);
                for (int z = 0; z < lambda; z++)
                {
                    int neighbor_cnt = graph->g[current_node].size();
                    int next_node = graph->g[current_node][rand() % neighbor_cnt];
                    //                        update_bandwidth(current_node, next_node, z, 3, bandwidth);
                    current_node = next_node;
                }
                pageranks[current_node] += 1.0 / neighbor_cnt;
            }
        }

        ofstream out("outfile.txt");
        if (!out)
        {
            cout << "file cannot open" << endl;
        }
        for (int i = 0; i < graph->n; i++)
        {
            out << i << "\t" << pageranks[i] / num_of_walks / graph->n << endl;
        }
        out.close();
    }
};
#endif // RADARPUSHIMPL_IPRA_FAST_H
