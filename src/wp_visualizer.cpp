#include <ros/ros.h>
#include <nav_msgs/Path.h>
#include <visualization_msgs/MarkerArray.h>
#include <string>
#include <math.h>
#include <sstream>
using namespace std;

nav_msgs::Path path;
void path_callback(const nav_msgs::Path& path_row){ 
    path.poses.resize(path_row.poses.size());
    path=path_row;
}
int main(int argc, char **argv){
    
    ros::init(argc, argv, "wp_visualizer");
    ros::NodeHandle n;
    ros::NodeHandle lSubscriber("");
    ros::Subscriber path_sub = lSubscriber.subscribe("/Path", 50, path_callback);
    ros::Publisher marker_pub   = n.advertise<visualization_msgs::MarkerArray>("marker_array", 1);
    ros::Publisher marker_pub1   = n.advertise<visualization_msgs::MarkerArray>("marker_array1", 1);
    //制御周期10ms
    ros::Rate loop_rate(100);
    while (n.ok())  {
        visualization_msgs::MarkerArray marker_array;
        marker_array.markers.resize(path.poses.size());
        visualization_msgs::MarkerArray marker_array1;
        marker_array1.markers.resize(path.poses.size());

        for(int i=0;i<path.poses.size();i++){
            //marker0
            marker_array.markers[i].header.frame_id = "/map";
            marker_array.markers[i].header.stamp = ros::Time::now();
            marker_array.markers[i].ns = "cmd_vel_display";
            marker_array.markers[i].id = i;
            marker_array.markers[i].lifetime = ros::Duration();

            marker_array.markers[i].type = visualization_msgs::Marker::ARROW;
            marker_array.markers[i].action = visualization_msgs::Marker::ADD;
            marker_array.markers[i].scale.x = 1.0;
            marker_array.markers[i].scale.y = 0.2;
            marker_array.markers[i].scale.z = 0.2;
            marker_array.markers[i].pose.position.x=path.poses.at(i).pose.position.x;
            marker_array.markers[i].pose.position.y=path.poses.at(i).pose.position.y;
            marker_array.markers[i].pose.position.z=path.poses.at(i).pose.position.z;
            marker_array.markers[i].pose.orientation.x=path.poses.at(i).pose.orientation.x;
            marker_array.markers[i].pose.orientation.y=path.poses.at(i).pose.orientation.y;
            marker_array.markers[i].pose.orientation.z=path.poses.at(i).pose.orientation.z;
            marker_array.markers[i].pose.orientation.w=path.poses.at(i).pose.orientation.w;
            marker_array.markers[i].color.r = 0.0f;
            marker_array.markers[i].color.g = 0.0f;
            marker_array.markers[i].color.b = 1.0f;
            marker_array.markers[i].color.a = 1.0f;

            

            //number
            marker_array1.markers[i].header.frame_id = "/map";
            marker_array1.markers[i].header.stamp = ros::Time::now();
            marker_array1.markers[i].ns = "cmd_vel_display";
            marker_array1.markers[i].id = i;
            marker_array1.markers[i].lifetime = ros::Duration();

            marker_array1.markers[i].type = visualization_msgs::Marker::TEXT_VIEW_FACING;
            marker_array1.markers[i].action = visualization_msgs::Marker::ADD;
            marker_array1.markers[i].scale.x = 0.7;
            marker_array1.markers[i].scale.y = 0.7;
            marker_array1.markers[i].scale.z = 0.7;
            marker_array1.markers[i].pose.position.x=path.poses.at(i).pose.position.x;
            marker_array1.markers[i].pose.position.y=path.poses.at(i).pose.position.y;
            marker_array1.markers[i].pose.position.z=path.poses.at(i).pose.position.z;
            marker_array1.markers[i].pose.orientation.x=path.poses.at(i).pose.orientation.x;
            marker_array1.markers[i].pose.orientation.y=path.poses.at(i).pose.orientation.y;
            marker_array1.markers[i].pose.orientation.z=path.poses.at(i).pose.orientation.z;
            marker_array1.markers[i].pose.orientation.w=path.poses.at(i).pose.orientation.w;
            marker_array1.markers[i].color.r = 0.0f;
            marker_array1.markers[i].color.g = 0.0f;
            marker_array1.markers[i].color.b = 0.0f;
            marker_array1.markers[i].color.a = 1.0f;
        
    
            ostringstream oss;
            oss<<i;
            marker_array1.markers[i].text= oss.str().c_str();
        }
        marker_pub.publish(marker_array);
        marker_pub1.publish(marker_array1);
        ros::spinOnce();//subsucriberの割り込み関数はこの段階で実装される
        loop_rate.sleep();
    }
    return 0;
}