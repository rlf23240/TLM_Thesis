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

extern bool is_designed_route_added;
extern bool iter_added;
extern bool col_deletion;

static const double  AIR_TRANS_COST_MULTIPLIER = 0.17;
static const double  SEA_TRANS_COST_MULTIPLIER = 13.4;
static const double  AIR_PROFIT_MULTIPLIER = 0.3;
static const double  SEA_PROFIT_MULTIPLIER = 0.3;
static const double  VIRTUAL_COST = 3.4;
static const int  AIR_ARC_COST_MULTIPLIER = 45;

static const int  SEA_ARC_COST_MULTIPLIER = 112;
static const int TIME_SLOT_A_DAY = 3;
static const int TOTAL_WEEK = 15;
static const int TIME_PERIOD = TOTAL_WEEK * 7;  //days
static const unsigned int TOTAL_TIME_SLOT = TIME_PERIOD * TIME_SLOT_A_DAY;

static const int SHIP_STOP_DAY = 3;
static const int FIX_COST_OF_VIRTUAL_ARC = 100;

static double MU_THRESHOLD = 0;
static double DW_STOP_THRESHOLD = 0.1;
static const int time_limit_for_gurobi = 36000; //second
static const int NUM_INIT_PATHS = 99999;
static int MAX_BP_ITER = 0; // if the number is very big -> useless
static int MAX_DW_ITER = 30; // if the number is very big -> useless

#endif //TLM_THESIS_PARAM_H
