//
//  C_TF.hpp
//  TF_Cpp
//
//  Created by 王 颖豪 on 2019/6/17.
//  Copyright © 2019 王 颖豪. All rights reserved.
//

#ifndef C_TF_hpp
#define C_TF_hpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <tensorflow/c/c_api.h>

class Model {
    
public:
    
    Model();
    
    ~Model();
    
    TF_Session* sess;
    
    TF_Graph* graph;
    
    void load_graph(const char* graph_path);
    
    void create_session();
    
    void restore_weights(const char* ckpt_prefix);
    
    void create_input(const char* input_layer_name, std::vector<float> data, std::vector<int64_t> dims);
    
    void create_output(const char* output_layer_name, std::vector<float> data, std::vector<int64_t> dims);
    
    void clear_input();
    
    int predict();
    
private:
    TF_Status* status;
    
    TF_Buffer* read_file(const char* file);
    
    std::vector<TF_Output> inputs, outputs;
    
    std::vector<TF_Tensor*> input_tensors, output_tensors;
};

#endif /* C_TF_hpp */
