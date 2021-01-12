
#include <ros/ros.h>
#include <nav_msgs/Path.h>
#include <std_msgs/Float32.h>
#include <std_msgs/Int32.h>
#include <visualization_msgs/MarkerArray.h>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include<cmath>

#include"tf_lis.h"


nav_msgs::Path path;
void path_callback(const nav_msgs::Path& sub_path){ 
     path=sub_path;
}

int now_wp;
void wp_num_callback(const std_msgs::Int32& sub_now_wp){ 
     now_wp=sub_now_wp.data;
}

int main(int argc, char **argv){
    
     ros::init(argc, argv, "path_error_calc");
     ros::NodeHandle n;

     //subscriber
     ros::NodeHandle lSubscriber("");
     ros::Subscriber path_sub = lSubscriber.subscribe("/wp_path", 50, path_callback);
     ros::Subscriber wp_num_sub = lSubscriber.subscribe("/now_wp", 50, wp_num_callback);

     //publisher
     ros::Publisher marker_pub   = n.advertise<visualization_msgs::MarkerArray>("path_error_marker", 1);

     //TF listener
     tf_lis base_tf("map","base_link");
    

    //制御周期10Hz
    ros::Rate loop_rate(10);

     while (n.ok())  {
          base_tf.update();
          if(0<now_wp && now_wp<path.poses.size()){
               //pathの接線を求める
               double x0=path.poses.at(now_wp-1).pose.position.x;
               double y0=path.poses.at(now_wp-1).pose.position.y;
               double x1=path.poses.at(now_wp).pose.position.x;
               double y1=path.poses.at(now_wp).pose.position.y;
          }


          ros::spinOnce();//subsucriberの割り込み関数はこの段階で実装される
          loop_rate.sleep();
     }
    
    return 0;
}