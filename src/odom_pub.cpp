#include <ros/ros.h>
#include <nav_msgs/Odometry.h>
#include <std_msgs/Float32.h>



nav_msgs::Odometry odom_data;
void odom_callback(std_msgs::Float32 odom_vel_x){
    odom_data.twist.twist.linear.x=odom_vel_x.data;
}


int main(int argc, char **argv){
    
    ros::init(argc, argv, "odom_pub");
    ros::NodeHandle n;
    ros::Rate loop_rate(10);

    ros::Subscriber odom_sub = n.subscribe("robot_odom", 50, odom_callback);
    ros::Publisher odom_pub = n.advertise<nav_msgs::Odometry>("rs/rs/odom_in", 50);
    while(ros::ok()){
        odom_pub.publish(odom_data);
        ros::spinOnce();
        loop_rate.sleep();
    }

    return 0;
    
}
