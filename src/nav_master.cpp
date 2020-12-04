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
#include <std_msgs/Int32.h>
#include <std_msgs/Int32MultiArray.h>
#include <std_msgs/Empty.h>
#include <kcctslam_msgs/WayPoint.h>


#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include<cmath>

#include"csvread2.h"
//#include"csvread.h"
//#include"wpdata.h"
#include"tf_lis.h"
#include"wpmarker.h"
#include"odom_mode.h"
#include"watch_position.h"

#include<time.h>

//typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

using namespace std;
geometry_msgs::Twist nav_vel;//navigation stack の速度指令

geometry_msgs::Twist final_cmd_vel;//ロボットに送る速度指令

int now_wp=0;

double double_constrain(double val,double down_limit,double up_limit){
  if(val>up_limit){
    return up_limit;
  }
  if(val<down_limit){
    return down_limit;
  }
  return val;
}



//前方の障害物の距離受信
double front_dis=0;
void dis_vel_callback(const geometry_msgs::Twist& vel_cmd){ 
     front_dis=vel_cmd.linear.x-0.255;
}

//cmd_vel subscribe
void cmd_vel_callback(const geometry_msgs::Twist& vel_cmd){ 
     nav_vel=vel_cmd;
}

//human subscribe
double human_angle;
void human_callback(const std_msgs::Int32MultiArray& human_angle_row){ 
     human_angle=human_angle_row.data.at(0);
     if(abs(human_angle_row.data.at(0))>290&&abs(human_angle_row.data.at(0))<50){
        human_angle=0;
     }
}

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

//初期座標のデータを生成
geometry_msgs::PoseWithCovarianceStamped vec_to_PoseWithCovarianceStamped(Vector pos){
            geometry_msgs::PoseWithCovarianceStamped initial_pose;
            initial_pose.pose.pose.position.x = pos.x;
            initial_pose.pose.pose.position.y =pos.y;
            initial_pose.pose.pose.position.z =pos.z;
            initial_pose.pose.pose.orientation.z = pos.get_qz();
            initial_pose.pose.pose.orientation.w = pos.get_qw();
            initial_pose.header.stamp = ros::Time::now();
            initial_pose.header.frame_id = "map";
            initial_pose.pose.covariance[0]=0.25;
            initial_pose.pose.covariance[7]=0.25;
            initial_pose.pose.covariance[35]=0.06853891945200942;
            return initial_pose;

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

//現在地と目標ウェイポイントから速度指令を計算
 geometry_msgs::Twist cmd_vel_calc(Vector nowpos,Vector wppos,double front_dis,bool back_drive,bool slow_mode){
     geometry_msgs::Twist calc_vel;
     //param
     const double angle_p=0.3;
     const double angle_stop_p=1.5;
     double angle_max=0.4;
     const double vel_p=0.3;
     double vel_max=0.25;
     const double curve_stop_angle=30.0*M_PI/180.0;
     const double front_ditect_dis=5.0;
     const double front_stop_distance=0.5;

     if(slow_mode){
        angle_max=025;
        vel_max=0.2;
     }

     double angle1=(wppos-nowpos).rad()-nowpos.yaw+M_PI*2.0+back_drive*M_PI;
     double angle2=(wppos-nowpos).rad()-nowpos.yaw+back_drive*M_PI;
     double angle3=(wppos-nowpos).rad()-nowpos.yaw-M_PI*2.0+back_drive*M_PI;

    if(abs(angle1)<abs(angle2)&&abs(angle1)<abs(angle3)){
        calc_vel.angular.z=angle1;
    }
    else if(abs(angle2)<abs(angle1)&&abs(angle2)<abs(angle3)){
        calc_vel.angular.z=angle2;
    }
    else if(abs(angle3)<abs(angle1)&&abs(angle3)<abs(angle2)){
        calc_vel.angular.z=angle3;
    }

    front_dis-=front_stop_distance;
    //front_dis=20.0;

    if(front_dis>front_ditect_dis){
        calc_vel.linear.x=vel_max;
    }
    else{
        calc_vel.linear.x=double_constrain(front_dis*vel_p,0,vel_max);
    }
    if((abs(calc_vel.angular.z)<curve_stop_angle)){
        calc_vel.angular.z*=angle_p;
    }
    else{
         calc_vel.angular.z*=angle_stop_p;
    }
    
    calc_vel.angular.z*=double_constrain(angle_p,-angle_max,angle_max);
    calc_vel.linear.x*=(abs(calc_vel.angular.z)<curve_stop_angle);
    calc_vel.linear.x*=back_drive?-1:1;
    //cout<<nowpos.yaw<<","<<wppos.yaw<<endl;
    return calc_vel;
}

geometry_msgs::Twist person_tracking(double front_angle,double front_dis){
    geometry_msgs::Twist calc_vel;
    //param
     const double angle_p=0.05;
     double angle_max=0.5;
     const double vel_p=0.3;
     double vel_max=0.2;
     const double front_stop_distance=0.5;
     const double front_ditect_dis=5.0;

     front_dis-=front_stop_distance;
     if(front_dis>front_ditect_dis){
        calc_vel.linear.x=vel_max;
    }
    else{
        calc_vel.linear.x=double_constrain(front_dis*vel_p,0,vel_max);
    }
        calc_vel.angular.z=-front_angle;
        calc_vel.angular.z*=angle_p;
    calc_vel.angular.z=double_constrain(calc_vel.angular.z,-angle_max,angle_max);
    //calc_vel.linear.x=0;//*=(abs(calc_vel.angular.z)<curve_stop_angle);
    return calc_vel;
}

void map_chenged_callback(std_msgs::Empty empty){
    now_wp++;
}



int main(int argc, char **argv){
    
    ros::init(argc, argv, "nav_master");
    ros::NodeHandle n;

    //ウェイポイントファイルのロード
    ros::NodeHandle pn("~");
    string filename;
    //２つの形式でwaypointを取得
    vector<Vector> wp_vec;
    nav_msgs::Path wp_path;
    pn.getParam("waypointfile",filename);
    csvread2 csv(filename.c_str(),wp_path,wp_vec);
    csv.print();


    //cmd_velの受信と送信
    ros::NodeHandle lSubscriber("");
    ros::Subscriber sub = lSubscriber.subscribe("/cmd_vel", 50, cmd_vel_callback);
    ros::Publisher cmd_pub = n.advertise<geometry_msgs::Twist>("final_cmd_vel", 10); 

    //前方の障害物距離 subscliber
    ros::Subscriber dis_sub = lSubscriber.subscribe("/pcl_handler/front_dist", 50, dis_vel_callback);

    //2D_NAV_GOAL publisher
    ros::Publisher goal_pub = n.advertise<geometry_msgs::PoseStamped>("/move_base_simple/goal", 1);
    
   //十字キー入力 subscliber
   ros::Subscriber ket_sub = lSubscriber.subscribe("/turtle1/cmd_vel", 50, key_vel_callback);
   //十字キー入力 subscliber
   ros::Subscriber human_sub = lSubscriber.subscribe("/position", 50, human_callback);
    

    //2D_POSE_ESTIMATE publisher
    ros::Publisher initial_pub = n.advertise<geometry_msgs::PoseWithCovarianceStamped>("/initialpose", 1);

    //直進速度　publisher
    ros::Publisher linear_vel = n.advertise<std_msgs::Float32>("linear_vel", 10);

    //旋回速度　publisher
    ros::Publisher angular_vel = n.advertise<std_msgs::Float32>("angular_vel", 10);

    //WayPointPath　publisher
    ros::Publisher path_pub = n.advertise<nav_msgs::Path>("wp_path", 10);

    //Now wp publisher
    ros::Publisher now_wp_pub = n.advertise<std_msgs::Int32>("now_wp", 10);

    //map chenged subscliber
    ros::Subscriber map_chenged_sub = lSubscriber.subscribe("map_chenged", 50, map_chenged_callback);

    //map number publisher
    ros::Publisher map_num_pub = n.advertise<std_msgs::Int32>("map_num", 10);

    //制御周期10ms
    ros::Rate loop_rate(10);

    
    int wp_mode=false;
    const int LIDAR_MODE=1;
    const int RS_MODE=2;
    int delay_count=0;

    tf_lis odom_tf("/map","/odom_link");
    tf_lis lidar_tf("/map","/base_link");

    geometry_msgs::Twist zero_vel;//停止
    zero_vel.linear.x=0;
    zero_vel.angular.z=0;

    //MoveBaseClient ac("move_base", true);
    

    wpmarker wpmarker;
    Wpdata rsdata;
    Vector pubodom;
    odom_mode odom_mode;
    watch_position watch_pos(3.0);
    initial_pub.publish(vec_to_PoseWithCovarianceStamped(wp_vec[now_wp]));

    //最初に一時停止
    wp_vec.at(now_wp).type=WP_STOP;

    while (n.ok())  {
        odom_tf.update();
        lidar_tf.update();
        odomtf_pub(odom_mode.update());
        wpmarker.update(wp_vec,now_wp);
        watch_pos.update(lidar_tf.pos);
        

        if(left_button){
            now_wp++;
            ROS_INFO("[SKIP] WAYPOINT[%d]",now_wp);
        }
        if(down_button){
            wp_vec.at(now_wp).type=WP_STOP;
            ROS_INFO("[PAUSE] WAYPOINT[%d]",now_wp);
        }
        if(right_button){
           initial_pub.publish(vec_to_PoseWithCovarianceStamped(wp_vec[now_wp]));
           ROS_INFO("[INTIAL POSE] WAYPOINT[%d]",now_wp);
           
        }

        //
        if(wp_vec[now_wp].map!=wp_vec[now_wp+1].map){
            std_msgs::Int32 map_num;
            map_num.data=wp_vec[now_wp+1].map;
            map_num_pub.publish(map_num);
        }


        switch (wp_vec[now_wp].type){

        //一時停止
        case WP_STOP:
            if(up_button){
                //入力がきたらナビゲーションを再開する
                wp_vec.at(now_wp).type=wp_vec.at(now_wp+1).type;
                ROS_INFO("[START] NOW WAYPOINT[%d]",now_wp);
            }
            else{
                final_cmd_vel=zero_vel;
                odom_mode.attach();
            }
            break;

        case LIDAR_NAVIGATION:
            final_cmd_vel=cmd_vel_calc(lidar_tf.pos,wp_vec[now_wp],front_dis,false,false);
            if(watch_pos.ok()){
                odom_mode.attach();
            }
            else{
                 initial_pub.publish(vec_to_PoseWithCovarianceStamped(odom_tf.pos));
            }
            if((lidar_tf.pos-wp_vec[now_wp]).size()<0.8){
                //pubodom=rs_odom_attach(rs_tf.pos,lidar_tf.pos,pubodom);
                if(now_wp<wp_vec.size()-1){
                    now_wp++;
                    //cout<<"p="<<now_wp<<endl;
                    ROS_INFO("NOW WAYPOINT[%d]",now_wp);
                }
                else{
                    final_cmd_vel=zero_vel;

                }
                
            }
            break;

        case ODOM_NAVIGATION:
            final_cmd_vel=cmd_vel_calc(odom_tf.pos,wp_vec[now_wp],front_dis,false,true);
            if((odom_tf.pos-wp_vec[now_wp]).size()<0.8){
                //pubodom=rs_odom_attach(rs_tf.pos,lidar_tf.pos,pubodom);
                if(now_wp<wp_vec.size()-1){
                    now_wp++;
                    ROS_INFO("NOW WAYPOINT[%d]",now_wp);
                }
                else{
                    final_cmd_vel=zero_vel;
                    
                }
            }
            break;
        case HUMAN_DITECTION:
            final_cmd_vel=person_tracking(human_angle,front_dis);
            ROS_INFO("PERSON DITECTION[%d]",now_wp);
            break;

        case SKIP_WP:
            now_wp++;
            break;

        default:
        //final_cmd_vel=nav_vel;
        final_cmd_vel=zero_vel;
        }
        //cout<<lidar_tf.pos.yaw<<","<<odom_tf.pos.yaw<<endl;

        //wp pub
        std_msgs::Int32 pub_now_wp;
        pub_now_wp.data=now_wp;
        now_wp_pub.publish(pub_now_wp);

        //直進速度表示
        std_msgs::Float32 linear_vel_data;
        linear_vel_data.data=final_cmd_vel.linear.x;
        linear_vel.publish(linear_vel_data);

        // 旋回速度表示
        std_msgs::Float32 angular_vel_data;
        angular_vel_data.data=final_cmd_vel.angular.z;
        angular_vel.publish(angular_vel_data);

        path_pub.publish(wp_path);

        final_cmd_vel.linear.y=front_dis;
        cmd_pub.publish(final_cmd_vel);
        key_reset();
        ros::spinOnce();//subsucriberの割り込み関数はこの段階で実装される
        loop_rate.sleep();
        
    }
    
    return 0;
}
