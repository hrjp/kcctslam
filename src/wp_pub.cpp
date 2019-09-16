#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>
#include <tf/transform_broadcaster.h>
#include <nav_msgs/Odometry.h>
#include <tf/tf.h>
#include <tf/transform_datatypes.h>
#include <tf/transform_listener.h>
#include <geometry_msgs/Twist.h>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include"goal.h"
#include"csvread.h"
#include"wpdata.h"
#include"tf_lis.h"
#include"wpmarker.h"

using namespace std;

geometry_msgs::Twist nav_vel;//navigation stack の速度指令

 void cmd_vel_callback(const geometry_msgs::Twist& vel_cmd){ 
     nav_vel=vel_cmd;
}

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
    // sleep(1000);
    int now_wp=0;
    ros::init(argc, argv, "wp_pub");
    ros::NodeHandle n;
   
    //ウェイポイントファイルのロード
    ros::NodeHandle pn("~");
    string filename;
    pn.getParam("waypointfile",filename);
    csvread csv(filename.c_str());


    //cmd_velの受信と送信
    ros::NodeHandle lSubscriber("");
    ros::Subscriber sub = lSubscriber.subscribe("/cmd_vel", 50, cmd_vel_callback);
    ros::Subscriber ket_sub = lSubscriber.subscribe("/turtle1/cmd_vel", 50, key_vel_callback);
    ros::Publisher cmd_pub = n.advertise<geometry_msgs::Twist>("final_cmd_vel", 10);  

    //2D_NAV_GOAL publisher
    ros::Publisher goal_pub = n.advertise<geometry_msgs::PoseStamped>("move_base_simple/goal", 1);
    geometry_msgs::PoseStamped goal_point;
    //制御周期10ms
    ros::Rate loop_rate(10);
    //csv.print();
    //cout<<endl<<csv.wp.size()<<endl;

    geometry_msgs::Twist final_cmd_vel;//ロボットに送る速度指令
    geometry_msgs::Twist zero_vel;//停止
    zero_vel.linear.x=0;
    zero_vel.angular.z=0;

    tf_lis base;
    wpmarker wpmarker;
    
    
    
   // Goal init_goal_ob(csv.wp.x(now_wp), csv.wp.y(now_wp),csv.wp.qz(now_wp), csv.wp.qw(now_wp));
    while (n.ok())  {

        //nav stack update
        base.update();
        wpmarker.update(csv.wp,now_wp);
        
         if((base.pos-csv.wp.vec[now_wp]).size()<1.0 && now_wp+1<csv.wp.size()){
            now_wp++;
            cout<<"publishwp="<<now_wp<<endl;
           // Goal goal_ob(csv.wp.x(now_wp), csv.wp.y(now_wp),csv.wp.qz(now_wp), csv.wp.qw(now_wp));
            goal_point.pose.position.x = csv.wp.x(now_wp);
            goal_point.pose.position.y = csv.wp.y(now_wp);
            goal_point.pose.orientation.z =  csv.wp.qz(now_wp);
            goal_point.pose.orientation.w = csv.wp.qw(now_wp);
            goal_point.header.stamp = ros::Time::now();
            goal_point.header.frame_id = "map";
            goal_pub.publish(goal_point);
        }

        static int pushonce=0;
        switch (int(csv.wp.type(now_wp))){
        
        //一時停止
        case WP_STOP:
            if(up_button){
                //入力がきたらナビゲーションを再開する
                csv.wp.typechenge(now_wp,WP_NAVIGATION);
                //now_wp++;
                //Goal goal_ob(csv.wp.x(now_wp), csv.wp.y(now_wp),csv.wp.qz(now_wp), csv.wp.qw(now_wp));
               // pushonce++;
            }
            else{
                final_cmd_vel=zero_vel;
            }
            break;
        
        default:
            final_cmd_vel=nav_vel;
           // pushonce=0;
            break;
        }
        

        
        //cout<<"base_link x="<<base.pos.x<<" y="<<base.pos.y<<" yaw="<<base.pos.yaw;
        //cout<<"waypoint x="<<csv.wp.vec[now_wp].x<<" y="<<csv.wp.vec[now_wp].y<<" yaw="<<csv.wp.vec[now_wp].yaw<<endl;
        cmd_pub.publish(final_cmd_vel);
        ros::spinOnce();
        loop_rate.sleep();
    
    }
    
    return 0;
}





