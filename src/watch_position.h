#pragma once
#include"Vector.h"
class watch_position{
public:
watch_position(double ac_circle_);
void update(Vector watch_pos);
bool ok();
private:
double ac_circle;
Vector pre_pos;
bool pos_ok;
};

watch_position::watch_position(double ac_circle_){
    ac_circle=ac_circle_;
}


void watch_position::update(Vector watch_pos){
    if((watch_pos-pre_pos).size()<ac_circle){
        pos_ok=true;
    }
    else{
        pos_ok=false;
    }
    pre_pos=watch_pos;
}

bool watch_position::ok(){
    return pos_ok;
}