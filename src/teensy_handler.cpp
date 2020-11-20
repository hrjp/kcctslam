#include <ros/ros.h>
#include <tf/transform_broadcaster.h>
#include <geometry_msgs/PoseStamped.h>
#include <std_msgs/Float32MultiArray.h>
#include <std_msgs/Float32.h>
#include <std_msgs/Int32MultiArray.h>
#include <sensor_msgs/Imu.h>
#include <nav_msgs/Odometry.h>
#include <string>
#include <math.h>
#include <vector>
using namespace std;

vector<int32_t> int_sensor_data(20);
vector<float> float_sensor_data(20);
std_msgs::Float32 linear_vel;

void int_sensor_data_callback(const std_msgs::Int32MultiArray& int_sensor_data_row){ 
     int_sensor_data=int_sensor_data_row.data;
}

void float_sensor_data_callback(const std_msgs::Float32MultiArray& float_sensor_data_row){ 
     static tf::TransformBroadcaster br;
     float_sensor_data=float_sensor_data_row.data;
     linear_vel.data=float_sensor_data_row.data[13]*3.6;
     tf::Transform transform;
          transform.setOrigin( tf::Vector3(float_sensor_data[1], float_sensor_data[0], float_sensor_data[2]) );
          tf::Quaternion q;
          //q.setRPY(0, 0, float_sensor_data[14]);
          q.setX(-float_sensor_data[10]);
          q.setY(float_sensor_data[9]);
          q.setZ(float_sensor_data[11]);
          q.setW(float_sensor_data[12]);
          transform.setRotation(q);
          br.sendTransform(tf::StampedTransform(transform, ros::Time::now(), "odom","base_link"));
}

int main(int argc, char **argv){
    
     ros::init(argc, argv, "teensy_handler");
     ros::NodeHandle n;
     ros::Rate loop_rate(10);

     ros::NodeHandle lSubscriber("");
     //ros::Subscriber int_sub = lSubscriber.subscribe("int_sensor_data", 50, int_sensor_data_callback);
     ros::Subscriber float_sub = lSubscriber.subscribe("float_sensor_data", 50, float_sensor_data_callback);
     ros::Publisher vel_pub = n.advertise<std_msgs::Float32>("robot_linear_vel", 10); 
     ros::Publisher odom_pub = n.advertise<nav_msgs::Odometry>("odom", 10); 
     ros::Publisher imu_pub = n.advertise<sensor_msgs::Imu>("imu/data", 10); 
     
     
     while (n.ok())  {
/*
          //TF
          tf::Transform transform;
          transform.setOrigin( tf::Vector3(float_sensor_data[0], float_sensor_data[1], 0.0) );
          tf::Quaternion q;
          q.setRPY(0, 0, float_sensor_data[2]);
          transform.setRotation(q);
          br.sendTransform(tf::StampedTransform(transform, ros::Time::now(), "map","base_link"));
*/
          
          nav_msgs::Odometry odom;
          static uint32_t seq_odom=0;
          odom.header.frame_id="odom";
          odom.header.stamp=ros::Time::now();
          odom.header.seq=seq_odom;
          odom.child_frame_id="base_link";
          odom.pose.pose.position.x=float_sensor_data[1];
          odom.pose.pose.position.y=float_sensor_data[0];
          odom.pose.pose.position.z=float_sensor_data[2];
          odom.pose.pose.orientation.x=-float_sensor_data[10];
          odom.pose.pose.orientation.y=float_sensor_data[9];
          odom.pose.pose.orientation.z=float_sensor_data[11];
          odom.pose.pose.orientation.w=float_sensor_data[12];
          odom.twist.twist.linear.x=float_sensor_data[13];
          odom.twist.twist.angular.x=-float_sensor_data.at(7);
          odom.twist.twist.angular.x=float_sensor_data.at(6);
          odom.twist.twist.angular.x=float_sensor_data.at(8);
          seq_odom++;
          
          sensor_msgs::Imu imu;
          static uint32_t seq_imu=0;
          imu.header.frame_id="imu_link";
          imu.header.stamp=ros::Time::now();
          imu.header.seq=seq_imu;
          imu.linear_acceleration.x=-float_sensor_data.at(4);
          imu.linear_acceleration.y=float_sensor_data.at(3);
          imu.linear_acceleration.z=float_sensor_data.at(5);
          imu.linear_acceleration_covariance.at(0)=0.0005356910249999999;
          imu.linear_acceleration_covariance.at(4)=0.0005356910249999999;
          imu.linear_acceleration_covariance.at(8)=0.0005356910249999999;
          imu.angular_velocity.x=-float_sensor_data.at(7);
          imu.angular_velocity.y=float_sensor_data.at(6);
          imu.angular_velocity.z=float_sensor_data.at(8);
          imu.angular_velocity_covariance.at(0)=1.12805641/1000000.0;
          imu.angular_velocity_covariance.at(4)=1.12805641/1000000.0;
          imu.angular_velocity_covariance.at(8)=1.12805641/1000000.0;
          imu.orientation.x=-float_sensor_data.at(10);
          imu.orientation.y=float_sensor_data.at(9);
          imu.orientation.z=-float_sensor_data.at(11);
          imu.orientation.w=float_sensor_data.at(12);
          seq_imu++;
          odom_pub.publish(odom);
          imu_pub.publish(imu);
          vel_pub.publish(linear_vel);
          ros::spinOnce();
          loop_rate.sleep();
     }
}
