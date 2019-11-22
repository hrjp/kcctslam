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

#include"csvread.h"
#include"wpdata.h"
#include"tf_lis.h"
#include"wpmarker.h"

#include<time.h>


typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

using namespace std;
geometry_msgs::Twist nav_vel;//navigation stack の速度指令

geometry_msgs::Twist final_cmd_vel;//ロボットに送る速度指令

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

//現在地と目標ウェイポイントから速度指令を計算
 geometry_msgs::Twist cmd_vel_calc(Vector nowpos,Vector wppos,double front_dis,bool back_drive){
     geometry_msgs::Twist calc_vel;
     //param
     const double angle_p=1.2;
     const double angle_max=0.5;
     const double vel_p=0.3;
     const double vel_max=0.35;
     const double curve_stop_angle=30.0*M_PI/180.0;
     const double front_ditect_dis=5.0;
     const double front_stop_distance=0.5;

     //double angle1=wppos.yaw-nowpos.yaw+M_PI*2.0;
     //double angle2=wppos.yaw-nowpos.yaw;
     //double angle3=wppos.yaw-nowpos.yaw-M_PI*2.0;

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
    front_dis=20.0;

    if(front_dis>front_ditect_dis){
        calc_vel.linear.x=vel_max;
    }
    else{
        calc_vel.linear.x=double_constrain(front_dis*vel_p,0,vel_max);
    }
    calc_vel.angular.z*=angle_p;
    calc_vel.angular.z*=double_constrain(angle_p,-angle_max,angle_max);
    calc_vel.linear.x*=(abs(calc_vel.angular.z)<curve_stop_angle);
    calc_vel.linear.x*=back_drive?-1:1;
    //cout<<nowpos.yaw<<","<<wppos.yaw<<endl;
    return calc_vel;
}



int main(int argc, char **argv){
    
    ros::init(argc, argv, "nav_master");
    ros::NodeHandle n;

    //ウェイポイントファイルのロード
    ros::NodeHandle pn("~");
    string filename;
    pn.getParam("waypointfile",filename);
    csvread csv(filename.c_str());


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
    

    //2D_POSE_ESTIMATE publisher
    ros::Publisher initial_pub = n.advertise<geometry_msgs::PoseWithCovarianceStamped>("/initialpose", 1);

    //直進速度　publisher
    ros::Publisher linear_vel = n.advertise<std_msgs::Float32>("linear_vel", 10);

    //旋回速度　publisher
    ros::Publisher angular_vel = n.advertise<std_msgs::Float32>("angular_vel", 10);
    
    //制御周期10ms
    ros::Rate loop_rate(10);

    int now_wp=0;
    int wp_mode=false;
    const int LIDAR_MODE=1;
    const int RS_MODE=2;

    tf_lis rs_tf("/map","/rs_camera_link");
    tf_lis lidar_tf("/map","/base_link");

    geometry_msgs::Twist zero_vel;//停止
    zero_vel.linear.x=0;
    zero_vel.angular.z=0;

    MoveBaseClient ac("move_base", true);
    

    wpmarker wpmarker;
    Wpdata rsdata;
    Vector pubodom;
    initial_pub.publish(init_pose(csv.wp.vec[now_wp]));
    while (n.ok())  {
        rs_tf.update();
        lidar_tf.update();
        rs_odom(pubodom);
        wpmarker.update(csv.wp,now_wp);

        switch (int(csv.wp.type(now_wp))){

        //一時停止
        case WP_STOP:
            if(up_button){
                //入力がきたらナビゲーションを再開する
                csv.wp.typechenge(now_wp,int(csv.wp.type(now_wp-1)));
            }
            else{
                final_cmd_vel=zero_vel;
            }
            break;

        case LIDAR_NAVIGATION:
            final_cmd_vel=cmd_vel_calc(lidar_tf.pos,csv.wp.vec[now_wp],front_dis,false);
            if((lidar_tf.pos-csv.wp.vec[now_wp]).size()<0.8){
                pubodom=rs_odom_attach(rs_tf.pos,lidar_tf.pos,pubodom);
                now_wp++;
                cout<<"publishwp="<<now_wp<<endl;
            }
            break;
        case RS_NAVIGATION:
            final_cmd_vel=cmd_vel_calc(rs_tf.pos,csv.wp.vec[now_wp],front_dis,false);
            if((rs_tf.pos-csv.wp.vec[now_wp]).size()<0.5){
                now_wp++;
                cout<<"publishwp="<<now_wp<<"type="<<csv.wp.type(now_wp)<<endl;
            }
            break;
        case RS_BACK_NAVIGATION:
            final_cmd_vel=cmd_vel_calc(rs_tf.pos,csv.wp.vec[now_wp],front_dis,true);
            if((rs_tf.pos-csv.wp.vec[now_wp]).size()<0.5){
                now_wp++;
                cout<<"publishwp="<<now_wp<<"type="<<csv.wp.type(now_wp)<<endl;
            }
            break;
        default:
        final_cmd_vel=nav_vel;
        }


        /*
        if(up_button){
            if(wp_mode==RS_MODE){
                initial_pub.publish(init_pose(rs_tf.pos));
            }
            wp_mode=LIDAR_MODE;
            
            pubodom=rs_odom_attach(rs_tf.pos,lidar_tf.pos,pubodom);
            cout<<"LIDAR MODE SELECT"<<endl;
            if(now_wp==0){
                rs_tf.update();
                lidar_tf.update();
                rs_odom(pubodom);
                cout<<"Waypoint NUMBER : [ "<<now_wp<<" ] (LiDAR)"<<endl;
                goal_pub.publish(csv_write(lidar_tf.pos,1));
                rsdata.vec[now_wp]=lidar_tf.pos;
            }
            
            rsdata.vtoa();
            wpmarker.update(rsdata,now_wp);
        }
        if(down_button){
            wp_mode=RS_MODE;
 
            pubodom=rs_odom_attach(rs_tf.pos,lidar_tf.pos,pubodom);

            cout<<"REALSENSE MODE SELECT"<<endl;
            if(now_wp==0){
                rs_tf.update();
                lidar_tf.update();
                rs_odom(pubodom);
                cout<<"Waypoint NUMBER : [ "<<now_wp<<" ] (REALSENSE)"<<endl;
                goal_pub.publish(csv_write(rs_tf.pos,2));
                rsdata.vec[now_wp]=rs_tf.pos;
            }
            rsdata.vtoa();
            wpmarker.update(rsdata,now_wp);
        }
        if(left_button){
            wp_mode=false;
        }

        if(wp_mode==LIDAR_MODE){
            if((lidar_tf.pos-rsdata.vec[now_wp]).size()>1.0){
                now_wp++;
                cout<<"Waypoint NUMBER : [ "<<now_wp<<" ] (LiDAR)"<<endl;
                goal_pub.publish(csv_write(lidar_tf.pos,1));
                rsdata.vec[now_wp]=lidar_tf.pos;
                rsdata.vtoa();
                wpmarker.update(rsdata,now_wp);
            }
        }

        if(wp_mode==RS_MODE){
            if((rs_tf.pos-rsdata.vec[now_wp]).size()>1.0){
                now_wp++;
                cout<<"Waypoint NUMBER : [ "<<now_wp<<" ] (REALSENSE)"<<endl;
                goal_pub.publish(csv_write(rs_tf.pos,2));
                rsdata.vec[now_wp]=rs_tf.pos;
                rsdata.vtoa();
                wpmarker.update(rsdata,now_wp);
            }
        }
        */
        //cout<<"x="<<lidar_tf.pos.x<<"y="<<lidar_tf.pos.y<<endl;
       // cout<<"rs="<<rs_tf.pos.yaw<<"  lidar="<<lidar_tf.pos.yaw<<"  odom="<<rsodom.yaw<<endl;
        //cout<<loop_rate.cycleTime()<<endl;

        //直進速度表示
        std_msgs::Float32 linear_vel_data;
        linear_vel_data.data=final_cmd_vel.linear.x;
        linear_vel.publish(linear_vel_data);

        // 旋回速度表示
        std_msgs::Float32 angular_vel_data;
        angular_vel_data.data=final_cmd_vel.angular.z;
        angular_vel.publish(angular_vel_data);

        final_cmd_vel.linear.y=front_dis;
        cmd_pub.publish(final_cmd_vel);
        key_reset();
        ros::spinOnce();//subsucriberの割り込み関数はこの段階で実装される
        loop_rate.sleep();
        
    }
    
    return 0;
}
