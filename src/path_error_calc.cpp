
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
#include"Vector.h"


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
     //制御周期10Hz
    ros::Rate loop_rate(10);

     //subscriber
     ros::NodeHandle lSubscriber("");
     ros::Subscriber path_sub = lSubscriber.subscribe("/wp_path", 50, path_callback);
     ros::Subscriber wp_num_sub = lSubscriber.subscribe("/now_wp", 50, wp_num_callback);

     //publisher
     ros::Publisher marker_pub   = n.advertise<visualization_msgs::MarkerArray>("path_error_marker", 1);
     

     //TF listener
     tf_lis base_tf("/map","/base_link");
    

    

     while (n.ok())  {
          base_tf.update();

          int wp_forward=1;
          if(wp_forward<now_wp && now_wp<path.poses.size()){
               //pathの接線を求める
               double x0=path.poses.at(now_wp-1-wp_forward).pose.position.x;
               double y0=path.poses.at(now_wp-1-wp_forward).pose.position.y;
               double x1=path.poses.at(now_wp-wp_forward).pose.position.x;
               double y1=path.poses.at(now_wp-wp_forward).pose.position.y;
               //pathの方程式を求める
               double a0=(y1-y0)/(x1-x0);
               double b0=-a0*x0+y0;
               //自己位置を通りpathに垂直な直線の方程式を求める
               double x2=base_tf.pos.x;
               double y2=base_tf.pos.y;
               double a1=-(1/a0);
               double b1=y2-a1*x2;
               //2直線の交点を求める
               double x3=(b1-b0)/(a0-a1);
               double y3=(a0*b1-b0*a1)/(a0-a1);

               //debag用の可視化処理
               visualization_msgs::MarkerArray debag_marker;
               debag_marker.markers.resize(2);
               debag_marker.markers[0].header.frame_id = "/map";
               debag_marker.markers[0].header.stamp = ros::Time::now();
               debag_marker.markers[0].ns = "path_error";
               debag_marker.markers[0].id = 0;
               debag_marker.markers[0].lifetime = ros::Duration();

               debag_marker.markers[0].type = visualization_msgs::Marker::ARROW;
               debag_marker.markers[0].action = visualization_msgs::Marker::ADD;
               debag_marker.markers[0].scale.x = 2.0;
               debag_marker.markers[0].scale.y = 0.1;
               debag_marker.markers[0].scale.z = 0.1;
               debag_marker.markers[0].pose.position.x=x3;
               debag_marker.markers[0].pose.position.y=y3;
               debag_marker.markers[0].pose.position.z=0;
               debag_marker.markers[0].pose.orientation.x=0;
               debag_marker.markers[0].pose.orientation.y=-1;
               debag_marker.markers[0].pose.orientation.z=0;
               debag_marker.markers[0].pose.orientation.w=1.0;
               debag_marker.markers[0].color.r = 0.0f;
               debag_marker.markers[0].color.g = 0.0f;
               debag_marker.markers[0].color.b = 1.0f;
               debag_marker.markers[0].color.a = 1.0f;

               //
               debag_marker.markers[1].header.frame_id = "/map";
               debag_marker.markers[1].header.stamp = ros::Time::now();
               debag_marker.markers[1].ns = "path_error";
               debag_marker.markers[1].id = 1;
               debag_marker.markers[1].lifetime = ros::Duration();

               debag_marker.markers[1].type = visualization_msgs::Marker::CYLINDER;
               debag_marker.markers[1].action = visualization_msgs::Marker::ADD;
               debag_marker.markers[1].scale.x = 6.0;
               debag_marker.markers[1].scale.y = 0.05;
               debag_marker.markers[1].scale.z = 0.05;
               debag_marker.markers[1].pose.position.x=x3;
               debag_marker.markers[1].pose.position.y=y3;
               debag_marker.markers[1].pose.position.z=0.2;
               Vector get_angle(x2,y1,atan(a0));
               debag_marker.markers[1].pose.orientation.x=get_angle.get_qx();
               debag_marker.markers[1].pose.orientation.y=get_angle.get_qy();
               debag_marker.markers[1].pose.orientation.z=get_angle.get_qz();
               debag_marker.markers[1].pose.orientation.w=get_angle.get_qw();
               debag_marker.markers[1].color.r = 1.0f;
               debag_marker.markers[1].color.g = 0.0f;
               debag_marker.markers[1].color.b = 0.0f;
               debag_marker.markers[1].color.a = 1.0f;


               ROS_INFO("now_wp:%d x:%f y:%f",now_wp,x2,y2);
                marker_pub.publish(debag_marker);

          }
         

          ros::spinOnce();//subsucriberの割り込み関数はこの段階で実装される
          loop_rate.sleep();
     }
    
    return 0;
}