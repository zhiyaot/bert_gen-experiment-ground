//
// Created by zhiyaot on 7/9/2020.
//
#include <cstdio>
#include <iostream>
#include <string>
#include <iomanip>
#include <map>
#include "../include/fpga_helper.h"
#include "../include/fpga_type.h"


int minFrame[6][3] = {{0x01000000, 0x01040000, 0x01080000},
                             {0x01000100, 0x01040100, 0x01080100},
                             {0x01000200, 0x01040200, 0x01080200},
                             {0x01000300, 0x01040300, 0x01080300},
                             {0x01000400, 0x01040400, 0x01080400},
                             {0x01000500, 0x01040500, 0x01080500}};

uint32_t calc_pos_7020(uint32_t frame_num, uint32_t offset)
{
    return frame_num * 3232 + offset;
}

uint64_t calc_bit_pos_ultra96(uint32_t x_pos, uint32_t y_pos, uint32_t bit_num, uint32_t type)
{
    if (type == 36)
    {
        return (x_pos * 36 + y_pos) * BRAM_SIZE + bit_num;
    }
    else
    {
        return (x_pos * 72 + y_pos) * BRAM_SIZE / 2 + bit_num;
    }
}

void gen_map()
{
    FILE *curr;

    char buffer[100] = {'\0'};

    curr = fopen("./test/1kb72/match.info", "r");
    uint32_t buff{0}, y{0}, fasmLine{0}, fasmBit{0}, bit_num{0};

    map<uint32_t, unique_ptr<fasm_pos>> mapping;

    while (fscanf(curr, "%[^\n]\n", buffer) != EOF)
    {
        if (sscanf(buffer, "init.mem[%d][%d] -> INITP Y%d [%x][%d] -> BIT:%d",
                   &buff, &buff, &y, &fasmLine, &fasmBit, &bit_num) != 6)
        {
            continue;
        }
        else
        {
            mapping.insert({bit_num, make_unique<fasm_pos>(fasmLine, fasmBit, y)});
        }
    }

    fclose(curr);
    curr = fopen("./data/maps/36p.map", "w");

    for (int i = 0; i < mapping.size(); i++)
    {
        auto bit = mapping[i].get();
        fprintf(curr, "PBIT%d -> [%02x][%03d], Y = %d\n", i, bit->fasm_line, bit->fasm_bit, bit->y);
    }

    fclose(curr);
}

void calc_nframe_range(list<unique_ptr<bram>> &all_logical, vector<vector<int>> &range_mark)
{
    for (auto &bram : all_logical)
    {
        if (bram->ramType == 36)
        {
            range_mark[bram->ram_pos_x][bram->ram_pos_y / 12] = 1;
        }
        else
        {
            range_mark[bram->ram_pos_x][bram->ram_pos_y / 24] = 1;
        }
    }
}

