#pragma once
#include<math.h>
#define PI 3.1415926535
class Vector{
public:
Vector(float init_x,float init_y);
Vector(float init_x,float init_y,float init_yaw);
Vector(){}

float x,y,z;
float yaw;
int type;
int map;
float rad();
float deg();
float size();

float get_qw();
float get_qx();
float get_qy();
float get_qz();

void clear();

Vector rot(float deg);
Vector rad_rot(float rad);

Vector operator+(Vector vec);
Vector operator-(Vector vec);
Vector operator*(float cons);

Vector operator+=(Vector vec);
Vector operator-=(Vector vec);
Vector operator*=(float cons);

Vector operator=(Vector vec);

bool operator==(Vector vec);
bool operator!=(Vector vec);

Vector operator()(float init_x,float init_y);

private:
void EulerAnglesToQuaternion(double roll, double pitch, double yaw,double& q0, double& q1, double& q2, double& q3);
};
Vector::Vector(float init_x,float init_y){
    x=init_x;
    y=init_y;
}
Vector::Vector(float init_x,float init_y,float init_yaw){
    x=init_x;
    y=init_y;
    yaw=init_yaw;
}
float Vector::rad(){return atan2(y,x);}
float Vector::deg(){return rad()*180/PI;}
float Vector::size(){return sqrt(x*x+y*y);}

float Vector::get_qw(){
    double q[4]={0};
    EulerAnglesToQuaternion(0,0,this->yaw,q[0],q[1],q[2],q[3]);
    return q[0];
}
float Vector::get_qx(){
    double q[4]={0};
    EulerAnglesToQuaternion(0,0,this->yaw,q[0],q[1],q[2],q[3]);
    return q[1];
}
float Vector::get_qy(){
    double q[4]={0};
    EulerAnglesToQuaternion(0,0,this->yaw,q[0],q[1],q[2],q[3]);
    return q[2];
}
float Vector::get_qz(){
    double q[4]={0};
    EulerAnglesToQuaternion(0,0,this->yaw,q[0],q[1],q[2],q[3]);
    return q[3];
}

Vector Vector::operator+(Vector vec){
    Vector ans;
    ans.x=this->x+vec.x;
    ans.y=this->y+vec.y;
    return ans;
}
Vector Vector::operator+=(Vector vec){
    x=this->x+vec.x;
    y=this->y+vec.y;
    return *this;
}

Vector Vector::operator-(Vector vec){
    Vector ans;
    ans.x=this->x-vec.x;
    ans.y=this->y-vec.y;
    return ans;
}
Vector Vector::operator-=(Vector vec){
    x=this->x-vec.x;
    y=this->y-vec.y;
    return *this;
}

Vector Vector::operator=(Vector vec){
    this->x=vec.x;
    this->y=vec.y;
    this->yaw=vec.yaw;
    return *this;
}

Vector Vector::operator*(float cons){
    Vector ans;
    ans.x=cons*x;
    ans.y=cons*y;
    return ans;
}
Vector Vector::operator*=(float cons){
    x=cons*x;
    y=cons*y;
    return *this;
}

bool Vector::operator==(Vector vec){
    if(x==vec.x&&y==vec.y){
        return true;
    }
    else{
        return false;
    }
}
bool Vector::operator!=(Vector vec){
    if(x==vec.x&&y==vec.y){
        return false;
    }
    else{
        return true;
    }
}

Vector Vector::rot(float deg){
    Vector ans;
    float rad=deg*PI/180;
    ans.x=cos(rad)*x-sin(rad)*y;
    ans.y=sin(rad)*x+cos(rad)*y;
    return ans;
}

Vector Vector::rad_rot(float rad){
    Vector ans;
    ans.x=cos(rad)*x-sin(rad)*y;
    ans.y=sin(rad)*x+cos(rad)*y;
    return ans;
}

void Vector::clear(){
    x=y=yaw=0;
}

Vector Vector::operator()(float init_x,float init_y){
    Vector ret_vector(init_x,init_y);
    return ret_vector;
}

void Vector::EulerAnglesToQuaternion(double roll, double pitch, double yaw,double& q0, double& q1, double& q2, double& q3){
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