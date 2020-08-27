#pragma once
#include <ros/ros.h>
#include <tf/tf.h>
#include <tf/transform_datatypes.h>
#include <tf/transform_listener.h>
#include <tf/transform_broadcaster.h>
#include"tf_lis.h"
#include"Vector.h"

class odom_mode{
    public:
    odom_mode();
    void attach();
    Vector update();
    private:
    void tf_pub(Vector pos);
    char *map_id;
    char *odom_id;
    char *base_link_id;
    char *pub_odom_id;
    tf_lis mapodom;
    tf_lis odomraw;
    Vector pub_odom;
    Vector odom_offset;
    Vector mapodom_offset;
    tf::TransformBroadcaster br;
};

void odom_mode::tf_pub(Vector pos){
   //static tf::TransformBroadcaster br;
   tf::Transform transform;
   transform.setOrigin( tf::Vector3(pos.x, pos.y, 0.0) );
   tf::Quaternion q;
   q.setRPY(0, 0, pos.yaw);
   transform.setRotation(q);
   br.sendTransform(tf::StampedTransform(transform, ros::Time::now(),"/map","/odom_link"));
}

odom_mode::odom_mode():
mapodom("/map","odom"),
odomraw("odom","base_link"),
br(){
}

Vector odom_mode::update(){
    mapodom.update();
    odomraw.update();
    Vector pub_vec=mapodom_offset+odomraw.pos.rad_rot(mapodom_offset.yaw);
    pub_vec.yaw=mapodom_offset.yaw+odomraw.pos.yaw;
    //tf_pub(pub_vec);
    return pub_vec;
}

void odom_mode::attach(){
    odom_offset=odomraw.pos;
    mapodom_offset=mapodom.pos;
}
