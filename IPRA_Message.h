//
// Created by lostrong on 10/31/19.
//



#ifndef RADARPUSHIMPL_IPRA_MESSAGE_H
#define RADARPUSHIMPL_IPRA_MESSAGE_H
#define INFO(...)  { std::cout << #__VA_ARGS__ << ":" << (__VA_ARGS__) << std::endl; }

class IPRA_message{
public:
    int source_node;
    int termination_node;
    int remain_walk_step;
    int pred;
    int coupon_id;
    int msg_size=0;
    IPRA_message(int source_node_v, int termination_node_v, int remain_walk_step_v, int pred_v, int coupon_id_v, int actual_size){
        source_node = source_node_v;
        termination_node = termination_node_v;
        remain_walk_step=remain_walk_step_v;
        pred=pred_v;
        coupon_id=coupon_id_v;
        msg_size=actual_size;
    }
    void set_size(int new_size){
        msg_size=new_size;
    }
    int get_size(){
        return msg_size;
    }
};

#endif //RADARPUSHIMPL_IPRA_MESSAGE_H
