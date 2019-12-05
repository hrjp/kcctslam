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
#include"wpmarker.h"

#include<time.h>

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;


using namespace std;

geometry_msgs::Twist nav_vel;//navigation stack の速度指令

geometry_msgs::Twist final_cmd_vel;//ロボットに送る速度指令

double human_dis=0;


//cmd_repub
clock_t cstart=clock();


void dis_vel_callback(const geometry_msgs::Twist& vel_cmd){ 
     human_dis=vel_cmd.linear.x;
}

 void cmd_vel_callback(const geometry_msgs::Twist& vel_cmd){ 
     nav_vel=vel_cmd;

     cstart=clock();
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
    ros::Subscriber dis_sub = lSubscriber.subscribe("/pcl_handler/front_dist", 50, dis_vel_callback);
    //2D_NAV_GOAL publisher
    ros::Publisher goal_pub = n.advertise<geometry_msgs::PoseStamped>("/move_base_simple/goal", 1);

    //2D_POSE_ESTIMATE pub
    ros::Publisher initial_pub = n.advertise<geometry_msgs::PoseWithCovarianceStamped>("/initialpose", 1);
    
    //制御周期10ms
    ros::Rate loop_rate(10);
    //csv.print();
    //cout<<endl<<csv.wp.size()<<endl;

    

    geometry_msgs::Twist zero_vel;//停止
    zero_vel.linear.x=0;
    zero_vel.angular.z=0;

    tf_lis base("/map","/base_link");
    tf_lis rs("/rs_link","/rs_odom_frame");
    wpmarker wpmarker;
    
    clock_t start=clock();
    clock_t end=clock();

    //tell the action client that we want to spin a thread by default
    MoveBaseClient ac("move_base", true);

    while (n.ok())  {

        //nav stack update
        base.update();
        rs.update();
        wpmarker.update(csv.wp,now_wp);
        
       if(nav_vel.linear.x==0&&nav_vel.angular.z==0&&(int(csv.wp.type(now_wp)))==0){
       }
       else{
             start= clock();
       }
       

        clock_t end = clock();
        
        const double repub_time = static_cast<double>(end - start) / CLOCKS_PER_SEC * 10000.0;
        //cout<<repub_time<<endl;
        if(repub_time>5000){
             start= clock();
            cout<<"publishwp="<<now_wp<<endl;


            move_base_msgs::MoveBaseGoal goal;

            //we'll send a goal to the robot to move 1 meter forward
            goal.target_pose.header.frame_id = "map";
           //goal.target_pose.header.frame_id = "base_footprint";
            goal.target_pose.header.stamp = ros::Time::now();

            goal.target_pose.pose.position.x = csv.wp.x(now_wp);
            goal.target_pose.pose.position.y = csv.wp.y(now_wp);
            goal.target_pose.pose.orientation.z = csv.wp.qz(now_wp);
            goal.target_pose.pose.orientation.w = csv.wp.qw(now_wp);
            ROS_INFO("Sending goal");
            ac.sendGoal(goal);

            ac.waitForResult();

            if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
	        ROS_INFO("Hooray, the base moved 1 meter forward");
            else
	        ROS_INFO("The base failed to move forward 1 meter for some reason");

            /*geometry_msgs::PoseStamped goal_point;
            goal_point.pose.position.x = csv.wp.x(now_wp);
            goal_point.pose.position.y = csv.wp.y(now_wp);
            goal_point.pose.orientation.z =  csv.wp.qz(now_wp);
            goal_point.pose.orientation.w = csv.wp.qw(now_wp);
            goal_point.header.stamp = ros::Time::now();
            goal_point.header.frame_id = "map";
            goal_pub.publish(goal_point);*/
        }
        
        //2d pose estimate 初期位置のパブリッシュ
        if(down_button){
            geometry_msgs::PoseWithCovarianceStamped initial_pose;
            initial_pose.pose.pose.position.x = csv.wp.x(now_wp);
            initial_pose.pose.pose.position.x = csv.wp.x(now_wp);
            initial_pose.pose.pose.position.y = csv.wp.y(now_wp);
            initial_pose.pose.pose.orientation.z =  csv.wp.qz(now_wp);
            initial_pose.pose.pose.orientation.w = csv.wp.qw(now_wp);
            initial_pose.header.stamp = ros::Time::now();
            initial_pose.header.frame_id = "map";
            initial_pub.publish(initial_pose);
        }






        

////////////////////////cmdrepub
/*
        const double cpub_time = static_cast<double>(clock() - start) / CLOCKS_PER_SEC * 10000.0;
        //cout<<repub_time<<endl;
        if(cpub_time>5000){
            cout<<"publishwp="<<now_wp<<endl;
            geometry_msgs::PoseStamped goal_point;
            goal_point.pose.position.x = csv.wp.x(now_wp);
            goal_point.pose.position.y = csv.wp.y(now_wp);
            goal_point.pose.orientation.z =  csv.wp.qz(now_wp);
            goal_point.pose.orientation.w = csv.wp.qw(now_wp);
            goal_point.header.stamp = ros::Time::now();
            goal_point.header.frame_id = "map";
            goal_pub.publish(goal_point);
        }

*/      






        static int ditect_trg=0;
        switch (int(csv.wp.type(now_wp))){
        
        //一時停止
        case WP_STOP:
            if(up_button){
                //入力がきたらナビゲーションを再開する
                csv.wp.typechenge(now_wp,WP_NAVIGATION);
            }
            else{
                final_cmd_vel=nav_vel;
            }
            break;
        
        case WP_DITECTION:
            if(up_button){
                //入力がきたらナビゲーションを再開する
                csv.wp.typechenge(now_wp,WP_NAVIGATION);
                now_wp++;
                    cout<<"publishwp="<<now_wp<<endl;
                    geometry_msgs::PoseStamped goal_point;
                    goal_point.pose.position.x = csv.wp.x(now_wp);
                    goal_point.pose.position.y = csv.wp.y(now_wp);
                    goal_point.pose.orientation.z =  csv.wp.qz(now_wp);
                    goal_point.pose.orientation.w = csv.wp.qw(now_wp);
                    goal_point.header.stamp = ros::Time::now();
                    goal_point.header.frame_id = "map";
                    goal_pub.publish(goal_point);
                    //final_cmd_vel.angular.x=0;
            }
            else{
                final_cmd_vel=nav_vel;
                
                if((base.pos-csv.wp.vec[now_wp]).size()<1.0 && now_wp+1<csv.wp.size()){
                    ditect_trg=true;
                    final_cmd_vel.angular.x=1.0;
                    

                }
                if(ditect_trg){
                    final_cmd_vel.angular.x=1.0;
                }
            }
            break;

        default:
            //
            ditect_trg=false;
            if((base.pos-csv.wp.vec[now_wp]).size()<1.0 && now_wp+1<csv.wp.size()){
                now_wp++;
                cout<<"publishwp="<<now_wp<<endl;
                geometry_msgs::PoseStamped goal_point;
                goal_point.pose.position.x = csv.wp.x(now_wp);
                goal_point.pose.position.y = csv.wp.y(now_wp);
                goal_point.pose.orientation.z =  csv.wp.qz(now_wp);
                goal_point.pose.orientation.w = csv.wp.qw(now_wp);
                goal_point.header.stamp = ros::Time::now();
                goal_point.header.frame_id = "map";
                goal_pub.publish(goal_point);
            }
            final_cmd_vel=nav_vel;
            break;
        }
        up_button=0;
        down_button=0;
        left_button=0;
        right_button=0;

        
        cout<<"rs_link x="<<rs.pos.x<<" y="<<rs.pos.y<<" yaw="<<rs.pos.yaw<<endl;
        //cout<<"waypoint x="<<csv.wp.vec[now_wp].x<<" y="<<csv.wp.vec[now_wp].y<<" yaw="<<csv.wp.vec[now_wp].yaw<<endl;
        final_cmd_vel.linear.y=human_dis;
        cmd_pub.publish(final_cmd_vel);
        ros::spinOnce();//subsucriberの割り込み関数はこの段階で実装される
        loop_rate.sleep();
        
    }
    
    return 0;
}





