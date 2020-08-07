#include <ros/ros.h>
#include <tf/transform_broadcaster.h>
#include <geometry_msgs/PoseStamped.h>
#include <std_msgs/Float32MultiArray.h>
#include <std_msgs/Int32MultiArray.h>
#include <string>
#include <math.h>
#include <vector>
using namespace std;

vector<int32_t> int_sensor_data;
vector<float> float_sensor_data;

void int_sensor_data_callback(const std_msgs::Int32MultiArray& int_sensor_data_row){ 
     int_sensor_data=int_sensor_data_row.data;
}

void float_sensor_data_callback(const std_msgs::Float32MultiArray& float_sensor_data_row){ 
     float_sensor_data=float_sensor_data_row.data;
}

int main(int argc, char **argv){
    
     ros::init(argc, argv, "teensy_handler");
     ros::NodeHandle n;
     ros::Rate loop_rate(10);

     ros::NodeHandle lSubscriber("");
     ros::Subscriber int_sub = lSubscriber.subscribe("int_sensor_data", 50, int_sensor_data_callback);
     ros::Subscriber float_sub = lSubscriber.subscribe("float_sensor_data", 50, float_sensor_data_callback);

     //ros::Publisher cmd_pub = n.advertise<geometry_msgs::Twist>("final_cmd_vel", 10); 
     tf::TransformBroadcaster br;
     
     while (n.ok())  {

          //TF
          tf::Transform transform;
          transform.setOrigin( tf::Vector3(float_sensor_data[0], float_sensor_data[1], 0.0) );
          tf::Quaternion q;
          q.setRPY(0, 0, float_sensor_data[2]);
          transform.setRotatio n(q);
          br.sendTransform(tf::StampedTransform(transform, ros::Time::now(), "map","base_link"));

          ros::spinOnce();
          loop_rate.sleep();
     }
}
