//
// Created by Ashee on 2019/7/7.
//

/*
	This is for reading the file into the program

	usage:
		read_files.cpp

	input files:
		f_glob_var.txt
		f_ports.txt
		f_line.txt
		f_k.txt
		f_graph.txt
		f_Gs_arc.txt
		f_Ga_arc.txt
		f_Gd_arc.txt
		f_eGs_arc.txt   //v200
		f_eGa_arc.txt   //v200
		info_k_path.txt
		info_scenario.txt

	output files:
		none

	compile:
		gcc -o read_files read_files.cpp

	version:
		v100:   origin
		v101:   void reverse_arc_sort(Graph *G);    //arc sorting by head
		void input_points(Graph *G);        //arc forward and reverse star point
		v102:   void arc_sort(Graph *G);            //arc sorting by tail
		v103:   struct design_a_flight{+ int **R)   //the potential pattern for flight a
		void r_lines_var{+ calculate the elements in R}
		v200
		v207 revised q
*/

#include "gurobi_c++.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <stdlib.h>
#include <random>
#include <chrono>

//using namespace std;
using std::cout;
using std::cin;
using std::get;
using std::ofstream;
using std::fstream;
using std::ifstream;
using std::ios;
using std::stringstream;
using std::endl;
using std::string;
using std::to_string;

#define M 99999
#define N 3
#define PERIOD 3
#define TOTALG 5
#define small 1
#define CURRENCY 0.8

//function for naming constraints
string itos(int i)
{
    stringstream s;
    s << i;
    return s.str();
};
/*basic struct*/
struct linklist
{
    int node;
    struct linklist *next;
};
typedef struct linklist Linklist;

struct record_path
{
    int k;  //commodity k
    int q;  //the qth group in commodity k
    //int i;  //the ith element in qth group
    int p;  //the no. path in kq
    struct record_path *next;
}; typedef struct record_path recordPath;

struct node_set
{
    struct linklist origin;	//the origins (origin[q][i]: the ith element in the qth set in Ok)
    struct linklist dest;	//the corresponding destinations
    struct node_set *next_set;
};
typedef struct node_set nodeSet;

/*arc struct*/
struct arc_set
{
    int tail;	//arc from, i
    int head;	//arc to, j
    int t;      //traveling time from i to j
    double r;	//margin revenue per volume(TEU?) transported on arc(i,j) in G[S], G[A] & G[E]
    double v_c; //variable cost per volume transported on arc(i,j) in G[S], G[A] & G[E]
    double fo_c;//fixed operating cost on arc (i,j)	in G[S] & G[A]
    double l_c;	//unit loading and unloading cost on arc (i,j) in G[S] & G[A]
    double s_c;	//unit holding and transporting cost on arc(i,j) in G[D]
    struct record_path inPath; //record the arc in which path
    struct record_path *rear;   //record the last one now;
};
typedef struct arc_set arcSet;

/*port struct*/
struct port
{
    double fb_c;	//fixed berthing(landing) charge at port(airport) i
    int s_t;	    //the minimal service time at port i
    double cap;     //the capacity for port i
};
typedef struct port Port;

/*design lines struct*/
struct design_s_line
{
    int cyc_d;	//cycle time for line s (measure in day)
    int cyc_t;	//cycle time for line s (measure in t)
    int n_voy;	//number of voyage during the planning horizon (= horizon/cyc_t)
    double cap_v;	//the capacity w.r.t volume for line s
    struct linklist *h_port;//the home port(departure nodes) for line s

};
typedef struct design_s_line designS;

struct design_a_flight
{
    int cyc_h;	//cycle time for flight a (measure in hour)
    int cyc_t;	//cycle time for flight a (measure in t)
    int freq;	//frequency for flight a per weeek
    int gap;	//the gap time (measure in t) between two flight for flight a
    double cap_v;	//the capacity w.r.t volume for flight a
    double cap_u;	//the capacity w.r.t weight for flight a
    int b_port;	//the based airport for flight a
    int **R;    //the potential pattern for flight a, v103
};
typedef struct design_a_flight designA;

struct small_k
{
    int num_p;
    double *path_cost;
    int *path_time;
    int *path_no;
    struct linklist *paths;
    double **estimate;
    double **path_utility;

}; typedef struct small_k SmallK;

/*commodity struct*/
struct commodity
{
    double d;			//the demand volume
    double w;			//the weight
    struct node_set *q;
    double b;           //the weight per volume
    double omega;       //the calculate parameter for air flight
    double apha;        //the utility per cost      //v200
    double beta;        //the utility per time unit //v200
    struct small_k *kq;
    //double **estimate;
    //double **path_utility;
    //double *scenarios;
};
typedef struct commodity Commodity;


/*graph struct*/
struct graph
{
    int n_node;			//number of nodes in network
    int n_arcs;			//number of arcs in network
    int n_line;			//number of desgin line in network
    int *forward_point;	//forward star for arcs
    int *reverse_point;	//reverse star for arcs
    int *arc_re_no;     //the arc no. in reverse star
    struct arc_set *arc;//the arc set in network
};
typedef struct graph Graph;

//v200
struct e_line
{
    int cyc_t;
    int n_voy;
    int depart_node;
    double cap_v;       //the volume capacity for existing line (S&A)
    double cap_u;       //the weight capacity for existing flight(A)
    int n_arc;          //the number of arcs in line e, in order to create the arc_no_in_G
    struct arc_set *arc;   //in order to point to the arc no in G[E]
    int *forward_point;
    int *reverse_point;
    int *arc_re_no;
};
typedef struct e_line eL;


/*functions*/
void r_global_var(string s);
void r_port_var(string s, Port *s_P, Port *a_P);
void r_lines_var(string s, designS *S, designA *A);
void r_k_var(string s, Commodity *k);
void r_graph_info(string s, Graph *G);
void r_arc_info(string s, Graph *G);
void arc_sort(Graph *G);            //v102
void reverse_arc_sort(Graph *G);    //v101
void input_points(Graph *G);        //v101
void r_e_arc(string s, eL *e, int num_l);       //v200
void e_arc_sort(eL *e, int n_line);             //v200
void reverse_e_arc_sort(eL *e, int n_line);     //v200
void input_e_points(eL *e, int n_line, int sORa); //v200
void r_k_path(string s, Commodity *k);
int check_ij_path_time(int k, int q, int p, int tail, int head); //v200
void r_scenario(string s);
double check_ij_path_cost(int k, int q, int p, int tail, int head);
void generate_random_e(double mean, double stdv);
void calculate_utility();

Graph* constr_grap(int n, Graph *g);
Commodity* constr_commodities(int n, int max_q, Commodity *k);
Port* constr_ports(int n, Port *p);
designS* constr_sL(int n, designS *s);
designA* constr_aL(int n, designA *a);
eL* constr_eL(int n, eL *e);


/*global variable*/
//problem information & initialize
int sPort = 0;	//number of candidate seaport
int aPort = 0;	//number of candidate airport
int n_k = 0;	//number of commodity
int n_total = 0;//number of total terminals

int sLine = 0;	//number of design sea line
int aLine = 0;	//number of design air flight
int e_sLine = 0;//number of existing sea line
int e_aLine = 0;//number of existing air flight
int my_sLine = 0;//number of my existing sea line
int my_aLine = 0;//number of my existing air flight

int lcm = 0;	//the lcm of design maritime line (measure in t)
int times = 0;	//how many times the lcm continue
int L = 0;		//planning horizon (= lcm*times)
int max_q = 0;	//the max cycle time in desgin maritime line

int total_path = 0;
int num_scenario = 0;
int n_voy = 0;

/*set parameters*/
Port *s_P;		//candidate sea port
Port *a_P;		//candidate airport
designS *S;		//design maritime lines
designA *A;		//desgin airline
Commodity *K;	//commodity
Graph *G;		//graph for maritime network(G[0]), air network(G[1]), dummy network(G[2]), existing maritime G[3], existing airline G[4]
eL *e_S;        //the existing sea lines
eL *e_A;        //the existing air flights
recordPath *tPath;     //the total path set

int main(int arge, char *argv[])
{
    /*construct variables*/
    r_global_var("../model_data/f_global_var.txt");
    G = constr_grap(N, G);
    K = constr_commodities(n_k, max_q, K);
    s_P = constr_ports(sPort, s_P);
    a_P = constr_ports(aPort, a_P);
    S = constr_sL(sLine, S);
    A = constr_aL(aLine, A);
    e_S = constr_eL(e_sLine, e_S);  //v200
    e_A = constr_eL(e_aLine, e_A);  //v200

    /*reads files*/
    r_port_var("../model_data/f_ports.txt", s_P, a_P);
    r_graph_info("../model_data/f_graph.txt", G);
    r_lines_var("../model_data/f_line.txt", S, A);
    r_k_var("../model_data/f_k.txt", K);
    r_arc_info("../model_data/f_Gs_arc.txt", G);
    r_arc_info("../model_data/f_Ga_arc.txt", G);
    r_arc_info("../model_data/f_Gd_arc.txt", G);
    arc_sort(G);            //v102
    reverse_arc_sort(G);    //v101
    input_points(G);        //v101
    r_e_arc("../model_data/f_eGs_arc.txt", e_S, e_sLine); //v200
    r_e_arc("../model_data/f_eGa_arc.txt", e_A, e_aLine); //v200
    e_arc_sort(e_S, e_sLine);
    e_arc_sort(e_A, e_aLine);
    reverse_e_arc_sort(e_S, e_sLine);
    reverse_e_arc_sort(e_A, e_aLine);
    input_e_points(e_S, e_sLine, 0);
    input_e_points(e_A, e_aLine, 1);
    r_k_path("../model_data/info_k_path.txt", K);
    //n_voy = lcm * (times - 1) / (max_q * 7 * PERIOD);
    generate_random_e(0, 1);
    r_scenario("../model_data/info_scenario.txt");
    calculate_utility();

    cout << "Files are read!\n";

    /*set the Gurobi environment*/
    try{
        GRBEnv env = GRBEnv();
        GRBModel model = GRBModel(env);

        //model.set(GRB_DoubleParam_NodefileStart,0.2);
        //model.set(GRB_IntParam_Threads, 2);

        /*test*/
        ofstream outconstr("../model_data/constr.txt");
        ofstream fout("../model_data/records.txt");

        ofstream fbefore("../model_data/test_before.txt");
        ofstream fafter("../model_data/test_after.txt");

        //region Variable
        //y[network][arc][line]
        GRBVar ***y = new GRBVar **[N - 1];
        for (int n = 0; n < N - 1; n++) {
            y[n] = new GRBVar *[G[n].n_arcs];

            for (int a = 0; a < G[n].n_arcs; a++) {
                y[n][a] = new GRBVar[G[n].n_line];
            }
        }

        for (int n = 0; n < N - 1; n++) {
            for (int a = 0; a < G[n].n_arcs; a++) {
                for (int l = 0; l < G[n].n_line; l++) {
                    string s;
                    stringstream s_tail, s_head;
                    s_tail << G[n].arc[a].tail;
                    s_head << G[n].arc[a].head;
                    s = "y[" + itos(n) + "][" + s_tail.str() + "][" + s_head.str() + "][" + itos(l) + "]";
                    //y[network][tail][head][line]
                    y[n][a][l] = model.addVar(0, 1, 0, GRB_BINARY, s);
                }
            }
        }

        //z[k][q]
        GRBVar **z = new GRBVar *[n_k];
        for (int k = 0; k < n_k; k++) {
            z[k] = new GRBVar[max_q];
        }

        for (int k = 0; k < n_k; k++) {
            for (int q = 0; q < max_q; q++) {
                string s;
                s = "z[" + itos(k) + "][" + itos(q) + "]";
                //z[commodity][q]
                z[k][q] = model.addVar(0, 1, 0, GRB_BINARY, s);
            }
        }

        //Q[a][r]
        int number_of_r = 0;
        GRBVar **Q = new GRBVar *[G[1].n_line];
        for (int l = 0; l < G[1].n_line; l++) {
            number_of_r = 7 - (A[l].gap / PERIOD) * (A[l].freq - 1);
            Q[l] = new GRBVar[number_of_r];
        }
        for (int l = 0; l < G[l].n_line; l++) {
            number_of_r = 7 - (A[l].gap / PERIOD) * (A[l].freq - 1);
            for (int r = 0; r < number_of_r; r++) {
                string s;
                s = "Q[" + itos(l) + "][" + itos(r) + "]";
                Q[l][r] = model.addVar(0, 1, 0, GRB_BINARY, s);
            }
        }
        cout << "y, z, Q are created!\n";

        //f[k][q][p] ---- flow on path p
        //E[k][q][p][r] ---- total logistic cost on path p
        //U[k][q][p][r] ---- utility on path p in scenario r
        //W[k][q][p][r] ----choose path p in scenario r
        //PH[k][q][p][r] --- avaliable path
        //PH_2[k][q][p][r][network][arc] ---- avaliable arc(volume)
        //PH_3[k][q][p][r][network][arc] ---- avaliable arc(weight)
        GRBVar ***f = new GRBVar **[n_k];
        //GRBVar ****E = new GRBVar ***[n_k];
        //GRBVar ****U = new GRBVar ***[n_k];
        GRBVar ****W = new GRBVar ***[n_k];
        GRBVar ****PH = new GRBVar ***[n_k];
        GRBVar ******PH_2 = new GRBVar *****[n_k];
        GRBVar ******PH_3 = new GRBVar *****[n_k];
        for (int k = 0; k < n_k; k++) {
            f[k] = new GRBVar *[max_q];
            //E[k] = new GRBVar **[max_q];
            //U[k] = new GRBVar **[max_q];
            W[k] = new GRBVar **[max_q];
            PH[k] = new GRBVar **[max_q];
            PH_2[k] = new GRBVar ****[max_q];
            PH_3[k] = new GRBVar ****[max_q];

            for (int q = 0; q < max_q; q++) {

                f[k][q] = new GRBVar[total_path];
                //E[k][q] = new GRBVar *[total_path];
                //U[k][q] = new GRBVar *[total_path];
                W[k][q] = new GRBVar *[total_path];
                PH[k][q] = new GRBVar *[total_path];
                PH_2[k][q] = new GRBVar ***[total_path];
                PH_3[k][q] = new GRBVar ***[total_path];

                for (int p = 0; p < total_path; p++) {
                    string s;
                    s = "f[" + itos(k) + "][" + itos(q) + "][" + itos(p) + "]";
                    f[k][q][p] = model.addVar(0, GRB_INFINITY, 0, GRB_CONTINUOUS, s);

                    //E[k][q][p] = new GRBVar[num_scenario];
                    //U[k][q][p] = new GRBVar[num_scenario];
                    W[k][q][p] = new GRBVar[num_scenario];
                    PH[k][q][p] = new GRBVar[num_scenario];
                    PH_2[k][q][p] = new GRBVar **[num_scenario];
                    PH_3[k][q][p] = new GRBVar **[num_scenario];
                    for (int r = 0; r < num_scenario; r++) {
                        //s = "E[" + itos(k) + "][" + itos(q) + "][" + itos(p) + "][" + itos(r) + "]";
                        //E[k][q][p][r] = model.addVar(0, GRB_INFINITY, 0, GRB_CONTINUOUS, s);

                        //s = "U[" + itos(k) + "][" + itos(q) + "][" + itos(p) + "][" + itos(r) + "]";
                        //U[k][q][p][r] = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0, GRB_CONTINUOUS, s);

                        s = "W[" + itos(k) + "][" + itos(q) + "][" + itos(p) + "][" + itos(r) + "]";
                        W[k][q][p][r] = model.addVar(0, 1, 0, GRB_BINARY, s);

                        s = "PH[" + itos(k) + "][" + itos(q) + "][" + itos(p) + "][" + itos(r) + "]";
                        PH[k][q][p][r] = model.addVar(0, 1, 0, GRB_BINARY, s);

                        PH_2[k][q][p][r] = new GRBVar *[TOTALG];
                        PH_3[k][q][p][r] = new GRBVar *[TOTALG];
                        for (int n = 0; n < TOTALG; n++) {
                            if (n < N - 1) {
                                PH_2[k][q][p][r][n] = new GRBVar[G[n].n_arcs];
                                PH_3[k][q][p][r][n] = new GRBVar[G[n].n_arcs];
                                for (int a = 0; a < G[n].n_arcs; a++) {
                                    s = "PH_2[" + itos(k) + "][" + itos(q) + "][" + itos(p) + "][" + itos(r) + "][" + itos(n) + "][" + itos(a) + "]";
                                    PH_2[k][q][p][r][n][a] = model.addVar(0, 1, 0, GRB_BINARY, s);
                                    //fout << s << endl;

                                    s = "PH_3[" + itos(k) + "][" + itos(q) + "][" + itos(p) + "][" + itos(r) + "][" + itos(n) + "][" + itos(a) + "]";
                                    PH_3[k][q][p][r][n][a] = model.addVar(0, 1, 0, GRB_BINARY, s);
                                    //fout << s << endl;
                                }
                            }
                            else if (n >= 3 && (n - 3) < e_sLine) {
                                int line = n - 3;
                                PH_2[k][q][p][r][n] = new GRBVar[e_S[line].n_arc];
                                PH_3[k][q][p][r][n] = new GRBVar[e_S[line].n_arc];
                                for (int a = 0; a < e_S[line].n_arc; a++) {
                                    s = "PH_2[" + itos(k) + "][" + itos(q) + "][" + itos(p) + "][" + itos(r) + "][" + itos(n) + "][" + itos(a) + "]";
                                    PH_2[k][q][p][r][n][a] = model.addVar(0, 1, 0, GRB_BINARY, s);
                                    //fout << s << endl;

                                    s = "PH_3[" + itos(k) + "][" + itos(q) + "][" + itos(p) + "][" + itos(r) + "][" + itos(n) + "][" + itos(a) + "]";
                                    PH_3[k][q][p][r][n][a] = model.addVar(0, 1, 0, GRB_BINARY, s);
                                    //fout << s << endl;
                                }
                            }
                            else if (n >= 3 && (n - 3) >= e_sLine) {
                                int line = n - 3 - e_sLine;
                                PH_2[k][q][p][r][n] = new GRBVar[e_A[line].n_arc];
                                PH_3[k][q][p][r][n] = new GRBVar[e_A[line].n_arc];
                                for (int a = 0; a < e_A[line].n_arc; a++) {
                                    s = "PH_2[" + itos(k) + "][" + itos(q) + "][" + itos(p) + "][" + itos(r) + "][" + itos(n) + "][" + itos(a) + "]";
                                    PH_2[k][q][p][r][n][a] = model.addVar(0, 1, 0, GRB_BINARY, s);
                                    //fout << s << endl;

                                    s = "PH_3[" + itos(k) + "][" + itos(q) + "][" + itos(p) + "][" + itos(r) + "][" + itos(n) + "][" + itos(a) + "]";
                                    PH_3[k][q][p][r][n][a] = model.addVar(0, 1, 0, GRB_BINARY, s);
                                    //fout << s << endl;
                                }
                            }

                        }
                    }

                }
            }
        }
        cout << "f, W, PH, PH_2, PH_3 are created!\n";

        //MU[k][q][p][l][r] ---- prefer variable
        //NG[k][q][p][l][r] ---- both available
        GRBVar *****MU = new GRBVar ****[n_k];
        GRBVar *****NG = new GRBVar ****[n_k];
        for (int k = 0; k < n_k; k++) {
            MU[k] = new GRBVar ***[max_q];
            NG[k] = new GRBVar ***[max_q];
            for (int q = 0; q < max_q; q++) {
                MU[k][q] = new GRBVar **[total_path];
                NG[k][q] = new GRBVar **[total_path];
                for (int p = 0; p < total_path; p++) {
                    MU[k][q][p] = new GRBVar *[total_path];
                    NG[k][q][p] = new GRBVar *[total_path];
                    for (int l = 0; l < total_path; l++) {
                        MU[k][q][p][l] = new GRBVar[num_scenario];
                        NG[k][q][p][l] = new GRBVar[num_scenario];
                        for (int r = 0; r < num_scenario; r++) {

                            if (l != p) {
                                string s;
                                s = "MU[" + itos(k) + "][" + itos(q) + "][" + itos(p) + "][" + itos(l) + "][" + itos(r) + "]";
                                MU[k][q][p][l][r] = model.addVar(0, 1, 0, GRB_BINARY, s);
                                s = "NG[" + itos(k) + "][" + itos(q) + "][" + itos(p) + "][" + itos(l) + "][" + itos(r) + "]";
                                NG[k][q][p][l][r] = model.addVar(0, 1, 0, GRB_BINARY, s);
                            }

                        }
                    }
                }
            }
        }
        cout << "MU, NG are created!\n";

        cout << "all Variables are created!\n";
        //endregion
        model.update();

        //region setObj
        /*set objective function*/
        GRBLinExpr p_s = 0;		//profit from transporting by sea
        GRBLinExpr p_a = 0;		//profit from transporting by air
        GRBLinExpr fixed_c = 0;		//fixed cost for operating new line
        GRBLinExpr total = 0;

//        p_s
//        double coefficient = 0;
//        for (int s = 0; s < sLine; s++) {
//            for (int a = 0; a < G[0].n_arcs; a++) {
//                coefficient = G[0].arc[a].r - G[0].arc[a].v_c;
//
//                recordPath *current = &G[0].arc[a].inPath;
//                current = current->next;
//                while (current != NULL) {
//                    int k = current->k;
//                    int q = current->q;
//                    //int m = current->i;
//                    int p = current->p;
//
//                    int real_p = K[k].kq[q].path_no[p];
//
//                    p_s += (coefficient * f[k][q][real_p]);
//                    current = current->next;
//                }
//            }
//        }
//        for (int s = 0; s < my_sLine; s++) {
//            for (int a = 0; a < e_S[s].n_arc; a++) {
//                coefficient = e_S[s].arc[a].r - e_S[s].arc[a].v_c;
//
//                recordPath *current = &e_S[s].arc[a].inPath;
//                current = current->next;
//                while (current != NULL) {
//                    int k = current->k;
//                    int q = current->q;
//                    //int m = current->i;
//                    int p = current->p;
//                    int real_p = K[k].kq[q].path_no[p];
//
//                    p_s += (coefficient * f[k][q][real_p]);
//                    current = current->next;
//                }
//            }
//        }
//
//        //p_a
//        for (int s = 0; s < aLine; s++) {
//            for (int a = 0; a < G[1].n_arcs; a++) {
//                coefficient = G[1].arc[a].r - G[1].arc[a].v_c;
//
//                recordPath *current = &G[1].arc[a].inPath;
//                current = current->next;
//                while (current != NULL) {
//                    int k = current->k;
//                    int q = current->q;
//                    //int m = current->i;
//                    int p = current->p;
//
//                    int real_p = K[k].kq[q].path_no[p];
//                    //p_a += (coefficient * f[k][q][real_p] * K[k].omega);
//                    p_a += (coefficient * f[k][q][real_p]);
//                    current = current->next;
//                }
//            }
//        }
//        for (int s = 0; s < my_aLine; s++) {
//            for (int a = 0; a < e_A[s].n_arc; a++) {
//                coefficient = e_A[s].arc[a].r - e_A[s].arc[a].v_c;
//
//                recordPath *current = &e_A[s].arc[a].inPath;
//                current = current->next;
//                while (current != NULL) {
//                    int k = current->k;
//                    int q = current->q;
//                    //int m = current->i;
//                    int p = current->p;
//
//                    int real_p = K[k].kq[q].path_no[p];
//                    //p_a += (coefficient * f[k][q][real_p] *K[k].omega);
//                    p_a += (coefficient * f[k][q][real_p]);
//                    current = current->next;
//                }
//            }
//        }

        //fixed_c
        for (int n = 0; n < N - 1; n++) {
            for (int l = 0; l < G[n].n_line; l++) {
                for (int a = 0; a < G[n].n_arcs; a++) {
                    fixed_c += (G[n].arc[a].fo_c * y[n][a][l]);
                }
            }
        }
        total = p_s + p_a - fixed_c;
        //total = 0;
        model.setObjective(total, GRB_MAXIMIZE);

        cout << "obj is set!\n";
        //endregion

        int sea = 0; //G[sea]
        int air = 1;	//G[air]

        //region Sea
        /*set constraints*/
        //maritime costraints////
        //----(2) flow balance: source node
        //S, Hs
        for (int l = 0; l < G[sea].n_line; l++) {
            Linklist *tmp;
            tmp = S[l].h_port;

            while (tmp != NULL) {
                GRBLinExpr sum = 0;
                int homeport;
                homeport = tmp->node;

                // node homeport's outgoing arc
                for (int a = G[sea].forward_point[homeport]; a < G[sea].forward_point[homeport + 1]; a++) {
                    sum += y[sea][a][l];
                }

                string s;
                s = "c2[line" + itos(l) + "][" + itos(homeport) + "]";
                model.addConstr(sum == 1, s);
                //outconstr << s << endl;
                tmp = tmp->next;
            }
        }
        cout << "C2\n";

        //----(3) flow balance: sink node
        //S, Hs
        for (int l = 0; l < G[sea].n_line; l++) {
            Linklist *tmp;
            tmp = S[l].h_port;

            while (tmp != NULL) {
                GRBLinExpr sum = 0;
                int destport;
                int real_a = 0;
                destport = tmp->node + sPort * S[l].cyc_t;

                // node destport's incoming arc;
                for (int a = G[sea].reverse_point[destport]; a < G[sea].reverse_point[destport + 1]; a++) {
                    real_a = G[sea].arc_re_no[a];
                    sum += y[sea][real_a][l];
                }

                string s;
                s = "c3[line" + itos(l) + "][" + itos(destport) + "]";
                model.addConstr(sum == 1, s);
                //outconstr << s << endl;
                tmp = tmp->next;
            }
        }
        cout << "C3\n";

        //----(4) flow balance
        //S, Ns\Hs
        for (int l = 0; l < G[sea].n_line; l++) {

            //fout << "l:" << l << endl;
            for (int n = 0; n < G[sea].n_node; n++) {
                int check_n = 0;	//n = 0: not Hs
                Linklist *tmp;		//check whether Hs
                tmp = S[l].h_port;
                //fout << n << endl;
                while (tmp != NULL) {

                    if (n == tmp->node){
                        check_n = 1;
                        break;
                    }
                    //check n != Hs & Hs + cyc_t
                    if (n == tmp->node + sPort * S[l].cyc_t){
                        check_n = 1;
                        break;
                    }
                    tmp = tmp->next;
                }

                if (check_n != 1){	//Ns\Hs
                    GRBLinExpr in = 0;
                    GRBLinExpr out = 0;
                    //fout << "n = " << n << endl;
                    //node n's outgoing arc
                    for (int a = G[sea].forward_point[n]; a < G[sea].forward_point[n + 1]; a++) {
                        //fout << "outgoing: " << G[sea].arc[a].tail << " " << G[sea].arc[a].head << endl;
                        out += y[sea][a][l];
                    }
                    //node n's incoming arc
                    int real_b = 0;
                    for (int b = G[sea].reverse_point[n]; b < G[sea].reverse_point[n + 1]; b++) {
                        real_b = G[sea].arc_re_no[b];
                        //fout << "incoming: " << G[sea].arc[real_b].tail << " " << G[sea].arc[real_b].head << endl;
                        in += y[sea][real_b][l];
                    }

                    string s;
                    s = "c4[line" + itos(l) + "][" + itos(n) + "]";
                    model.addConstr(in == out, s);
                    //outconstr << s << endl;
                }
            }
        }
        cout << "C4\n";

        //----(5) every cycle the same
        //S, As
        //revised
        for (int l = 0; l < G[sea].n_line; l++) {
            int from = S[l].h_port->node;
            int to = S[l].h_port->node + (S[l].n_voy - 1) * sPort * S[l].cyc_t;

            for (int a = G[sea].forward_point[from]; a < G[sea].forward_point[to]; a++) {

                int tail = G[sea].arc[a].tail;
                int head = G[sea].arc[a].head;

                int next_tail = tail + sPort * S[l].cyc_t;
                int next_head = head + sPort * S[l].cyc_t;

                if (next_tail < G[sea].n_node) {
                    //search the next_tail's outgoing arc
                    for (int b = G[sea].forward_point[next_tail]; b < G[sea].forward_point[next_tail + 1]; b++) {
                        int tmp_head = G[sea].arc[b].head;

                        if (tmp_head == next_head){
                            string s;
                            stringstream a_tail, a_head, b_tail, b_head;
                            a_tail << G[sea].arc[a].tail;
                            a_head << G[sea].arc[a].head;
                            b_tail << G[sea].arc[b].tail;
                            b_head << G[sea].arc[b].head;
                            s = "c5[" + a_tail.str() + "][" + a_head.str() + "]->[" + b_tail.str() + "][" + b_head.str() + "][line:" + itos(l) + "]";
                            model.addConstr(y[sea][a][l] == y[sea][b][l], s);
                            //outconstr << s << endl;
                        }
                    }
                }
            }
        }
        cout << "C5\n";

        //----(6) minimal service time at port i
        //S, Ns
        for (int l = 0; l < G[sea].n_line; l++) {
            for (int i = 0; i < G[sea].n_node; i++) {

                GRBLinExpr left = 0, right = 0;
                int port = i % sPort;
                //fout << port << " = " << i << "%" << sPort << endl;

                //search i's incoming arc
                int real_b = 0;
                for (int b = G[sea].reverse_point[i]; b < G[sea].reverse_point[i + 1]; b++) {
                    real_b = G[sea].arc_re_no[b];
                    int tail = G[sea].arc[real_b].tail;
                    // check the tail whether the same port, if not then plus
                    if (tail % sPort != port) {
                        left += y[sea][real_b][l];
                        //fout << "tail\t" << tail << "\t(" << tail << "%" << sPort << " = " << tail % sPort << endl;
                    }
                }

                for (int m = i; m <= i + sPort*(s_P[port].s_t - 1); m += sPort) {
                    int tail = m;
                    int head = m + sPort;

                    //check whether large than G[sea]'s node number
                    if (tail > G[sea].n_node || head > G[sea].n_node)
                        break;

                    //check tail's outgoing arc
                    for (int a = G[sea].forward_point[tail]; a < G[sea].forward_point[tail + 1]; a++) {
                        int tmp_head = G[sea].arc[a].head;
                        if (tmp_head == head) {
                            right += y[sea][a][l];
                            //fout << tmp_head << endl;
                        }
                    }
                }

                string s;
                s = "c6[port: " + itos(port) + "][line: " + itos(l) + "][" + itos(i) + "]";
                left = (s_P[port].s_t) * left;
                model.addConstr(left <= right, s);
                //outconstr << s << endl;
            }
        }
        cout << "C6\n";
        //endregion

        //region Air
        ////air flight constraints////


        //----(7) flow balance: source node
        //A
        for (int l = 0; l < G[air].n_line; l++) {
            GRBLinExpr sum = 0;
            int base = A[l].b_port;

            for (int i = G[sea].n_node + base; i < G[sea].n_node + base + aPort * PERIOD * 7; i += aPort) {
                int tail = i;
                //check tail's outgoing arc;
                for (int a = G[air].forward_point[tail - G[sea].n_node]; a < G[air].forward_point[tail - G[sea].n_node + 1]; a++) {
                    sum += y[air][a][l];
                }
            }
            string s;
            s = "c7[line" + itos(l) + "]";
            model.addConstr(sum == A[l].freq, s);
            //outconstr << s << endl;
        }
        cout << "C7\n";

        //----(8) flow balance: sink node
        //A, Ba
        for (int l = 0; l < G[air].n_line; l++) {
            GRBLinExpr depa = 0, arri = 0;
            int base = A[l].b_port;

            for (int i = G[sea].n_node + base; i < G[sea].n_node + base + aPort * PERIOD *7; i += aPort) {
                int tail = i;
                int head = tail + aPort * A[l].cyc_t;

                for (int a = G[air].forward_point[tail - G[sea].n_node]; a < G[air].forward_point[tail - G[sea].n_node + 1]; a++) {
                    depa += y[air][a][l];
                }
                for (int b = G[air].reverse_point[head - G[sea].n_node]; b < G[air].reverse_point[head - G[sea].n_node + 1]; b++) {
                    int real_b = G[air].arc_re_no[b];
                    arri += y[air][real_b][l];
                }

                string s;
                s = "c8[line" + itos(l) + "][i:" + itos(tail) + "]";
                model.addConstr(depa == arri, s);
                //outconstr << s << endl;
            }
        }
        cout << "C8\n";

        //----(9) flow balance
        //A, Na\Ba
        for (int l = 0; l < G[air].n_line; l++) {
            int base = A[l].b_port;

            for (int n = G[sea].n_node; n < G[sea].n_node + G[air].n_node; n++) {
                GRBLinExpr in = 0, out = 0;
                int port = (n - G[sea].n_node) % aPort;	//check whether Ba

                if (port != base){	//Na\Ba
                    for (int a = G[air].forward_point[n - G[sea].n_node]; a < G[air].forward_point[n - G[sea].n_node + 1]; a++) {
                        out += y[air][a][l];
                    }

                    for (int b = G[air].reverse_point[n - G[sea].n_node]; b < G[air].reverse_point[n - G[sea].n_node + 1]; b++) {
                        int real_b = G[air].arc_re_no[b];
                        in += y[air][real_b][l];
                    }

                    string s;
                    s = "c9[line" + itos(l) + "][i:" + itos(n) + "]";
                    model.addConstr(in == out, s);
                    //outconstr << s << endl;
                }
            }
        }
        cout << "C9\n";

        //----(10) every cycle the same
        //A, Aa
        //revised
        for (int l = 0; l < G[air].n_line; l++) {
            for (int a = 0; a < G[air].forward_point[(L - PERIOD *7)*aPort]; a++) {

                int tail = G[air].arc[a].tail;
                int head = G[air].arc[a].head;

                int next_tail = tail + aPort * PERIOD * 7;
                int next_head = head + aPort * PERIOD * 7;

                for (int b = G[air].forward_point[next_tail - G[sea].n_node]; b < G[air].forward_point[next_tail - G[sea].n_node + 1]; b++) {
                    int tmp_head = G[air].arc[b].head;
                    if (tmp_head == next_head){
                        string s;
                        stringstream a_tail, a_head, b_tail, b_head;
                        a_tail << G[air].arc[a].tail;
                        a_head << G[air].arc[a].head;
                        b_tail << G[air].arc[b].tail;
                        b_head << G[air].arc[b].head;
                        s = "c10[" + a_tail.str() + "][" + a_head.str() + "]->[" + b_tail.str() + "][" + b_head.str() + "][line:" + itos(l) + "]";
                        model.addConstr(y[air][a][l] == y[air][b][l], s);
                        //outconstr << s << endl;
                    }
                }
            }
        }
        cout << "C10\n";

        //----(11)&(12)&(13)	gap: choose one kind of pattern ; only for those freqency >1 's flight
        //A,Aa
        for (int l = 0; l < G[air].n_line; l++) {

            if (A[l].freq == 1)
                break;

            int number_of_r = 7 - (A[l].gap / PERIOD) * (A[l].freq - 1);
            //fout << number_of_r << endl;

            for (int a = 0; a < G[air].forward_point[aPort * (7 * PERIOD + A[l].cyc_t) + 1]; a++) {
                int tail = G[air].arc[a].tail;
                //fout << "tail\t" << tail << "\t";

                //check whether tail < aPort * (PERIOD * 7 + A[l].cyc_t)
                if (tail < G[sea].n_node + aPort * (PERIOD * 7 + A[l].cyc_t)) {
                    int now_t = (tail - G[sea].n_node) / aPort +1;

                    //fout << "now_t\t" << now_t << endl;

                    //check whether tail in R[r][i]
                    for (int r = 0; r < number_of_r; r++) {
                        int check_t = 0;

                        for (int i = 0; i < A[l].freq - 1; i++) {
                            if (now_t >= A[l].R[r][i] && now_t <= A[l].R[r][i + A[l].freq - 1]) {
                                //fout << "A[" << l << "].R[" << r << "][" << i << "]=" << A[l].R[r][i] << endl;
                                //fout << "A[" << l << "].R[" << r << "][" << i + A[l].freq - 1 << "]=" << A[l].R[r][i + A[l].freq - 1] << endl;
                                int head = G[air].arc[a].head;
                                int gap_tail = tail + aPort * A[l].gap;
                                int gap_head = head + aPort * A[l].gap;
                                //fout << "h:\t" << head << "\tg_t:" << gap_tail << "\tg_h:" << gap_head << endl;
                                for (int b = G[air].forward_point[gap_tail - G[sea].n_node]; b < G[air].forward_point[gap_tail - G[sea].n_node + 1]; b++) {
                                    int tmp_head = G[air].arc[b].head;
                                    //fout << "tmp_h:\t" << tmp_head << "\n";
                                    if (tmp_head == gap_head) {
                                        string s;
                                        stringstream a_tail, a_head, b_tail, b_head;
                                        a_tail << G[air].arc[a].tail;
                                        a_head << G[air].arc[a].head;
                                        b_tail << G[air].arc[b].tail;
                                        b_head << G[air].arc[b].head;
                                        s = "c11[" + a_tail.str() + "][" + a_head.str() + "]->[" + b_tail.str() + "][" + b_head.str() + "][line:" + itos(l) + "]Q[" + itos(l) + "][" + itos(r) + "]";
                                        model.addConstr(y[air][a][l] <= y[air][b][l] + (1 - Q[l][r]), s);
                                        //outconstr << s << endl;
                                        s = "c12[" + b_tail.str() + "][" + b_head.str() + "]->[" + a_tail.str() + "][" + a_head.str() + "][line:" + itos(l) + "]Q[" + itos(l) + "][" + itos(r) + "]";
                                        model.addConstr(y[air][b][l] <= y[air][a][l] + (1 - Q[l][r]), s);
                                        //outconstr << s << endl;
                                    }
                                }
                                check_t++;
                            }
                            if (now_t >= A[l].R[r][i] + A[l].gap && now_t <= A[l].R[r][i + A[l].freq - 1] + A[l].gap)
                                check_t++;

                        }

                        if (check_t == 0) {
                            string s;
                            stringstream a_tail, a_head, b_tail, b_head;
                            a_tail << G[air].arc[a].tail;
                            a_head << G[air].arc[a].head;
                            s = "c13[" + a_tail.str() + "][" + a_head.str() + "][line:" + itos(l) + "]Q[" + itos(l) + "][" + itos(r) + "]";
                            model.addConstr(y[air][a][l] <= (1 - Q[l][r]), s);
                            //outconstr << s << endl;
                        }
                    }
                }
            }
        }
        cout << "C11 & C12 & C13\n";


        //----(14)	only choose one pattern for flight a
        //A
        for (int l = 0; l < G[air].n_line; l++) {

            if (A[l].freq == 1)
                break;

            GRBLinExpr sum = 0;
            int number_of_r = 7 - (A[l].gap / PERIOD) * (A[l].freq - 1);
            for (int r = 0; r < number_of_r; r++) {
                sum += Q[l][r];
            }
            string s;
            s = "c14[" + itos(l) + "]";
            model.addConstr(sum == 1, s);
            //outconstr << s << endl;
        }
        cout << "C14\n";
        //endregion

        outconstr.close();
        fout.close();

        cout << "all constraints are set\n";
        model.update();
        model.optimize();

        /*output results*/
        cout << "obj: " << model.get(GRB_DoubleAttr_ObjVal) << endl;

        /*output data*/
        ofstream outfile("results.txt");
        outfile << "Obj: " << model.get(GRB_DoubleAttr_ObjVal) << endl;
        outfile << "runtime: " << model.get(GRB_DoubleAttr_Runtime) << endl;

        //y[network][arc][line]
        for (int n = 0; n < N - 1; n++) {
            for (int a = 0; a < G[n].n_arcs; a++) {
                for (int l = 0; l < G[n].n_line; l++) {

                    //y[network][tail][head][line]
                    if (y[n][a][l].get(GRB_DoubleAttr_X) == 1) {
                        string s;
                        stringstream s_tail, s_head;
                        s_tail << G[n].arc[a].tail;
                        s_head << G[n].arc[a].head;
                        s = "y[" + itos(n) + "][" + s_tail.str() + "][" + s_head.str() + "][" + itos(l) + "]";
                        outfile << s << " = " << y[n][a][l].get(GRB_DoubleAttr_X) << "\t" << G[n].arc[a].fo_c << endl;
                    }

                }
            }
        }

        //z[k][q]
        for (int k = 0; k < n_k; k++) {
            for (int q = 0; q < max_q; q++) {
                string s;
                s = "z[" + itos(k) + "][" + itos(q) + "]";
                //z[commodity][q]
                if (z[k][q].get(GRB_DoubleAttr_X) == 1) {
                    outfile << s << " = " << z[k][q].get(GRB_DoubleAttr_X) << endl;
                }
            }
        }

        //Q[l][r]
        for (int l = 0; l < G[l].n_line; l++) {
            number_of_r = 7 - (A[l].gap / PERIOD) * (A[l].freq - 1);
            for (int r = 0; r < number_of_r; r++) {
                string s;
                s = "Q[" + itos(l) + "][" + itos(r) + "]";
                if (Q[l][r].get(GRB_DoubleAttr_X) == 1) {
                    outfile << s << " = " << Q[l][r].get(GRB_DoubleAttr_X) << endl;
                }
            }
        }

        //f[k][q][p] ---- flow on path p
        for (int k = 0; k < n_k; k++) {
            for (int q = 0; q < max_q; q++) {
                for (int p = 0; p < total_path; p++) {
                    string s;
                    s = "f[" + itos(k) + "][" + itos(q) + "][" + itos(p) + "]";
                    if (f[k][q][p].get(GRB_DoubleAttr_X) > 0) {
                        outfile << s << "=" << f[k][q][p].get(GRB_DoubleAttr_X) << endl;
                    }
                }
            }
        }


        //W[k][q][p][r] ----choose path p in scenario r
        for (int r = 0; r < num_scenario; r++) {
            for (int k = 0; k < n_k; k++) {
                for (int q = 0; q < max_q; q++) {
                    for (int p = 0; p < total_path; p++) {
                        string s;
                        s = "W[" + itos(k) + "][" + itos(q) + "][" + itos(p) + "][" + itos(r) + "]";
                        if (W[k][q][p][r].get(GRB_DoubleAttr_X) == 1){
                            outfile << s << "=" << W[k][q][p][r].get(GRB_DoubleAttr_X) << endl;
                        }
                    }
                }
            }
        }


        // PH[k][q][p][r] --- avaliable path
        for (int r = 0; r < num_scenario; r++) {
            for (int k = 0; k < n_k; k++) {
                for (int q = 0; q < max_q; q++) {
                    for (int p = 0; p < total_path; p++) {
                        string s;
                        s = "PH[" + itos(k) + "][" + itos(q) + "][" + itos(p) + "][" + itos(r) + "]";
                        if (PH[k][q][p][r].get(GRB_DoubleAttr_X) == 1) {
                            outfile << s << "=" << PH[k][q][p][r].get(GRB_DoubleAttr_X) << endl;
                        }
                    }
                }
            }
        }

        // PH_2[k][q][p][r][n][a] --- avaliable arc(volumn)
        // PH_3[k][q][p][r][n][a] --- avaliable arc(weight)
        for (int r = 0; r < num_scenario; r++) {
            for (int k = 0; k < n_k; k++) {
                for (int q = 0; q < max_q; q++) {
                    for (int p = 0; p < total_path; p++) {
                        for (int n = 0; n < TOTALG; n++){
                            string s;
                            if (n < N - 1) {
                                for (int a = 0; a < G[n].n_arcs; a++) {
                                    s = "PH_2[" + itos(k) + "][" + itos(q) + "][" + itos(p) + "][" + itos(r) + "][t_" + itos(G[n].arc[a].tail) + "][" + itos(G[n].arc[a].head) + "]";
                                    if (PH_2[k][q][p][r][n][a].get(GRB_DoubleAttr_X) == 1) {
                                        outfile << s << "=" << PH_2[k][q][p][r][n][a].get(GRB_DoubleAttr_X) << endl;
                                    }
                                    s = "PH_3[" + itos(k) + "][" + itos(q) + "][" + itos(p) + "][" + itos(r) + "][t_" + itos(G[n].arc[a].tail) + "][" + itos(G[n].arc[a].head) + "]";
                                    if (PH_3[k][q][p][r][n][a].get(GRB_DoubleAttr_X) == 1) {
                                        outfile << s << "=" << PH_3[k][q][p][r][n][a].get(GRB_DoubleAttr_X) << endl;
                                    }
                                }
                            }
                            else if (n >= 3 && (n - 3) < e_sLine) {
                                int line = n - 3;
                                for (int a = 0; a < e_S[line].n_arc; a++) {
                                    s = "PH_2[" + itos(k) + "][" + itos(q) + "][" + itos(p) + "][" + itos(r) + "][t_" + itos(e_S[line].arc[a].tail) + "][" + itos(e_S[line].arc[a].head) + "]";
                                    if (PH_2[k][q][p][r][n][a].get(GRB_DoubleAttr_X) == 1) {
                                        outfile << s << "=" << PH_2[k][q][p][r][n][a].get(GRB_DoubleAttr_X) << endl;
                                    }
                                }
                            }
                            else if (n >= 3 && (n - 3) >= e_sLine) {
                                int line = n - 3 - e_sLine;
                                for (int a = 0; a < e_A[line].n_arc; a++) {
                                    s = "PH_2[" + itos(k) + "][" + itos(q) + "][" + itos(p) + "][" + itos(r) + "][t_" + itos(e_A[line].arc[a].tail) + "][" + itos(e_A[line].arc[a].head) + "]";
                                    if (PH_2[k][q][p][r][n][a].get(GRB_DoubleAttr_X) == 1) {
                                        outfile << s << "=" << PH_2[k][q][p][r][n][a].get(GRB_DoubleAttr_X) << endl;
                                    }
                                    s = "PH_3[" + itos(k) + "][" + itos(q) + "][" + itos(p) + "][" + itos(r) + "][t_" + itos(e_A[line].arc[a].tail) + "][" + itos(e_A[line].arc[a].head) + "]";
                                    if (PH_3[k][q][p][r][n][a].get(GRB_DoubleAttr_X) == 1) {
                                        outfile << s << "=" << PH_3[k][q][p][r][n][a].get(GRB_DoubleAttr_X) << endl;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        //MU[k][q][p][l][r] ---- prefer variable
        for (int r = 0; r < num_scenario; r++) {
            for (int k = 0; k < n_k; k++) {
                for (int q = 0; q < max_q; q++) {
                    for (int p = 0; p < total_path; p++) {
                        for (int l = 0; l < total_path; l++) {
                            if (l != p) {
                                string s;
                                s = "MU[" + itos(k) + "][" + itos(q) + "][" + itos(p) + "][" + itos(l) + "][" + itos(r) + "]";
                                if (MU[k][q][p][l][r].get(GRB_DoubleAttr_X) == 1) {
                                    outfile << s << "=" << MU[k][q][p][l][r].get(GRB_DoubleAttr_X) << endl;
                                }
                            }
                        }
                    }
                }
            }
        }


        //NG[k][q][p][l][r] ---- both available
        for (int r = 0; r < num_scenario; r++) {
            for (int k = 0; k < n_k; k++) {
                for (int q = 0; q < max_q; q++) {
                    for (int p = 0; p < total_path; p++) {
                        for (int l = 0; l < total_path; l++) {
                            if (l != p) {
                                string s;
                                s = "NG[" + itos(k) + "][" + itos(q) + "][" + itos(p) + "][" + itos(l) + "][" + itos(r) + "]";
                                if (NG[k][q][p][l][r].get(GRB_DoubleAttr_X) == 1){
                                    outfile << s << "=" << NG[k][q][p][l][r].get(GRB_DoubleAttr_X) << endl;
                                }
                            }
                        }
                    }
                }
            }
        }

        outfile.close();

    }
    catch (GRBException e){	//Error code
        cout << "Error code = " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    }
    catch (...){
        cout << "Exception during optimization" << endl;
    }
    return 0;
}

void r_global_var(string s)
{
    ifstream fin;

    fin.open(s.c_str(), ios::in);
    if (!fin.is_open())
    {
        cout << "failed to open r_gloable_var file!";
        exit(1);
    }
    //cout << "in r_gloabl_var!\n";

    char tmp;
    while (!fin.eof())
    {

        fin.get(tmp);
        //cout << tmp;
        switch (tmp)
        {

            case 'c':
                //cout << "in c\n";
                fin.ignore(M, '\n');
                break;

            case 'p':
                fin >> sPort;
                fin >> aPort;
                fin >> n_k;
                fin >> n_total;
                fin >> sLine;
                fin >> aLine;
                fin >> e_sLine;
                fin >> e_aLine;
                fin >> my_sLine;
                fin >> my_aLine;
                fin >> lcm;
                fin >> times;
                fin >> L;
                fin >> max_q;
                fin >> num_scenario;
                //cout << "s:" << sPort << " a:" << aPort << endl;
                //cout << "n_k: " << n_k << " n_total:" << n_total << endl;
                //cout << "sL:" << sLine << " aL:" << aLine << endl;
                //cout << "lcm:" << lcm << " times:" << times << " L:" << L << " q:" << max_q << endl;
                break;

            default:
                break;
        }
    }
    fin.close();
    return;
}

Graph* constr_grap(int n, Graph *g)
{
    g = new Graph[n];
    for (int i = 0; i < n; i++)
    {
        g[i].n_node = 0;
        g[i].n_arcs = 0;
        g[i].n_line = 0;
    }
    return(g);
}

Commodity* constr_commodities(int n, int max_q, Commodity *k)
{
    //cout << "in contr_comm\n";
    //int n_voy = lcm * (times - 1) / ( max_q * 7 * PERIOD );
    k = new Commodity[n];
    for (int i = 0; i < n; i++)
    {
        k[i].d = 0;
        k[i].w = 0;
        k[i].b = 0;
        k[i].omega = 0;
        k[i].apha = 0;
        k[i].beta = 0;
        k[i].q = new nodeSet[max_q];
        //cout << k[i].d << " " << k[i].w << endl;
        for (int j = 0; j < max_q; j++)
        {
            k[i].q[j].origin.node = 0;
            k[i].q[j].origin.next = NULL;
            //cout << k[i].q[j].origin.next << endl;
            k[i].q[j].dest.node = 0;
            k[i].q[j].dest.next = NULL;
            k[i].q[j].next_set = NULL;
            //cout << k[i].q[j].origin.node << " " << k[i].q[j].dest.node << endl;
        }

        k[i].kq = new SmallK[max_q];
        for (int q = 0; q < max_q; q++)
        {
            k[i].kq[q].num_p = 0;
        }

        /*k[i].scenarios = new double[num_scenario];
		for(int s = 0; s < num_scenario; s++)
		{
		k[i].scenarios[s] = 0;
		}*/
    }
    //cout << "finish commodity\n";
    return (k);
}

Port* constr_ports(int n, Port *p)
{
    //cout << "in constr_port\n";
    p = new Port[n];
    for (int i = 0; i < n; i++)
    {
        p[i].fb_c = 0;
        p[i].s_t = 0;
        p[i].cap = 0;
        //cout << p[i].fb_c << " " << p[i].s_t << endl;
    }
    //cout << "finish constr_port\n";
    return(p);
}

designS* constr_sL(int n, designS *s)
{
    //cout << "in constr_sL\n";
    s = new designS[n];
    for (int i = 0; i < n; i++)
    {
        s[i].cyc_d = 0;
        s[i].cyc_t = 0;
        s[i].n_voy = 0;
        s[i].cap_v = 0;
        //cout << s[i].cyc_d << " " <<  s[i].cyc_t << " " << s[i].n_voy << " " << s[i].cap_v << endl;

        s[i].h_port = new Linklist;
        s[i].h_port->node = 0;
        s[i].h_port->next = NULL;
        //cout << s[i].h_port->node << endl;
    }
    //cout << "finish constr_sL\n";
    return(s);
}

designA* constr_aL(int n, designA *a)
{
    //cout << "in constr_aL\n";
    a = new designA[n];
    for (int i = 0; i < n; i++)
    {
        a[i].cyc_h = 0;
        a[i].cyc_t = 0;
        a[i].freq = 0;
        a[i].gap = 0;
        a[i].cap_v = 0;
        a[i].cap_u = 0;
        a[i].b_port = 0;
        //cout << a[i].cyc_h << " " << a[i].cyc_t << " " << a[i].freq << " " << a[i].gap << " " << a[i].cap_v << " " << a[i].cap_u << " " << a[i].b_port << endl;
    }
    //cout << "finish constr_aL\n";
    return(a);
}

eL* constr_eL(int n, eL *e)
{
    e = new eL[n];
    for (int i = 0; i < n; i++)
    {
        e[i].cap_v = 0;
        e[i].cap_u = 0;
        e[i].n_arc = 0;
        //cout << i << "\t" << e[i].cap_v << "\t" << e[i].cap_u << "\t" << e[i].n_arc << endl;
    }
    return(e);
}

void r_port_var(string s, Port *s_P, Port *a_P)
{
    ifstream fin;

    fin.open(s.c_str(), ios::in);
    if (!fin.is_open())
    {
        cout << "failed to open r_port_var file!";
        exit(1);
    }
    //cout << "in r_port_var!\n";

    char tmp;
    int no = 0;
    while (!fin.eof())
    {

        fin.get(tmp);
        //cout << tmp;
        switch (tmp)
        {

            case 'c':
                //cout << "in c\n";
                fin.ignore(M, '\n');
                break;

            case 's':
                fin >> no;
                fin >> s_P[no].fb_c;
                fin >> s_P[no].s_t;
                fin >> s_P[no].cap;
                //cout << no << " " << s_P[no].fb_c << " " << s_P[no].s_t << "\t" << s_P[no].cap << endl;
                break;

            case 'a':
                fin >> no;
                fin >> a_P[no].fb_c;
                fin >> a_P[no].s_t;
                fin >> a_P[no].cap;
                //cout << no << " " << a_P[no].fb_c << " " << a_P[no].s_t << "\t" << a_P[no].cap << endl;
                break;

            default:
                break;
        }
    }
    fin.close();
    //cout << "finish r_port_var!\n";
    return;
}

void r_lines_var(string s, designS *S, designA *A)
{
    ifstream fin;

    fin.open(s.c_str(), ios::in);
    if (!fin.is_open())
    {
        cout << "failed to open r_lines_var file!";
        exit(1);
    }
    //cout << "in r_lines_var!\n";

    char tmp;
    int tmp_s;
    int no = 0;
    int number_of_r = 0;
    int gener_nodes = 0;

    Linklist *current;
    while (!fin.eof())
    {

        fin.get(tmp);
        //cout << tmp;
        switch (tmp)
        {

            case 'c':
                //cout << "in c\n";
                fin.ignore(M, '\n');
                break;

            case 's':
                fin >> no;
                fin >> S[no].cyc_d;
                fin >> S[no].cyc_t;
                fin >> S[no].n_voy;
                fin >> S[no].cap_v;
                fin >> S[no].h_port->node;
                //cout << no << " " << S[no].cyc_d << " " << S[no].cyc_t << " " << S[no].n_voy << " " << S[no].cap_v << " " << S[no].h_port->node << endl;

                current = S[no].h_port;
                fin >> tmp_s;
                while (tmp_s != -1)
                {
                    Linklist *new_node;
                    new_node = new Linklist;
                    new_node->node = tmp_s;
                    new_node->next = NULL;

                    current->next = new_node;
                    current = current->next;
                    //cout << new_node->node << endl;
                    fin >> tmp_s;
                }

                break;

            case 'a':
                fin >> no;
                fin >> A[no].cyc_h;
                fin >> A[no].cyc_t;
                fin >> A[no].freq;
                fin >> A[no].gap;
                fin >> A[no].cap_v;
                fin >> A[no].cap_u;
                fin >> A[no].b_port;
                //cout << no << " " << A[no].cyc_h << " " << A[no].cyc_t << " " << A[no].freq << " " << A[no].gap << " " << A[no].cap_v << " " << A[no].cap_u << " " << A[no].b_port << endl;

                //construct set R, v103
                number_of_r = 7 - (A[no].gap / PERIOD) * (A[no].freq - 1);
                //cout << number_of_r << endl;
                A[no].R = new int*[number_of_r];
                for (int i = 0; i < number_of_r; i++)
                {
                    A[no].R[i] = new int[2 * (A[no].freq - 1)];
                    for (int j = 0; j < A[no].freq - 1; j++)
                    {
                        A[no].R[i][j] = PERIOD * i + j * A[no].gap + 1;
                        //A[no].R[i][j + A[no].freq - 1] = A[no].R[i][j] + PERIOD + A[no].cyc_t - 1;
                        A[no].R[i][j + A[no].freq - 1] = A[no].R[i][j] + PERIOD + A[no].cyc_t - 1 + A[no].cyc_t + A[no].cyc_t;

                        //cout << A[no].R[i][j] << "\t" << A[no].R[i][j + A[no].freq - 1] << endl;
                    }
                    //cout << endl;
                }

                break;

            case 'e':
                fin >> tmp;
                fin >> no;
                if (tmp == 's')
                {
                    fin >> e_S[no].n_voy;
                    fin >> e_S[no].cyc_t;
                    fin >> e_S[no].cap_v;
                    fin >> e_S[no].cap_u;
                    fin >> e_S[no].n_arc;
                    fin >> e_S[no].depart_node;
                    e_S[no].arc = new arcSet[e_S[no].n_arc];
                    e_S[no].arc_re_no = new int[e_S[no].n_arc];
                    for (int i = 0; i < e_S[no].n_arc; i++)
                    {
                        e_S[no].arc[i].tail = 0;
                        e_S[no].arc[i].head = 0;
                        e_S[no].arc[i].t = 0;
                        e_S[no].arc[i].r = 0;
                        e_S[no].arc[i].v_c = 0;
                        e_S[no].arc[i].fo_c = 0;
                        e_S[no].arc[i].l_c = 0;
                        e_S[no].arc[i].s_c = 0;

                        e_S[no].arc_re_no[i] = 0;
                        e_S[no].arc[i].inPath.k = -1;
                        e_S[no].arc[i].inPath.q = -1;
                        //e_S[no].arc[i].inPath.i = -1;
                        e_S[no].arc[i].inPath.p = -1;
                        e_S[no].arc[i].inPath.next = NULL;
                        e_S[no].arc[i].rear = &e_S[no].arc[i].inPath;
                    }

                    gener_nodes = G[0].n_node;

                    e_S[no].forward_point = new int[gener_nodes + 1];//(+1) for saving last arc number
                    e_S[no].reverse_point = new int[gener_nodes + 1];
                    for (int i = 0; i <= gener_nodes; i++)
                    {
                        e_S[no].forward_point[i] = e_S[no].n_arc;
                        e_S[no].reverse_point[i] = e_S[no].n_arc;
                        //cout << e_S[no].forward_point[i] << "\t" << e_S[no].reverse_point[i] << endl;
                    }
                    //cout << no << "\t" << e_S[no].cap_v << "\t" << e_S[no].cap_u << "\t" << e_S[no].n_arc << endl;
                }
                if (tmp == 'a')
                {
                    fin >> e_A[no].n_voy;
                    fin >> e_A[no].cyc_t;
                    fin >> e_A[no].cap_v;
                    fin >> e_A[no].cap_u;
                    fin >> e_A[no].n_arc;
                    e_A[no].arc = new arcSet[e_A[no].n_arc];
                    e_A[no].arc_re_no = new int[e_A[no].n_arc];
                    for (int i = 0; i < e_A[no].n_arc; i++)
                    {
                        e_A[no].arc[i].tail = 0;
                        e_A[no].arc[i].head = 0;
                        e_A[no].arc[i].t = 0;
                        e_A[no].arc[i].r = 0;
                        e_A[no].arc[i].v_c = 0;
                        e_A[no].arc[i].fo_c = 0;
                        e_A[no].arc[i].l_c = 0;
                        e_A[no].arc[i].s_c = 0;

                        e_A[no].arc_re_no[i] = 0;
                        e_A[no].arc[i].inPath.k = -1;
                        e_A[no].arc[i].inPath.q = -1;
                        //e_A[no].arc[i].inPath.i = -1;
                        e_A[no].arc[i].inPath.p = -1;
                        e_A[no].arc[i].inPath.next = NULL;
                        e_A[no].arc[i].rear = &e_A[no].arc[i].inPath;
                    }

                    gener_nodes = G[1].n_node;
                    e_A[no].forward_point = new int[gener_nodes + 1];//(+1) for saving last arc number
                    e_A[no].reverse_point = new int[gener_nodes + 1];
                    for (int i = 0; i <= gener_nodes; i++)
                    {
                        e_A[no].forward_point[i] = e_A[no].n_arc;
                        e_A[no].reverse_point[i] = e_A[no].n_arc;
                    }
                    //cout << no << "\t" << e_A[no].cap_v << "\t" << e_A[no].cap_u << "\t" << e_A[no].n_arc << endl;
                }
                break;

            default:
                break;
        }
    }
    fin.close();
    //cout << "finish r_line_var!\n";
    return;
}

void r_k_var(string s, Commodity *k)
{
    ifstream fin;

    fin.open(s.c_str(), ios::in);
    if (!fin.is_open())
    {
        cout << "failed to open r_k_var file!";
        exit(1);
    }
    //cout << "in r_k_var!\n";

    char tmp;
    int m = 0, no = 0, tmp_node = 0;
    int n_voy;
    Linklist *current;
    nodeSet *cur_set;

    while (!fin.eof())
    {

        fin.get(tmp);
        //cout << tmp;
        switch (tmp)
        {

            case 'c':
                //cout << "in c\n";
                fin.ignore(M, '\n');
                break;

            case 'k':
                fin >> no;
                fin >> k[no].d;
                fin >> k[no].w;
                fin >> k[no].b;
                fin >> k[no].apha;
                fin >> k[no].beta;
                fin >> k[no].omega;

                //cout << no << " " << k[no].d << " " << k[no].w << "\t" << k[no].b << "\t" << k[no].apha << "\t" << k[no].beta << endl;
                break;

            case 'q':
                fin >> m;
                fin >> no;
                //cout << m << " " << no << endl;

                // find the current last set
                cur_set = &k[m].q[no];
                //cout << cur_set->origin.node << endl;
                while (cur_set->next_set != NULL)
                {
                    //cout<<"in while 1\n";
                    cur_set = cur_set->next_set;
                }

                // decide whether first r, if not create new_set
                if (k[m].q[no].origin.node != 0)
                {
                    //cout << "in if\n";
                    nodeSet *new_set;
                    new_set = new nodeSet;
                    new_set->origin.node = 0;
                    new_set->origin.next = NULL;
                    new_set->dest.node = 0;
                    new_set->dest.next = NULL;
                    new_set->next_set = NULL;

                    cur_set->next_set = new_set;
                    cur_set = cur_set->next_set;
                }


                fin >> cur_set->origin.node;
                current = &cur_set->origin;
                //cout << current->node << endl;
                fin >> tmp_node;
                while (tmp_node != -1)
                {
                    Linklist *new_node;
                    new_node = new Linklist;
                    new_node->node = tmp_node;
                    new_node->next = NULL;
                    current->next = new_node;
                    current = current->next;
                    //cout << new_node->node << " ";
                    fin >> tmp_node;
                }
                //cout << endl;
                current = &cur_set->dest;
                fin >> cur_set->dest.node;
                fin >> tmp_node;
                //cout << current->node << endl;
                while (tmp_node != -1)
                {
                    Linklist *new_node;
                    new_node = new Linklist;
                    new_node->node = tmp_node;
                    new_node->next = NULL;
                    current->next = new_node;
                    current = current->next;
                    //cout << new_node->node << " ";
                    fin >> tmp_node;
                }
                //cout << endl;
                break;

            default:
                break;
        }
    }
    fin.close();
    //cout << "finish r_k_var!\n";
    return;
}

void r_graph_info(string s, Graph *G)
{
    ifstream fin;

    fin.open(s.c_str(), ios::in);
    if (!fin.is_open())
    {
        cout << "failed to open r_graph_info file!";
        exit(1);
    }
    //cout << "in r_graph_info!\n";

    char tmp;
    int no = 0;
    int i = 0;
    int gener_nodes = 0;

    while (!fin.eof())
    {

        fin.get(tmp);
        //cout << tmp;
        switch (tmp)
        {

            case 'c':
                //cout << "in c\n";
                fin.ignore(M, '\n');
                break;

            case 'g':
                fin >> no;
                fin >> G[no].n_node;
                fin >> G[no].n_arcs;
                fin >> G[no].n_line;
                //cout << no << " " << G[no].n_node << " " << G[no].n_arcs << " " << G[no].n_line << endl;

                G[no].arc = new arcSet[G[no].n_arcs];
                G[no].arc_re_no = new int[G[no].n_arcs];
                for (i = 0; i < G[no].n_arcs; i++)
                {
                    G[no].arc[i].tail = 0;
                    G[no].arc[i].head = 0;
                    G[no].arc[i].r = 0;
                    G[no].arc[i].fo_c = 0;
                    G[no].arc[i].l_c = 0;
                    G[no].arc[i].s_c = 0;
                    G[no].arc[i].v_c = 0;
                    G[no].arc[i].t = 0;

                    G[no].arc_re_no[i] = 0;     //v102
                    //cout << G[no].arc[i].tail << " " << G[no].arc[i].head << " " << G[no].arc[i].m_p << " " << G[no].arc[i].fo_c << " " << G[no].arc[i].l_c << " " << G[no].arc[i].s_c << endl;
                    G[no].arc[i].inPath.k = -1;
                    G[no].arc[i].inPath.q = -1;
                    //G[no].arc[i].inPath.i = -1;
                    G[no].arc[i].inPath.p = -1;
                    G[no].arc[i].inPath.next = NULL;
                    G[no].arc[i].rear = &G[no].arc[i].inPath;
                }
                /*v101*/
                //v102
                if (no < N - 1)
                    gener_nodes = G[no].n_node;
                else
                    gener_nodes = 2 * G[0].n_node + 2 * G[1].n_node + G[2].n_node;

                G[no].forward_point = new int[gener_nodes + 1];    // (+1) for saving last arc number
                G[no].reverse_point = new int[gener_nodes + 1];
                for (i = 0; i < gener_nodes; i++)
                {
                    G[no].forward_point[i] = G[no].n_arcs;
                    G[no].reverse_point[i] = G[no].n_arcs;
                }
                G[no].forward_point[gener_nodes] = G[no].n_arcs;   // last(dummy node) for saving the last arc number
                G[no].reverse_point[gener_nodes] = G[no].n_arcs;
                //cout << "finish create arcs\n";


                break;

            default:
                break;
        }
    }
    fin.close();
    //cout << "finish r_graph_info & construct graph_arcs!\n";
    return;
}

void r_arc_info(string s, Graph *G)
{
    ifstream fin;

    fin.open(s.c_str(), ios::in);
    if (!fin.is_open())
    {
        cout << "failed to open r_arc_info file!";
        exit(1);
    }
    //cout << "in r_arc_info!\n";

    char tmp;
    int no = 0;
    int i = 0;
    int now_tail = 0;

    while (!fin.eof())
    {

        fin.get(tmp);
        //cout << tmp;
        switch (tmp)
        {

            case 'c':
                //cout << "in c\n";
                fin.ignore(M, '\n');
                break;

            case 'n':
                fin >> no;
                //cout << "graph:" << no << endl;
                break;

            case 'a':
                fin >> G[no].arc[i].tail;
                fin >> G[no].arc[i].head;
                if (no != N - 1)
                {
                    fin >> G[no].arc[i].r;
                    fin >> G[no].arc[i].v_c;
                    fin >> G[no].arc[i].fo_c;
                    fin >> G[no].arc[i].t;

                }
                else
                {
                    fin >> G[no].arc[i].l_c;
                    fin >> G[no].arc[i].s_c;
                    fin >> G[no].arc[i].t;
                }

                /*v101*/
                G[no].arc_re_no[i] = i;     //reverse_point initialize

                /*
			if(now_tail != G[no].arc[i].tail){
			G[no].forward_point[G[no].arc[i].tail] = i;
			now_tail = G[no].arc[i].tail;
			}*/
                i++;

                //cout << G[no].arc[i].tail << " " << G[no].arc[i].head << " " << G[no].arc[i].m_p << " " <<  G[no].arc[i].fo_c << " " << G[no].arc[i].l_c << " " << G[no].arc[i].s_c << endl;
                break;

            default:
                break;
        }
    }

    fin.close();
    //cout << "finish r_arc_info\n";
    return;
}

void arc_sort(Graph *G) //v102
{
    //cout << "in arc sort!\n";
    /*selection can test*/
    arcSet tmp;
    for (int n = 0; n < N; n++)
    {
        //selection sort
        int min_tail = 0;
        int loc = 0;
        for (int a = 0; a < G[n].n_arcs - 1; a++)
        {
            min_tail = G[n].arc[a].tail;
            loc = a;
            for (int b = a + 1; b < G[n].n_arcs; b++)
            {
                //compare head
                if (G[n].arc[b].tail < min_tail)
                {
                    min_tail = G[n].arc[b].tail;
                    loc = b;
                }
            }
            //swap
            tmp = G[n].arc[a];
            G[n].arc[a] = G[n].arc[loc];
            G[n].arc[loc] = tmp;
        }
    }
    return;
}

void reverse_arc_sort(Graph *G)  //v101
{
    /*selection can test*/
    int tmp;
    for (int n = 0; n < N; n++)
    {
        /*selection sort*/
        int min_head = 0;
        int loc = 0;
        for (int a = 0; a < G[n].n_arcs - 1; a++)
        {
            min_head = G[n].arc[G[n].arc_re_no[a]].head;
            loc = a;
            for (int b = a + 1; b < G[n].n_arcs; b++)
            {
                //compare head
                if (G[n].arc[G[n].arc_re_no[b]].head < min_head)
                {
                    min_head = G[n].arc[G[n].arc_re_no[b]].head;
                    loc = b;
                }
            }
            //swap
            tmp = G[n].arc_re_no[a];
            G[n].arc_re_no[a] = G[n].arc_re_no[loc];
            G[n].arc_re_no[loc] = tmp;
        }
    }
    return;
}

void input_points(Graph *G)
{
    /*forward star*/
    //sea network
    int n = 0;
    int now_arc = 0;
    for (int i = 0; i < G[n].n_node; i++)
    {
        for (int a = now_arc; a < G[n].n_arcs; a++)
        {
            if (G[n].arc[a].tail > G[n].n_node)      //last arc
            {
                G[n].forward_point[i] = G[n].n_arcs;
                now_arc = a;
                break;
            }

            if (G[n].arc[a].tail == i)               //the first arc for tail = i
            {
                G[n].forward_point[i] = a;
                now_arc = a;
                break;
            }
            else if (i < G[n].arc[a].tail)          //there is no arc outgoing from tail i
            {
                G[n].forward_point[i] = a;
                now_arc = a;
                break;
            }
        }
    }
    //air network
    n = 1;
    now_arc = 0;
    for (int i = 0; i < G[n].n_node; i++)
    {
        for (int a = now_arc; a < G[n].n_arcs; a++)
        {
            if (G[n].arc[a].tail > G[0].n_node + G[n].n_node)
            {
                G[n].forward_point[i] = G[n].n_arcs;
                now_arc = a;
                break;
            }

            if (G[n].arc[a].tail == i + G[0].n_node)
            {
                G[n].forward_point[i] = a;
                now_arc = a;
                break;
            }
            else if (i + G[0].n_node < G[n].arc[a].tail)
            {
                G[n].forward_point[i] = a;
                now_arc = a;
                break;
            }
        }
    }
    //v102_dummy forward point [sea_nodes + air_nodes + dummy_nodes]
    n = 2;
    now_arc = 0;
    int t_nodes = 2 * G[0].n_node + 2 * G[1].n_node + G[2].n_node;
    for (int i = 0; i < t_nodes; i++)
    {
        for (int a = now_arc; a < G[n].n_arcs; a++)
        {
            if (G[n].arc[a].tail == i)
            {
                G[n].forward_point[i] = a;
                now_arc = a;
                break;
            }
            else if (i < G[n].arc[a].tail)
            {
                G[n].forward_point[i] = a;
                now_arc = a;
                break;
            }
        }
    }

    /*reverse_forward star*/
    //sea network
    n = 0;
    now_arc = 0;
    for (int i = 0; i < G[n].n_node; i++)
    {
        for (int a = now_arc; a < G[n].n_arcs; a++)
        {
            if (G[n].arc[G[n].arc_re_no[a]].head == i)          //the first ingoing arc in node i
            {
                G[n].reverse_point[i] = a;
                now_arc = a;
                break;
            }
            else if (i < G[n].arc[G[n].arc_re_no[a]].head)      //there is no incoming arc in node i
            {
                G[n].reverse_point[i] = a;
                now_arc = a;
                break;
            }
        }
    }

    //air network
    n = 1;
    now_arc = 0;
    for (int i = 0; i < G[n].n_node; i++)
    {
        for (int a = now_arc; a < G[n].n_arcs; a++)
        {
            if (G[n].arc[G[n].arc_re_no[a]].head == i + G[0].n_node)          //the first ingoing arc in node i
            {
                G[n].reverse_point[i] = a;
                now_arc = a;
                break;
            }
            else if (i + G[0].n_node < G[n].arc[G[n].arc_re_no[a]].head)      //there is no incoming arc in node i
            {
                G[n].reverse_point[i] = a;
                now_arc = a;
                break;
            }
        }
    }

    //v102_dummy forward point [sea_nodes + air_nodes + dummy_nodes]
    n = 2;
    now_arc = 0;
    for (int i = 0; i < t_nodes; i++)
    {
        for (int a = now_arc; a < G[n].n_arcs; a++)
        {
            if (G[n].arc[G[n].arc_re_no[a]].head == i)          //the first ingoing arc in node i
            {
                G[n].reverse_point[i] = a;
                now_arc = a;
                break;
            }
            else if (i < G[n].arc[G[n].arc_re_no[a]].head)      //there is no incoming arc in node i
            {
                G[n].reverse_point[i] = a;
                now_arc = a;
                break;
            }
        }
    }
    return;
}

void r_e_arc(string s, eL *e, int num_l)
{
    ifstream fin;

    fin.open(s.c_str(), ios::in);
    if (!fin.is_open())
    {
        cout << "failed to open r_e_arc file!";
        exit(1);
    }
    //cout << "in r_e_arc_info!\n";

    char tmp;
    int n = 0;
    int no = 0;

    int now_tail = 0;
    int *tmp_i;
    tmp_i = new int[num_l];
    for (int i = 0; i < num_l; i++)
    {
        tmp_i[i] = 0;
        //cout << tmp_i[i] << endl;
    }

    while (!fin.eof())
    {

        fin.get(tmp);
        //cout << tmp;
        switch (tmp)
        {

            case 'c':
                //cout << "in c\n";
                fin.ignore(M, '\n');
                break;

            case 'n':
                fin >> n;
                //cout << "graph:" << n << endl;
                break;

            case 'a':
                fin >> no;
                fin >> e[no].arc[tmp_i[no]].tail;
                fin >> e[no].arc[tmp_i[no]].head;
                fin >> e[no].arc[tmp_i[no]].r;
                fin >> e[no].arc[tmp_i[no]].v_c;
                fin >> e[no].arc[tmp_i[no]].fo_c;
                fin >> e[no].arc[tmp_i[no]].t;
                e[no].arc_re_no[tmp_i[no]] = tmp_i[no];

                //fout << no << "\t" << tmp_i[no] << "\t" << e[no].arc[tmp_i[no]].tail << "\t" << e[no].arc[tmp_i[no]].head << "\t" << e[no].arc[tmp_i[no]].r << "\t" << e[no].arc[tmp_i[no]].v_c << "\t" << e[no].arc[tmp_i[no]].fo_c << "\t" << e[no].arc[tmp_i[no]].t << "\t" << e[no].arc_re_no[tmp_i[no]] << endl;

                tmp_i[no]++;
                break;

            default:
                break;
        }
    }

    fin.close();
    //cout << "finish r_e_arc_info\n";
    return;
}

void e_arc_sort(eL *e, int n_line) //v200
{
    //cout << "in e arc sort!\n";
    /*selection can test*/
    arcSet tmp;
    for (int n = 0; n < n_line; n++)
    {
        //selection sort
        int min_tail = 0;
        int loc = 0;
        for (int a = 0; a < e[n].n_arc - 1; a++)
        {
            min_tail = e[n].arc[a].tail;
            loc = a;
            for (int b = a + 1; b < e[n].n_arc; b++)
            {
                //compare head
                if (e[n].arc[b].tail < min_tail)
                {
                    min_tail = e[n].arc[b].tail;
                    loc = b;
                }
            }
            //swap
            tmp = e[n].arc[a];
            e[n].arc[a] = e[n].arc[loc];
            e[n].arc[loc] = tmp;
        }
    }
    return;
}

void reverse_e_arc_sort(eL *e, int n_line)  //v200
{
    /*selection can test*/
    int tmp;
    for (int n = 0; n < n_line; n++)
    {
        /*selection sort*/
        int min_head = 0;
        int loc = 0;
        for (int a = 0; a < e[n].n_arc - 1; a++)
        {
            min_head = e[n].arc[e[n].arc_re_no[a]].head;
            loc = a;
            for (int b = a + 1; b < e[n].n_arc; b++)
            {
                //compare head
                if (e[n].arc[e[n].arc_re_no[b]].head < min_head)
                {
                    min_head = e[n].arc[e[n].arc_re_no[b]].head;
                    loc = b;
                }
            }
            //swap
            tmp = e[n].arc_re_no[a];
            e[n].arc_re_no[a] = e[n].arc_re_no[loc];
            e[n].arc_re_no[loc] = tmp;
        }
    }
    return;
}

void input_e_points(eL *e, int n_line, int sORa) //v200
{
    int before_nodes = G[0].n_node + G[1].n_node + G[2].n_node;
    if (sORa == 1)
        before_nodes += G[0].n_node;
    /*forward star*/
    //sea network
    int now_arc = 0;
    for (int n = 0; n < n_line; n++)
    {
        for (int i = 0; i < G[sORa].n_node; i++)
        {
            for (int a = now_arc; a < e[n].n_arc; a++)
            {
                if (e[n].arc[a].tail > G[sORa].n_node + before_nodes)      //last arc
                {
                    e[n].forward_point[i] = e[n].n_arc;
                    now_arc = a;
                    break;
                }

                if (e[n].arc[a].tail == i + before_nodes)               //the first arc for tail = i
                {
                    e[n].forward_point[i] = a;
                    now_arc = a;
                    break;
                }
                else if (i + before_nodes < e[n].arc[a].tail)          //there is no arc outgoing from tail i
                {
                    e[n].forward_point[i] = a;
                    now_arc = a;
                    break;
                }
            }
        }
    }


    /*reverse_forward star*/
    //sea network
    now_arc = 0;
    for (int n = 0; n < n_line; n++)
    {
        for (int i = 0; i < G[sORa].n_node; i++)
        {
            for (int a = now_arc; a < e[n].n_arc; a++)
            {
                if (e[n].arc[e[n].arc_re_no[a]].head == i + before_nodes)          //the first ingoing arc in node i
                {
                    e[n].reverse_point[i] = a;
                    now_arc = a;
                    break;
                }
                else if (i + before_nodes < e[n].arc[G[n].arc_re_no[a]].head)      //there is no incoming arc in node i
                {
                    e[n].reverse_point[i] = a;
                    now_arc = a;
                    break;
                }
            }
        }
    }

    return;
}

void r_k_path(string s, Commodity *k)
{
    ifstream fin;
    ofstream fout("../model_data/check_ij_path.txt");
    fout << "k\tq\tp\tfrom\tto\ttime[p]\tcost[p]" << endl;

    fin.open(s.c_str(), ios::in);
    if (!fin.is_open())
    {
        cout << "failed to open r_k_path file!";
        exit(1);
    }
    //cout << "in r_k_path_info!\n";

    char tmp;
    int no = 0;
    int q = 0;
    //int i = 0;
    int p = 0;
    int tmp_stop = 0;
    int from = 0;
    int to = 0;
    int count_path = 0;
    Linklist *current;

    while (!fin.eof())
    {

        fin.get(tmp);
        //cout << tmp;
        switch (tmp)
        {

            case 'c':
                //cout << "in c\n";
                fin.ignore(M, '\n');
                break;
            case 'a':
                fin >> total_path;
                tPath = new recordPath[total_path];
                for (int np = 0; np < total_path; np++) {
                    tPath[np].k = -1;
                    tPath[np].q = -1;
                    //tPath[np].i = -1;
                    tPath[np].p = -1;
                    tPath[np].next = NULL;
                }
                break;

            case 'n':
                fin >> no;
                fin >> q;
                //fin >> i;
                fin >> k[no].kq[q].num_p;
                k[no].kq[q].paths = new Linklist[k[no].kq[q].num_p];
                k[no].kq[q].path_time = new int[k[no].kq[q].num_p];
                k[no].kq[q].path_cost = new double[k[no].kq[q].num_p];
                k[no].kq[q].path_no = new int[k[no].kq[q].num_p];


                for (int j = 0; j < k[no].kq[q].num_p; j++)
                {
                    k[no].kq[q].paths[j].node = -1;
                    k[no].kq[q].paths[j].next = NULL;

                    k[no].kq[q].path_time[j] = 0;
                    k[no].kq[q].path_cost[j] = 0;
                    k[no].kq[q].path_no[j] = -1;
                    //k[no].kq[q].estimate[j] = new double[num_scenario];
                    //k[no].kq[q].path_utility[j] = new double[num_scenario];
                    //for(int r = 0; r < num_scenario; r++) {
                    //    k[no].kq[q].estimate[j][r] = 0;
                    //    k[no].kq[q].path_utility[j][r] = 0;
                    //}
                }
                k[no].kq[q].estimate = new double*[total_path];
                k[no].kq[q].path_utility = new double*[total_path];
                //k[no].estimate = new double*[total_path];
                //k[no].path_utility = new double*[total_path];
                for (int pp = 0; pp < total_path; pp++) {
                    k[no].kq[q].estimate[pp] = new double[num_scenario];
                    k[no].kq[q].path_utility[pp] = new double[num_scenario];
                    //k[no].estimate[pp] = new double[num_scenario];
                    //k[no].path_utility[pp] = new double[num_scenario];
                    for (int r = 0; r < num_scenario; r++) {
                        k[no].kq[q].estimate[pp][r] = 0;
                        k[no].kq[q].path_utility[pp][r] = 0;

                        //k[no].estimate[pp][r] = 0;
                        //k[no].path_utility[pp][r] = 0;
                    }
                }

                break;

            case 'k':
                fin >> no;
                fin >> q;
                //fin >> i;
                fin >> p;
                fin >> k[no].kq[q].paths[p].node;
                fin >> tmp_stop;
                k[no].kq[q].path_no[p] = count_path;

                //fout << no << "\t" << q << "\t" << i << "\t" << p << "\t" << count_path << endl;

                current = &k[no].kq[q].paths[p];
                from = k[no].kq[q].paths[p].node;
                while (tmp_stop != -1)
                {
                    //cout << tmp_stop << endl;
                    Linklist *add_next;
                    add_next = new Linklist;
                    add_next->next = NULL;
                    add_next->node = tmp_stop;

                    to = add_next->node;
                    ////check which arc is included in this path & add the time
                    //cout << "here!\n";
                    k[no].kq[q].path_time[p] = k[no].kq[q].path_time[p] + check_ij_path_time(no, q, p, from, to);
                    k[no].kq[q].path_cost[p] = k[no].kq[q].path_cost[p] + check_ij_path_cost(no, q, p, from, to);
                    //cout << "back\n";

                    fout << no << "\t" << q << "\t" << p << "\t" << from << "\t" << to << "\t" << k[no].kq[q].path_time[p] << "\t" << k[no].kq[q].path_cost[p] << endl;

                    current->next = add_next;
                    current = current->next;

                    fin >> tmp_stop;
                    from = to;
                }
                tPath[count_path].k = no;
                tPath[count_path].q = q;
                //tPath[count_path].i = i;
                tPath[count_path].p = p;
                //fout << "count_path\t" << count_path << "\t" << tPath[count_path].k << "\t" << tPath[count_path].q << "\t" << tPath[count_path].i << "\t" << tPath[count_path].p << endl;
                count_path++;

                break;

            default:
                break;
        }
    }

    fin.close();
    //fout.close();
    //cout << "finish r_k_path_info\n";
    return;
}

int check_ij_path_time(int k, int q, int p, int tail, int head)
{
    int arc_time = 0;
    //ofstream fout("check_ij_path.txt",std::ofstream::app);
    //cout << "int check_ij_path\n";
    //fout << k << "\t" << q << "\t" << i << "\t" << p << "\t" << tail << "\t" << head << "\t";
    if (tail < G[0].n_node)
    {
        //cout << "tail in G[sea]\n";
        //tail in G[sea]
        if (head < G[0].n_node)
        {
            //fout << "arc in G[sea]\n";
            //head in G[sea] & arc belong to G[sea]
            for (int l = G[0].forward_point[tail]; l < G[0].forward_point[tail + 1]; l++)
            {
                if (head == G[0].arc[l].head)
                {
                    recordPath *add;
                    add = new recordPath;
                    add->k = k;
                    add->q = q;
                    //add->i = i;
                    add->p = p;
                    add->next = NULL;

                    arc_time = G[0].arc[l].t;
                    //fout << "r\t" << G[0].arc[l].r << "\tl_c\t" << G[0].arc[l].l_c << "\ts_c\t" << G[0].arc[l].s_c << "\t";

                    G[0].arc[l].rear->next = add;
                    G[0].arc[l].rear = G[0].arc[l].rear->next;

                }
            }
        }
        else
        {
            //fout << "arc in G[dummy]\n";
            //head not in G[sea] & arc belong to G[dummy]
            for (int l = G[2].forward_point[tail]; l < G[2].forward_point[tail + 1]; l++)
            {
                if (head == G[2].arc[l].head)
                {
                    recordPath *add;
                    add = new recordPath;
                    add->k = k;
                    add->q = q;
                    //add->i = i;
                    add->p = p;
                    add->next = NULL;

                    arc_time = G[2].arc[l].t;
                    //fout << "r\t" << G[2].arc[l].r << "\tl_c\t" << G[2].arc[l].l_c << "\ts_c\t" << G[2].arc[l].s_c << "\t";

                    G[2].arc[l].rear->next = add;
                    G[2].arc[l].rear = G[2].arc[l].rear->next;
                }
            }

        }
    }
    else if (tail >= G[0].n_node && tail < G[0].n_node + G[1].n_node)
    {
        //tail in G[air]
        if (head < G[0].n_node + G[1].n_node)
        {
            //fout << "arc in G[air]\n";
            //head in G[air] & arc belong to G[air]
            for (int l = G[1].forward_point[tail - G[0].n_node]; l < G[1].forward_point[tail - G[0].n_node + 1]; l++)
            {
                if (head == G[1].arc[l].head)
                {
                    recordPath *add;
                    add = new recordPath;
                    add->k = k;
                    add->q = q;
                    //add->i = i;
                    add->p = p;
                    add->next = NULL;

                    arc_time = G[1].arc[l].t;
                    //fout << "r\t" << G[1].arc[l].r << "\tl_c\t" << G[1].arc[l].l_c << "\ts_c\t" << G[1].arc[l].s_c << "\t";

                    G[1].arc[l].rear->next = add;
                    G[1].arc[l].rear = G[1].arc[l].rear->next;
                }
            }
        }
        else
        {
            //fout << "arc in G[dummy]\n";
            //head not in G[air] & arc belong to G[dummy]
            for (int l = G[2].forward_point[tail]; l < G[2].forward_point[tail + 1]; l++)
            {
                if (head == G[2].arc[l].head)
                {
                    recordPath *add;
                    add = new recordPath;
                    add->k = k;
                    add->q = q;
                    //add->i = i;
                    add->p = p;
                    add->next = NULL;

                    arc_time = G[2].arc[l].t;
                    //fout << "r\t" << G[2].arc[l].r << "\tl_c\t" << G[2].arc[l].l_c << "\ts_c\t" << G[2].arc[l].s_c << "\t";

                    G[2].arc[l].rear->next = add;
                    G[2].arc[l].rear = G[2].arc[l].rear->next;
                }
            }
        }
    }
    else if (tail >= G[0].n_node + G[1].n_node && tail < G[0].n_node + G[1].n_node + G[2].n_node)
    {
        //fout << "arc in G[dummy]\n";
        //tail in G[dummy] & arc belong to G[dummy]
        for (int l = G[2].forward_point[tail]; l < G[2].forward_point[tail + 1]; l++)
        {
            //cout << i << endl;
            if (head == G[2].arc[l].head)
            {
                recordPath *add;
                add = new recordPath;
                add->k = k;
                add->q = q;
                //add->i = i;
                add->p = p;
                add->next = NULL;

                arc_time = G[2].arc[l].t;
                //fout << "r\t" << G[2].arc[l].r << "\tl_c\t" << G[2].arc[l].l_c << "\ts_c\t" << G[2].arc[l].s_c << "\t";

                G[2].arc[l].rear->next = add;
                G[2].arc[l].rear = G[2].arc[l].rear->next;
            }
        }
    }
    else if (tail >= G[0].n_node + G[1].n_node + G[2].n_node && tail < 2 * G[0].n_node + G[1].n_node + G[2].n_node)
    {
        //tail in G[e_sea]
        if (head >= G[0].n_node + G[1].n_node + G[2].n_node)
        {
            //fout << "arc in G[e_sea]\n";
            //head in G[e_sea] & arc belong to G[e_sea]
            for (int l = 0; l < e_sLine; l++)
            {
                for (int a = e_S[l].forward_point[tail - (G[0].n_node + G[1].n_node + G[2].n_node)]; a < e_S[l].forward_point[tail - (G[0].n_node + G[1].n_node + G[2].n_node) + 1]; a++)
                {
                    if (head == e_S[l].arc[a].head)
                    {
                        recordPath *add;
                        add = new recordPath;
                        add->k = k;
                        add->q = q;
                        //add->i = i;
                        add->p = p;
                        add->next = NULL;

                        arc_time = e_S[l].arc[a].t;
                        //fout << "r\t" << e_S[l].arc[a].r << "\tl_c\t" << e_S[l].arc[a].l_c << "\ts_c\t" << e_S[l].arc[a].s_c << "\t";

                        e_S[l].arc[a].rear->next = add;
                        e_S[l].arc[a].rear = e_S[l].arc[a].rear->next;
                    }
                }
            }
        }
        else
        {
            //fout << "arc in G[dummy]\n";
            //head not in G[e_sea] & arc belong to G[dummy]
            for (int a = G[2].forward_point[tail]; a < G[2].forward_point[tail + 1]; a++)
            {
                if (head == G[2].arc[a].head)
                {
                    recordPath *add;
                    add = new recordPath;
                    add->k = k;
                    add->q = q;
                    //add->i = i;
                    add->p = p;
                    add->next = NULL;

                    arc_time = G[2].arc[a].t;
                    //fout << "r\t" << G[2].arc[a].r << "\tl_c\t" << G[2].arc[a].l_c << "\ts_c\t" << G[2].arc[a].s_c << "\t";

                    G[2].arc[a].rear->next = add;
                    G[2].arc[a].rear = G[2].arc[a].rear->next;
                }
            }
        }
    }
    else if (tail >= 2 * G[0].n_node + G[1].n_node + G[2].n_node)
    {
        //tail in G[e_air]
        if (head >= 2 * G[0].n_node + G[1].n_node + G[2].n_node)
        {
            //fout << "arc in G[e_air]\n";
            //head in G[e_air] & arc belong to G[e_air]
            for (int l = 0; l < e_aLine; l++)
            {
                for (int a = e_A[l].forward_point[tail - (2 * G[0].n_node + G[1].n_node + G[2].n_node)]; a < e_A[l].forward_point[tail - (2 * G[0].n_node + G[1].n_node + G[2].n_node) + 1]; a++)
                {
                    if (head == e_A[l].arc[a].head)
                    {
                        recordPath *add;
                        add = new recordPath;
                        add->k = k;
                        add->q = q;
                        //add->i = i;
                        add->p = p;
                        add->next = NULL;

                        arc_time = e_A[l].arc[a].t;
                        //fout << "r\t" << e_A[l].arc[a].r << "\tl_c\t" << e_A[l].arc[a].l_c << "\ts_c\t" << e_A[l].arc[a].s_c << "\t";

                        e_A[l].arc[a].rear->next = add;
                        e_A[l].arc[a].rear = e_A[l].arc[a].rear->next;
                    }
                }
            }
        }
        else
        {
            //fout << "arc in G[dummy]\n";
            // head not in G[e_air] & arc belong to G[dummy]
            for (int a = G[2].forward_point[tail]; a < G[2].forward_point[tail + 1]; a++)
            {
                if (head == G[2].arc[a].head)
                {
                    recordPath *add;
                    add = new recordPath;
                    add->k = k;
                    add->q = q;
                    //add->i = i;
                    add->p = p;
                    add->next = NULL;

                    arc_time = G[2].arc[a].t;
                    //fout << "r\t" << G[2].arc[a].r << "\tl_c\t" << G[2].arc[a].l_c << "\ts_c\t" << G[2].arc[a].s_c << "\t";

                    G[2].arc[a].rear->next = add;
                    G[2].arc[a].rear = G[2].arc[a].rear->next;
                }
            }
        }

    }
    //fout.close();
    return arc_time;
}

double check_ij_path_cost(int k, int q, int p, int tail, int head)
{
    double arc_cost = 0;
    //ofstream fout("check_ij_path.txt",std::ofstream::app);
    //cout << "int check_ij_path\n";
    //fout << k << "\t" << q << "\t" << i << "\t" << p << "\t" << tail << "\t" << head << "\t";
    if (tail < G[0].n_node)
    {
        //cout << "tail in G[sea]\n";
        //tail in G[sea]
        if (head < G[0].n_node)
        {
            //fout << "arc in G[sea]\n";
            //head in G[sea] & arc belong to G[sea]
            for (int l = G[0].forward_point[tail]; l < G[0].forward_point[tail + 1]; l++)
            {
                if (head == G[0].arc[l].head)
                {
                    arc_cost = G[0].arc[l].r;
                    //fout << "r\t" << G[0].arc[l].r << "\tl_c\t" << G[0].arc[l].l_c << "\ts_c\t" << G[0].arc[l].s_c << "\t";
                }
            }
        }
        else
        {
            //fout << "arc in G[dummy]\n";
            //head not in G[sea] & arc belong to G[dummy]
            for (int l = G[2].forward_point[tail]; l < G[2].forward_point[tail + 1]; l++)
            {
                if (head == G[2].arc[l].head)
                {

                    //arc_cost = G[2].arc[l].l_c + G[2].arc[l].s_c;
                    arc_cost = G[2].arc[l].s_c;
                    //fout << "r\t" << G[2].arc[l].r << "\tl_c\t" << G[2].arc[l].l_c << "\ts_c\t" << G[2].arc[l].s_c << "\t";

                }
            }

        }
    }
    else if (tail >= G[0].n_node && tail < G[0].n_node + G[1].n_node)
    {
        //tail in G[air]
        if (head < G[0].n_node + G[1].n_node)
        {
            //fout << "arc in G[air]\n";
            //head in G[air] & arc belong to G[air]
            for (int l = G[1].forward_point[tail - G[0].n_node]; l < G[1].forward_point[tail - G[0].n_node + 1]; l++)
            {
                if (head == G[1].arc[l].head)
                {
                    //arc_cost = K[k].omega * G[1].arc[l].r;
                    arc_cost = G[1].arc[l].r;
                    //fout << "r\t" << G[1].arc[l].r << "\tl_c\t" << G[1].arc[l].l_c << "\ts_c\t" << G[1].arc[l].s_c << "\t";

                }
            }
        }
        else
        {
            //fout << "arc in G[dummy]\n";
            //head not in G[air] & arc belong to G[dummy]
            for (int l = G[2].forward_point[tail]; l < G[2].forward_point[tail + 1]; l++)
            {
                if (head == G[2].arc[l].head)
                {
                    //arc_cost = G[2].arc[l].l_c + G[2].arc[l].s_c;
                    arc_cost = G[2].arc[l].s_c;
                    //fout << "r\t" << G[2].arc[l].r << "\tl_c\t" << G[2].arc[l].l_c << "\ts_c\t" << G[2].arc[l].s_c << "\t";

                }
            }
        }
    }
    else if (tail >= G[0].n_node + G[1].n_node && tail < G[0].n_node + G[1].n_node + G[2].n_node)
    {
        //fout << "arc in G[dummy]\n";
        //tail in G[dummy] & arc belong to G[dummy]
        for (int l = G[2].forward_point[tail]; l < G[2].forward_point[tail + 1]; l++)
        {
            //cout << i << endl;
            if (head == G[2].arc[l].head)
            {
                //arc_cost = G[2].arc[l].l_c + G[2].arc[l].s_c;
                arc_cost = G[2].arc[l].s_c;
                //fout << "r\t" << G[2].arc[l].r << "\tl_c\t" << G[2].arc[l].l_c << "\ts_c\t" << G[2].arc[l].s_c << "\t";

            }
        }
    }
    else if (tail >= G[0].n_node + G[1].n_node + G[2].n_node && tail < 2 * G[0].n_node + G[1].n_node + G[2].n_node)
    {
        //tail in G[e_sea]
        if (head >= G[0].n_node + G[1].n_node + G[2].n_node)
        {
            //fout << "arc in G[e_sea]\n";
            //head in G[e_sea] & arc belong to G[e_sea]
            for (int l = 0; l < e_sLine; l++)
            {
                for (int a = e_S[l].forward_point[tail - (G[0].n_node + G[1].n_node + G[2].n_node)]; a < e_S[l].forward_point[tail - (G[0].n_node + G[1].n_node + G[2].n_node) + 1]; a++)
                {
                    if (head == e_S[l].arc[a].head)
                    {
                        arc_cost = e_S[l].arc[a].r;
                        //fout << "r\t" << e_S[l].arc[a].r << "\tl_c\t" << e_S[l].arc[a].l_c << "\ts_c\t" << e_S[l].arc[a].s_c << "\t";

                    }
                }
            }
        }
        else
        {
            //fout << "arc in G[dummy]\n";
            //head not in G[e_sea] & arc belong to G[dummy]
            for (int a = G[2].forward_point[tail]; a < G[2].forward_point[tail + 1]; a++)
            {
                if (head == G[2].arc[a].head)
                {
                    //arc_cost = G[2].arc[a].l_c + G[2].arc[a].s_c;
                    arc_cost = G[2].arc[a].s_c;
                    //fout << "r\t" << G[2].arc[a].r << "\tl_c\t" << G[2].arc[a].l_c << "\ts_c\t" << G[2].arc[a].s_c << "\t";

                }
            }
        }
    }
    else if (tail >= 2 * G[0].n_node + G[1].n_node + G[2].n_node)
    {
        //tail in G[e_air]
        if (head >= 2 * G[0].n_node + G[1].n_node + G[2].n_node)
        {
            //fout << "arc in G[e_air]\n";
            //head in G[e_air] & arc belong to G[e_air]
            for (int l = 0; l < e_aLine; l++)
            {
                for (int a = e_A[l].forward_point[tail - (2 * G[0].n_node + G[1].n_node + G[2].n_node)]; a < e_A[l].forward_point[tail - (2 * G[0].n_node + G[1].n_node + G[2].n_node) + 1]; a++)
                {
                    if (head == e_A[l].arc[a].head)
                    {
                        //arc_cost = K[k].omega * e_A[l].arc[a].r;
                        arc_cost = e_A[l].arc[a].r;
                        //fout << "r\t" << e_A[l].arc[a].r << "\tl_c\t" << e_A[l].arc[a].l_c << "\ts_c\t" << e_A[l].arc[a].s_c << "\t";

                    }
                }
            }
        }
        else
        {
            //fout << "arc in G[dummy]\n";
            // head not in G[e_air] & arc belong to G[dummy]
            for (int a = G[2].forward_point[tail]; a < G[2].forward_point[tail + 1]; a++)
            {
                if (head == G[2].arc[a].head)
                {
                    //arc_cost = G[2].arc[a].l_c + G[2].arc[a].s_c;
                    arc_cost = G[2].arc[a].s_c;
                    //fout << "r\t" << G[2].arc[a].r << "\tl_c\t" << G[2].arc[a].l_c << "\ts_c\t" << G[2].arc[a].s_c << "\t";

                }
            }
        }

    }
    //fout.close();
    return arc_cost;
}

void generate_random_e(double mean, double stdv)
{
    ofstream outfile("../model_data/info_scenario.txt");
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    //std::default_random_engine generator;
    std::normal_distribution<double> distribution(mean, stdv);
    //the description
    outfile << "\nc\tno\tq\tp\tr\n";
    for (int k = 0; k < n_k; k++) {
        for (int q = 0; q < max_q; q++) {
            for (int p = 0; p < total_path; p++) {
                outfile << "k\t" << k << "\t" << q << "\t" << p << "\t";
                for (int r = 0; r < num_scenario; r++) {
                    /*double tmp = distribution(generator);
					if (tmp < 0)
						tmp = -tmp;
					outfile << tmp << "\t";*/
                    outfile << distribution(generator) << "\t";
                }
                outfile << endl;
            }
        }
    }

    outfile.close();
    return;
}

void r_scenario(string s)
{
    ifstream fin;

    fin.open(s.c_str(), ios::in);
    if (!fin.is_open())
    {
        cout << "failed to open r_scenario file!";
        exit(1);
    }
    //cout << "in r_scenario_info!\n";

    char tmp;
    int no = 0;
    int q = 0;
    //int n = 0;
    int p = 0;
    double scenario_e = 0;

    while (!fin.eof())
    {

        fin.get(tmp);
        //cout << tmp;
        switch (tmp)
        {

            case 'c':
                //cout << "in c\n";
                fin.ignore(M, '\n');
                break;

            case 'k':
                fin >> no;
                /*for(int r = 0; r < num_scenario; r++) {
			fin >> K[no].scenarios[r];
			//cout << K[no].scenarios[r] << endl;
			}*/
                fin >> q;
                //fin >> n;
                fin >> p;
                for (int r = 0; r < num_scenario; r++) {
                    //fin >> K[no].estimate[p][r];
                    fin >> K[no].kq[q].estimate[p][r];
                }

                break;

            default:
                break;
        }
    }
    fin.close();
    //cout << "finish r_scenario!\n";
    return;
}

void calculate_utility()
{
    ofstream outfile("../model_data/utility.txt");
    outfile << "k\tq\tr\tp\tcost\ttime\tutility\te\tutility_e\n";
    for (int k = 0; k < n_k; k++) {
        for (int q = 0; q < max_q; q++) {
            for (int r = 0; r < num_scenario; r++) {
                for (int p = 0; p < K[k].kq[q].num_p; p++) {
                    int real_p = K[k].kq[q].path_no[p];
                    //cost unit: dollar / * d /time unit: timeperiod
                    //K[k].kq[q].path_utility[real_p][r] = K[k].apha * K[k].kq[q].path_cost[p] * K[k].d + K[k].beta * K[k].kq[q].path_time[p];
                    //cost unit: dollar / * d /time unit: hour
                    //K[k].kq[q].path_utility[real_p][r] = K[k].apha * K[k].kq[q].path_cost[p] * K[k].d + K[k].beta * K[k].kq[q].path_time[p] * (24/PERIOD);
                    //cost unit: Euro / without *d / time unit: day
                    //K[k].kq[q].path_utility[real_p][r] = K[k].apha * K[k].kq[q].path_cost[p] * CURRENCY + K[k].beta * (K[k].kq[q].path_time[p] / PERIOD);
                    //cost unit: Euro / *d /time unit: day
                    //K[k].kq[q].path_utility[real_p][r] = K[k].apha * K[k].kq[q].path_cost[p] * CURRENCY * K[k].d + K[k].beta * (K[k].kq[q].path_time[p] / PERIOD);
                    //cost unit: Euro / *d /time unit: hour
                    //K[k].kq[q].path_utility[real_p][r] = K[k].apha * K[k].kq[q].path_cost[p] * CURRENCY * K[k].d + K[k].beta * K[k].kq[q].path_time[p] * (24 / PERIOD);
                    //cost unit: Euro / without *d/ time unit: hour
                    K[k].kq[q].path_utility[real_p][r] = K[k].apha * K[k].kq[q].path_cost[p] * CURRENCY + K[k].beta * K[k].kq[q].path_time[p] * (24 / PERIOD);
                    //cost unit: Euro / without *d/ using value-of-time
                    //K[k].kq[q].path_utility[real_p][r] = K[k].apha * K[k].kq[q].path_cost[p] * CURRENCY + K[k].apha * K[k].beta * K[k].kq[q].path_time[p] * (24 / PERIOD);
                    //cost unit: Euro / *d/ using value-of-time
                    //K[k].kq[q].path_utility[real_p][r] = K[k].apha * K[k].kq[q].path_cost[p] * CURRENCY * K[k].d + K[k].apha * K[k].beta * K[k].kq[q].path_time[p] * (24 / PERIOD);


                    outfile << k << "\t" << q << "\t" << r << "\t" << real_p << "\t" << K[k].kq[q].path_cost[p] << "\t" << K[k].kq[q].path_time[p] << "\t" << K[k].kq[q].path_utility[real_p][r] << "\t";
                    //K[k].kq[q].path_utility[real_p][r] += K[k].kq[q].estimate[real_p][r];
                    K[k].kq[q].path_utility[real_p][r] -= K[k].kq[q].estimate[real_p][r];
                    outfile << K[k].kq[q].estimate[real_p][r] << "\t" << K[k].kq[q].path_utility[real_p][r] << endl;
                }
                if (K[k].kq[q].num_p != 0) {
                    for (int pp = 0; pp < K[k].kq[q].path_no[0]; pp++) {
                        K[k].kq[q].path_utility[pp][r] = 0;
                        //outfile << k << "\t" << q << "\t" << r << "\t" << pp << "\t0\t0\t" << K[k].kq[q].path_utility[pp][r] << "\t";
                        //K[k].kq[q].path_utility[pp][r] += K[k].kq[q].estimate[pp][r];
                        //K[k].kq[q].path_utility[pp][r] -= K[k].kq[q].estimate[pp][r];
                        //outfile << K[k].kq[q].estimate[pp][r] << "\t" << K[k].kq[q].path_utility[pp][r] << endl;
                    }
                    for (int pp = K[k].kq[q].path_no[K[k].kq[q].num_p - 1] + 1; pp < total_path; pp++) {
                        K[k].kq[q].path_utility[pp][r] = 0;
                        //outfile << k << "\t" << q << "\t" << r << "\t" << pp << "\t0\t0\t" << K[k].kq[q].path_utility[pp][r] << "\t";
                        //K[k].kq[q].path_utility[pp][r] += K[k].kq[q].estimate[pp][r];
                        //K[k].kq[q].path_utility[pp][r] -= K[k].kq[q].estimate[pp][r];
                        //outfile << K[k].kq[q].estimate[pp][r] << "\t" << K[k].kq[q].path_utility[pp][r] << endl;
                    }
                }
                if (K[k].kq[q].num_p == 0) {
                    for (int pp = 0; pp < total_path; pp++) {
                        K[k].kq[q].path_utility[pp][r] = 0;
                        //outfile << k << "\t" << q << "\t" << r << "\t" << pp << "\t0\t0\t" << K[k].kq[q].path_utility[pp][r] << "\t";
                        //K[k].kq[q].path_utility[pp][r] += K[k].kq[q].estimate[pp][r];
                        //K[k].kq[q].path_utility[pp][r] -= K[k].kq[q].estimate[pp][r];
                        //outfile << K[k].kq[q].estimate[pp][r] << "\t" << K[k].kq[q].path_utility[pp][r] << endl;
                    }
                }
            }

        }
    }
    return;
}
