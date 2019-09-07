#pragma once
#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>


class Goal{
public:
    Goal(double px, double py, double pz, double ow);
    ~Goal();
 
private:
    ros::Publisher pub;
    ros::NodeHandle nh;
};
 
Goal::Goal(double px, double py, double pz, double ow){
    pub = nh.advertise<geometry_msgs::PoseStamped>("move_base_simple/goal", 1);
    //pub = nh.advertise<geometry_msgs::PoseStamped>("/move_base/goal", 1);
    ros::Rate one_sec(1);
    one_sec.sleep();
     
    ros::Time time = ros::Time::now();
    geometry_msgs::PoseStamped goal_point;
 
    goal_point.pose.position.x = px;
    goal_point.pose.position.y = py;
    goal_point.pose.position.z =  pz;
    goal_point.pose.orientation.w = ow;
    goal_point.header.stamp = time;
    goal_point.header.frame_id = "map";
 
    pub.publish(goal_point);
 
}
 
Goal::~Goal(){
 
}
