/**
* @file FlexPose.h
* @brief Flexible pose calculating class
* @author Shunya Hara
* @date 2021.1.31
* @details General purpose class for calculating coordinates and posture in ros.
*          Mutual conversion between tf, Pose, and PoseStamped
*          tf listen/broadcast
*/

#pragma once
#include <ros/ros.h>
#include <geometry_msgs/Pose.h>
#include <geometry_msgs/PoseStamped.h>
#include <tf/tf.h>
#include <tf/transform_broadcaster.h>
#include <tf/transform_listener.h>
#include <tf/transform_datatypes.h>
#include <string>


class FlexPose{
    public:
    
    /// @brief constructor
    FlexPose(double x,double y,double z=0.0);
    FlexPose(geometry_msgs::Pose pose);
    FlexPose(geometry_msgs::PoseStamped pose);
    FlexPose(const char *parent_id,const char *child_id);

    void setPosition(double x,double y,double z=0.0);
    void setPosition(geometry_msgs::Point position);
    void setOrientation(double x,double y,double z,double w);
    void setOrientation(geometry_msgs::Quaternion orientation);
    
    void setYaw(double angle);
    void setRoll(double angle);
    void setPitch(double angle);
    void setRPY(double roll,double pitch,double yaw);

    void setPose(geometry_msgs::Pose pose);
    void setPose(geometry_msgs::PoseStamped pose);
    void setPose(const char *parent_id,const char *child_id);

    geometry_msgs::Pose toPose();
    geometry_msgs::PoseStamped toPoseStamped();
    
    void updateTF();
    void sendTF();

    FlexPose operator+(FlexPose pose);
    FlexPose operator-(FlexPose pose);
    FlexPose operator*(double gain);

    FlexPose operator+=(FlexPose pose);
    FlexPose operator-=(FlexPose pose);
    FlexPose operator*=(double gain);



    private:
    tf::TransformListener listener;
    geometry_msgs::PoseStamped pos;
    geometry_msgs::PoseStamped TFtoPoseStamped(const char *parent_id,const char *child_id);
    std::string child_id;
    void EulerAnglesToQuaternion(double roll, double pitch, double yaw,double& q0, double& q1, double& q2, double& q3);
};


FlexPose::FlexPose(double x,double y,double z=0.0){
    pos.pose.position.x=x;
    pos.pose.position.y=y;
    pos.pose.position.z=z;
}

FlexPose::FlexPose(geometry_msgs::Pose pose){
    pos.pose=pose;
}

FlexPose::FlexPose(geometry_msgs::PoseStamped pose){
    pos=pose;
}

FlexPose::FlexPose(const char *parent_id,const char *child_id){
    pos=TFtoPoseStamped(parent_id,child_id);
}

geometry_msgs::PoseStamped FlexPose::TFtoPoseStamped(const char *parent_id,const char *child_id){
    tf::StampedTransform listf;
    try {
        listener.lookupTransform(parent_id,child_id,ros::Time(0), listf);
        pos.pose.position.x=listf.getOrigin().x();
        pos.pose.position.y=listf.getOrigin().y();
        pos.pose.position.z=listf.getOrigin().z();
        pos.pose.orientation.x=listf.getRotation().x();
        pos.pose.orientation.y=listf.getRotation().y();
        pos.pose.orientation.z=listf.getRotation().z();
        pos.pose.orientation.w=listf.getRotation().w();
        pos.header.frame_id=parent_id;
        pos.header.stamp=ros::Time::now();
        this->child_id=child_id;
    }
    catch (tf::TransformException &ex)  {
        ROS_ERROR("%s", ex.what());
        ros::Duration(1.0).sleep();
    }
}

void FlexPose::EulerAnglesToQuaternion(double roll, double pitch, double yaw,double& q0, double& q1, double& q2, double& q3){
            //q0:w q1:x q2:y q3:z
            double cosRoll = cos(roll / 2.0);
            double sinRoll = sin(roll / 2.0);
            double cosPitch = cos(pitch / 2.0);
            double sinPitch = sin(pitch / 2.0);
            double cosYaw = cos(yaw / 2.0);
            double sinYaw = sin(yaw / 2.0);

            q0 = cosRoll * cosPitch * cosYaw + sinRoll * sinPitch * sinYaw;
            q1 = sinRoll * cosPitch * cosYaw - cosRoll * sinPitch * sinYaw;
            q2 = cosRoll * sinPitch * cosYaw + sinRoll * cosPitch * sinYaw;
            q3 = cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw;
}