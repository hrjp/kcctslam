#pragma once
#include<math.h>
#define PI 3.1415926535
class Vector{
public:
Vector(float init_x,float init_y);
Vector(float init_x,float init_y,float init_yaw);
Vector(){}

float x,y;
float yaw;
float rad();
float deg();
float size();

void clear();

Vector rot(float deg);

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

void Vector::clear(){
    x=y=yaw=0;
}

Vector Vector::operator()(float init_x,float init_y){
    Vector ret_vector(init_x,init_y);
    return ret_vector;
}

