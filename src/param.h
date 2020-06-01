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

/// Debug log output settings.
/// Comment out specific option to disable corresponding debug output.

/// Global debug mode enable.
#define DEBUG_MODE
#include "TLMLogger.hpp"

/// DW iteration debug mode.
#define DEBUG_DW_ITER

/// DW iteration debug output options.
#ifdef DEBUG_DW_ITER
    //#define DEBUG_DW_ITER_R_MATRIX
    #define DEBUG_DW_ITER_THETA_AND_SIGMA
    #define DEBUG_DW_ITER_LAMBDA
    #define DEBUG_DW_ITER_SOL_OBJS
    //#define DEBUG_DW_ITER_DUAL_VALUES
    #define DEBUG_DW_ITER_SOL
#endif

/// Column generation debug mode.
#define DEBUG_COLUMN_GENERATIONS

/// Column generation debug output options.
#ifdef DEBUG_COLUMN_GENERATIONS
    #define DEBUG_COLUMN_GENERATIONS_REDUCED_COST
    #define DEBUG_COLUMN_GENERATIONS_COLUMN_DELETIONS
    #define DEBUG_COLUMN_GENERATIONS_OBJ
#endif

/// Subproblems debug mode.
#define DEBUG_SUBPROBLEMS

/// Subproblem debug output options.
#ifdef DEBUG_SUBPROBLEMS
    #define DEBUG_SUBPROBLEMS_ROUTE_BUILD
    #define DEBUG_SUBPROBLEMS_ONE
    #define DEBUG_SUBPROBLEMS_TWO
    #define DEBUG_SUBPROBLEMS_PROFIT
#endif

extern bool is_designed_route_added;
extern bool iter_added;
extern bool not_col_deletion;

static const double  AIR_TRANS_COST_MULTIPLIER = 0.17;
static const double  SEA_TRANS_COST_MULTIPLIER = 13.4;
static const double  AIR_PROFIT_MULTIPLIER = 0.3;
static const double  SEA_PROFIT_MULTIPLIER = 0.3;
static const double  VIRTUAL_COST = 3.4;
static const int  AIR_ARC_COST_MULTIPLIER = 45;

static const int  SEA_ARC_COST_MULTIPLIER = 112;
static const int TIME_SLOT_A_DAY = 3;
static const int TOTAL_WEEK = 3;
static const int TIME_PERIOD = TOTAL_WEEK * 7;  //days
static const unsigned int TOTAL_TIME_SLOT = TIME_PERIOD * TIME_SLOT_A_DAY;

static const int SHIP_STOP_DAY = 1;
static const int FIX_COST_OF_VIRTUAL_ARC = 100;

// TODO: Test this.
static double MU_THRESHOLD = 0;
static const int time_limit_for_gurobi = 3600; //second
static const int NUM_INIT_PATHS = 99999;

// TODO: Test this.
static int MAX_BP_ITER = 20; // if the number is very big -> useless

// DW iterations.
static double DW_STOP_THRESHOLD = 0.1;
static int MAX_DW_ITER = 30; // if the number is very big -> useless

// TODO: Test this.
static double EPSILON = 0.006; //體積轉體積重量之參數(體積/ epsilon =體積重量)

// Column Generations.
static double COLUMN_GENERATION_THRESHOLE = 0.001;
static int COLUMN_GENERATION_MAX_CONTINUE_IN_THRESHOLD = 20;
// Maximum of times of column not being use before delete.
static int COLUMN_GENERATION_MAX_NOT_USE_IN_THRESHOLD = 20;

#endif //TLM_THESIS_PARAM_H
