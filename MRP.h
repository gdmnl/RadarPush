//
// Created by lostrong on 11/4/19.
//

#ifndef RADARPUSHIMPL_MRP_H
#define RADARPUSHIMPL_MRP_H

#include <chrono>
#include <random>
#include <unordered_map>
#include <algorithm>
#include "graph.h"
#define INFO(...)                                                       \
    {                                                                   \
        std::cout << #__VA_ARGS__ << ":" << (__VA_ARGS__) << std::endl; \
    }

class MRP
{
public:
    Graph *graph;
    double alpha;
    double epsilon;
    int total_walks;
    int expected_walks;
    bool is_rp = false;
    bool is_acc = false;
    std::chrono::steady_clock::time_point start;

    MRP(Graph &graph_v, double alpha_v, double epsilon_v)
    {
        graph = &graph_v;
        alpha = alpha_v;
        epsilon = epsilon_v;
    }
    inline void update_bandwidth(int from_node, int to_node, int num_rounds, int message_size,
                                 std::vector<std::vector<std::unordered_map<int, int>>> &bandwidth)
    {
        std::unordered_map<int, int>::const_iterator got = bandwidth[num_rounds][from_node].find(to_node);
        if (got == bandwidth[num_rounds][from_node].end())
        {
            bandwidth[num_rounds][from_node][to_node] = message_size;
        }
        else
        {
            bandwidth[num_rounds][from_node][to_node] = got->second + message_size;
        }
    }
    void run_single(double num_of_walks, int length, vector<double> &prs,
                    std::vector<std::vector<std::unordered_map<int, int>>> &bandwidth, vector<vector<vector<int>>> &graph_copies)
    {
        long num_w = ceil(num_of_walks);
        // INFO(num_of_walks);
        double rescale_down = 1.0 * num_w / num_of_walks;
        if (length == 0)
        {
            for (int i = 0; i < graph->n; i++)
            {
                prs[i] += num_of_walks;
            }
        }
        else
        {
            std::random_device rd;
            std::mt19937 g(rd());

            int ceil_length = pow(2, ceil(log2(length)));
            // ceil_length is the minimal power of two which is larger than length

            // if we are calculating accuracy, we use the original length
            if (is_acc)
                ceil_length = length;
            for (int z = 0; z < num_w; z++)
            {
                total_walks += ceil_length;
                // if(total_walks%(expected_walks/5000) == 0){
                double percent = total_walks * 1.0 / double(expected_walks);
                auto end = std::chrono::steady_clock::now();
                std::chrono::duration<double> elapsed_seconds = end - start;
                // INFO(elapsed_seconds.count()/60.0);
                double left_minutes = (elapsed_seconds.count() / 60.0) * (1 - percent) / percent;
                cout << "\r"
                     << "Current progress: " << percent * 100 << "%.\t"
                     << "Left time: " << left_minutes << endl;
                //}
                // cout<<z<< " "<<num_w<<endl;
                vector<vector<int>> valid_indexes;
                for (int j = 0; j < ceil_length; j++)
                {
                    vector<int> temp;
                    for (int i = 0; i < graph->n; i++)
                    {
                        temp.push_back(0);
                    }
                    valid_indexes.push_back(temp);
                }
                // valid indexes is a (ceil_length * n) matrix, fill with zero

                for (int i = 0; i < ceil_length; i++)
                {
                    for (int j = 0; j < graph->n; j++)
                    {
                        std::shuffle(graph_copies[i][j].begin(), graph_copies[i][j].end(), g);
                        // shuffle neighbour order on each graph copy
                    }
                }
                vector<int> temp_walk;
                temp_walk.reserve(ceil_length);
                for (int i = 0; i < graph->n; i += 1)
                {
                    int neighbor_size = graph->g[i].size();

                    temp_walk.clear();
                    for (int j = 0; j < neighbor_size; j++)
                    {

                        temp_walk.push_back(i);
                        int current_node = i;
                        for (int u = 0; u < ceil_length; u++)
                        {
                            // if(valid_indexes[u][current_node] >= graph_copies[u][current_node].size()){
                            //     cout<<current_node<<" "<<valid_indexes[u][current_node]<<" "<<graph_copies[u][current_node].size()<<endl;
                            //     cout<<"happens"<<endl;
                            // }
                            current_node = graph_copies[u][current_node][valid_indexes[u][current_node]++];
                            temp_walk.push_back(current_node);
                            // generate neighbor_size walks
                        }
                        prs[temp_walk[length]] += 1.0 / neighbor_size / rescale_down;
                        if (is_acc)
                            continue;
                        int r = 0;
                        if (is_rp)
                        {
                            for (int j = 0; j <= ceil_length; j += 1)
                            {
                                update_bandwidth(temp_walk[j], temp_walk[j + 1], length, 1, bandwidth);
                            }
                        }
                        else
                        {
                            for (int i = 1; i <= ceil_length; i = i * 2)
                            {
                                r++;
                                for (int j = 0; j <= ceil_length; j += i)
                                {
                                    update_bandwidth(temp_walk[j], temp_walk[j + i], i, r, bandwidth);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    void run_single_multi(double num_of_walks, int length, vector<double> &prs,
                          std::vector<std::vector<std::unordered_map<int, int>>> &bandwidth, vector<vector<vector<int>>> &graph_copies)
    {
        long num_w = ceil(num_of_walks);
        double rescale_down = 1.0 * num_w / num_of_walks;
        if (length == 0)
        {
            for (int i = 0; i < graph->n; i++)
            {
                prs[i] += num_of_walks;
            }
        }
        else
        {
            std::random_device rd;
            std::mt19937 g(rd());

            int ceil_length = pow(2, ceil(log2(length)));
            for (int z = 0; z < num_w; z++)
            {
                vector<vector<int>> valid_indexes;
                for (int j = 0; j < ceil_length; j++)
                {
                    vector<int> temp;
                    for (int i = 0; i < graph->n; i++)
                    {
                        temp.push_back(0);
                    }
                    valid_indexes.push_back(temp);
                }
                // valid indexes is a (ceil_length * n) matrix, fill with zero

                for (int i = 0; i < ceil_length; i++)
                {
                    for (int j = 0; j < graph->n; j++)
                    {
                        std::shuffle(graph_copies[i][j].begin(), graph_copies[i][j].end(), g);
                        // shuffle neighbour order on each graph copy
                    }
                }
                vector<int> temp_walk;
                temp_walk.reserve(ceil_length);
                for (int i = 0; i < graph->n; i += 1)
                {
                    int neighbor_size = graph->g[i].size();
                    //                    cout<<i<<endl;

                    temp_walk.clear();
                    for (int j = 0; j < neighbor_size; j++)
                    {

                        temp_walk.push_back(i);
                        int current_node = i;
                        for (int u = 0; u < ceil_length; u++)
                        {
                            current_node = graph_copies[u][current_node][valid_indexes[u][current_node]++];
                            temp_walk.push_back(current_node);
                            // generate neighbor_size walks
                        }
                        prs[temp_walk[length]] += 1.0 / neighbor_size / rescale_down;
                        if (is_acc)
                            continue;
                        int r = 0;
                        if (is_rp)
                        {
                            for (int j = 0; j <= ceil_length; j += 1)
                            {
                                update_bandwidth(temp_walk[j], temp_walk[j + 1], length, 1, bandwidth);
                            }
                        }
                        else
                        {
                            for (int i = 1; i <= ceil_length; i = i * 2)
                            {
                                r++;
                                for (int j = 0; j <= ceil_length; j += i)
                                {
                                    update_bandwidth(temp_walk[j], temp_walk[j + i], i, r, bandwidth);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    double log2(double v)
    {
        return log(v) / log(2);
    }
    void run()
    {
        int num_rounds;
        std::vector<std::vector<std::unordered_map<int, int>>> bandwidth;
        cout << "start running" << endl;
        double frac = alpha;
        int L = 0;
        double beta = 2.0;
        long K = floor(6 * log(2 * graph->n) / epsilon / epsilon / alpha) / beta;

        // long K = 256*floor(log(graph->n));

        INFO(K);
        while (true)
        {
            if (K * frac < 0.5)
            {
                break;
            }
            frac = frac * (1 - alpha);
            L++;
        }

        cout << L << " " << log2(L) << " " << ceil(log2(L)) << endl;
        // Calculate number of rounds
        L = log(graph->n) / log(1.0 / (1.0 - alpha));
        num_rounds = ceil(log2(L)) + 1;
        cout << "num_rounds: " << num_rounds << endl;
        num_rounds = pow(2, num_rounds); 

        // Init graph copies
        vector<vector<vector<int>>> graph_copies;
        for (int i = 0; i < num_rounds; i++)
        {
            INFO(i);
            vector<vector<int>> graph_i;
            for (int j = 0; j < graph->n; j++)
            {
                vector<int> gz;
                for (int z = 0; z < graph->g[j].size(); z++)
                {
                    gz.push_back(graph->g[j][z]);
                }
                graph_i.push_back(gz);
            }
            graph_copies.push_back(graph_i);
        }

        // Init bandwith
        for (int i = 0; i < num_rounds + 1; i++)
        {
            std::vector<std::unordered_map<int, int>> temp;
            for (int j = 0; j < graph->n; j++)
            {
                std::unordered_map<int, int> temp_map;
                temp.push_back(temp_map);
            }
            bandwidth.push_back(temp);
        }

        vector<double> pageranks;
        for (int i = 0; i < graph->n; i++)
            pageranks.push_back(0.0);
        frac = alpha;
        L = 0;
        //        while(true){
        total_walks = 0;
        expected_walks = 0;
        for (; L <= num_rounds; L++)
        {
            expected_walks += ceil(K * frac) * pow(2, ceil(log2(L)));
            frac = frac * (1 - alpha);
        }
        INFO(expected_walks);
        frac = alpha;
        L = 0;
        start = std::chrono::steady_clock::now();
        for (; L <= num_rounds; L++)
        {
            // L is the length of random walks. For random walks with the 
            // same length
            cout << L << endl;
            run_single(K * frac, L, pageranks, bandwidth, graph_copies);
            frac = frac * (1 - alpha);
        }

        ofstream out("outfile.txt");
        if (!out)
        {
            cout << "file cannot open" << endl;
        }
        for (int i = 0; i < graph->n; i++)
        {
            out << i << "\t" << pageranks[i] / K / graph->n << endl;
        }
        out.close();

        int max_bandwidth = 0;
        for (int i = 0; i < num_rounds + 1; i++)
        {
            cout << "i: " << i << endl;
            for (int j = 0; j < graph->n; j++)
            {
                for (std::unordered_map<int, int>::iterator iter = bandwidth[i][j].begin(); iter != bandwidth[i][j].end(); iter++)
                {
                    if (iter->second > max_bandwidth)
                        max_bandwidth = iter->second;
                }
            }
        }
        cout << "bandwidth: " << max_bandwidth << endl;
    }

    void run_multi()
    {
        int num_rounds;
        std::vector<std::vector<std::unordered_map<int, int>>> bandwidth;
        cout << "start running" << endl;
        double frac = alpha;
        int L = 0;
        double beta = 2.0;
        long K = floor(6 * log(2 * graph->n) / epsilon / epsilon / alpha) / beta;
        int NUM_CORES = 8;

        // long K = 256*floor(log(graph->n));

        INFO(K);
        while (true)
        {
            if (K * frac < 0.5)
            {
                break;
            }
            frac = frac * (1 - alpha);
            L++;
        }

        cout << L << " " << log2(L) << " " << ceil(log2(L)) << endl;

        L = log(graph->n) / log(1.0 / (1.0 - alpha));
        num_rounds = ceil(log2(L)) + 1;
        cout << "num_rounds: " << num_rounds << endl;
        num_rounds = pow(2, num_rounds); // why pow 2?

        vector<vector<vector<vector<int>>>> graph_copies_multi;
        for (int mul = 0; mul < NUM_CORES; mul++)
        {
            vector<vector<vector<int>>> graph_copies;
            for (int i = 0; i < num_rounds; i++)
            {
                INFO(i);
                vector<vector<int>> graph_i;
                for (int j = 0; j < graph->n; j++)
                {
                    vector<int> gz;
                    for (int z = 0; z < graph->g[j].size(); z++)
                    {
                        gz.push_back(graph->g[j][z]);
                    }
                    graph_i.push_back(gz);
                }
                graph_copies.push_back(graph_i);
            }
            graph_copies_multi.push_back(graph_copies);
        }
        for (int i = 0; i < num_rounds + 1; i++)
        {
            std::vector<std::unordered_map<int, int>> temp;
            for (int j = 0; j < graph->n; j++)
            {
                std::unordered_map<int, int> temp_map;
                temp.push_back(temp_map);
            }
            bandwidth.push_back(temp);
        }
        vector<double> pageranks;
        for (int i = 0; i < graph->n; i++)
            pageranks.push_back(0.0);
        frac = alpha;
        L = 0;
        //        while(true){
        total_walks = 0;
        expected_walks = 0;
        for (; L <= num_rounds; L++)
        {
            expected_walks += ceil(K * frac) * pow(2, ceil(log2(L)));
            frac = frac * (1 - alpha);
        }
        INFO(expected_walks);
        frac = alpha;
        L = 0;
        start = std::chrono::steady_clock::now();
        for (; L <= num_rounds; L++)
        {
            cout << L << endl;
            // run_single(K*frac, L, pageranks, bandwidth, graph_copies);
            frac = frac * (1 - alpha);
        }

        ofstream out("outfile.txt");
        if (!out)
        {
            cout << "file cannot open" << endl;
        }
        for (int i = 0; i < graph->n; i++)
        {
            out << i << "\t" << pageranks[i] / K / graph->n << endl;
        }
        out.close();

        int max_bandwidth = 0;
        for (int i = 0; i < num_rounds + 1; i++)
        {
            cout << "i: " << i << endl;
            for (int j = 0; j < graph->n; j++)
            {
                for (std::unordered_map<int, int>::iterator iter = bandwidth[i][j].begin(); iter != bandwidth[i][j].end(); iter++)
                {
                    if (iter->second > max_bandwidth)
                        max_bandwidth = iter->second;
                }
            }
        }
        cout << "bandwidth: " << max_bandwidth << endl;
    }

    void run_rp()
    {
        is_rp = true;
        int num_rounds;
        std::vector<std::vector<std::unordered_map<int, int>>> bandwidth;
        cout << "start running" << endl;
        double frac = alpha;
        int L = 0;
        long K = floor(6 * log(2 * graph->n) / epsilon / epsilon / alpha);

        // long K = 256*floor(log(graph->n));

        INFO(K);
        while (true)
        {
            if (K * frac < 0.5)
            {
                break;
            }
            frac = frac * (1 - alpha);
            L++;
        }

        cout << L << " " << log2(L) << " " << ceil(log2(L)) << endl;

        L = log(graph->n) / log(1.0 / (1.0 - alpha));
        num_rounds = ceil(log2(L)) + 1;
        cout << "num_rounds: " << num_rounds << endl;
        num_rounds = pow(2, num_rounds); // why pow 2?

        vector<vector<vector<int>>> graph_copies;
        for (int i = 0; i < num_rounds; i++)
        {
            INFO(i);
            vector<vector<int>> graph_i;
            for (int j = 0; j < graph->n; j++)
            {
                vector<int> gz;
                for (int z = 0; z < graph->g[j].size(); z++)
                {
                    gz.push_back(graph->g[j][z]);
                }
                graph_i.push_back(gz);
            }
            graph_copies.push_back(graph_i);
        }
        for (int i = 0; i < num_rounds + 1; i++)
        {
            std::vector<std::unordered_map<int, int>> temp;
            for (int j = 0; j < graph->n; j++)
            {
                std::unordered_map<int, int> temp_map;
                temp.push_back(temp_map);
            }
            bandwidth.push_back(temp);
        }
        vector<double> pageranks;
        for (int i = 0; i < graph->n; i++)
            pageranks.push_back(0.0);
        frac = alpha;
        L = 0;
        //        while(true){
        for (; L <= num_rounds; L++)
        {
            cout << L << endl;

            run_single(K * frac, L, pageranks, bandwidth, graph_copies);
            frac = frac * (1 - alpha);
        }
        ofstream out("outfile.txt");
        if (!out)
        {
            cout << "file cannot open" << endl;
        }
        for (int i = 0; i < graph->n; i++)
        {
            out << i << "\t" << pageranks[i] / K / graph->n << endl;
        }
        out.close();

        int max_bandwidth = 0;
        for (int i = 0; i < num_rounds + 1; i++)
        {
            cout << "i: " << i << endl;
            for (int j = 0; j < graph->n; j++)
            {
                for (std::unordered_map<int, int>::iterator iter = bandwidth[i][j].begin(); iter != bandwidth[i][j].end(); iter++)
                {
                    if (iter->second > max_bandwidth)
                        max_bandwidth = iter->second;
                }
            }
        }
        cout << "bandwidth: " << max_bandwidth << endl;
    }

    void run_acc()
    {
        is_acc = true;
        int num_rounds;
        std::vector<std::vector<std::unordered_map<int, int>>> bandwidth;
        cout << "start running" << endl;
        double frac = alpha;
        int L = 0;
        long K = floor(6 * log(2 * graph->n) / epsilon / epsilon / alpha);

        // long K = 256*floor(log(graph->n));

        INFO(K);
        while (true)
        {
            if (K * frac < 0.5)
            {
                break;
            }
            frac = frac * (1 - alpha);
            L++;
        }

        cout << L << " " << log2(L) << " " << ceil(log2(L)) << endl;

        L = log(graph->n) / log(1.0 / (1.0 - alpha));
        num_rounds = ceil(log2(L)) + 1;
        cout << "num_rounds: " << num_rounds << endl;
        num_rounds = pow(2, num_rounds); // why pow 2?

        vector<vector<vector<int>>> graph_copies;
        for (int i = 0; i < num_rounds; i++)
        {
            INFO(i);
            vector<vector<int>> graph_i;
            for (int j = 0; j < graph->n; j++)
            {
                vector<int> gz;
                for (int z = 0; z < graph->g[j].size(); z++)
                {
                    gz.push_back(graph->g[j][z]);
                }
                graph_i.push_back(gz);
            }
            graph_copies.push_back(graph_i);
        }
        for (int i = 0; i < num_rounds + 1; i++)
        {
            std::vector<std::unordered_map<int, int>> temp;
            for (int j = 0; j < graph->n; j++)
            {
                std::unordered_map<int, int> temp_map;
                temp.push_back(temp_map);
            }
        }
        vector<double> pageranks;
        for (int i = 0; i < graph->n; i++)
            pageranks.push_back(0.0);
        frac = alpha;
        L = 0;
        //        while(true){
        for (; L <= num_rounds; L++)
        {
            cout << L << endl;

            run_single(K * frac, L, pageranks, bandwidth, graph_copies);
            frac = frac * (1 - alpha);
        }
        ofstream out("outfile.txt");
        if (!out)
        {
            cout << "file cannot open" << endl;
        }
        for (int i = 0; i < graph->n; i++)
        {
            out << i << "\t" << pageranks[i] / K / graph->n << endl;
        }
        out.close();
    }
};
#endif // RADARPUSHIMPL_MRP_H
