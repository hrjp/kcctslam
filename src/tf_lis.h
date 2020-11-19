#pragma once
#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>
#include <tf/transform_broadcaster.h>
#include <nav_msgs/Odometry.h>
#include <tf/tf.h>
#include <tf/transform_datatypes.h>
#include <tf/transform_listener.h>
#include <geometry_msgs/Pose.h>
#include <geometry_msgs/PoseStamped.h>
#include <string>
#include "Vector.h"
using namespace std;

class tf_lis{
    public:
    tf_lis(const char *base_id,const char *child_id);
    Vector update();
    Vector pos;
    geometry_msgs::Pose pose;
    geometry_msgs::PoseStamped pose_stamped;

    private:
    ros::NodeHandle n;
    tf::TransformListener listener;
    string tf_name1;
    string tf_name2;
    tf::StampedTransform trans_slam;
};

tf_lis::tf_lis(const char *base_id,const char *child_id):listener(ros::Duration(10)){
    ros::NodeHandle private_nh("~");
    private_nh.param("tf_name1",tf_name1,std::string(child_id));
    private_nh.param("tf_name2",tf_name2,std::string(base_id));

}

Vector tf_lis::update(){
    
     try {
        listener.lookupTransform(tf_name2, tf_name1,ros::Time(0), trans_slam);
        pos.x = trans_slam.getOrigin().x();
        pos.y= trans_slam.getOrigin().y();
        pos.yaw = tf::getYaw(trans_slam.getRotation());
        pose.position.x=trans_slam.getOrigin().x();
        pose.position.y=trans_slam.getOrigin().y();
        pose.position.z=trans_slam.getOrigin().z();
        pose.orientation.x=trans_slam.getRotation().x();
        pose.orientation.y=trans_slam.getRotation().y();
        pose.orientation.z=trans_slam.getRotation().z();
        pose.orientation.w=trans_slam.getRotation().w();
        pose_stamped.header.frame_id=tf_name2;
        pose_stamped.header.stamp=ros::Time::now();
        pose_stamped.pose=pose;
    }
    catch (tf::TransformException &ex)  {
        ROS_ERROR("%s", ex.what());
        ros::Duration(1.0).sleep();
        
    }
    return pos;
}