//
//  C_TF.cpp
//  TF_Cpp
//
//  Created by 王 颖豪 on 2019/6/17.
//  Copyright © 2019 王 颖豪. All rights reserved.
//

#include "C_TF.hpp"

void free_buffer(void* data, size_t length) { free(data); }

void deallocator(void* ptr, size_t len, void* arg) { free((void*)ptr); }

Model::Model() {
    status = TF_NewStatus();
}

Model::~Model() {
    if (sess) {
        TF_DeleteSession(sess, status);
    }
    
    if (graph) {
        TF_DeleteGraph(graph);
    }
    
    TF_DeleteStatus(status);
}

void Model::load_graph(const char *graph_path) {
    
    TF_Buffer* graph_def = read_file(graph_path);
    graph = TF_NewGraph();
    TF_ImportGraphDefOptions* opts = TF_NewImportGraphDefOptions();
    TF_GraphImportGraphDef(graph, graph_def, opts, status);
    
    if (TF_GetCode(status) != TF_OK) {
        fprintf(stderr, "ERROR: Unable to import graph %s\n", TF_Message(status));
        return;
    }
    
    TF_DeleteBuffer(graph_def);
    TF_DeleteImportGraphDefOptions(opts);
}

void Model::create_session() {
    
    if (graph == nullptr) {
        fprintf(stderr, "ERROR: graph is undefined.\n");
        return;
    }
    
    TF_SessionOptions* opt = TF_NewSessionOptions();
    sess = TF_NewSession(graph, opt, status);
    
    if (TF_GetCode(status) != TF_OK) {
        fprintf(stderr, "ERROR: Unable to create session %s\n", TF_Message(status));
        exit(255);
    }
    
    TF_DeleteSessionOptions(opt);
}

void Model::restore_weights(const char *ckpt_prefix) {
    
    if (graph == nullptr) {
        fprintf(stderr, "ERROR: graph is undefined.\n");
    }
    
    TF_Operation* checkpoint_op = TF_GraphOperationByName(graph, "save/Const");
    TF_Operation* restore_op = TF_GraphOperationByName(graph, "save/restore_all");
    
    size_t checkpoint_path_str_len = strlen(ckpt_prefix);
    size_t encoded_size = TF_StringEncodedSize(checkpoint_path_str_len);
    
    // The format for TF_STRING tensors is:
    //   start_offset: array[uint64]
    //   data:         byte[...]
    size_t total_size = sizeof(int64_t) + encoded_size;
    char* input_encoded = (char*)malloc(total_size);
    memset(input_encoded, 0, total_size);
    TF_StringEncode(ckpt_prefix, checkpoint_path_str_len,
                    input_encoded + sizeof(int64_t), encoded_size, status);
    if (TF_GetCode(status) != TF_OK) {
        fprintf(stderr, "ERROR: something wrong with encoding: %s",
                TF_Message(status));
        return;
    }
    
    TF_Tensor* path_tensor = TF_NewTensor(TF_STRING, NULL, 0, input_encoded,
                                          total_size, &deallocator, 0);
    
    TF_Output* run_path = (TF_Output*)malloc(1 * sizeof(TF_Output));
    run_path[0].oper = checkpoint_op;
    run_path[0].index = 0;
    
    TF_Tensor** run_path_tensors = (TF_Tensor**)malloc(1 * sizeof(TF_Tensor*));
    run_path_tensors[0] = path_tensor;
    
    TF_SessionRun(sess,
                  /* RunOptions */ NULL,
                  /* Input tensors */ run_path, run_path_tensors, 1,
                  /* Output tensors */ NULL, NULL, 0,
                  /* Target operations */ &restore_op, 1,
                  /* RunMetadata */ NULL,
                  /* Output status */ status);
    if (TF_GetCode(status) != TF_OK) {
        fprintf(stderr, "ERROR: Unable to run restore_op: %s\n",
                TF_Message(status));
        exit(255);
    }
}

void Model::create_input(const char* input_layer_name, std::vector<float> data, std::vector<int64_t> dims) {
    TF_Operation* input_op = TF_GraphOperationByName(graph, input_layer_name);
    if (input_op == nullptr) {
        fprintf(stderr, "Failed to retrieve input op.\n");
        exit(255);
    }
    inputs = {{input_op, 0}};
    
    TF_Tensor* input_val = TF_AllocateTensor(TF_FLOAT, &dims[0], (int)dims.size(), data.size() * sizeof(float));
    memcpy(TF_TensorData(input_val), &data[0], data.size() * sizeof(float));
    
    input_tensors.push_back(input_val);
}

void Model::create_output(const char *output_layer_name
                          , std::vector<float> data
                          , std::vector<int64_t> dims) {
    TF_Operation* output_op = TF_GraphOperationByName(graph, output_layer_name);
    if (output_op == nullptr) {
        fprintf(stderr, "Failed to retrieve input op.\n");
        return;
    }
    outputs = {{output_op, 0}};
    
    TF_Tensor* output_val = TF_AllocateTensor(TF_FLOAT, &dims[0], (int)dims.size(), data.size() * sizeof(float));
    memcpy(TF_TensorData(output_val), &data[0], data.size() * sizeof(float));
    
    output_tensors.push_back(output_val);
}

void Model::clear_input() {
    inputs.clear();
    input_tensors.clear();
}

int Model::predict() {
    
    if (inputs.size() != input_tensors.size()) {
        fprintf(stderr, "Mismatched input\n");
        return -1;
    }
    
    if (outputs.size() != output_tensors.size()) {
        fprintf(stderr, "Mismatched output\n");
        return -1;
    }
    
    //Run network
    TF_SessionRun(                          sess,
                  /* RunOptions */          NULL,
                  /* Input tensors */       &inputs[0], &input_tensors[0], (int)inputs.size(),
                  /* Output tensors */      &outputs[0], &output_tensors[0], (int)outputs.size(),
                  /* Target operations */   NULL, 0,
                  /* RunMetadata */         NULL,
                  /* Output status */       status);
    
    if (TF_GetCode(status) != TF_OK) {
        fprintf(stderr, "ERROR: Unable to run output_op: %s\n", TF_Message(status));
        return -1;
    }
    
    //Retreive Predictions
    float* output_data = (float*)TF_TensorData(output_tensors[0]);
    float m = output_data[0];
    int mi = 0;
    for (int i = 0; i < 10; i++) {
        if (output_data[i] > m) {
            mi = i;
            m = output_data[i];
        }
    }
    
    return mi;
}

TF_Buffer* Model::read_file(const char* file) {
    FILE* f = fopen(file, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  // same as rewind(f);
    
    void* data = malloc(fsize);
    fread(data, fsize, 1, f);
    fclose(f);
    
    TF_Buffer* buf = TF_NewBuffer();
    buf->data = data;
    buf->length = fsize;
    buf->data_deallocator = free_buffer;
    return buf;
}
