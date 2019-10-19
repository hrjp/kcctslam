#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include"wpdata.h"
#include "Vector.h"

using namespace std;

class csvwrite{
private:
ofstream ofs;
void EulerAnglesToQuaternion(double roll, double pitch, double yaw,double& q0, double& q1, double& q2, double& q3);
int counter;
public:
csvwrite(const char *st);
void write(Vector now_pos);

};


void csvwrite::EulerAnglesToQuaternion(double roll, double pitch, double yaw,double& q0, double& q1, double& q2, double& q3){
    double cosRoll = cos(roll / 2.0);
    double sinRoll = sin(roll / 2.0);
    double cosPitch = cos(pitch / 2.0);
    double sinPitch = sin(pitch / 2.0);
    double cosYaw = cos(yaw / 2.0);
    double sinYaw = sin(yaw / 2.0);

    q0 = cosRoll * cosPitch * cosYaw + sinRoll * sinPitch * sinYaw;
    q1 = sinRoll * cosPitch * cosYaw - cosRoll * sinPitch * sinYaw;
    q2 = cosRoll * sinPitch * cosYaw + sinRoll * cosPitch * sinYaw;
    q3 = cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw;
}

csvwrite::csvwrite(const char *st){
    ofs.open(st);
    counter=0;
    // 開かなかったらエラー
    if (!ofs)
    {
        cout << "Error! File can not be opened" << endl;
        //return 0;
    }
    else{
        cout << "Sucsessful opened" << endl;
        ofs<<",x,y,z,qx,qy,qz,qw,type"<<endl;
    }



}

void csvwrite::write(Vector now_pos){
    double q[4]={0};
    EulerAnglesToQuaternion(0,0,now_pos.yaw,q[0],q[1],q[2],q[3]);
    ofs<<counter<<","<<now_pos.x<<","<<now_pos.y<<","<<0<<","<<q[1]<<","<<q[2]<<","<<q[3]<<","<<q[0]<<endl;
    counter++;
}