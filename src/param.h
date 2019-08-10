//
// Created by Ashee on 2019/5/22.
//
#pragma once
#ifndef TLM_THESIS_PARAM_H
#define TLM_THESIS_PARAM_H


#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <random>
#include <unordered_set>
#include <math.h>
#include <map>

#include "Path.cpp"

static const int TIME_SLOT_A_DAY = 3;
<<<<<<< Updated upstream
static const int TIME_PERIOD = 28;  //days
=======
static const int TOTAL_WEEK = 4;
static const int TIME_PERIOD = TOTAL_WEEK * 7;  //days
>>>>>>> Stashed changes
static const unsigned int TOTAL_TIME_SLOT = TIME_PERIOD * TIME_SLOT_A_DAY;

static const int SHIP_STOP_DAY = 1;
static const int FIX_COST_OF_VIRTUAL_ARC = 100;



#endif //TLM_THESIS_PARAM_H
