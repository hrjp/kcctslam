#pragma once
#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>
#include <tf/transform_broadcaster.h>
#include <nav_msgs/Odometry.h>
#include <tf/tf.h>
#include <tf/transform_datatypes.h>
#include <tf/transform_listener.h>
#include <string>
#include "Vector.h"
using namespace std;

class tf_lis{
    public:
    tf_lis();
    Vector update();
    Vector pos;
    private:
    ros::NodeHandle n;
    tf::TransformListener listener;
    string tf_name1;
    string tf_name2;
    tf::StampedTransform trans_slam;
};

tf_lis::tf_lis():listener(ros::Duration(10)){
    ros::NodeHandle private_nh("~");
    private_nh.param("tf_name1",tf_name1,std::string("/base_link"));
    private_nh.param("tf_name2",tf_name2,std::string("/map"));

}

Vector tf_lis::update(){
    
     try {
        listener.lookupTransform(tf_name2, tf_name1,ros::Time(0), trans_slam);
        pos.x = trans_slam.getOrigin().x();
        pos.y= trans_slam.getOrigin().y();
        pos.yaw = tf::getYaw(trans_slam.getRotation());
    }
    catch (tf::TransformException &ex)  {
        ROS_ERROR("%s", ex.what());
        ros::Duration(1.0).sleep();
        
    }
    return pos;
}