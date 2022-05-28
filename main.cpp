#include <iostream>
#include <string>
#include "graph.h"
#include "IPRA.h"
#include "IPRA_fast.h"
#include "GroundTruth.h"
#include "MRP.h"
#include "config.h"

void compare_rounds(double alpha, double epsilon)
{

    Graph graph(config.graph_folder);
    long lambda = floor(sqrt(log(graph.n)));
    std::default_random_engine generator;
    std::geometric_distribution<int> distribution(alpha);
    //        int K = floor(6 * log(2 * graph->n) / epsilon / epsilon / alpha);
    double prev_walks = 0;
    int max_length = 0;
    for (double c = 0.5; c >= 0.1; c -= 0.1)
    {
        vector<int> lengths;
        epsilon = c;
        int total_walks = floor(6 * log(2 * graph.n) / epsilon / epsilon / alpha) - prev_walks;
        prev_walks = total_walks;
        for (int i = 0; i < total_walks; i++)
        {
            int walk_length = distribution(generator);
            if (walk_length > max_length)
            {
                max_length = walk_length;
            }
        }
        //        cout << "max_length: " << max_length << endl;
        int max_round = lambda + floor(max_length / lambda) + max_length % lambda;
        cout << "IPRA round: " << alpha << " " << epsilon << " " << c << " " << max_round << endl;

        // MRP
        double L_ = log(graph.n) / log(1.0 / (1.0 - alpha));
        int num_rounds = ceil(log2(L_)) + 1;
        cout << "MRP round: " << alpha << " " << epsilon << " " << c << " " << num_rounds << endl;

        // RP

        cout << "RP round: " << alpha << " " << epsilon << " " << c << " " << ceil(L_) << endl;
    }
}

void calc_acc(int n)
{
    ifstream ground_in("groundtruth.txt");
    ifstream result_in("outfile.txt");
    double abs_err_sum = 0;
    double pagerank_sum = 0;
    for (int i = 0; i < n; i++)
    {
        int a;
        ground_in >> a;
        result_in >> a;
        double ground_pagerank;
        double result_pagerank;
        ground_in >> ground_pagerank;
        result_in >> result_pagerank;
        abs_err_sum += abs(ground_pagerank - result_pagerank);
        pagerank_sum += result_pagerank;
    }
    cout << "average absolute error:\t" << abs_err_sum / n << endl;
    cout << "pagerank sum:\t" << pagerank_sum << endl;
}

int main(int argc, char *argv[])
{
    for (int i = 0; i < argc; i++)
    {
        string arg = argv[i];
        if (arg == "--dataset")
        {
            config.graph_folder = string(argv[i + 1]);
        }
        else if (arg == "--action")
        {
            config.action = string(argv[i + 1]);
        }
        else if (arg == "--epsilon")
        {
            config.epsilon = atof(argv[i + 1]);
        }
    }
    INFO(config.epsilon);
    Graph graph(config.graph_folder);
    if (config.action == "compare_rounds")
    {
        compare_rounds(0.1, 0.5);
    }
    else if (config.action == "ipra")
    {
        IPRA_fast ipra(graph, 0.1, config.epsilon);
        ipra.run();
    }
    else if (config.action == "mrp")
    {
        MRP mrp(graph, 0.1, config.epsilon);
        mrp.run();
    }
    else if (config.action == "rp")
    {
        MRP mrp(graph, 0.1, config.epsilon);
        mrp.run_rp();
    }
    else if (config.action == "ground_truth")
    {
        groundtruth ground(graph, 0.1, 100);
        ground.power_iteration();
    }
    else if (config.action == "mrp_acc")
    {
        MRP mrp(graph, 0.1, config.epsilon);
        mrp.run_acc();
        calc_acc(graph.n);
    }
    else if (config.action == "ipra_acc")
    {
        IPRA_fast ipra(graph, 0.1, config.epsilon);
        ipra.run_acc();
        calc_acc(graph.n);
    }

    //    IPRA_fast ipra(graph, 0.1, 0.5);
    //    ipra.run();

    return 0;
}
