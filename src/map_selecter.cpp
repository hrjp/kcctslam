#include <ros/ros.h>
#include <nav_msgs/OccupancyGrid.h>
#include<geometry_msgs/PoseWithCovarianceStamped.h>
#include <std_srvs/Empty.h>
#include <nav_msgs/SetMap.h>
#include <std_msgs/Int32.h>
#include <nav_msgs/Path.h>
#include <tf/transform_broadcaster.h>
#include <vector>
#include "Vector.h"
#include "tf_lis.h"
using namespace std;
const int select_maps=5;

vector<nav_msgs::OccupancyGrid> map_array(select_maps);

std_msgs::Int32 map_num;

void map_callback0(const nav_msgs::OccupancyGrid& map){ map_array.at(0)=map;}
void map_callback1(const nav_msgs::OccupancyGrid& map){ map_array.at(1)=map;}
void map_callback2(const nav_msgs::OccupancyGrid& map){ map_array.at(2)=map;}
void map_callback3(const nav_msgs::OccupancyGrid& map){ map_array.at(3)=map;}
void map_callback4(const nav_msgs::OccupancyGrid& map){ map_array.at(4)=map;}


geometry_msgs::PoseWithCovarianceStamped vec_to_PoseWithCovarianceStamped(Vector pos){
            geometry_msgs::PoseWithCovarianceStamped initial_pose;
            initial_pose.pose.pose.position.x = pos.x;
            initial_pose.pose.pose.position.y =pos.y;
            initial_pose.pose.pose.orientation.z = pos.get_qz();
            initial_pose.pose.pose.orientation.w = pos.get_qw();
            initial_pose.header.stamp = ros::Time::now();
            initial_pose.header.frame_id = "map";
            initial_pose.pose.covariance[0]=0.05;//0.25;
            initial_pose.pose.covariance[7]=0.05;//0.25;
            initial_pose.pose.covariance[35]=0.01;//0.06853891945200942;
            return initial_pose;

}

double geometry_quat_to_yaw(geometry_msgs::Quaternion geometry_quat){
    tf::Quaternion quat;
    double roll,pitch,yaw;
    quaternionMsgToTF(geometry_quat, quat);
    tf::Matrix3x3(quat).getRPY(roll, pitch, yaw);  //rpy are Pass by Reference
    return yaw;
}

Vector Pose_to_vec(geometry_msgs::Pose pose){
    Vector ret_vec;
    ret_vec.x=pose.position.x;
    ret_vec.y=pose.position.y;
    ret_vec.yaw=geometry_quat_to_yaw(pose.orientation);
    return ret_vec;
}

bool map_chenge_flag=false;
void int_callback(const std_msgs::Int32& int_data){
    if(map_num.data!=int_data.data){
        map_num=int_data;
        map_chenge_flag=true;
    }

}
int now_wp=0;
void wp_callback(const std_msgs::Int32& wp_data){
    now_wp=wp_data.data;
}
nav_msgs::Path wp_path;
Vector now_wp_vec;
Vector next_wp_vec;
void path_callback(const nav_msgs::Path& path_data){
    wp_path=path_data;
    now_wp_vec=Pose_to_vec(wp_path.poses.at(now_wp).pose);
    next_wp_vec=Pose_to_vec(wp_path.poses.at(now_wp+1).pose);
}
/*
bool map_service_callback(nav_msgs::GetMap::Request &get_map,nav_msgs::GetMap::Response &return_map){
    
    return_map.map=map_array.at(0);
    return true;
}*/

int main(int argc, char **argv){
    ros::init(argc, argv, "map_selecter");
    ros::NodeHandle n;
    
    ros::NodeHandle lSubscriber("");

    ros::Subscriber map_sub0 = lSubscriber.subscribe("/map0", 50, map_callback0);
    ros::Subscriber map_sub1 = lSubscriber.subscribe("/map1", 50, map_callback1);
    ros::Subscriber map_sub2 = lSubscriber.subscribe("/map2", 50, map_callback2);
    ros::Subscriber map_sub3 = lSubscriber.subscribe("/map3", 50, map_callback3);
    ros::Subscriber map_sub4 = lSubscriber.subscribe("/map4", 50, map_callback4);

    map_num.data=0;
    ros::Subscriber int_sub = lSubscriber.subscribe("/map_num", 50, int_callback);
    ros::Subscriber wp_sub = lSubscriber.subscribe("/now_wp", 50, wp_callback);
    ros::Subscriber path_sub = lSubscriber.subscribe("/wp_path", 50, path_callback);
    //ros::ServiceServer map_service = n.advertiseService("static_map",map_service_callback);

    tf_lis lidar_tf("/map","/base_link");

    //制御周期10ms
    ros::Rate loop_rate(100);

    //Map　publisher
    ros::Publisher map_pub = n.advertise<nav_msgs::OccupancyGrid>("/map", 10);
    ros::ServiceClient map_client = n.serviceClient<nav_msgs::SetMap>("set_map");
    while (n.ok())  {
        lidar_tf.update();
        if(map_chenge_flag){
            nav_msgs::SetMap set_map;
            //Vector def_pose(0,0,0);
            //initial poseの計算
            
            Vector diff_vec=lidar_tf.pos-now_wp_vec;
            double diff_rad=next_wp_vec.yaw-now_wp_vec.yaw;
            diff_vec=diff_vec.rad_rot(diff_rad);
            Vector new_vec=next_wp_vec+diff_vec;
            new_vec.yaw=lidar_tf.pos.yaw+diff_rad;
            set_map.request.initial_pose=vec_to_PoseWithCovarianceStamped(new_vec);
            set_map.request.map=map_array.at(map_num.data);
            if(map_client.call(set_map)){
                ROS_INFO("MAP IS CHENGED!! MAP NUMBER[%d]",map_num.data);
            }
            else{
                ROS_INFO("DON'T SET MAP");
            }
            map_chenge_flag=false;
        }
        map_pub.publish(map_array.at(map_num.data));
        ros::spinOnce();
        loop_rate.sleep();
        
    }

        
}