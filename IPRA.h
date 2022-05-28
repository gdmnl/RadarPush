//
// Created by lostrong on 10/31/19.
//

#ifndef RADARPUSHIMPL_IPRA_H
#define RADARPUSHIMPL_IPRA_H

#include <unordered_map>
#include <vector>
#include <chrono>
#include <random>
#include "graph.h"
#include "IPRA_Message.h"

class IPRA {
    // K the number of walks starting from each node
    // n the number of nodes in the graph
public:
//    unsigned long long K;
    // map round --> map from node --> to node
    std::unordered_map<int, std::unordered_map<int, std::unordered_map<int, int> > > bandwidth;
    int num_rounds = 0;
    Graph *graph;
    double alpha;
    double epsilon;
    int lambda;
    unsigned long total_walks =0;
    std::vector<std::vector<int> > coupons;
    std::vector<int> pass_counts;
    std::vector<std::vector<IPRA_message> > received_message;
    std::vector<std::vector<IPRA_message> > local_storage;// for storing results in phase 1
    std::vector<std::vector<IPRA_message> > phase2_memory;// for memorizing short walks in phase 2

    IPRA(Graph &graph_v, double alpha_v, double epsilon_v) {
        graph = &graph_v;
        alpha = alpha_v;
        epsilon = epsilon_v;
        lambda = floor(sqrt(log(graph->n)));
        for (int i = 0; i < graph->n; i++) {
            std::vector<IPRA_message> temp;
            received_message.push_back(temp);
            std::vector<IPRA_message> temp2;
            local_storage.push_back(temp2);
            std::vector<IPRA_message> temp3;
            phase2_memory.push_back(temp3);

            pass_counts.push_back(0);
        }
        srand(time(NULL));
    }

    void send(int from_node, int to_node, IPRA_message &message, std::vector<std::vector<IPRA_message> > &storage) {
        storage[to_node].push_back(message);
        if (bandwidth.find(num_rounds) == bandwidth.end()) {
            std::unordered_map<int, std::unordered_map<int, int> > tmp;
            bandwidth[num_rounds] = tmp;
        }
        if (bandwidth[num_rounds].find(from_node) == bandwidth[num_rounds].end()) {
            std::unordered_map<int, int> tmp;
            bandwidth[num_rounds][from_node] = tmp;
        }
        if (bandwidth[num_rounds][from_node].find(to_node) == bandwidth[num_rounds][from_node].end()) {
            bandwidth[num_rounds][from_node][to_node] = 0;
        }
        bandwidth[num_rounds][from_node][to_node] =
                bandwidth[num_rounds][from_node][to_node] + message.get_size();
    }

    void phase_one() {
        std::cout << "starting phase 1..." << std::endl;
//        std::random_device rd;  //Will be used to obtain a seed for the random number engine
//        std::mt19937_64 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
//        std::uniform_int_distribution<double> uni_dist(0, 1);

        int remain_steps = lambda;
        std::cout << "lambda: " << lambda << std::endl;
        for (int z = 0; z < lambda; z++) {
            num_rounds++;
            std::cout << "round " << num_rounds << std::endl;
            if (z == 0) {
                int coupon_id = 0;
                for (int i = 0; i < graph->n; i++) {
                    std::cout<<"i: "<<i<<std::endl;
                    // each initializes d(u)log2(n)/alpha walks
                    int neighbor_cnt = ((graph->g)[i]).size();
                    int walks = floor(neighbor_cnt * log(graph->n) * log(graph->n) / alpha);
                    for (int j = 0; j < walks; j++) {
                        int send_node = graph->g[i][rand() % neighbor_cnt];
                        IPRA_message message(i, 0, remain_steps, i, coupon_id, 4);
                        std::vector<int> coupon_i;
                        coupon_i.push_back(i);
                        coupons.push_back(coupon_i);
                        coupon_id++;
                        send(i, send_node, message, received_message);

                    }

                }
            } else {
                for (int i = 0; i < graph->n; i++) {
                    for (IPRA_message message : received_message[i]) {
                        int neighbor_cnt = ((graph->g)[i]).size();
                        int send_node = graph->g[i][rand() % neighbor_cnt];
                        // we need to store source_node, remain_steps, coupon_id (based on paper), also we need to store
                        // pred locally for backtrack. We assume during Phase 1, each short walk will have footprints at each node
                        // it passes through
                        IPRA_message message_send(message.source_node, 0, remain_steps, i, message.coupon_id, 3);
                        coupons[message.coupon_id].push_back(i);
                        send(i, send_node, message_send, received_message);

                    }
                    received_message[i].clear();

                }
            }
        }

        // send back to source
        num_rounds++;
        for (int i = 0; i < graph->n; i++) {
            for (IPRA_message message : received_message[i]) {
                IPRA_message message_local(message.source_node, i, remain_steps, message.pred, message.coupon_id, 3);
                send(i, message.source_node, message_local, local_storage);
            }
            received_message[i].clear();
        }
        std::random_device rd;
        std::mt19937 g(rd());
        for (int i = 0; i < graph->n; i++) {
            std::shuffle(local_storage.begin(), local_storage.end(), g);
        }

        std::cout << "finishing phase 1..." << std::endl;
    }

    void phase_two() {
        std::cout << "starting phase 2..." << std::endl;

        std::default_random_engine generator;
        std::geometric_distribution<int> distribution(alpha);
        int K = floor(6 * log(2 * graph->n) / epsilon / epsilon / alpha);
        //init messages
        num_rounds++;
        std::cout<<"round: "<<num_rounds<<std::endl;
        for (int i = 0; i < graph->n; i++) {

            for (int j = 0; j < K; j++) {
                int walk_length = distribution(generator);
                total_walks+=walk_length;
                if (local_storage[i].size() > 0 && walk_length >= lambda) {
                    IPRA_message last_message = local_storage[i][local_storage[i].size()];
//                        last_message.set_size(3);
                    send(i, last_message.termination_node, last_message, phase2_memory);
                    local_storage[i].pop_back();
                    IPRA_message walk_message(i, last_message.termination_node, walk_length - lambda, 0, 0, 1);
                    send(i, last_message.termination_node, walk_message, received_message);
                } else if (walk_length > 0) {
                    int neighbor_cnt = (graph->g[i]).size();
                    int sendto_node = graph->g[i][rand() % neighbor_cnt];
                    IPRA_message walk_message(i, sendto_node, walk_length - 1, 0, 0, 2);
                    send(i, sendto_node, walk_message, received_message);
                    pass_counts[i]++;

                } else {
                    pass_counts[i]++;
                }

            }
            std::cout<<"i : "<<i<<std::endl;
        }
        int still_has_messages = 1;
        while (still_has_messages) {
            still_has_messages = 0;
            num_rounds++;
            std::cout<<"num_rounds: "<<num_rounds<<std::endl;
            for (int i = 0; i < graph->n; i++) {
                for (IPRA_message message: received_message[i]) {
                    if (local_storage[i].size() > 0 && message.remain_walk_step >= lambda) {
                        IPRA_message last_message = local_storage[i][local_storage[i].size()];
//                        last_message.set_size(3);
                        send(i, last_message.termination_node, last_message, phase2_memory);
                        local_storage[i].pop_back();
                        IPRA_message walk_message(i, last_message.termination_node, message.remain_walk_step - lambda,
                                                  0, 0, 1);
                        send(i, last_message.termination_node, walk_message, received_message);
                        still_has_messages = 1;
                    } else if (message.remain_walk_step > 0) {
                        int neighbor_cnt = ((graph->g)[i]).size();
                        int sendto_node = graph->g[i][rand() % neighbor_cnt];
                        IPRA_message walk_message(i, sendto_node, message.remain_walk_step - 1, 0, 0, 2);
                        send(i, sendto_node, walk_message, received_message);
                        still_has_messages = 1;
                        pass_counts[i]++;
                    } else {
                        pass_counts[i]++;
                    }

                }
                received_message[i].clear();
            }


        }

        std::cout << "finishing phase 2..." << std::endl;

    }

    void phase_three() {
        std::cout << "starting phase 3..." << std::endl;
        // the bandwidth of phase 3 will not exceed phase 1
        num_rounds += lambda;
        for (int i = 0; i < graph->n; i++) {
            for (IPRA_message message : phase2_memory[i]) {
                for (int node : coupons[message.coupon_id]) {
                    pass_counts[node]++;
                }
            }
        }

        std::cout << "finishing phase 3..." << std::endl;

    }

    void output(){
        ofstream out("/Users/lostrong/distributedpprtheory_correct/VLDBJ/code/test.out");
        if (!out){
            cout<<"file cannot open"<<endl;
        }
        out<<"round: "<<num_rounds<<endl;
        std::cout<<"round: "<<num_rounds<<std::endl;
        int max_bandwidth=0;
        for(std::unordered_map<int, std::unordered_map<int, std::unordered_map<int, int> > >::iterator iter =
                bandwidth.begin(); iter != bandwidth.end(); iter++)
        {

            std::unordered_map<int, std::unordered_map<int, int> > temp = iter->second;
            for(std::unordered_map<int, std::unordered_map<int, int> >::iterator iter2=temp.begin();iter2!=temp.end();iter2++){
                std::unordered_map<int, int> temp2 = iter2->second;
                for(std::unordered_map<int, int>::iterator iter3=temp2.begin();iter3!=temp2.end();iter3++){
                    if(iter3->second>max_bandwidth) max_bandwidth=iter3->second;
                }
            }

        }
        out<<"bandwidth: "<<max_bandwidth<<endl;
        std::cout<<"bandwidth: "<<max_bandwidth<<std::endl;
        for(int i=0;i<graph->n;i++){
            out<<i<<"\t"<<1.0*pass_counts[i]/total_walks<<endl;
        }
        out.close();


    }
    void run_IPRA() {
        phase_one();
        phase_two();
        phase_three();
        output();
    }

};

#endif //RADARPUSHIMPL_IPRA_H
