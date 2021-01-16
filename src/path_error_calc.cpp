
#include <ros/ros.h>
#include <nav_msgs/Path.h>
#include <std_msgs/Float32.h>
#include <std_msgs/Int32.h>
#include <visualization_msgs/MarkerArray.h>
#include <std_msgs/Float32MultiArray.h>

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

     //debug marker publisher
     ros::Publisher marker_pub   = n.advertise<visualization_msgs::MarkerArray>("path_error_marker", 1);
     
     //path_error publisher
     ros::Publisher path_error_pub   = n.advertise<std_msgs::Float32>("path_error", 1);
     //path_error area publisher
     ros::Publisher error_area_pub   = n.advertise<std_msgs::Float32>("error_area", 1);

     //data_record publisher
     ros::Publisher data_record_pub   = n.advertise<std_msgs::Float32MultiArray>("data_record", 1);


     //TF listener
     tf_lis base_tf("/map","/base_link");
    

    

     while (n.ok())  {
          base_tf.update();
          static int start_wp=2;
          static int now_path=start_wp;
          if(start_wp<now_wp && now_wp<path.poses.size()-1){
               //pathの座標取得
               double x0=path.poses.at(now_path-1).pose.position.x;
               double y0=path.poses.at(now_path-1).pose.position.y;
               double x1=path.poses.at(now_path).pose.position.x;
               double y1=path.poses.at(now_path).pose.position.y;
               double x2=base_tf.pos.x;
               double y2=base_tf.pos.y;
               double xn=path.poses.at(now_path+1).pose.position.x;
               double yn=path.poses.at(now_path+1).pose.position.y;
               
               //
               double next_path_angle=atan2(yn-y1,xn-x1);
               double now_pos_angle=atan2(y2-y1,x2-x1);
               if(std::abs(next_path_angle-now_pos_angle)<M_PI/2.0){
                    now_path++;
               }
               //pathの方程式を求める
               double a0=(y1-y0)/(x1-x0);
               double b0=-a0*x0+y0;
               //自己位置を通りpathに垂直な直線の方程式を求める
               double a1=-(1/a0);
               double b1=y2-a1*x2;
               //2直線の交点を求める
               double x3=(b1-b0)/(a0-a1);
               double y3=(a0*b1-b0*a1)/(a0-a1);

               //path error
               std_msgs::Float32 path_error;
               path_error.data=sqrt((x2-x3)*(x2-x3)+(y2-y3)*(y2-y3));
               path_error_pub.publish(path_error);

               //偏差の台形積分
               //何メートル動くごとに積分するか　積分の刻み幅
               const double calc_diff=0.1;
               static double pre_x3=x3;
               static double pre_y3=y3;
               static double pre_x2=x2;
               static double pre_y2=y2;
               static double diff_area=0;
               static int cou=1;
               static double veri_sum=0.0;
               static double veri=0.0;
               if(calc_diff<sqrt((x2-pre_x2)*(x2-pre_x2)+(y2-pre_y2)*(y2-pre_y2))){
                    //微小四角形を求める
                    double darea=((pre_x2-x2)*(pre_y2+y2)+(x2-x3)*(y2+y3)+(x3-pre_x3)*(y3+pre_y3)+(pre_x3-pre_x2)*(pre_y3+pre_y2))/2.0;
                    darea=std::abs(darea);
                    //積分
                    diff_area+=darea;
                    //次の計算用に今の値を記録
                    pre_x3=x3;
                    pre_y3=y3;
                    pre_x2=x2;
                    pre_y2=y2;

                    //標準偏差を求める
                    double pos_error=sqrt((x2-x3)*(x2-x3)+(y2-y3)*(y2-y3));
                    veri_sum+=pos_error*pos_error;
                    veri=sqrt(veri_sum/cou);
                    cou++;
               }

               //area pub
               std_msgs::Float32 pub_area;
               pub_area.data=diff_area;
               error_area_pub.publish(pub_area);





               //debug用の可視化処理
               visualization_msgs::MarkerArray debug_marker;
               debug_marker.markers.resize(2);
               debug_marker.markers[0].header.frame_id = "/map";
               debug_marker.markers[0].header.stamp = ros::Time::now();
               debug_marker.markers[0].ns = "path_error";
               debug_marker.markers[0].id = 0;
               debug_marker.markers[0].lifetime = ros::Duration();

               debug_marker.markers[0].type = visualization_msgs::Marker::ARROW;
               debug_marker.markers[0].action = visualization_msgs::Marker::ADD;
               debug_marker.markers[0].scale.x = 2.0;
               debug_marker.markers[0].scale.y = 0.1;
               debug_marker.markers[0].scale.z = 0.1;
               debug_marker.markers[0].pose.position.x=x3;
               debug_marker.markers[0].pose.position.y=y3;
               debug_marker.markers[0].pose.position.z=0;
               debug_marker.markers[0].pose.orientation.x=0;
               debug_marker.markers[0].pose.orientation.y=-1;
               debug_marker.markers[0].pose.orientation.z=0;
               debug_marker.markers[0].pose.orientation.w=1.0;
               debug_marker.markers[0].color.r = 0.0f;
               debug_marker.markers[0].color.g = 0.0f;
               debug_marker.markers[0].color.b = 1.0f;
               debug_marker.markers[0].color.a = 1.0f;

               //
               debug_marker.markers[1].header.frame_id = "/map";
               debug_marker.markers[1].header.stamp = ros::Time::now();
               debug_marker.markers[1].ns = "path_error";
               debug_marker.markers[1].id = 1;
               debug_marker.markers[1].lifetime = ros::Duration();

               debug_marker.markers[1].type = visualization_msgs::Marker::CYLINDER;
               debug_marker.markers[1].action = visualization_msgs::Marker::ADD;
               debug_marker.markers[1].scale.x = 6.0;
               debug_marker.markers[1].scale.y = 0.05;
               debug_marker.markers[1].scale.z = 0.05;
               debug_marker.markers[1].pose.position.x=x3;
               debug_marker.markers[1].pose.position.y=y3;
               debug_marker.markers[1].pose.position.z=0.2;
               Vector get_angle(x2,y1,atan(a0));
               debug_marker.markers[1].pose.orientation.x=get_angle.get_qx();
               debug_marker.markers[1].pose.orientation.y=get_angle.get_qy();
               debug_marker.markers[1].pose.orientation.z=get_angle.get_qz();
               debug_marker.markers[1].pose.orientation.w=get_angle.get_qw();
               debug_marker.markers[1].color.r = 1.0f;
               debug_marker.markers[1].color.g = 0.0f;
               debug_marker.markers[1].color.b = 0.0f;
               debug_marker.markers[1].color.a = 1.0f;
               //ROS_INFO("now_wp:%d x:%f y:%f",now_wp,x2,y2);
               ROS_INFO("sd:%f[m]",veri);
               marker_pub.publish(debug_marker);

               //record_pub
               std_msgs::Float32MultiArray data_record;
               data_record.data.resize(4);
               data_record.data[0]=path_error.data;
               data_record.data[1]=pub_area.data;
               data_record.data[2]=veri;
               data_record_pub.publish(data_record);


          }
         

          ros::spinOnce();//subsucriberの割り込み関数はこの段階で実装される
          loop_rate.sleep();
     }
    
    return 0;
}