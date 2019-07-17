//
// Created by Ashee on 2019/5/22.
//
#pragma once
#ifndef TLM_THESIS_PARAM_H
#define TLM_THESIS_PARAM_H
#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <random>
#include <unordered_set>
#include <math.h>
#include <queue>
#include <stack>
#include <time.h>
#include <numeric>
#include <iterator>


#include "Path.cpp"

static const int TIME_SLOT_A_DAY = 3;
static const int TIME_PERIOD = 28;  //days
static const unsigned int TOTAL_TIME_SLOT = TIME_PERIOD * TIME_SLOT_A_DAY;

static const int SHIP_STOP_DAY = 1;
static const int FIX_COST_OF_VIRTUAL_ARC = 100;

static const int NUM_INIT_PATHS = 1;
static const int MAX_BP_ITER = 20;

#endif //TLM_THESIS_PARAM_H
