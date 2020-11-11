#include <ros/ros.h>
#include <nav_msgs/OccupancyGrid.h>
#include <std_srvs/Empty.h>
#include <nav_msgs/GetMap.h>
#include <std_msgs/Int32.h>
#include <vector>
using namespace std;
const int select_maps=5;

vector<nav_msgs::OccupancyGrid> map_array(select_maps);

std_msgs::Int32 map_num;

void map_callback0(const nav_msgs::OccupancyGrid& map){ map_array.at(0)=map;}
void map_callback1(const nav_msgs::OccupancyGrid& map){ map_array.at(1)=map;}
void map_callback2(const nav_msgs::OccupancyGrid& map){ map_array.at(2)=map;}
void map_callback3(const nav_msgs::OccupancyGrid& map){ map_array.at(3)=map;}
void map_callback4(const nav_msgs::OccupancyGrid& map){ map_array.at(4)=map;}
void int_callback(const std_msgs::Int32& int_data){ map_num=int_data;}
bool map_service_callback(nav_msgs::GetMap::Request &get_map,nav_msgs::GetMap::Response &return_map){
    
    return_map.map=map_array.at(0);
    return true;
}

int main(int argc, char **argv){
    ros::init(argc, argv, "map_selecter");
    ros::NodeHandle n;
    
    ros::NodeHandle lSubscriber("");

    ros::Subscriber map_sub0 = lSubscriber.subscribe("/map0", 50, map_callback0);
    ros::Subscriber map_sub1 = lSubscriber.subscribe("/map1", 50, map_callback1);
    ros::Subscriber map_sub2 = lSubscriber.subscribe("/map2", 50, map_callback2);
    ros::Subscriber map_sub3 = lSubscriber.subscribe("/map3", 50, map_callback3);
    ros::Subscriber map_sub4 = lSubscriber.subscribe("/map4", 50, map_callback4);

    map_num.data=0;
    ros::Subscriber int_sub = lSubscriber.subscribe("/map_num", 50, int_callback);
    //ros::ServiceServer map_service = n.advertiseService("static_map",map_service_callback);

    //制御周期10ms
    ros::Rate loop_rate(100);

    //Map　publisher
    
    ros::Publisher map_pub = n.advertise<nav_msgs::OccupancyGrid>("/map", 10);
    
    while (n.ok())  {
        
        map_pub.publish(map_array.at(map_num.data));
        ros::spinOnce();
        loop_rate.sleep();
        
    }

        
}