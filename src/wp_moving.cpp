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

#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include"csvread2.h"
#include"wpdata.h"
#include"tf_lis.h"
#include"wp_realtime_marker.h"
#include"odom_mode.h"

#include<time.h>
#include <nav_msgs/Path.h>




using namespace std;

//key input
int up_button,down_button,right_button,left_button;
void key_vel_callback(const geometry_msgs::Twist& vel_cmd){ 
    up_button=(vel_cmd.linear.x==2.0);
    down_button=(vel_cmd.linear.x==-2.0);
    left_button=(vel_cmd.angular.z==2.0);
    right_button=(vel_cmd.angular.z==-2.0);
}

//key reset
void key_reset(){
    up_button=0;
    down_button=0;
    left_button=0;
    right_button=0;
}

//2d nav goal pub用データ
geometry_msgs::PoseStamped csv_write(Vector pos,int type){
            geometry_msgs::PoseStamped goal_point;
            goal_point.pose.position.x = pos.x;
            goal_point.pose.position.y = pos.y;
            goal_point.pose.position.z = type;
            goal_point.pose.orientation.z =  pos.get_qz();
            goal_point.pose.orientation.w = pos.get_qw();
            goal_point.header.stamp = ros::Time::now();
            goal_point.header.frame_id = "map";
            return goal_point;
}
geometry_msgs::PoseWithCovarianceStamped init_pose(Vector pos){
            geometry_msgs::PoseWithCovarianceStamped initial_pose;
            initial_pose.pose.pose.position.x = pos.x;
            initial_pose.pose.pose.position.y =pos.y;
            initial_pose.pose.pose.orientation.z = pos.get_qz();
            initial_pose.pose.pose.orientation.w = pos.get_qw();
            initial_pose.header.stamp = ros::Time::now();
            initial_pose.header.frame_id = "map";
            return initial_pose;

}

geometry_msgs::PoseStamped Vec_to_PoseStamped(Vector pos,int type){
            geometry_msgs::PoseStamped goal_point;
            goal_point.pose.position.x = pos.x;
            goal_point.pose.position.y = pos.y;
            goal_point.pose.position.z = pos.z;
            goal_point.pose.orientation.z =  pos.get_qz();
            goal_point.pose.orientation.w = pos.get_qw();
            goal_point.header.stamp = ros::Time::now();
            goal_point.header.frame_id = "map";
            return goal_point;
}
//TF rs_odomの配信
void rs_odom(Vector pos){
   static tf::TransformBroadcaster br;
   tf::Transform transform;
   transform.setOrigin( tf::Vector3(pos.x, pos.y, 0.0) );
   tf::Quaternion q;
   q.setRPY(0, 0, pos.yaw);
   transform.setRotation(q);
   
   br.sendTransform(tf::StampedTransform(transform, ros::Time::now(),"/map", "/rs_odom"));
}

//realsenseのTFをLIDARのTFと一致させる
Vector rs_odom_attach(Vector rs_tf,Vector lidar_tf,Vector pubodom){
    Vector rsodom(0,0,0);
    rsodom=(lidar_tf-rs_tf);     
    rsodom=(rsodom-lidar_tf).rad_rot(lidar_tf.yaw-rs_tf.yaw)+lidar_tf;
    rsodom.yaw=(lidar_tf.yaw-rs_tf.yaw);
    pubodom+=rsodom;
    pubodom.yaw+=rsodom.yaw;
    return pubodom;
}

//TF odom_linkの配信
void odomtf_pub(Vector pos){
   static tf::TransformBroadcaster br;
   tf::Transform transform;
   transform.setOrigin( tf::Vector3(pos.x, pos.y, 0.0) );
   tf::Quaternion q;
   q.setRPY(0, 0, pos.yaw);
   transform.setRotation(q);
   
   br.sendTransform(tf::StampedTransform(transform, ros::Time::now(),"/map", "/odom_link"));
}
nav_msgs::Path path;
void posestamped_to_path(geometry_msgs::PoseStamped pose_stamped){
    path.poses.push_back(pose_stamped);
}

Vector pose_position_to_vec(geometry_msgs::Pose pose){
    Vector vec;
    vec.x=pose.position.x;
    vec.y=pose.position.y;
    vec.z=pose.position.z;
    //vec.yaw=pose.position.;
}

int main(int argc, char **argv){
    
    ros::init(argc, argv, "wp_moving_node");
    ros::NodeHandle n;

    //2D_NAV_GOAL publisher
    ros::Publisher goal_pub = n.advertise<geometry_msgs::PoseStamped>("/move_base_simple/goal", 1);
    
   //十字キー入力 subscliber
   ros::NodeHandle lSubscriber("");
   ros::Subscriber ket_sub = lSubscriber.subscribe("/turtle1/cmd_vel", 50, key_vel_callback);
    

    //2D_POSE_ESTIMATE publisher
    ros::Publisher initial_pub = n.advertise<geometry_msgs::PoseWithCovarianceStamped>("/initialpose", 1);

    //Path publisher
    ros::Publisher path_pub = n.advertise<nav_msgs::Path>("/Path", 1);
    
    
    //制御周期10ms
    ros::Rate loop_rate(10);

    int now_wp=0;
    int wp_mode=false;
    int wp_off=10;
    //const int LIDAR_NAVIGATION=1;
    //const int RS_NAVIGATION=2;

    //tf_lis rs_tf("/map","/rs_link");
    tf_lis lidar_tf("/map","/base_link");
    tf_lis odom_tf("/map","/odom_link");
    Vector pre_vec;

   // wpmarker wpmarker;
    //Wpdata rsdata;
    //Vector pubodom;
    //odom_mode odom_mode;

    while (n.ok())  {
       //rs_tf.update();
       lidar_tf.update();
       //rs_odom(pubodom);
       //odomtf_pub(odom_mode.update());
        
        if(wp_mode==LIDAR_NAVIGATION){
            //pubodom=rs_odom_attach(rs_tf.pos,lidar_tf.pos,pubodom);
            //odom_mode.attach();
            if((lidar_tf.pos-pre_vec).size()>1.0){
                posestamped_to_path(lidar_tf.pose_stamped);
                pre_vec=lidar_tf.pos;
                now_wp++;
                cout<<"Waypoint NUMBER : [ "<<now_wp<<" ] (LiDAR)"<<endl;
                //goal_pub.publish(csv_write(lidar_tf.pos,LIDAR_NAVIGATION));
                //rsdata.vec[now_wp]=lidar_tf.pos;
                //rsdata.vtoa();
                //wpmarker.update(rsdata,now_wp);
            }
        }
/*
        if(wp_mode==ODOM_NAVIGATION){
            if((lidar_tf.pos-rsdata.vec[now_wp]).size()>1.0){
                now_wp++;
                cout<<"Waypoint NUMBER : [ "<<now_wp<<" ] (ODOM)"<<endl;
                goal_pub.publish(csv_write(lidar_tf.pos,ODOM_NAVIGATION));
                rsdata.vec[now_wp]=lidar_tf.pos;
                rsdata.vtoa();
                wpmarker.update(rsdata,now_wp);
            }
        }
        */
/*
        if(wp_mode==RS_BACK_NAVIGATION){
            if((rs_tf.pos-rsdata.vec[now_wp]).size()>1.0){
                now_wp++;
                cout<<"Waypoint NUMBER : [ "<<now_wp<<" ] (REALSENSE_BACK)"<<endl;
                goal_pub.publish(csv_write(rs_tf.pos,RS_BACK_NAVIGATION));
                rsdata.vec[now_wp]=rs_tf.pos;
                rsdata.vtoa();
                wpmarker.update(rsdata,now_wp);
            }
        }
*/
        if(up_button){
            /*
            if(wp_mode==RS_NAVIGATION){
                initial_pub.publish(init_pose(rs_tf.pos));
            }
            */
            wp_mode=LIDAR_NAVIGATION;
            /*
           // pubodom=rs_odom_attach(rs_tf.pos,lidar_tf.pos,pubodom);
            cout<<"LIDAR MODE SELECT"<<endl;
            if(now_wp==0){
                //odom_tf.update();
                lidar_tf.update();
                rs_odom(pubodom);
                cout<<"Waypoint NUMBER : [ "<<now_wp<<" ] (LiDAR)"<<endl;
                goal_pub.publish(csv_write(lidar_tf.pos,LIDAR_NAVIGATION));
                rsdata.vec[now_wp]=lidar_tf.pos;
            }
            
            rsdata.vtoa();
            wpmarker.update(rsdata,now_wp);
            */
        }
        /*
        if(down_button){
            wp_mode=ODOM_NAVIGATION;
            
           // pubodom=rs_odom_attach(rs_tf.pos,lidar_tf.pos,pubodom);
            cout<<"ODOM MODE SELECT"<<endl;
            if(now_wp==0){
                //odom_tf.update();
                lidar_tf.update();
                rs_odom(pubodom);
                cout<<"Waypoint NUMBER : [ "<<now_wp<<" ] (ODOM)"<<endl;
                goal_pub.publish(csv_write(lidar_tf.pos,ODOM_NAVIGATION));
                rsdata.vec[now_wp]=lidar_tf.pos;
            }
            
            rsdata.vtoa();
            wpmarker.update(rsdata,now_wp);
           
        }
        
        if(left_button){
            cout<<"STOP POINT MARKING"<<endl;
            
                //rs_tf.update();
                lidar_tf.update();
                rs_odom(pubodom);
                cout<<"Waypoint NUMBER : [ "<<now_wp<<" ] (STOP)"<<endl;
                goal_pub.publish(csv_write(lidar_tf.pos,WP_STOP));
                rsdata.vec[now_wp]=lidar_tf.pos;
            
            rsdata.vtoa();
            wpmarker.update(rsdata,now_wp);
        }
        */
        if(right_button){
            wp_mode=false;
        }
        static int pre_wp_mode;
        pre_wp_mode=wp_mode?wp_mode:pre_wp_mode;
        if(wp_off<3){
            wp_mode=false;
            wp_off++;
            if(wp_off<3){

            }
            else{
                wp_mode=pre_wp_mode;
            }
        }
       

        //cout<<"x="<<lidar_tf.pos.x<<"y="<<lidar_tf.pos.y<<endl;
       // cout<<"rs="<<rs_tf.pos.yaw<<"  lidar="<<lidar_tf.pos.yaw<<"  odom="<<rsodom.yaw<<endl;
        //cout<<loop_rate.cycleTime()<<endl;
        path.header.frame_id="map";
        path.header.stamp=ros::Time::now();
        path_pub.publish(path);
        key_reset();
        ros::spinOnce();//subsucriberの割り込み関数はこの段階で実装される
        loop_rate.sleep();
        
    }
    
    return 0;
}

