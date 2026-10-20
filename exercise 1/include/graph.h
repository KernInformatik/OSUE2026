/**
 * @file graph.h
 * @author Cihat Simsek (e12519647@student.tuwien.ac.at)
 * @brief
 * @version 0.1
 * @date 2026-10-20
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef GRAPH_H
#define GRAPH_H
#include "../include/common.h"
struct Node
{
    unsigned int node_number;
};

struct Edge
{
    struct Node u;
    struct Node v;
};

struct EdgeList
{
    struct Edge NumberOfEdges[1028];
};

#endif
