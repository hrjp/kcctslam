#pragma once
#include "Vector.h"

const int WP_NAVIGATION=0;
const int WP_STOP=2;

class Wpdata{
    public:
    Wpdata();
    void atov();
    static const int width=10;
    static const int height=100;
    double data[width][height];
    Vector vec[height];
    double x(int num){return data[0][num];}
    double y(int num){return data[1][num];}
    double z(int num){return data[2][num];}
    double qx(int num){return data[3][num];}
    double qy(int num){return data[4][num];}
    double qz(int num){return data[5][num];}
    double qw(int num){return data[6][num];}
    double type(int num){return data[7][num];}
    void typechenge(int num,int type);
    int size();
    private:
};

Wpdata::Wpdata(){
    for(int i;i<width;i++){
        for(int j=0;j<height;j++){
            data[i][j]=0;
        }
    }
}

int Wpdata::size(){
    int data_size=0;
    while(x(data_size)||y(data_size)){
        data_size++;
    }
    return data_size;
}

void Wpdata::atov(){
        for(int j=0;j<height;j++){
            vec[j].x=x(j);
            vec[j].y=y(j);
        }
}

void Wpdata::typechenge(int num,int type){
    data[7][num]=type;
}