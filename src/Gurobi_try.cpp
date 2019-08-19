/* Copyright 2019, King Wong */

/* 
Version 1.0
Layer:
0: Design Sea
1: Design Air
2: Virtual
3: TA Sea
4: TA Air
5: Rival Sea
6: Rival Air
*/

#include "gurobi_c++.h"
#include <iostream>
#include <math.h>
#include "sstream"
#include <stdlib.h>
#include "time.h"
#include "fstream"
#include <vector>
#include <iomanip>
#include <cstring>

using namespace std;

//Set Parameter
#define L 63		//Planning Horizon
#define T 3			//Time Period
#define M 4			//# of Port
#define N 4			//# of Airport
#define K 5			//# of Cargo
#define S 1			//# of Designed Sea Route
#define A 1			//# of Designed Air Route
#define Am 1		//# of Designed Air Route(>2)
#define Sn 2		//# of Current Sea Route
#define An 2		//# of Current Air Route
#define Sc 1		//# of Current Sea Route Competitor
#define Ac 1		//# of Current Air Route Competitor
#define e 0.006
#define BigM 10000

struct flight
{
	int startnode;
	int t;
	int freq;
	int gap;
	int vol;
	int wei;
	int FT;
};

struct ship
{
	int startnode;
	int starttime;
	int freq;
	int t;
	int vol;
};

struct cargo
{
	int O;
	int D;
	int starttime;
	int endtime;
	int wei;
	int vol;
	double alpha;
	double belta;
	double weight;
	char TS;
	char CV;
};

struct sea_route
{
	vector<int> port;
	vector<int> time;
	int vol;
};

struct air_route
{
	vector<int> port;
	vector<int> time;
	int vol;
	int wei;
	int gap;
	int t;
};

int
main(int   argc,
	char *argv[])
{
	//Define Variable
	vector<vector<int>> V_S(S, vector<int>(0));
	vector<vector<vector<int>>> V_Si(S, vector<vector<int>>(M, vector<int>(0)));
	vector<vector<int>> V_A(A, vector<int>(0));
	vector<vector<int>> V_N(1, vector<int>(0));
	vector<vector<int>> V_Sn(Sn, vector<int>(0));
	vector<vector<int>> V_An(An, vector<int>(0));
	vector<vector<vector<int>>> E_S(S, vector<vector<int>>(0, vector<int>(1)));
	vector<vector<vector<int>>> E_A(A, vector<vector<int>>(0, vector<int>(1)));
	vector<vector<vector<int>>> E_N(1, vector<vector<int>>(0, vector<int>(2)));
	vector<vector<vector<int>>> E_Sn(Sn, vector<vector<int>>(0, vector<int>(2)));
	vector<vector<vector<int>>> E_An(An, vector<vector<int>>(0, vector<int>(2)));
	vector<vector<int>> H_S(S, vector<int>(0));
	vector<vector<int>> B_A(A, vector<int>(0));
	vector<vector<int>> R_A(Am, vector<int>(0));
	vector<vector<vector<vector<int>>>> E_Am(Am, vector<vector<vector<int>>>(0, vector<vector<int>>(0,vector<int>(2))));
	vector<vector<int>> P_CurrentTA(K, vector<int>(0));
	vector<vector<int>> P_Current(K, vector<int>(0));
	vector<vector<int>> P_CurrentC(K, vector<int>(0));
	vector<vector<int>> P_All(K, vector<int>(0));
	vector<vector<int>> P_Design(K, vector<int>(0));
	vector<int> P_AllCargo(0);
	vector<vector<int>> A_P(0, vector<int>(0));
	int c_A[N][N];															//Travel_Cost_Air
	flight info_A[A];														//Flight_Info
	double r_A[N][N];														//Profit_Air
	int F_A[N];																//Stop_Cost_Air
	int time_A[N][N];														//Time_Cost_Air
	double trans_A[N][N];													//Trans_Cost_Air
	cargo cargo[K];															//Cargo_Info
	int c_N[N];																//Virtual_Cost
	int c_L[N];																//Load_Cost
	int c_S[N][N];															//Travel_Cost_Sea
	ship info_S[S];															//Ship_Info
	double r_S[M][M];														//Profit_Sea
	int F_S[M];																//Stop_Cost_Sea
	int time_S[M][M];														//Time_Cost_Sea
	double trans_S[M][M];													//Trans_Cost_Sea
	

	time_t t1 = time(NULL);
	//Input Parameter
	ifstream fin("../Data/A1_air_arccost.txt");
	if (!fin) { cout << "open fail" << endl; }
	else {
		string str_buf;
		for (int i = 0;i < N + 1;i++) {
			getline(fin, str_buf);
			istringstream stream(str_buf);
			getline(stream, str_buf, '\t');
			if (i > 0)
				for (int j = 0;j < N;j++)
				{
					if(str_buf != "M")
						c_A[i - 1][j] = atoi(str_buf.c_str());
					else c_A[i - 1][j] = BigM;
					getline(stream, str_buf, '\t');
				}
		}
	}
	fin.close();
	fin.clear();
	fin.open("../Data/A1_air_flights_param.txt");
	if (!fin) { cout << "open fail" << endl; }
	else {
		string str_buf;
		for (int a = 0;a < A + 1;a++) {
			getline(fin, str_buf);
			istringstream stream(str_buf);
			getline(stream, str_buf, '\t');
			if (a > 0)
				for (int i = 0;i < 6;i++)
				{
					if (i == 0)
					{
						string tmp = str_buf.c_str();
						char tmp1[2];
						strcpy(tmp1, tmp.c_str());
						info_A[a - 1].startnode = (int) tmp1[0]-65;
					}
					if (i == 1)
						info_A[a - 1].t = atoi(str_buf.c_str());
					if (i == 2)
						info_A[a - 1].freq = atoi(str_buf.c_str());
					if (i == 3)
						info_A[a - 1].gap = atoi(str_buf.c_str());
					if (i == 4)
						info_A[a - 1].vol = atoi(str_buf.c_str());
					if (i == 5)
						info_A[a - 1].wei = atoi(str_buf.c_str());
					getline(stream, str_buf, '\t');
				}			
		}
	}
	for (int a = 0;a < A;a++)
		info_A[a].FT = (info_A[a].freq - 1)*info_A[a].gap + info_A[a].t;
	for (int a = 0;a < A;a++)
		for (int j = 0;j < L*M - info_A[a].t*N;j = j + 4)
			B_A[a].push_back(info_A[a].startnode + j);
	for (int a = 0;a < Am;a++)
		for (int i = 0;i < 7 * T - info_A[a+A-Am].FT;i++)
			R_A[a].push_back(i);
	fin.close();
	fin.clear();
	fin.open("../Data/A1_air_profit.txt");
	if (!fin) { cout << "open fail" << endl; }
	else {
		string str_buf;
		for (int i = 0;i < N;i++) {
			getline(fin, str_buf);
			istringstream stream(str_buf);
			getline(stream, str_buf, '.');
			if (i > 0)
				getline(stream, str_buf, '.');
			for (int j = 0;j < N;j++)
			{
				r_A[i][j] = atoi(str_buf.c_str());
				r_A[i][j] = r_A[i][j] / 100;
				getline(stream, str_buf, '.');
			}
		}
	}
	fin.close();
	fin.clear();
	fin.open("../Data/A1_air_stopcost.txt");
	if (!fin) { cout << "open fail" << endl; }
	else {
		string str_buf;
		getline(fin, str_buf);
		istringstream stream(str_buf);
		for (int i = 0;i < N;i++) 
		{
			getline(stream, str_buf, '\t');
			F_A[i] = atoi(str_buf.c_str());
		}
	}
	fin.close();
	fin.clear();
	fin.open("../Data/A1_air_timecost.txt");
	if (!fin) { cout << "open fail" << endl; }
	else {
		string str_buf;
		for (int i = 0;i < N + 1;i++) {
			getline(fin, str_buf);
			istringstream stream(str_buf);
			getline(stream, str_buf, '\t');
			if (i > 0)
				for (int j = 0;j < N;j++)
				{
					if (str_buf != "M")
						time_A[i - 1][j] = atoi(str_buf.c_str());
					else time_A[i - 1][j] = 0;
					getline(stream, str_buf, '\t');
				}
		}
	}
	fin.close();
	fin.clear();
	fin.open("../Data/A1_air_trans_cost.txt");
	if (!fin) { cout << "open fail" << endl; }
	else
		for (int i = 0;i < N;i++)
			for (int j = 0;j < N;j++)
				fin >> trans_A[i][j];
	fin.close();
	fin.clear();
	fin.open("../Data/A1_cargo.txt");
	if (!fin) { cout << "open fail" << endl; }
	else {
		string str_buf;
		for (int i = 0;i < K + 1;i++) {
			getline(fin, str_buf);
			istringstream stream(str_buf);
			getline(stream, str_buf, '\t');
			if (i > 0)
				for (int j = 0;j < 8;j++)
				{
					if (j == 0)
					{
						string tmp = str_buf.c_str();
						char tmp1[2];
						strcpy(tmp1, tmp.c_str());
						cargo[i - 1].O = (int)tmp1[0] - 65;
					}
					if (j == 1)
					{
						string tmp = str_buf.c_str();
						char tmp1[2];
						strcpy(tmp1, tmp.c_str());
						cargo[i - 1].D = (int)tmp1[0] - 65;
					}
					if (j == 2)
						cargo[i - 1].starttime = atoi(str_buf.c_str());
					if (j == 3)
						cargo[i - 1].endtime = atoi(str_buf.c_str());
					if (j == 4)
						cargo[i - 1].wei = atoi(str_buf.c_str());
					if (j == 5)
						cargo[i - 1].vol = atoi(str_buf.c_str());
					if (j == 6)
					{
						string tmp = str_buf.c_str();
						char tmp1[2];
						strcpy(tmp1, tmp.c_str());
						cargo[i - 1].TS = tmp1[0];
					}
					if (j == 7)
					{
						string tmp = str_buf.c_str();
						char tmp1[2];
						strcpy(tmp1, tmp.c_str());
						cargo[i - 1].CV = tmp1[0];
					}
					getline(stream, str_buf, '\t');
				}
		}	
		for (int k = 0;k < K;k++)
		{
			cargo[k].weight = cargo[k].wei / cargo[k].vol;
			if (cargo[k].weight < 1 / e)
				cargo[k].weight = 1 / e;
		}
	}
	fin.close();
	fin.clear();
	/*cout << cargo[3].TS <<" ; " << cargo[3].CV<< endl;
	system("pause");*/
	fin.open("../Data/A1_alpha_belta.txt");
	if (!fin) { cout << "open fail" << endl; }
	else {
		for (int k = 0;k < K;k++) {
			for (int j = 0;j < 2;j++)
			{
				if (j == 0) 
					fin >> cargo[k].alpha;							
				if (j == 1) 
					fin >> cargo[k].belta; 						
			}
		}
	}
	fin.close();
	fin.clear();
	fin.open("../Data/A1_virtual.txt");
	if (!fin) { cout << "open fail" << endl; }
	else {
		string str_buf;
		for (int i = 0;i < N;i++) {
			fin >> c_N[i];
		}
	}
	fin.close();
	fin.clear();
	fin.open("../Data/A1_unload_cost.txt");
	if (!fin) { cout << "open fail" << endl; }
	else {
		string str_buf;
		for (int i = 0;i < N;i++) {
			fin >> c_L[i];
		}
	}
	fin.close();
	fin.clear();
	fin.open("../Data/A1_sea_arccost.txt");
	if (!fin) { cout << "open fail" << endl; }
	else {
		string str_buf;
		for (int i = 0;i < M + 1;i++) {
			getline(fin, str_buf);
			istringstream stream(str_buf);
			getline(stream, str_buf, '\t');
			if (i > 0)
				for (int j = 0;j < M;j++)
				{
					if (str_buf != "M")
						c_S[i - 1][j] = atoi(str_buf.c_str());
					else c_S[i - 1][j] = BigM;
					getline(stream, str_buf, '\t');
				}
		}
	}
	fin.close();
	fin.clear();
	fin.open("../Data/A1_sea_ships_param.txt");
	if (!fin) { cout << "open fail" << endl; }
	else {
		string str_buf;
		for (int a = 0;a < S + 1;a++) {
			getline(fin, str_buf);
			istringstream stream(str_buf);
			getline(stream, str_buf, '\t');
			if (a > 0)
				for (int i = 0;i < 5;i++)
				{
					if (i == 0)
					{
						string tmp = str_buf.c_str();
						char tmp1[2];
						strcpy(tmp1, tmp.c_str());
						info_S[a - 1].startnode = (int)tmp1[0] - 65;
					}
					if (i == 1)
						info_S[a - 1].starttime = atoi(str_buf.c_str());
					if (i == 2)
						info_S[a - 1].freq = atoi(str_buf.c_str());
					if (i == 3)
						info_S[a - 1].t = atoi(str_buf.c_str());
					if (i == 4)
						info_S[a - 1].vol = atoi(str_buf.c_str());
					getline(stream, str_buf, '\t');
				}
		}
	}
	for (int s = 0;s < S;s++)
		for (int j = 0;j < L*M ;j = j + 21*M)
			H_S[s].push_back(info_S[s].starttime*M + info_S[s].startnode + j);
	//for (int i = 0;i < H_S[0].size();i++)
	//	cout << H_S[0][i] << endl;
	fin.close();
	fin.clear();
	fin.open("../Data/A1_sea_profit.txt");
	if (!fin) { cout << "open fail" << endl; }
	else {
		string str_buf;
		for (int i = 0;i < M;i++) {
			getline(fin, str_buf);
			istringstream stream(str_buf);
			getline(stream, str_buf, '.');
			getline(stream, str_buf, '.');
			for (int j = 0;j < M;j++)
			{
				r_S[i][j] = atoi(str_buf.c_str());
				r_S[i][j] = r_S[i][j] / 100;
				getline(stream, str_buf, '.');
			}
		}
	}
	fin.close();
	fin.clear();
	fin.open("../Data/A1_sea_stopcost.txt");
	if (!fin) { cout << "open fail" << endl; }
	else {
		string str_buf;
		getline(fin, str_buf);
		istringstream stream(str_buf);
		for (int i = 0;i < M;i++)
		{
			getline(stream, str_buf, '\t');
			F_S[i] = atoi(str_buf.c_str());
		}
	}
	fin.close();
	fin.clear();
	fin.open("../Data/A1_sea_timecost.txt");
	if (!fin) { cout << "open fail" << endl; }
	else {
		string str_buf;
		for (int i = 0;i < M + 1;i++) {
			getline(fin, str_buf);
			istringstream stream(str_buf);
			getline(stream, str_buf, '\t');
			if (i > 0)
				for (int j = 0;j < M;j++)
				{
					if (str_buf != "M")
						time_S[i - 1][j] = atoi(str_buf.c_str());
					else time_S[i - 1][j] = 0;
					getline(stream, str_buf, '\t');
				}
		}
	}
	fin.close();
	fin.clear();
	fin.open("../Data/A1_sea_trans_cost.txt");
	if (!fin) { cout << "open fail" << endl; }
	else 
		for (int i = 0;i <M;i++) 
			for (int j = 0;j < M;j++)		
				fin >> trans_S[i][j]; 
	fin.close();
	fin.clear();

	//Input Network & Path
	sea_route rival_S[Sc];
	sea_route TA_S[Sn - Sc];
	air_route rival_A[Ac];
	air_route TA_A[An - Ac];
	fin.open("../Data/A1_sea_rival_routes.csv");
	if (!fin) { cout << "open fail" << endl; }
	else {
		string str_buf;
		for (int i = 0;i < Sc + 1;i++) {
			getline(fin, str_buf);
			istringstream stream(str_buf);
			getline(stream, str_buf, ',');
			if (i > 0)
			{
				int j = 0;
				string prove;
				do
				{
					if (j == 0)
						rival_S[i - 1].vol = atoi(str_buf.c_str());
					if (j >0)
					{
						int port, time;
						string tmp = str_buf.c_str();
						char tmp1[4];
						strcpy(tmp1, tmp.c_str());
						port = (int)tmp1[0] - 65;
						if (tmp1[2] == NULL)
							time = tmp1[1] - '0';
						else
							time = (tmp1[1] - '0') * 10 + tmp1[2] - '0';

						rival_S[i - 1].port.push_back(port);
						rival_S[i - 1].time.push_back(time);
					} 
					j++;
					prove = str_buf.c_str();
					getline(stream, str_buf, ',');
				} while (str_buf.c_str() != prove);
			}
		}
	}
	fin.close();
	fin.clear();
	fin.open("../Data/A1_sea_target_routes.csv");
	if (!fin) { cout << "open fail" << endl; }
	else {
		string str_buf;
		for (int i = 0;i < Sn-Sc + 1;i++) {
			getline(fin, str_buf);
			istringstream stream(str_buf);
			getline(stream, str_buf, ',');
			if (i > 0)
			{
				int j = 0;
				string prove;
				do
				{
					if (j == 0)
						TA_S[i - 1].vol = atoi(str_buf.c_str());
					if (j > 0)
					{
						int port, time;
						string tmp = str_buf.c_str();
						char tmp1[4];
						strcpy(tmp1, tmp.c_str());
						port = (int)tmp1[0] - 65;
						if (tmp1[2] == NULL)
							time = tmp1[1] - '0';
						else
							time = (tmp1[1] - '0') * 10 + tmp1[2] - '0';
						TA_S[i - 1].port.push_back(port);
						TA_S[i - 1].time.push_back(time);
					}
					j++;
					prove = str_buf.c_str();
					getline(stream, str_buf, ',');
				} while (str_buf.c_str() != prove);
			}
		}
	}
	fin.close();
	fin.clear();
	fin.open("../Data/A1_air_rival_routes.csv");
	if (!fin) { cout << "open fail" << endl; }
	else {
		string str_buf;
		for (int i = 0;i < Ac + 1;i++) {
			getline(fin, str_buf);
			istringstream stream(str_buf);
			getline(stream, str_buf, ',');
			if (i > 0)
			{
				int j = 0;
				string prove;
				do
				{
					if (j == 0)
						rival_A[i - 1].vol = atoi(str_buf.c_str());
					if (j == 1)
						rival_A[i - 1].wei = atoi(str_buf.c_str());
					if (j > 1)
					{
						int port, time;
						string tmp = str_buf.c_str();
						char tmp1[4];
						strcpy(tmp1, tmp.c_str());
						port = (int)tmp1[0] - 65;
						if (tmp1[2] == NULL)
							time = tmp1[1] - '0';
						else
							time = (tmp1[1] - '0') * 10 + tmp1[2] - '0';
						rival_A[i - 1].port.push_back(port);
						rival_A[i - 1].time.push_back(time);				
					}
					j++;
					if(j>1)
						prove = str_buf.c_str();
					getline(stream, str_buf, ',');
				} while (str_buf.c_str() != prove);
			}
		}
	}
	for (int a = 0;a < Ac;a++)
		for (int i = 0;i < rival_A[a].port.size();i++)
			if (rival_A[a].port[i] < 0)
			{
				rival_A[a].gap = rival_A[a].time[i + 1] - rival_A[a].time[i - 1];
				rival_A[a].t = rival_A[a].time[i - 1] - rival_A[a].time[0];
			}

	fin.close();
	fin.clear();
	fin.open("../Data/A1_air_target_routes.csv");
	if (!fin) { cout << "open fail" << endl; }
	else {
		string str_buf;
		for (int i = 0;i < An-Ac + 1;i++) {
			getline(fin, str_buf);
			istringstream stream(str_buf);
			getline(stream, str_buf, ',');
			if (i > 0)
			{
				int j = 0;
				string prove;
				do
				{
					if (j == 0)
						TA_A[i - 1].vol = atoi(str_buf.c_str());
					if (j == 1)
						TA_A[i - 1].wei = atoi(str_buf.c_str());
					if (j > 1)
					{
						int port, time;
						string tmp = str_buf.c_str();
						char tmp1[4];
						strcpy(tmp1, tmp.c_str());
						port = (int)tmp1[0] - 65;
						if (tmp1[2] == NULL)
							time = tmp1[1] - '0';
						else
							time = (tmp1[1] - '0') * 10 + tmp1[2] - '0';
						TA_A[i - 1].port.push_back(port);
						TA_A[i - 1].time.push_back(time);
					}
					j++;
					if (j > 1)
						prove = str_buf.c_str();
					getline(stream, str_buf, ',');
				} while (str_buf.c_str() != prove);
			}
		}
	}
	fin.close();
	fin.clear();
	for (int a = 0;a < An-Ac;a++)
		for (int i = 0;i < TA_A[a].port.size();i++)
			if (TA_A[a].port[i] < 0)
			{
				TA_A[a].gap = TA_A[a].time[i + 1] - TA_A[a].time[i - 1];
				TA_A[a].t = TA_A[a].time[i - 1] - TA_A[a].time[0];
			}


	//Set V_Sn & E_Sn
	for (int s = 0;s < Sn;s++)
	{
		if (s < Sc)
		{
			int tmp = 0;
			for (int j = 0;j < L;j = j + 21)
				for (int i = 0;i < rival_S[s].port.size();i++)
					if (rival_S[s].time[i] + j < L)
					{
						if (tmp != 0)
							if (rival_S[s].port[i] + M * (rival_S[s].time[i] + j) != V_Sn[s][tmp - 1])
							{
								tmp++;
								V_Sn[s].push_back(rival_S[s].port[i] + M * (rival_S[s].time[i] + j));
							}
						if (tmp == 0)
						{
							tmp++;
							V_Sn[s].push_back(rival_S[s].port[i] +M* (rival_S[s].time[i] + j));
						}
					}
		}
		if(s>=Sc)
		{
			int tmp = 0;
			for (int j = 0;j < L;j = j + 21)
				for (int i = 0;i < TA_S[s-Sc].port.size();i++)
					if (TA_S[s - Sc].time[i] + j < L)
					{
						if (tmp != 0)
							if (TA_S[s - Sc].port[i] * (TA_S[s - Sc].time[i] + j) != V_Sn[s][tmp - 1])
							{
								tmp++;
								V_Sn[s].push_back(TA_S[s - Sc].port[i] + M * (TA_S[s - Sc].time[i] + j));
							}
						if (tmp == 0)
						{
							tmp++;
							V_Sn[s].push_back(TA_S[s - Sc].port[i] + M * (TA_S[s - Sc].time[i] + j));
						}
					}
		}
		E_Sn[s].resize(V_Sn[s].size() - 1);
		for (int i = 0;i < V_Sn[s].size() - 1;i++)
		{
			E_Sn[s][i].push_back(V_Sn[s][i]);
			E_Sn[s][i].push_back(V_Sn[s][i + 1]);
		}
	}

	//Set V_An & E_An
	for (int a = 0;a < An;a++)
	{
		if (a < Ac)
		{
			for (int j = 0;j < L;j = j + rival_A[a].gap + rival_A[a].t)
				for (int i = 0;i < rival_A[a].port.size() && rival_A[a].port[i]>=0;i++)
					if (rival_A[a].time[i] + j < L)
						V_An[a].push_back(rival_A[a].port[i] + M * (rival_A[a].time[i] + j));
		}
		if (a >= Ac)
		{
			for (int j = 0;j < L;j = j + TA_A[a - Ac].gap + TA_A[a - Ac].t)
				for (int i = 0;i < TA_A[a - Ac].port.size() && TA_A[a - Ac].port[i]>=0;i++)
					if (TA_A[a - Ac].time[i] + j < L)
						V_An[a].push_back(TA_A[a - Ac].port[i] + M * (TA_A[a - Ac].time[i] + j));
		}
		E_An[a].resize(V_An[a].size() - 1);
		for (int i = 0;i < V_An[a].size() - 1;i++)
		{
			E_An[a][i].push_back(V_An[a][i]);
			E_An[a][i].push_back(V_An[a][i + 1]);
		}
	}

	//Set V_N & E_N
	E_N[0].resize(L*M);
	for (int i = 0;i < L*M;i++)
	{
		V_N[0].push_back(i);	
		E_N[0][i].push_back(i);
		if (i + M < L*M)
			E_N[0][i].push_back(i + M);
	}

	//Input V_S & V_A
	E_A[0].resize(1000);
	int count = 0;
	fin.open("../Data/A1_air_arcs.csv");
	if (!fin) { cout << "open fail" << endl; }
	else {
		string str_buf;
		for (int i = 0;i < 738;i++) {
			getline(fin, str_buf);
			istringstream stream(str_buf);
			getline(stream, str_buf, ',');

			for(int j=0;j<2;j++)
			{
				int port, time;
				string tmp = str_buf.c_str();
				char tmp1[5];
				strcpy(tmp1, tmp.c_str());
				port = (int)tmp1[1] - 65;
				if (tmp1[3] == NULL)
					time = tmp1[2] - '0';
				else
					time = (tmp1[2] - '0') * 10 + tmp1[3] - '0';
				E_A[(tmp1[0] - '0') - 1][i].push_back(port + M * time);
				
				getline(stream, str_buf, ',');
			} 
			count++;
		}
	}
	E_A[0].resize(count);
	fin.close();
	fin.clear();
	count = 0;
	for (int i = 0;i < E_A[0].size();i++)	
		for (int j = 0;j < 2;j++)
		{
			int tmp = 0;
			if (i != 0)
			{
				for (int k = 0;k < count;k++)
				{
					if (V_A[0][k] == E_A[0][i][j])
						tmp++;
				}
			}
			if (tmp == 0) {
				V_A[0].push_back(E_A[0][i][j]);
				count++;
			}
		}
	//Set E_Am
	for (int a = 0;a < Am;a++)
	{
		E_Am[a].resize(R_A[a].size());
		for (int m = 0;m < R_A[a].size();m++)
		{
			E_Am[a][m].resize(1000);
			count = 0;
			for (int i = 0;i < E_A[a + A - Am].size();i++)
				if (E_A[a + A - Am][i][0] >= m * N && E_A[a + A - Am][i][0] < (m + info_A[a].FT)*N)
					if (E_A[a + A - Am][i][1] >= m * N && E_A[a + A - Am][i][1] < (m + info_A[a].FT)*N)
					{
						E_Am[a][m][count].push_back(E_A[a + A - Am][i][0]);
						E_Am[a][m][count].push_back(E_A[a + A - Am][i][1]);
						count++;
					}
			E_Am[a][m].resize(count);
		}
	}
	

	E_S[0].resize(1000);
	count = 0;
	fin.open("../Data/A1_sea_arcs.csv");
	if (!fin) { cout << "open fail" << endl; }
	else {
		string str_buf;
		for (int i = 0;i < 938;i++) {
			getline(fin, str_buf);
			istringstream stream(str_buf);
			getline(stream, str_buf, ',');
			for (int j = 0;j < 2;j++)
			{
				int port, time;
				string tmp = str_buf.c_str();
				char tmp1[5];
				strcpy(tmp1, tmp.c_str());
				port = (int)tmp1[1] - 65;
				if (tmp1[3] == NULL)
					time = tmp1[2] - '0';
				else
					time = (tmp1[2] - '0') * 10 + tmp1[3] - '0';
				if (time >= info_S[0].starttime)
				{
					E_S[(tmp1[0] - '0')][count].push_back(port + M * time);
				}
				else if (time < info_S[0].starttime)
					goto skip2;
				getline(stream, str_buf, ',');
			}
			count++;
		skip2:
			count = count;
		}
	}

	E_S[0].resize(count);
	fin.close();
	fin.clear();
	count = 0;
	for (int i = 0;i < E_S[0].size();i++)
		for (int j = 0;j < 2;j++)
		{
			int tmp = 0;
			if (i != 0)
			{
				for (int k = 0;k < count;k++)
				{
					if (V_S[0][k] == E_S[0][i][j])
						tmp++;
				}
			}
			if (tmp == 0) {
				V_S[0].push_back(E_S[0][i][j]);
				count++;
			}
		}

	//Input Path
	A_P.resize(1000);
	count = 0;
	fin.open("../Data/A1_all_paths.csv");
	if (!fin) { cout << "open fail" << endl; }
	else {
		string str_buf;
		for (int i = 0;i < 587;i++) {
			getline(fin, str_buf);
			istringstream stream(str_buf);
			getline(stream, str_buf, ',');
			int j = 0;
			string prove;
			P_AllCargo.push_back(i);
			do
			{
				int port, time;
				string tmp = str_buf.c_str();
				char tmp1[5];
				strcpy(tmp1, tmp.c_str());
				port = (int)tmp1[1] - 65;
				if (tmp1[3] == NULL)
					time = tmp1[2] - '0';
				else
					time = (tmp1[2] - '0') * 10 + tmp1[3] - '0';
				A_P[P_AllCargo[i]].push_back(port + M * time + (L*M* (tmp1[0] - '0')));
				
				j++;
				prove = str_buf.c_str();
				getline(stream, str_buf, ',');
			} while (str_buf.c_str() != prove);
			count++;
		}
	}
	fin.close();
	fin.clear();
	A_P.resize(count);

	for (int k = 0 ; k < K;k++)
	{
		int O, D, st, ed;
		for (int p = 0;p < P_AllCargo.size();p++)
		{
			int count1 = 0, count2 = 0, count3 = 0, count4 = 0;
			for (int a = 0;a < A_P[p].size();a++)	
			{
				//cout << A_P[p][a] << endl;
				if(cargo[k].TS == 'H')
				{
					if (cargo[k].CV == 'H')
						if (A_P[p][a] < L*M*S || (A_P[p][a] >= L * M*(S + A + 1) && A_P[p][a] < L * M*(S + A + 1 + Sn - Sc)) || (A_P[p][a] >= L * M*(S + A + 1 + Sn - Sc + An - Ac) && A_P[p][a] < L * M*(S + A + 1 + Sn + An - Ac)))
							count3++;
					if (cargo[k].CV == 'L')
						if (A_P[p][a] < L*M*S || (A_P[p][a] >= L * M*(S + A + 1) && A_P[p][a] < L * M*(S + A + 1 + Sn - Sc)) || (A_P[p][a] >= L * M*(S + A + 1 + Sn - Sc + An - Ac) && A_P[p][a] < L * M*(S + A + 1 + Sn + An - Ac)))
							count1++;
				}
				else if (cargo[k].TS == 'L')
				{
					if (cargo[k].CV == 'H')
						if ((A_P[p][a] >= L * M*S && A_P[p][a] < L * M*(S + A)) || (A_P[p][a] >= L * M*(S + A + 1 + Sn - Sc) && A_P[p][a] < L * M*(S + A + 1 + Sn - Sc + An - Ac)) || (A_P[p][a] >= L * M*(S + A + 1 + Sn + An - Ac) && A_P[p][a] < L * M*(S + A + 1 + Sn + An)))
							count2++;
					if (cargo[k].CV == 'L')
						if ((A_P[p][a] >= L * M*S && A_P[p][a] < L * M*(S + A)) || (A_P[p][a] >= L * M*(S + A + 1 + Sn - Sc) && A_P[p][a] < L * M*(S + A + 1 + Sn - Sc + An - Ac)) || (A_P[p][a] >= L * M*(S + A + 1 + Sn + An - Ac) && A_P[p][a] < L * M*(S + A + 1 + Sn + An)))
							count4++;
				}
			}
			if (cargo[k].TS == 'H')
				if (cargo[k].CV == 'L')
					if (count1 == 0)
						goto skip1;
			if (cargo[k].TS == 'L')
				if (cargo[k].CV == 'H')
					if (count2 == 0)
						goto skip1;
			if (cargo[k].TS == 'H')
				if (cargo[k].CV == 'H')
					if (count3 != 0)
						goto skip1;
			if (cargo[k].TS == 'L')
				if (cargo[k].CV == 'L')
					if (count4 != 0)
						goto skip1;	
			O = A_P[p][0] % M;
			D = A_P[p][A_P[p].size() - 1] % M;
			st = (A_P[p][0] - (A_P[p][0] % M)) / M - 2 * L;
			ed = (A_P[p][A_P[p].size() - 1] - (A_P[p][A_P[p].size() - 1] % M)) / M -L*(A_P[p][A_P[p].size() - 1]/(L*M));
			/*cout << ed << endl;*/
			if (cargo[k].O == O && D == cargo[k].D && st >= cargo[k].starttime && cargo[k].endtime >= ed)
				P_All[k].push_back(P_AllCargo[p]);
		skip1:
			O = 0;
		}
	}
	/*system("pause");*/
	//for (int k = 0; k < K;k++)
	//{
	//	cout << "ii" << k << endl;
	//	for (int p = 0;p < P_All[k].size();p++)
	//		cout << P_All[k][p] << endl;
	//}
	//system("pause");
	for (int k = 0; k < K;k++)
	{
		for (int p = 0;p < P_All[k].size();p++)
		{
			count = 0;
			for (int a = 0;a < A_P[P_All[k][p]].size();a++)
				if (A_P[P_All[k][p]][a] < L*M*(S + A))
					count++;		
			if (count != 0)
				P_Design[k].push_back(P_All[k][p]);
			else if(count == 0)
				P_Current[k].push_back(P_All[k][p]);
		}
	}

	for (int k = 0; k < K;k++)
	{
		for (int p = 0;p < P_Current[k].size();p++)
		{
			count = 0;
			for (int a = 0;a < A_P[P_Current[k][p]].size();a++)
				if (A_P[P_Current[k][p]][a] >= L*M*(S + A+1+Sn+An-Sc-Ac))
					count++;
			if (count != 0)
				P_CurrentC[k].push_back(P_Current[k][p]);
			else if (count == 0)
				P_CurrentTA[k].push_back(P_Current[k][p]);
		}
	}

	int Psize_CurrentC = 0;
	for (int k = 0;k < K;k++)
		Psize_CurrentC += P_CurrentC[k].size();
	int Psize_CurrentTA = 0;
	for (int k = 0;k < K;k++)
		Psize_CurrentTA += P_CurrentTA[k].size();
	int Psize_Design = 0;
	for (int k = 0;k < K;k++)
		Psize_Design += P_Design[k].size();

	int ****PArc = new int ***[587];
	for (int i = 0;i < 587;i++)
	{
		PArc[i] = new int **[(A + S + An + Sn + 1)];
		for (int j = 0;j < (A + S + An + Sn + 1);j++)
		{
			PArc[i][j] = new int*[L*M];
			for (int k = 0;k < L*M;k++)
				PArc[i][j][k] = new int[L*M];
		}
	}
	for (int i = 0;i < 587;i++)
		for (int j = 0;j < (A + S + An + Sn + 1);j++)		
			for (int k = 0;k < L*M;k++)
				for (int h = 0;h < L*M;h++)
					PArc[i][j][k][h] = 0;

	vector<vector<double>> v(K, vector<double>(0));
	vector<vector<double>> p_C(K, vector<double>(0));
	vector<vector<double>> p_TA(K, vector<double>(0));

	for (int p = 0;p < P_AllCargo.size();p++)
	{
		for (int a = 0;a < A_P[P_AllCargo[p]].size() - 1;a++)
		{
			if (A_P[P_AllCargo[p]][a] < L*M*S)
			{
				if (A_P[P_AllCargo[p]][a + 1] < L*M*S)
					PArc[p][S - 1][A_P[P_AllCargo[p]][a]][A_P[P_AllCargo[p]][a + 1]] = 1;
				if (A_P[P_AllCargo[p]][a + 1] < L*M*(S + A + 1) && A_P[P_AllCargo[p]][a + 1] >= L * M*(S + A))
					PArc[p][S + A][A_P[P_AllCargo[p]][a + 1] - M - L * M*(S + A)][A_P[P_AllCargo[p]][a + 1] - M - L * M*(S + A)] = 1;
			}
			else if (A_P[P_AllCargo[p]][a] < L*M*(S + A))
			{
				if (A_P[P_AllCargo[p]][a + 1] < L*M*(S + A))
					PArc[p][S + A - 1][A_P[P_AllCargo[p]][a] - L * M*S][A_P[P_AllCargo[p]][a + 1] - L * M*S] = 1;
				if (A_P[P_AllCargo[p]][a + 1] < L*M*(S + A + 1) && A_P[P_AllCargo[p]][a + 1] >= L * M*(S + A))
					PArc[p][A + S][A_P[P_AllCargo[p]][a + 1] - M - L * M*(S + A)][A_P[P_AllCargo[p]][a + 1] - M - L * M*(S + A)] = 1;
			}
			else if (A_P[P_AllCargo[p]][a] < L*M*(A + S + 1))
			{
				if (A_P[P_AllCargo[p]][a + 1] >= L * M*(S + A + 1) || A_P[P_AllCargo[p]][a + 1] < L * M*(S + A))
					PArc[p][S + A][A_P[P_AllCargo[p]][a] - L * M*(S + A)][A_P[P_AllCargo[p]][a] - L * M*(S + A)] = 1;
				else if (A_P[P_AllCargo[p]][a + 1] < L*M*(A + S + 1))
					PArc[p][S + A][A_P[P_AllCargo[p]][a] - L * M*(S + A)][A_P[P_AllCargo[p]][a + 1] - L * M*(S + A)] = 1;
			}
			else if (A_P[P_AllCargo[p]][a] < L*M*(Sn + A + S + 1 - Sc))
			{
				if (A_P[P_AllCargo[p]][a + 1] < L*M*(S + A + 1) && A_P[P_AllCargo[p]][a + 1] >= L * M*(S + A))
					PArc[p][A + S][A_P[P_AllCargo[p]][a + 1] - M - L * M*(S + A)][A_P[P_AllCargo[p]][a + 1] - M - L * M*(S + A)] = 1;
				else if (A_P[P_AllCargo[p]][a + 1] < L*M*(Sn + A + S + 1 - Sc))
					PArc[p][Sn + A + S + 1 - Sc - 1][A_P[P_AllCargo[p]][a] - L * M*(Sn + A + S + 1 - Sc - 1)][A_P[P_AllCargo[p]][a + 1] - L * M*(Sn + A + S + 1 - Sc - 1)] = 1;
			}
			else if (A_P[P_AllCargo[p]][a] < L*M*(Sn + An + S + A + 1 - Sc - Ac))
			{
				if (A_P[P_AllCargo[p]][a + 1] < L*M*(S + A + 1) && A_P[P_AllCargo[p]][a + 1] >= L * M*(S + A))
					PArc[p][A + S][A_P[P_AllCargo[p]][a + 1] - M - L * M*(S + A)][A_P[P_AllCargo[p]][a + 1] - M - L * M*(S + A)] = 1;
				else if (A_P[P_AllCargo[p]][a + 1] < L*M*(Sn + An + S + A + 1 - Sc - Ac))
					PArc[p][Sn + An + S + A + 1 - Sc - Ac - 1][A_P[P_AllCargo[p]][a] - L * M*(Sn + An + S + A + 1 - Sc - Ac - 1)][A_P[P_AllCargo[p]][a + 1] - L * M*(Sn + An + S + A + 1 - Sc - Ac - 1)] = 1;
			}
			else if (A_P[P_AllCargo[p]][a] < L*M*(Sn + An + S + A + 1 - Ac))
			{
				if (A_P[P_AllCargo[p]][a + 1] < L*M*(S + A + 1) && A_P[P_AllCargo[p]][a + 1] >= L * M*(S + A))
					PArc[p][A + S][A_P[P_AllCargo[p]][a + 1] - M - L * M*(S + A)][A_P[P_AllCargo[p]][a + 1] - M - L * M*(S + A)] = 1;
				else if (A_P[P_AllCargo[p]][a + 1] < L*M*(Sn + An + S + A + 1 - Ac))
					PArc[p][Sn + An + S + A + 1 - Ac - 1][A_P[P_AllCargo[p]][a] - L * M*(Sn + An + S + A + 1 - Ac - 1)][A_P[P_AllCargo[p]][a + 1] - L * M*(Sn + An + S + A + 1 - Ac - 1)] = 1;
			}
			else if (A_P[P_AllCargo[p]][a] < L*M*(Sn + An + S + A + 1))
			{
				if (A_P[P_AllCargo[p]][a + 1] < L*M*(S + A + 1) && A_P[P_AllCargo[p]][a + 1] >= L * M*(S + A))
					PArc[p][A + S][A_P[P_AllCargo[p]][a + 1] - M - L * M*(S + A)][A_P[P_AllCargo[p]][a + 1] - M - L * M*(S + A)] = 1;
				else if (A_P[P_AllCargo[p]][a + 1] < L*M*(Sn + An + S + A + 1))
					PArc[p][Sn + An + S + A][A_P[P_AllCargo[p]][a] - L * M*(Sn + An + S + A)][A_P[P_AllCargo[p]][a + 1] - L * M*(Sn + An + S + A)] = 1;
			}
		}
		//Last node
		int tmp6 = A_P[P_AllCargo[p]][A_P[P_AllCargo[p]].size() - 1] / (L*M);
		int LastNode = (A_P[P_AllCargo[p]][A_P[P_AllCargo[p]].size() - 1] - tmp6 * L*M);
		PArc[p][S + A][LastNode][LastNode] = 1;
	}

	//for (int s = 0;s < S;s++)
	//	for (int k = 0;k < E_S[s].size();k++)
	//		cout << E_S[s][k][0] << " ; " << E_S[s][k][1] << endl;
	//Calculate v & p
	for (int k = 0;k < K;k++)
	{
		for (int p = 0;p < P_All[k].size();p++)
		{
			double tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0, tmp5 = 0, tmp6 = 0;
			for (int s = 0;s < S;s++)
				for (int i = 0;i < E_S[s].size();i++)
					if (PArc[P_All[k][p]][s][E_S[s][i][0]][E_S[s][i][1]] == 1)
						tmp1 += trans_S[E_S[s][i][0] % M][E_S[s][i][1] % M];				
			for (int s = 0;s < Sn-Sc;s++)
				for (int i = 0;i < E_Sn[s+Sc].size();i++)
					if (PArc[P_All[k][p]][s + S + A + 1][E_Sn[s + Sc][i][0]][E_Sn[s + Sc][i][1]] == 1)
						tmp1 +=  trans_S[E_Sn[s + Sc][i][0] % M][E_Sn[s + Sc][i][1] % M];
			for (int s = 0;s < Sc;s++)
				for (int i = 0;i < E_Sn[s].size();i++)
					if (PArc[P_All[k][p]][s + S + A + 1 + Sn - Sc + An - Ac][E_Sn[s][i][0]][E_Sn[s][i][1]]  == 1)
						tmp1 +=  trans_S[E_Sn[s][i][0] % M][E_Sn[s][i][1] % M];
			for (int a = 0;a < A;a++)
				for (int i = 0;i < E_A[a].size();i++)
					if (PArc[P_All[k][p]][a + S][E_A[a][i][0]][E_A[a][i][1]]  == 1)
						tmp2 +=  trans_A[E_A[a][i][0] % N][E_A[a][i][1] % N];
			for (int a = 0;a < An-Ac;a++)
				for (int i = 0;i < E_An[a+Ac].size();i++)
					if (PArc[P_All[k][p]][a + S + A + 1 + Sn - Sc][E_An[a + Ac][i][0]][E_An[a + Ac][i][1]] == 1)
						tmp2 +=  trans_A[E_An[a + Ac][i][0] % N][E_An[a + Ac][i+ Ac][1] % N];
			for (int a = 0;a < Ac;a++)
				for (int i = 0;i < E_An[a].size();i++)
					if (PArc[P_All[k][p]][a + S + A + 1 + Sn + An - Ac][E_An[a][i][0]][E_An[a][i][1]]  == 1)
						tmp2 +=  trans_A[E_An[a][i][0] % N][E_An[a][i][1] % N];
			for (int i = 0;i < E_N[0].size();i++)
				for (int j = 0;j < E_N[0][i].size();j++)
					if (E_N[0][i][0] != E_N[0][i][j])
						tmp3 += PArc[P_All[k][p]][S+A][E_N[0][i][0]][E_N[0][i][j]] * c_N[E_N[0][i][0] % M];
					else
						tmp3 += PArc[P_All[k][p]][S+A][E_N[0][i][0]][E_N[0][i][0]] * c_L[E_N[0][i][0] % M];
			for (int s = 0;s < S;s++)
				for (int i = 0;i < E_S[s].size();i++)
					if (PArc[P_All[k][p]][s][E_S[s][i][0]][E_S[s][i][1]]  == 1)
						tmp4 +=  time_S[E_S[s][i][0] % M][E_S[s][i][1] % M];
			for (int s = 0;s < Sn-Sc;s++)
				for (int i = 0;i < E_Sn[s+Sc].size();i++)
					if (PArc[P_All[k][p]][s + S + A + 1][E_Sn[s + Sc][i][0]][E_Sn[s + Sc][i][1]] == 1)
						tmp4 += time_S[E_Sn[s + Sc][i][0] % M][E_Sn[s + Sc][i][1] % M];
			for (int s = 0;s < Sc;s++)
				for (int i = 0;i < E_Sn[s].size();i++)
					if (PArc[P_All[k][p]][s + S + A + 1 + Sn - Sc + An - Ac][E_Sn[s][i][0]][E_Sn[s][i][1]]  == 1)
						tmp4 +=  time_S[E_Sn[s][i][0] % M][E_Sn[s][i][1] % M];
			for (int a = 0;a < A;a++)
				for (int i = 0;i < E_A[a].size();i++)
					if (PArc[P_All[k][p]][a + S][E_A[a][i][0]][E_A[a][i][1]] == 1)
						tmp5 +=  time_A[E_A[a][i][0] % N][E_A[a][i][1] % N];
			for (int a = 0;a < An-Ac;a++)
				for (int i = 0;i < E_An[a+Ac].size();i++)
					if (PArc[P_All[k][p]][a + S + A + 1 + Sn - Sc][E_An[a + Ac][i][0]][E_An[a + Ac][i][1]]  == 1)
						tmp5 +=  time_A[E_An[a + Ac][i][0] % N][E_An[a + Ac][i][1] % N];
			for (int a = 0;a < Ac;a++)
				for (int i = 0;i < E_An[a].size();i++)
					if (PArc[P_All[k][p]][a + S + A + 1 + Sn + An - Ac][E_An[a][i][0]][E_An[a][i][1]] == 1)
						tmp5 +=time_A[E_An[a][i][0] % N][E_An[a][i][1] % N];
			for (int i = 0;i < E_N[0].size();i++)
				for (int j = 0;j < E_N[0][i].size();j++)
					tmp6 += PArc[P_All[k][p]][S+A][E_N[0][i][0]][E_N[0][i][j]] * time_S[E_N[0][i][0] % M][E_N[0][i][j] % M];
			v[k].push_back(cargo[k].alpha*(tmp1 + tmp2 + tmp3) + cargo[k].belta*(tmp4 + tmp5 + tmp6));
			//cout << v[k][p] << endl;
		}
	}

	for (int k = 0;k < K;k++)
		for(int l = 0;l<P_All[k].size();l++)
			for (int i = 0;i < P_CurrentC[k].size();i++)
				if (P_All[k][l] == P_CurrentC[k][i])
				{
					double tmp1 = 0, tmp2 = 0;
					for (int j = 0;j < P_All[k].size();j++)
						for (int h = 0;h < P_CurrentC[k].size();h++)
							if (P_All[k][j] == P_CurrentC[k][h])
								tmp1 += exp(v[k][j]);
					for (int j = 0;j < P_All[k].size();j++)
					{
						for (int h = 0;h < P_CurrentTA[k].size();h++)
							if (P_All[k][j] == P_CurrentTA[k][h])
								tmp2 += exp(v[k][j]);
						for (int h = 0;h < P_Design[k].size();h++)
							if (P_All[k][j] == P_Design[k][h])
								tmp2 += exp(v[k][j]);

					}
					p_C[k].push_back(exp(v[k][l]) / (tmp1 + tmp2));
					//cout << p_C[k][i] << endl;
				}
	for (int k = 0;k < K;k++)
		for (int l = 0;l < P_All[k].size();l++)
			for (int i = 0;i < P_Design[k].size();i++)
				if (P_All[k][l] == P_Design[k][i])
				{
					double tmp1 = 0, tmp2 = 0;
					for (int j = 0;j < P_All[k].size();j++)
						for (int h = 0;h < P_CurrentC[k].size();h++)
							if (P_All[k][j] == P_CurrentC[k][h])
								tmp1 += exp(v[k][j]);
					for (int j = 0;j < P_All[k].size();j++)
					{
						for (int h = 0;h < P_CurrentTA[k].size();h++)
							if (P_All[k][j] == P_CurrentTA[k][h])
								tmp2 += exp(v[k][j]);
						for (int h = 0;h < P_Design[k].size();h++)
							if (P_All[k][j] == P_Design[k][h])
								tmp2 += exp(v[k][j]);

					}
					p_TA[k].push_back(exp(v[k][l]) / (tmp1 + tmp2));
					//cout << p_TA[k][i] << endl;
				}
	for (int k = 0;k < K;k++)
		for (int l = 0;l < P_All[k].size();l++)
			for (int i = 0;i < P_CurrentTA[k].size();i++)
				if (P_All[k][l] == P_CurrentTA[k][i])
				{
					double tmp1 = 0, tmp2 = 0;
					for (int j = 0;j < P_All[k].size();j++)
						for (int h = 0;h < P_CurrentC[k].size();h++)
							if (P_All[k][j] == P_CurrentC[k][h])
								tmp1 += exp(v[k][j]);
					for (int j = 0;j < P_All[k].size();j++)
					{
						for (int h = 0;h < P_CurrentTA[k].size();h++)
							if (P_All[k][j] == P_CurrentTA[k][h])
								tmp2 += exp(v[k][j]);
						for (int h = 0;h < P_Design[k].size();h++)
							if (P_All[k][j] == P_Design[k][h])
								tmp2 += exp(v[k][j]);

					}
					p_TA[k].push_back(exp(v[k][l]) / (tmp1 + tmp2));
					//cout << p_TA[k][i] << endl;
				}

	//Gurobi
	try {
		GRBEnv env = GRBEnv();
		GRBModel model = GRBModel(env);
//		model.set(GRB_DoubleParam_NodefileStart, 0);
//		model.set(GRB_IntParam_Threads, 1);

		// Create variables

		GRBVar y_S[S][L*M][L*M];
		for (int s = 0;s < S;s++)
			for (int i = 0;i < E_S[s].size();i++)
				y_S[s][E_S[s][i][0]][E_S[s][i][1]] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
		GRBVar y_A[A][L*M][L*M];
		for (int a = 0;a < A;a++)
			for (int i = 0;i < E_A[a].size();i++)
				y_A[a][E_A[a][i][0]][E_A[a][i][1]] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
		GRBVar o[Am][L];
		for (int a = 0;a < Am;a++)
			for (int i = 0;i < R_A[a].size();i++)
				o[a][i] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
		vector<GRBVar> x(0);
		for (int i = 0;i < P_AllCargo.size();i++)
		{
			x.push_back(model.addVar(0.0, 1.0, 0.0, GRB_BINARY));
		}
		vector<vector<GRBVar>> u(K, vector<GRBVar>(0));
		for (int k = 0;k < K;k++)
			for (int i = 0;i < P_All[k].size();i++)
			{
				u[k].push_back(model.addVar(0.0, 1.0, 0.0, GRB_BINARY));
			}
		vector<vector<GRBVar>> z_TA(K, vector<GRBVar>(0));
		for (int k = 0;k < K;k++)
		{
			for (int i = 0;i < P_CurrentTA[k].size();i++)
			{
				z_TA[k].push_back(model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS));
			}
			for (int i = 0;i < P_Design[k].size();i++)
			{
				z_TA[k].push_back(model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS));
			}
		}
		vector<vector<GRBVar>> z_C(K, vector<GRBVar>(0));
		for (int k = 0;k < K;k++)
			for (int i = 0;i < P_CurrentC[k].size();i++)
			{
				z_C[k].push_back(model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS));
			}

		// Set Objective
		GRBLinExpr obj1 = 0, obj2 = 0, obj3 = 0;
		double  obj11 = 0, obj12 = 0;
		for (int k = 0;k < K;k++)
		{
			for (int i = 0;i < P_Design[k].size();i++)
			{
				for (int s = 0;s < S;s++)
					for (int j = 0;j < E_S[s].size();j++)
					{
						obj11 += r_S[E_S[s][j][0] % M][E_S[s][j][1] % M] * PArc[P_Design[k][i]][s][E_S[s][j][0]][E_S[s][j][1]];
						obj11 += r_S[E_S[s][j][0] % M][E_S[s][j][1] % M] * PArc[P_Design[k][i]][s + S + A + 1][E_S[s][j][0]][E_S[s][j][1]];
					}
				for (int s = Sc;s < Sn;s++)
					for (int j = 0;j < E_Sn[s].size();j++)
					{
						obj11 += r_S[E_Sn[s][j][0] % M][E_Sn[s][j][1] % M] * PArc[P_Design[k][i]][s - 1][E_Sn[s][j][0]][E_Sn[s][j][1]];
						obj11 += r_S[E_Sn[s][j][0] % M][E_Sn[s][j][1] % M] * PArc[P_Design[k][i]][s + S + A][E_Sn[s][j][0]][E_Sn[s][j][1]];
					}
				for (int a = 0;a < A;a++)
					for (int j = 0;j < E_A[a].size();j++)
					{
						obj12 += r_A[E_A[a][j][0] % N][E_A[a][j][1] % N] * PArc[P_Design[k][i]][a + S][E_A[a][j][0]][E_A[a][j][1]] * cargo[k].weight;
						obj12 += r_A[E_A[a][j][0] % N][E_A[a][j][1] % N] * PArc[P_Design[k][i]][a + S + A + 1 + Sn - Sc][E_A[a][j][0]][E_A[a][j][1]] * cargo[k].weight;
					}
				for (int a = Ac;a < An;a++)
					for (int j = 0;j < E_An[a].size();j++)
					{
						obj12 += r_A[E_An[a][j][0] % N][E_An[a][j][1] % N] * PArc[P_Design[k][i]][a + S - 1][E_An[a][j][0]][E_An[a][j][1]] * cargo[k].weight;
						obj12 += r_A[E_An[a][j][0] % N][E_An[a][j][1] % N] * PArc[P_Design[k][i]][a + S + A + Sn - Sc][E_An[a][j][0]][E_An[a][j][1]] * cargo[k].weight;
					}
				obj1 += z_TA[k][i] * cargo[k].vol*(obj11 + obj12);
			}
			for (int i = 0;i < P_CurrentTA[k].size();i++)
			{
				for (int s = 0;s < S;s++)
					for (int j = 0;j < E_S[s].size();j++)
					{
						obj11 += r_S[E_S[s][j][0] % M][E_S[s][j][1] % M] * PArc[P_CurrentTA[k][i]][s][E_S[s][j][0]][E_S[s][j][1]];
						obj11 += r_S[E_S[s][j][0] % M][E_S[s][j][1] % M] * PArc[P_CurrentTA[k][i]][s + S + A + 1][E_S[s][j][0]][E_S[s][j][1]];
					}
				for (int s = Sc;s < Sn;s++)
					for (int j = 0;j < E_Sn[s].size();j++)
					{
						obj11 += r_S[E_Sn[s][j][0] % M][E_Sn[s][j][1] % M] * PArc[P_CurrentTA[k][i]][s - 1][E_Sn[s][j][0]][E_Sn[s][j][1]];
						obj11 += r_S[E_Sn[s][j][0] % M][E_Sn[s][j][1] % M] * PArc[P_CurrentTA[k][i]][s + S + A][E_Sn[s][j][0]][E_Sn[s][j][1]];
					}
				for (int a = 0;a < A;a++)
					for (int j = 0;j < E_A[a].size();j++)
					{
						obj12 += r_A[E_A[a][j][0] % N][E_A[a][j][1] % N] * PArc[P_CurrentTA[k][i]][a + S][E_A[a][j][0]][E_A[a][j][1]] * cargo[k].weight;
						obj12 += r_A[E_A[a][j][0] % N][E_A[a][j][1] % N] * PArc[P_CurrentTA[k][i]][a + S + A + 1 + Sn - Sc][E_A[a][j][0]][E_A[a][j][1]] * cargo[k].weight;
					}
				for (int a = Ac;a < An;a++)
					for (int j = 0;j < E_An[a].size();j++)
					{
						obj12 += r_A[E_An[a][j][0] % N][E_An[a][j][1] % N] * PArc[P_CurrentTA[k][i]][a + S - 1][E_An[a][j][0]][E_An[a][j][1]] * cargo[k].weight;
						obj12 += r_A[E_An[a][j][0] % N][E_An[a][j][1] % N] * PArc[P_CurrentTA[k][i]][a + S + A + Sn - Sc][E_An[a][j][0]][E_An[a][j][1]] * cargo[k].weight;
					}
				if (z_TA[k].size() != 0)
					obj1 += z_TA[k][i + P_Design.size()] * cargo[k].vol*(obj11 + obj12);
			}
		}
		for (int s = 0;s < S;s++)
			for (int k = 0;k < E_S[s].size();k++)
				obj2 += (F_S[E_S[s][k][0] % M] + trans_S[E_S[s][k][0] % M][E_S[s][k][1] % M])*y_S[s][E_S[s][k][0]][E_S[s][k][1]];
		for (int a = 0;a < A;a++)
			for (int k = 0;k < E_A[a].size();k++)
				obj2 += (F_A[E_A[a][k][0] % N] + trans_A[E_A[a][k][0] % N][E_A[a][k][1] % N])*y_A[a][E_A[a][k][0]][E_A[a][k][1]];
		model.setObjective(obj1 - obj2 - obj3, GRB_MAXIMIZE);

		// Sea Route Constraints
		//cout << H_S[0].size() << endl;
		GRBLinExpr tmpp = 0;
		for (int s = 0;s < S;s++)	
			for (int i = 0; i < H_S[s].size();i++)
			{
				tmpp = 0;
				for (int j = 0; j < E_S[s].size();j++)
					if (H_S[s][i] == E_S[s][j][0])
						tmpp += y_S[s][E_S[s][j][0]][E_S[s][j][1]];
				model.addConstr(tmpp == 1, "#2_Sea_1");
			}

		for (int s = 0;s < S;s++)
			for (int i = 0; i < H_S[s].size();i++)
				if (H_S[s][i] + M * info_S[s].t < L*M)
				{
					GRBLinExpr tmp = 0;
					for (int k = 0;k < E_S[s].size();k++)
						if (E_S[s][k][1] == H_S[s][i] + M * info_S[s].t)
							tmp += y_S[s][E_S[s][k][0]][H_S[s][i] + M * info_S[s].t];
					model.addConstr(tmp == 1, "#3_Sea_2");
				}
	
		for (int s = 0;s < S;s++)
			for (int i = 0; i < V_S[s].size();i++)
			{
				GRBLinExpr tmp1 = 0, tmp2 = 0;
				int count1 = 0, count2 = 0;
				for (int k = 0;k < H_S[s].size();k++)
				{
					if (V_S[s][i] == H_S[s][k])
						goto pass;
					if (V_S[s][i] == H_S[s][k] + M * info_S[s].t)
						goto pass;
				}

				for (int j = 0; j < E_S[s].size();j++)
					if (E_S[s][j][0] == V_S[s][i])
					{
						tmp1 += y_S[s][V_S[s][i]][E_S[s][j][1]];
						count1++;
					}
				for (int k = 0;k < E_S[s].size();k++)
					if (E_S[s][k][1] == V_S[s][i])
					{
						tmp2 += y_S[s][E_S[s][k][0]][V_S[s][i]];
						count2++;
					}
				if (count1 != 0 && count2 != 0)
					model.addConstr(tmp1 - tmp2 == 0, "#4_Sea_3");
			pass:
				tmp1 = 0; tmp2 = 0;
			}

		for (int s = 0;s < S;s++)
			for (int k = 0;k < E_S[s].size();k++)
				if (E_S[s][k][0] + M * info_S[s].t < L*M && E_S[s][k][1] + M * info_S[s].t < L*M)
					model.addConstr(y_S[s][E_S[s][k][0]][E_S[s][k][1]] == y_S[s][E_S[s][k][0] + M * info_S[s].t][E_S[s][k][1] + M * info_S[s].t], "#5_Sea_4");

		for (int s = 0;s < S;s++)
			for (int m = 0;m < M;m++)
				for (int i = 0; i < V_Si[s][m].size();i++)
				{
					GRBLinExpr tmp1 = 0, tmp2 = 0;
					for (int k = 0;k < E_S[s].size();k++)
						for (int j = 1; j < E_S[s][k].size();j++)
							if (E_S[s][k][j] == V_Si[s][m][i])
								if (E_S[s][k][0] % M != V_Si[s][m][i] % M)
									tmp1 += y_S[s][E_S[s][k][0]][E_S[s][k][j]];
					for (int k = V_Si[s][m][i];k <= V_Si[s][m][i] + M * (time_S[m][m] - 1);k = k + M)
						for (int j = 0; j < E_S[s].size();j++)
							if (E_S[s][j][0] == k)
								for (int h = 1;h < E_S[s][j].size();h++)
									if (E_S[s][j][h] == k + M)
										tmp2 += y_S[s][k][E_S[s][j][h]];
					model.addConstr(time_S[m][m] * tmp1 - tmp2 <= 0, "#6_Sea_5");
				}

		//Air Route Constraints
		for (int a = 0;a < A;a++)
		{
			GRBLinExpr tmp = 0;
			for (int i = 0;i < B_A[a].size();i++)
				if (B_A[a][i] < N * 7 * T)
					for (int j = 0;j < E_A[a].size();j++)
						if (B_A[a][i] == E_A[a][j][0])
							tmp += y_A[a][E_A[a][j][0]][E_A[a][j][1]];
			model.addConstr(tmp == info_A[a].freq, "#7_Air_1");
		}

		for (int a = 0;a < A;a++)
		{
			for (int i = 0;i < B_A[a].size();i++)
				if (B_A[a][i] < L*N - N * info_A[a].t)
				{
					GRBLinExpr tmp1 = 0, tmp2 = 0;
					for (int j = 0;j < E_A[a].size();j++)
					{
						if (B_A[a][i] == E_A[a][j][0])
							tmp1 += y_A[a][E_A[a][j][0]][E_A[a][j][1]];
						if (B_A[a][i] + N * info_A[a].t == E_A[a][j][1])
							tmp2 += y_A[a][E_A[a][j][0]][E_A[a][j][1]];
					}
					model.addConstr(tmp1 == tmp2, "#Extra_Air_1.1");
				}
		}

		for (int a = 0;a < A;a++)
		{
			GRBLinExpr tmp = 0;
			for (int i = 0;i < B_A[a].size();i++)
				if (B_A[a][i] >= N * info_A[a].t && B_A[a][i] < N *(7 * T + info_A[a].t))
					for (int k = 0;k < E_A[a].size();k++)
							if (E_A[a][k][1] == B_A[a][i])
								tmp += y_A[a][E_A[a][k][0]][B_A[a][i]];
			model.addConstr(tmp == info_A[a].freq, "#8_Air_2");
		}

		for (int a = 0;a < A;a++)
			for (int i = 0;i < V_A[a].size();i++)
			{
				GRBLinExpr tmp1 = 0, tmp2 = 0;
				int count1 = 0, count2 = 0;
				for (int k = 0;k < B_A[a].size();k++)
					if (V_A[a][i] == B_A[a][k])
						goto pass1;

				for (int j = 0; j < E_A[a].size();j++)
					if (E_A[a][j][0] == V_A[a][i])
					{
						tmp1 += y_A[a][V_A[a][i]][E_A[a][j][1]];
						count1++;
					}
				for (int k = 0;k < E_A[a].size();k++)
					if (E_A[a][k][1] == V_A[a][i])
					{
						tmp2 += y_A[a][E_A[a][k][0]][V_A[a][i]];
						count2++;
					}
				//if (count1 != 0 && count2 != 0)
					model.addConstr(tmp1 - tmp2 == 0, "#9_Air_3");

			pass1:
				tmp1 = 0;tmp2 = 0;
			}

		for (int a = 0;a < A;a++)
			for (int i = 0;i < E_A[a].size();i++)
				if (E_A[a][i][0] < (L - 7 * T)*N&&E_A[a][i][1] < (L - 7 * T)*N)
					model.addConstr(y_A[a][E_A[a][i][0]][E_A[a][i][1]] == y_A[a][E_A[a][i][0] + N * 7 * T][E_A[a][i][1] + N * 7 * T], "#10_Air_4");

		for (int a = 0;a < Am;a++)
			for (int m = 0;m < R_A[a].size();m++)
				for (int i = 0;i < E_Am[a][m].size();i++)
					if(E_Am[a][m][i][0] + (N * info_A[a + (A - Am)].gap) < (m + info_A[a + A - Am].FT)*N&&E_Am[a][m][i][1] + (N * info_A[a + (A - Am)].gap) < (m + info_A[a + A - Am].FT)*N)
					model.addConstr(y_A[a + (A - Am)][E_Am[a][m][i][0]][E_Am[a][m][i][1]] <= y_A[a + (A - Am)][E_Am[a][m][i][0] + N * info_A[a + (A - Am)].gap][E_Am[a][m][i][1] + N * info_A[a + (A - Am)].gap] + (1 - o[a][R_A[a][m]]), "#11_Air_5");

		for (int a = 0;a < Am;a++)
			for (int m = 0;m < R_A[a].size();m++)
				for (int i = 0;i < E_Am[a][m].size();i++)
					model.addConstr(y_A[a + (A - Am)][E_Am[a][m][i][0] + N * info_A[a + (A - Am)].gap][E_Am[a][m][i][1] + N * info_A[a + (A - Am)].gap] - (1 - o[a][R_A[a][m]]) <= y_A[a + (A - Am)][E_Am[a][m][i][0]][E_Am[a][m][i][1]], "#12_Air_6");

		for (int a = 0;a < Am;a++)
			for (int m = 0;m < R_A[a].size();m++)
				for (int k = 0;k < E_A[a].size();k++)
					if (E_A[a][k][0] < (7 * T*N) && E_A[a][k][1] < (7 * N*T))
						if (E_A[a][k][0] < N*m || E_A[a][k][0] >= (m + info_A[a + A - Am].FT)*N)
							if (E_A[a][k][1] < N*m || E_A[a][k][1] >= (m + info_A[a + A - Am].FT)*N)
								model.addConstr(y_A[a + (A - Am)][E_A[a][k][0]][E_A[a][k][1]] <= 1 - o[a][R_A[a][m]], "#13_Air_7");

		for (int a = 0;a < Am;a++)
		{
			GRBLinExpr tmp = 0;
			for (int m = 0;m < R_A[a].size();m++)
				tmp += o[a][R_A[a][m]];
			model.addConstr(tmp == 1, "#14_Air_8");
		}

		//Cargo Constraints		
		for (int k = 0;k < K;k++)
			for (int p = 0;p < P_AllCargo.size();p++)
			{
				for (int i = 0;i < P_Design[k].size();i++)
					if (p == P_Design[k][i])
						for (int h = 0;h < P_All[k].size();h++)
							if (P_All[k][h] == P_Design[k][i])
								model.addConstr(u[k][h] <= x[p], "#15_Cargo_1");
				for (int i = 0;i < P_CurrentTA[k].size();i++)
					if (p == P_CurrentTA[k][i])
						for (int h = 0;h < P_All[k].size();h++)
							if (P_All[k][h] == P_CurrentTA[k][i])
								model.addConstr(u[k][h] <= x[p], "#15_Cargo_1");
			}
		
		for (int k = 0;k < K;k++)
			for (int p = 0;p < P_CurrentTA[k].size();p++)
			{
				model.addConstr(x[P_CurrentTA[k][p]] == 1, "#24_Cargo_2");
			}

		for (int k = 0;k < K;k++)
		{
			GRBLinExpr tmp1 = 0, tmp2 = 0;
			for (int i = 0;i < P_CurrentC[k].size();i++)
				tmp1 += z_C[k][i];
			for (int i = 0;i < P_Design[k].size();i++)
				tmp2 += z_TA[k][i];
			for (int i = 0;i < P_CurrentTA[k].size();i++)
				tmp2 += z_TA[k][i+ P_Design[k].size()];
			model.addConstr(tmp1 +tmp2 == 1, "#30_Cargo_3");
		}

		for (int k = 0;k < K;k++)
			for (int j = 0;j < P_All[k].size();j++)
			{
				for (int i = 0;i < P_Design[k].size();i++)
					if (P_All[k][j] == P_Design[k][i])
						model.addConstr(z_TA[k][i] <= u[k][j], "#31_Cargo_4");
				for (int i = 0;i < P_CurrentTA[k].size();i++)
					if (P_All[k][j] == P_CurrentTA[k][i])
						model.addConstr(z_TA[k][i + P_Design[k].size()] <= u[k][j], "#31_Cargo_4");
			}

		for (int k = 0;k < K;k++)
			for (int j = 0;j < P_All[k].size();j++)
			{
				for (int i = 0;i < P_Design[k].size();i++)
					if (P_All[k][j] == P_Design[k][i])
					{
						GRBLinExpr tmp1 = 0, tmp2 = 0;
						for (int h = 0;h < P_CurrentC[k].size();h++)
							tmp1 += p_C[k][h];
						for (int h = 0;h < P_CurrentC[k].size();h++)
							tmp2 += z_C[k][h];
						model.addConstr(tmp1* z_TA[k][i] <= p_TA[k][i] * tmp2, "#32_Cargo_5");
					}
				for (int i = 0;i < P_CurrentTA[k].size();i++)
					if (P_All[k][j] == P_CurrentTA[k][i])
					{
						GRBLinExpr tmp1 = 0, tmp2 = 0;
						for (int h = 0;h < P_CurrentC[k].size();h++)
							tmp1 += p_C[k][h];
						for (int h = 0;h < P_CurrentC[k].size();h++)
							tmp2 += z_C[k][h];
						model.addConstr(tmp1 * z_TA[k][i + P_Design[k].size()] <= p_TA[k][i + P_Design[k].size()] * tmp2, "#32_Cargo_5");
					}
			}

		for (int k = 0;k < K;k++)
			for (int j = 0;j < P_All[k].size();j++)
			{
				for (int i = 0;i < P_Design[k].size();i++)
					if (P_All[k][j] == P_Design[k][i])
					{
						GRBLinExpr tmp1 = 0, tmp2 = 0;
						for (int h = 0;h < P_CurrentC[k].size();h++)
							tmp1 += p_C[k][h];
						for (int h = 0;h < P_CurrentC[k].size();h++)
							tmp2 += z_C[k][h];
						model.addConstr(tmp1  * z_TA[k][i] >= p_TA[k][i] * tmp2 + u[k][j] - 1, "#33_Cargo_6");
					}
				for (int i = 0;i < P_CurrentTA[k].size();i++)
					if (P_All[k][j] == P_CurrentTA[k][i])
					{
						GRBLinExpr tmp1 = 0, tmp2 = 0;
						for (int h = 0;h < P_CurrentC[k].size();h++)
							tmp1 += p_C[k][h];
						for (int h = 0;h < P_CurrentC[k].size();h++)
							tmp2 += z_C[k][h];
						model.addConstr(tmp1* z_TA[k][i + P_Design[k].size()] >= p_TA[k][i + P_Design[k].size()] * tmp2 + u[k][j] - 1, "#33_Cargo_6");
					}
			}

		for (int s = Sc;s < Sn;s++)
			for (int i = 0;i < E_Sn[s].size();i++)
			{
				GRBLinExpr tmp = 0;
				for (int k = 0;k < K;k++)
					for (int p = 0;p < P_CurrentTA[k].size();p++)
						tmp += PArc[P_CurrentTA[k][p]][s + S + A][E_Sn[s][i][0]][E_Sn[s][i][1]] * cargo[k].vol*z_TA[k][p + P_Design[k].size()];
				model.addConstr(tmp <= TA_S[s].vol, "#39_Cargo_7");
			}

		for (int a = Ac;a < An;a++)
			for (int i = 0;i < E_An[a].size();i++)
			{
				GRBLinExpr tmp = 0;
				for (int k = 0;k < K;k++)
					for (int p = 0;p < P_CurrentTA[k].size();p++)
						tmp += PArc[P_CurrentTA[k][p]][a + S + A + 1][E_An[a][i][0]][E_An[a][i][1]] * cargo[k].vol*z_TA[k][p + P_Design[k].size()];
				model.addConstr(tmp <= TA_A[a].vol, "#40_Cargo_8");
			}

		for (int a = Ac;a < An;a++)
			for (int i = 0;i < E_An[a].size();i++)
				{
					GRBLinExpr tmp = 0;
					for (int k = 0;k < K;k++)
						for (int p = 0;p < P_CurrentTA[k].size();p++)
							tmp += PArc[P_CurrentTA[k][p]][a + S + A + 1][E_An[a][i][0]][E_An[a][i][1]] * cargo[k].wei*z_TA[k][p + P_Design[k].size()];
					model.addConstr(tmp <= TA_A[a].wei, "#41_Cargo_9");
				}

		//Complex Constraints
		for (int p = 0;p < P_AllCargo.size();p++)
			for (int s = 0;s < S;s++)
				for (int k = 0;k < E_S[s].size();k++)
						if (PArc[p][s][E_S[s][k][0]][E_S[s][k][1]]==1)
							model.addConstr(y_S[s][E_S[s][k][0]][E_S[s][k][1]] >= x[p], "#22_Complex_1");

		for (int p = 0;p < P_AllCargo.size();p++)
			for (int a = 0;a < A;a++)
				for (int k = 0;k < E_A[a].size();k++)
					if (PArc[a + S][0][E_A[a][k][0]][E_A[a][k][1]] == 1)
						model.addConstr(y_A[a][E_A[a][k][0]][E_A[a][k][1]] >= x[p], "#23_Complex_2");

		for (int s = 0;s < S;s++)
			for (int i = 0;i < E_S[s].size();i++)
			{
				GRBLinExpr tmp = 0;
				for (int k = 0;k < K;k++)
					for (int p = 0;p < P_Design[k].size();p++)
						tmp += PArc[P_Design[k][p]][s][E_S[s][i][0]][E_S[s][i][1]] * cargo[k].vol*z_TA[k][p];
				model.addConstr(tmp <= info_S[s].vol * y_S[s][E_S[s][i][0]][E_S[s][i][1]], "#42_Complex_3");
			}

		for (int a = 0;a < A;a++)
			for (int i = 0;i < E_A[a].size();i++)
				{
					GRBLinExpr tmp = 0;
					for (int k = 0;k < K;k++)
						for (int p = 0;p < P_Design[k].size();p++)
							tmp += PArc[P_Design[k][p]][a+S][E_A[a][i][0]][E_A[a][i][1]] * cargo[k].vol*z_TA[k][p];
					model.addConstr(tmp <= info_A[a].vol * y_A[a][E_A[a][i][0]][E_A[a][i][1]], "#43_Complex_4");
				}

		for (int a = 0;a < A;a++)
			for (int i = 0;i < E_A[a].size();i++)
				for (int j = 1; j < E_A[a][i].size();j++)
				{
					GRBLinExpr tmp = 0;
					for (int k = 0;k < K;k++)
						for (int p = 0;p < P_Design[k].size();p++)
							tmp += PArc[P_Design[k][p]][a+S][E_A[a][i][0]][E_A[a][i][1]] * cargo[k].wei*z_TA[k][p];
					model.addConstr(tmp <= info_A[a].wei * y_A[a][E_A[a][i][0]][E_A[a][i][1]], "#44_Complex_5");
				}/**/

		// Optimize model
		model.optimize();
		time_t t2 = time(NULL);
		//要輸出設計的海運航線(y_S)、設計的空運航線(y_A)、貨物走每條path的比例(z_C & TA)和那條path上的arc(A_P)、還有總共的利潤(Obj)
		cout << "Needed Time: " << t2-t1 << endl;
		cout << "Total Profit: " << model.get(GRB_DoubleAttr_ObjVal) << endl;
		cout << "Sea_Route(s):" << endl;
		for (int s = 0;s < S;s++)
		{
			cout << "Ship " << s + 1 << ":" << endl;
			for (int i = 0;i < E_S[s].size();i++)
				if (y_S[s][E_S[s][i][0]][E_S[s][i][1]].get(GRB_DoubleAttr_X) == 1)
					cout << "Port: " << E_S[s][i][0] % 4 << " , Time: " << E_S[s][i][0] / 4 << "\t=>\t " << "Port: " << E_S[s][i][1] % 4 << " , Time: " << E_S[s][i][1] / 4 << endl;
			cout << "=================================================" << endl;
		}
		cout << "Air_Route(s):" << endl;
		for (int a = 0;a < A;a++)
		{
			cout << "Aircraft " << a + 1 << ":" << endl;
			for (int i = 0;i < E_A[a].size();i++)
				if (y_A[a][E_A[a][i][0]][E_A[a][i][1]].get(GRB_DoubleAttr_X) > 0)
					cout << "Port: " << E_A[a][i][0] % 4 << " , Time: " << E_A[a][i][0] / 4 << "\t=>\t" << "Port: " << E_A[a][i][1] % 4 << " , Time: " << E_A[a][i][1] / 4 << endl;
			cout << "=================================================" << endl;
		}
		for (int k = 0;k < K;k++)
			for (int i = 0;i < P_All[k].size();i++)
				if (u[k][i].get(GRB_DoubleAttr_X) > 0.9)
					cout << k << " ; " << P_All[k][i] << endl;
		cout << "=================================================" << endl;
		for (int k = 0;k < K;k++)
			for (int i = 0;i < P_Design[k].size() + P_CurrentTA[k].size();i++)
				if (z_TA[k][i].get(GRB_DoubleAttr_X) > 0.0001)
					if (i < P_Design[k].size())
						cout << k << " ; " << P_Design[k][i] << " ; " << z_TA[k][i].get(GRB_DoubleAttr_X) << endl;
					else
						cout << k << " ; " << P_CurrentTA[k][i-P_Design[k].size()] << " ; " << z_TA[k][i].get(GRB_DoubleAttr_X) << endl;
		cout << "=================================================" << endl;
		for (int k = 0;k < K;k++)
			for (int i = 0;i < P_CurrentC[k].size();i++)
				if (z_C[k][i].get(GRB_DoubleAttr_X) > 0.0001)
					cout << k << " ; " << P_CurrentC[k][i] << " ; " << z_C[k][i].get(GRB_DoubleAttr_X) << endl;
		cout << "=================================================" << endl;
		for (int k = 0;k < K;k++)
		{
			
			int count4 = 0;
			cout << "Cargo " << k + 1 << ":" << endl;
			for (int i = 0;i < P_All[k].size();i++)
			{				
				for (int h = 0;h < P_Design[k].size();h++)
				{
					if(P_All[k][i] == P_Design[k][h])
					if (u[k][i].get(GRB_DoubleAttr_X) > 0.9 && z_TA[k][h].get(GRB_DoubleAttr_X) > 0.0001)
					{
						count4++;
						cout << "Path " << count4 << ": " << z_TA[k][h].get(GRB_DoubleAttr_X) << endl;
						for (int j = 0;j < A_P[P_All[k][i]].size();j++)
						{
							if (A_P[P_All[k][i]][j] < L*M*S)
								cout << "0:" << A_P[P_All[k][i]][j] % M << ":" << A_P[P_All[k][i]][j] / M<< "\t=>";
							else if (A_P[P_All[k][i]][j] < L*M*(S + A))
								cout << "1:" << A_P[P_All[k][i]][j] % M << ":" << (A_P[P_All[k][i]][j] - L * M*S) / M << "\t=>";
							else if (A_P[P_All[k][i]][j] < L*M*(A + S + 1))
								cout << "2:" << A_P[P_All[k][i]][j] % M << ":" << (A_P[P_All[k][i]][j] - L * M*(S + A)) / M << "\t=>";
							else if (A_P[P_All[k][i]][j] < L*M*(Sn + A + S + 1 - Sc))
								cout << "3:" << A_P[P_All[k][i]][j] % M << ":" << (A_P[P_All[k][i]][j] - L * M*(A + S + 1)) / M << "\t=>";
							else if (A_P[P_All[k][i]][j] < L*M*(Sn + An + S + A + 1 - Sc - Ac))
								cout << "4:" << A_P[P_All[k][i]][j] % M << ":" << (A_P[P_All[k][i]][j] - L * M*(Sn + A + S + 1 - Sc)) / M << "\t=>";
							else if (A_P[P_All[k][i]][j] < L*M*(Sn + An + S + A + 1 - Ac))
								cout << "5:" << A_P[P_All[k][i]][j] % M << ":" << (A_P[P_All[k][i]][j] - L * M*(Sn + An + S + A + 1 - Sc - Ac)) / M << "\t=>";
							else if (A_P[P_All[k][i]][j] < L*M*(Sn + An + S + A + 1))
								cout << "6:" << A_P[P_All[k][i]][j] % M << ":" << (A_P[P_All[k][i]][j] - L * M*(Sn + An + S + A + 1 - Ac)) / M << "\t=>";
						}
						cout << endl<<"----------------------------------------" << endl;
					}
				}
				for (int h = 0;h < P_CurrentTA[k].size();h++)
				{
					if (P_All[k][i] == P_CurrentTA[k][h])
					if (u[k][i].get(GRB_DoubleAttr_X) > 0.9 && z_TA[k][h + P_Design[k].size()].get(GRB_DoubleAttr_X) > 0.0001)
					{
						count4++;
						cout << "Path " << count4 << ": " << z_TA[k][h + P_Design[k].size()].get(GRB_DoubleAttr_X) << endl;
						for (int j = 0;j < A_P[P_All[k][i]].size();j++)
						{
							if (A_P[P_All[k][i]][j] < L*M*S)
								cout << "0:" << A_P[P_All[k][i]][j] % M << ":" << A_P[P_All[k][i]][j] / M << "=>";
							else if (A_P[P_All[k][i]][j] < L*M*(S + A))
								cout << "1:" << A_P[P_All[k][i]][j] % M << ":" << (A_P[P_All[k][i]][j] - L * M*S) / M << "=>";
							else if (A_P[P_All[k][i]][j] < L*M*(A + S + 1))
								cout << "2:" << A_P[P_All[k][i]][j] % M << ":" << (A_P[P_All[k][i]][j] - L * M*(S + A)) / M << "=>";
							else if (A_P[P_All[k][i]][j] < L*M*(Sn + A + S + 1 - Sc))
								cout << "3:" << A_P[P_All[k][i]][j] % M << ":" << (A_P[P_All[k][i]][j] - L * M*(A + S + 1)) / M << "=>";
							else if (A_P[P_All[k][i]][j] < L*M*(Sn + An + S + A + 1 - Sc - Ac))
								cout << "4:" << A_P[P_All[k][i]][j] % M << ":" << (A_P[P_All[k][i]][j] - L * M*(Sn + A + S + 1 - Sc)) / M << "=>";
							else if (A_P[P_All[k][i]][j] < L*M*(Sn + An + S + A + 1 - Ac))
								cout << "5:" << A_P[P_All[k][i]][j] % M << ":" << (A_P[P_All[k][i]][j] - L * M*(Sn + An + S + A + 1 - Sc - Ac)) / M << "=>";
							else if (A_P[P_All[k][i]][j] < L*M*(Sn + An + S + A + 1))
								cout << "6:" << A_P[P_All[k][i]][j] % M << ":" << (A_P[P_All[k][i]][j] - L * M*(Sn + An + S + A + 1 - Ac)) / M << "=>";
						}
						cout <<endl<< "----------------------------------------" << endl;
					}
				}
			}
			
			cout << "=================================================" << endl;
		}/**/
	}
	catch (GRBException ee) {
		cout << "Error code = " << ee.getErrorCode() << endl;
		cout << ee.getMessage() << endl;
	}
	catch (...) {
		cout << "Exception during optimization" << endl;
	}
	system("pause");
	return 0;
}


