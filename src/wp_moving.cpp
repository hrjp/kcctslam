#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>
#include <tf/transform_broadcaster.h>
#include <nav_msgs/Odometry.h>
#include <tf/tf.h>
#include <tf/transform_datatypes.h>
#include <tf/transform_listener.h>
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
#include"realsense_lis.h"
#include"csvwrite.h"

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




int main(int argc, char **argv){
    
    ros::init(argc, argv, "wp_moving_node");
    ros::NodeHandle n;
   
    //ウェイポイントファイルのロード
    /*
    ros::NodeHandle pn("~");
    string filename;
    pn.getParam("waypointfile",filename);
    csvread csv(filename.c_str());*/
    csvwrite csvw("wptest.csv");
    //csvw.write(a);
    //csvw.write(a);
    //cmd_velの受信と送信
    /*
    ros::NodeHandle lSubscriber("");
    ros::Subscriber sub = lSubscriber.subscribe("/cmd_vel", 50, cmd_vel_callback);
    ros::Subscriber ket_sub = lSubscriber.subscribe("/turtle1/cmd_vel", 50, key_vel_callback);
    ros::Publisher cmd_pub = n.advertise<geometry_msgs::Twist>("final_cmd_vel", 10);  
    ros::Subscriber dis_sub = lSubscriber.subscribe("/pcl_handler/front_dist", 50, dis_vel_callback);
    */
   ros::NodeHandle lSubscriber("");
   ros::Subscriber ket_sub = lSubscriber.subscribe("/turtle1/cmd_vel", 50, key_vel_callback);
    //2D_NAV_GOAL publisher
    //ros::Publisher goal_pub = n.advertise<geometry_msgs::PoseStamped>("/move_base_simple/goal", 1);

    //2D_POSE_ESTIMATE pub
    //ros::Publisher initial_pub = n.advertise<geometry_msgs::PoseWithCovarianceStamped>("/initialpose", 1);
    
    //制御周期10ms
    ros::Rate loop_rate(10);
    //csv.print();
    //cout<<endl<<csv.wp.size()<<endl;
    int now_wp=0;
    bool wp_mode=false;
    realsense_lis now_tf;
    wpmarker wpmarker;
    Wpdata rsdata;

    while (n.ok())  {
       now_tf.update();
       
        if(up_button){
            cout<<"Waypoint_mark_start"<<endl;
            csvw.write(now_tf.pos);
            rsdata.vec[now_wp]=now_tf.pos;
            rsdata.vtoa();
            wp_mode=true;
            wpmarker.update(rsdata,now_wp);
        }

        if(wp_mode){
            if((now_tf.pos-rsdata.vec[now_wp]).size()>1.0){
                now_wp++;
                cout<<"Waypoint NUMBER < "<<now_wp<<" >"<<endl;
                csvw.write(now_tf.pos);
                rsdata.vec[now_wp]=now_tf.pos;
                rsdata.vtoa();
                wpmarker.update(rsdata,now_wp);
            }
        }



        up_button=0;
        down_button=0;
        left_button=0;
        right_button=0;
        ros::spinOnce();//subsucriberの割り込み関数はこの段階で実装される
        loop_rate.sleep();
        
    }
    
    return 0;
}

