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
#include "common.h"
#include <sys/types.h>

#define MAX_SOLUTION_EDGES 8
struct Node
{
    u_int8_t node_number : 4;
};

struct Edge
{
    struct Node u;
    struct Node v;
};

struct EdgeList
{
    struct Edge NumberOfEdges[128];
    ssize_t length;
};

#endif
