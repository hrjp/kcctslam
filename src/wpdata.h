#pragma once
#include "Vector.h"
/*
const int WP_NAVIGATION=0;
const int WP_DITECTION=1;
const int WP_STOP=2;
*/
const int LIDAR_NAVIGATION=1;
const int RS_NAVIGATION=2;
const int WP_STOP=3;
const int RS_BACK_NAVIGATION=4;
const int CHENGE_RS_NAVIGATION=5;
const int SKIP_WP=6;

class Wpdata{
    public:
    Wpdata();
    void atov();
    void vtoa();
    static const int width=10;
    static const int height=2000;
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
    void EulerAnglesToQuaternion(double roll, double pitch, double yaw,double& q0, double& q1, double& q2, double& q3);
    void QuaternionToEulerAngles(double q0, double q1, double q2, double q3,double& roll, double& pitch, double& yaw);
};

Wpdata::Wpdata(){
    for(int i;i<width;i++){
        for(int j=0;j<height;j++){
            data[i][j]=0;
        }
    }
}

void Wpdata::QuaternionToEulerAngles(double q0, double q1, double q2, double q3,double& roll, double& pitch, double& yaw)
{
    double q0q0 = q0 * q0;
    double q0q1 = q0 * q1;
    double q0q2 = q0 * q2;
    double q0q3 = q0 * q3;
    double q1q1 = q1 * q1;
    double q1q2 = q1 * q2;
    double q1q3 = q1 * q3;
    double q2q2 = q2 * q2;
    double q2q3 = q2 * q3;
    double q3q3 = q3 * q3;
    roll = atan2(2.0 * (q2q3 + q0q1), q0q0 - q1q1 - q2q2 + q3q3);
    pitch = asin(2.0 * (q0q2 - q1q3));
    yaw = atan2(2.0 * (q1q2 + q0q3), q0q0 + q1q1 - q2q2 - q3q3);
}

void Wpdata::EulerAnglesToQuaternion(double roll, double pitch, double yaw,double& q0, double& q1, double& q2, double& q3){
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
            double roll_,pitch_,yaw_;
            QuaternionToEulerAngles(qw(j),qx(j),qy(j),qz(j),roll_,pitch_,yaw_);
            vec[j].yaw=yaw_;
        }
}

void Wpdata::vtoa(){
        for(int j=0;j<height;j++){
            data[0][j]=vec[j].x;
            data[1][j]=vec[j].y;
            double q[4];
            EulerAnglesToQuaternion(0, 0,vec[j].yaw,q[0], q[1], q[2], q[3]);
            data[5][j]=q[3];
            data[6][j]=q[0];
        }
}

void Wpdata::typechenge(int num,int type){
    data[7][num]=type;
}