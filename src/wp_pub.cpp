#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>
#include <tf/transform_broadcaster.h>
#include <nav_msgs/Odometry.h>
#include <tf/tf.h>
#include <tf/transform_datatypes.h>
#include <tf/transform_listener.h>

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
 
int main(int argc, char **argv){
    int now_wp=0;
    ros::init(argc, argv, "wp_pub");
    ros::NodeHandle pn("~");
    string filename;
    pn.getParam("waypointfile",filename);
    //csvread csv("/home/ryo/catkin_ws/src/kcctslam/config/waypointdata/wpdata.csv");
    //csvread csv("../config/waypointdata/wpdata.csv");
    csvread csv(filename.c_str());
    csv.print();
    cout<<endl<<csv.wp.size()<<endl;
    tf_lis base;
    wpmarker wpmarker;
    
    ros::NodeHandle n;
    
    Goal init_goal_ob(csv.wp.x(now_wp), csv.wp.y(now_wp),csv.wp.qz(now_wp), csv.wp.qw(now_wp));
    while (n.ok())  {
        base.update();
        wpmarker.update(csv.wp,now_wp);
        if((base.pos-csv.wp.vec[now_wp]).size()<1.0 && now_wp+1<csv.wp.size()){
            now_wp++;
            Goal goal_ob(csv.wp.x(now_wp), csv.wp.y(now_wp),csv.wp.qz(now_wp), csv.wp.qw(now_wp));
        }
        
        //cout<<"base_link x="<<base.pos.x<<" y="<<base.pos.y<<" yaw="<<base.pos.yaw;
        //cout<<"waypoint x="<<csv.wp.vec[now_wp].x<<" y="<<csv.wp.vec[now_wp].y<<" yaw="<<csv.wp.vec[now_wp].yaw<<endl;
    }
    
    return 0;
}
