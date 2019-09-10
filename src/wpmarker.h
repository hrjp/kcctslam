#pragma once
#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <visualization_msgs/MarkerArray.h>

#include <string>
#include <math.h>
#include <sstream>
#include"wpdata.h"


using namespace std;
class wpmarker{
    public:
    wpmarker();
    void update(Wpdata wp,int now_wp);
    private:
    ros::NodeHandle n;
    ros::Publisher marker_pub;
    ros::Publisher marker_pub1;
};


wpmarker::wpmarker(){
    marker_pub   = n.advertise<visualization_msgs::MarkerArray>("marker_array", 1);
    marker_pub1   = n.advertise<visualization_msgs::MarkerArray>("marker_array1", 1);
}

void wpmarker::update(Wpdata wp,int now_wp){
    visualization_msgs::MarkerArray marker_array;
    marker_array.markers.resize(wp.size());
    visualization_msgs::MarkerArray marker_array1;
    marker_array1.markers.resize(wp.size());
    for(int i=0;i<wp.size();i++){
        //marker0
    marker_array.markers[i].header.frame_id = "/map";
    marker_array.markers[i].header.stamp = ros::Time::now();
    marker_array.markers[i].ns = "cmd_vel_display";
    marker_array.markers[i].id = i;
    marker_array.markers[i].lifetime = ros::Duration();

    marker_array.markers[i].type = visualization_msgs::Marker::ARROW;
    marker_array.markers[i].action = visualization_msgs::Marker::ADD;
    marker_array.markers[i].scale.x = 1.0;
    marker_array.markers[i].scale.y = 0.2;
    marker_array.markers[i].scale.z = 0.2;
    marker_array.markers[i].pose.position.x=wp.x(i);
    marker_array.markers[i].pose.position.y=wp.y(i);
    marker_array.markers[i].pose.position.z=wp.z(i);
    marker_array.markers[i].pose.orientation.x=wp.qx(i);
    marker_array.markers[i].pose.orientation.y=wp.qy(i);
    marker_array.markers[i].pose.orientation.z=wp.qz(i);
    marker_array.markers[i].pose.orientation.w=wp.qw(i);
    if(i==now_wp){
    marker_array.markers[i].color.r = 0.0f;
    marker_array.markers[i].color.g = 0.0f;
    marker_array.markers[i].color.b = 1.0f;
    marker_array.markers[i].color.a = 1.0f;
    }
    else{
    marker_array.markers[i].color.r = 0.0f;
    marker_array.markers[i].color.g = 1.0f;
    marker_array.markers[i].color.b = 0.0f;
    marker_array.markers[i].color.a = 1.0f;
    }

//number
    marker_array1.markers[i].header.frame_id = "/map";
    marker_array1.markers[i].header.stamp = ros::Time::now();
    marker_array1.markers[i].ns = "cmd_vel_display";
    marker_array1.markers[i].id = i;
    marker_array1.markers[i].lifetime = ros::Duration();

    marker_array1.markers[i].type = visualization_msgs::Marker::TEXT_VIEW_FACING;
    marker_array1.markers[i].action = visualization_msgs::Marker::ADD;
    marker_array1.markers[i].scale.x = 1;
    marker_array1.markers[i].scale.y = 1;
    marker_array1.markers[i].scale.z = 1;
    marker_array1.markers[i].pose.position.x=wp.x(i);
    marker_array1.markers[i].pose.position.y=wp.y(i);
    marker_array1.markers[i].pose.position.z=wp.z(i)+1;
    marker_array1.markers[i].pose.orientation.x=wp.qx(i);
    marker_array1.markers[i].pose.orientation.y=wp.qy(i);
    marker_array1.markers[i].pose.orientation.z=wp.qz(i);
    marker_array1.markers[i].pose.orientation.w=wp.qw(i);
    if(i>=now_wp){
    marker_array1.markers[i].color.r = 0.0f;
    marker_array1.markers[i].color.g = 0.0f;
    marker_array1.markers[i].color.b = 0.0f;
    marker_array1.markers[i].color.a = 1.0f;
    }
    else{
         marker_array1.markers[i].color.r = 0.5f;
        marker_array1.markers[i].color.g = 0.5f;
        marker_array1.markers[i].color.b = 0.5f;
        marker_array1.markers[i].color.a = 1.0f;
    }
    ostringstream oss;
    oss<<i;
    marker_array1.markers[i].text= oss.str().c_str();
    }
    marker_pub.publish(marker_array);
    marker_pub1.publish(marker_array1);
}