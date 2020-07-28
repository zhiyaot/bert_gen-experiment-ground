//
// Created by zhiyaot on 7/9/2020.
//
#include <cstdio>
#include <iostream>
#include <string>
#include <iomanip>
#include <list>
#include <map>
#include <vector>
#include "fpga_type.h"

#ifndef BERT_GEN_FPGA_HELPER_H
#define BERT_GEN_FPGA_HELPER_H

const int BRAM_SIZE = 36864;

extern int minFrame[6][3];

uint32_t calc_pos_7020(uint32_t frame_num, uint32_t offset);

uint64_t calc_bit_pos_ultra96(uint32_t x_pos, uint32_t y_pos, uint32_t bit_num, uint32_t type);

void gen_map();

void calc_nframe_range(std::list<unique_ptr<bram>>& all_logical, vector<vector<int>> &range_mark);

#endif //BERT_GEN_FPGA_HELPER_H
