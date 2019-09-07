#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>


#include <tf/transform_broadcaster.h>
#include <nav_msgs/Odometry.h>
#include <tf/tf.h>
#include <tf/transform_datatypes.h>
#include <tf/transform_listener.h>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include"goal.h"
#include"csvread.h"
using namespace std;

 
int main(int argc, char **argv){
    ros::init(argc, argv, "wp_pub");
 
    Goal goal_ob(0.5, -0.5, 0, 1.0);
    csvread csv("~/catkin_ws/src/kcctslam/config/waypointdata/wpdata.csv");
    csv.print();
    cout<<csv.wpdata[1][1];
    
    
    ros::NodeHandle n;
    tf::TransformListener listener(ros::Duration(10));

    ros::NodeHandle private_nh("~");
    string tf_name1;
    string tf_name2;
    private_nh.param("tf_name1",tf_name1,std::string("/base_link"));
    private_nh.param("tf_name2",tf_name2,std::string("/map"));

    double x_m=0.0, y_m=0.0, th_m=0.0;

    tf::StampedTransform trans_slam;

    while (n.ok())  {
    try {
        listener.lookupTransform(tf_name2, tf_name1,ros::Time(0), trans_slam);
        x_m = trans_slam.getOrigin().x();
        y_m = trans_slam.getOrigin().y();
        th_m = tf::getYaw(trans_slam.getRotation());
    }
    catch (tf::TransformException &ex)  {
        ROS_ERROR("%s", ex.what());
        ros::Duration(1.0).sleep();
        continue;
    }
    
    cout<< "SLAM result" << ",";
    cout<< x_m << ",";
    cout<< y_m << ",";
    cout<< th_m/M_PI*180.0<< endl;
    }
    
    return 0;
}
