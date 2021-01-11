
#include <ros/ros.h>
#include <nav_msgs/Path.h>
#include <std_msgs/Float32.h>
#include <std_msgs/Int32.h>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include<cmath>

#include"tf_lis.h"

int main(int argc, char **argv){
    
    ros::init(argc, argv, "path_error_calc");
    ros::NodeHandle n;
    //制御周期10ms
    ros::Rate loop_rate(10);

    while (n.ok())  {


        ros::spinOnce();//subsucriberの割り込み関数はこの段階で実装される
        loop_rate.sleep();
        
    }
    
    return 0;