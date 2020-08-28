#include <ros/ros.h>
#include <tf/transform_broadcaster.h>
#include <geometry_msgs/PoseStamped.h>
#include <std_msgs/Float32MultiArray.h>
#include <std_msgs/Float32.h>
#include <std_msgs/Int32MultiArray.h>
#include <string>
#include <math.h>
#include <vector>
using namespace std;

vector<int32_t> int_sensor_data;
vector<float> float_sensor_data;
std_msgs::Float32 linear_vel;

void int_sensor_data_callback(const std_msgs::Int32MultiArray& int_sensor_data_row){ 
     int_sensor_data=int_sensor_data_row.data;
}

void float_sensor_data_callback(const std_msgs::Float32MultiArray& float_sensor_data_row){ 
     static tf::TransformBroadcaster br;
     float_sensor_data=float_sensor_data_row.data;
     linear_vel.data=float_sensor_data_row.data[3]*3.6;
     tf::Transform transform;
          transform.setOrigin( tf::Vector3(float_sensor_data[0], float_sensor_data[1], 0.0) );
          tf::Quaternion q;
          q.setRPY(0, 0, float_sensor_data[2]);
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
          vel_pub.publish(linear_vel);
          ros::spinOnce();
          loop_rate.sleep();
     }
}
