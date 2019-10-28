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

#include"csvread.h"
#include"wpdata.h"
#include"tf_lis.h"
#include"wp_realtime_marker.h"

#include<time.h>



using namespace std;


int up_button,down_button,right_button,left_button;
void key_vel_callback(const geometry_msgs::Twist& vel_cmd){ 
     if(vel_cmd.linear.x==2.0){
         up_button=1;
     }
     else{
         up_button=0;
     }
     if(vel_cmd.linear.x==-2.0){
         down_button=1;
     }
     else{
         down_button=0;
     }
     if(vel_cmd.angular.z==2.0){
         left_button=1;
     }
     else{
         left_button=0;
     }
     if(vel_cmd.angular.z==-2.0){
         right_button=1;
     }
     else{
         right_button=0;
     }
}


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

void rs_odom(Vector pos){
   static tf::TransformBroadcaster br;
   tf::Transform transform;
   transform.setOrigin( tf::Vector3(pos.x, pos.y, 0.0) );
   tf::Quaternion q;
   q.setRPY(0, 0, pos.yaw);
   transform.setRotation(q);
   
   br.sendTransform(tf::StampedTransform(transform, ros::Time::now(),"/map", "/rs_odom"));
}

int main(int argc, char **argv){
    
    ros::init(argc, argv, "wp_moving_node");
    ros::NodeHandle n;
    //2D_NAV_GOAL publisher
    ros::Publisher goal_pub = n.advertise<geometry_msgs::PoseStamped>("/move_base_simple/goal", 1);
    
   //十字キー入力 sub
   ros::NodeHandle lSubscriber("");
   ros::Subscriber ket_sub = lSubscriber.subscribe("/turtle1/cmd_vel", 50, key_vel_callback);
    

    //2D_POSE_ESTIMATE pub
    //ros::Publisher initial_pub = n.advertise<geometry_msgs::PoseWithCovarianceStamped>("/initialpose", 1);
    
    //制御周期10ms
    ros::Rate loop_rate(10);
    //csv.print();
    //cout<<endl<<csv.wp.size()<<endl;
    int now_wp=0;
    bool wp_mode=false;
    tf_lis rs_tf("/map","/rs_link");
    tf_lis lidar_tf("/map","/base_link");
    

    wpmarker wpmarker;
    Wpdata rsdata;
    Vector pubodom;
    while (n.ok())  {
       rs_tf.update();
       lidar_tf.update();
       rs_odom(pubodom);
        if(up_button){
            Vector rsodom(0,0,0);
            //rsodom.yaw=0;
            //rsodom=Vector(0,0,0);
            rsodom=(lidar_tf.pos-rs_tf.pos);     
            rsodom=(rsodom-lidar_tf.pos).rad_rot(lidar_tf.pos.yaw-rs_tf.pos.yaw)+lidar_tf.pos;
            rsodom.yaw=(lidar_tf.pos.yaw-rs_tf.pos.yaw);
            pubodom+=rsodom;
            pubodom.yaw+=rsodom.yaw;
            cout<<"Waypoint_mark_start"<<endl;
            goal_pub.publish(csv_write(rs_tf.pos,1));
            rsdata.vec[now_wp]=rs_tf.pos;
            rsdata.vtoa();
            wp_mode=true;
            wpmarker.update(rsdata,now_wp);
        }
        if(down_button){
            //rsodom+=diff_tf.pos;
            //rsodom=lidar_tf.pos;
            //rsodom.yaw+=rs_tf.pos.yaw-lidar_tf.pos.yaw;
            cout<<"Waypoint_mark_start"<<endl;
            goal_pub.publish(csv_write(lidar_tf.pos,1));
            rsdata.vec[now_wp]=lidar_tf.pos;
            rsdata.vtoa();
            wp_mode=true;
            wpmarker.update(rsdata,now_wp);
        }

        if(wp_mode){
            /*
            if((lidar_tf.pos-rsdata.vec[now_wp]).size()>1.0){
                now_wp++;
                cout<<"Waypoint NUMBER < "<<now_wp<<" >"<<endl;
                goal_pub.publish(csv_write(lidar_tf.pos,1));
                rsdata.vec[now_wp]=lidar_tf.pos;
                rsdata.vtoa();
                wpmarker.update(rsdata,now_wp);
            }
            */
            if((rs_tf.pos-rsdata.vec[now_wp]).size()>1.0){
                now_wp++;
                cout<<"Waypoint NUMBER < "<<now_wp<<" >"<<endl;
                goal_pub.publish(csv_write(rs_tf.pos,1));
                rsdata.vec[now_wp]=rs_tf.pos;
                rsdata.vtoa();
                wpmarker.update(rsdata,now_wp);
            }
            
        }
        //cout<<"x="<<lidar_tf.pos.x<<"y="<<lidar_tf.pos.y<<endl;
       // cout<<"rs="<<rs_tf.pos.yaw<<"  lidar="<<lidar_tf.pos.yaw<<"  odom="<<rsodom.yaw<<endl;


        up_button=0;
        down_button=0;
        left_button=0;
        right_button=0;
        ros::spinOnce();//subsucriberの割り込み関数はこの段階で実装される
        loop_rate.sleep();
        
    }
    
    return 0;
}

