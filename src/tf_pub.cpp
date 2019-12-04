#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>
#include <tf/transform_broadcaster.h>
#include <nav_msgs/Odometry.h>
#include <tf/tf.h>
#include <tf/transform_datatypes.h>
#include <tf/transform_listener.h>
#include <tf/transform_broadcaster.h>
#include <geometry_msgs/Twist.h>
#include<geometry_msgs/PoseWithCovarianceStamped.h>
#include <std_msgs/Float32.h>


#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include<cmath>
#include"tf_lis.h"

void rs_odom(Vector pos){
   static tf::TransformBroadcaster br;
   tf::Transform transform;
   transform.setOrigin( tf::Vector3(pos.x, pos.y, 0.0) );
   tf::Quaternion q;
   q.setRPY(0, 0, pos.yaw);
   transform.setRotation(q);
   
   br.sendTransform(tf::StampedTransform(transform, ros::Time::now(),"/odom", "/base_link"));
}


int main(int argc, char **argv){


    
    ros::init(argc, argv, "tf_pub");
    ros::NodeHandle n;
    //制御周期10ms
    ros::Rate loop_rate(10);

    tf_lis rs_tf("/rs_odom_frame","/rs_camera_link");

     while (n.ok())  {
         rs_tf.update();
         rs_odom(rs_tf.pos);
         ros::spinOnce();//subsucriberの割り込み関数はこの段階で実装される
        loop_rate.sleep();

     }
}