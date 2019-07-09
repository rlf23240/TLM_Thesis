//
// Created by Ashee on 2019/7/7.
//

#include "gurobi_c++.h"
#include "iostream"
#include "src/EntireNetwork.h"

using namespace std;

int main(){

    try{
        GRBEnv env = GRBEnv();
        GRBModel model = GRBModel(env);

        //set cruise


    } catch(GRBException e) {
        cout << "Error code = " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    } catch(...) {
        cout << "Exception during optimization" << endl;
    }
}