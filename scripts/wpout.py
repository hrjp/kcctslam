#!/usr/bin/env python
import pandas as pd
import rospy
import csv
import sys
from move_base_msgs.msg import MoveBaseActionGoal
from visualization_msgs.msg import Marker
from std_msgs.msg import Int32
from nav_msgs.msg import Path
#from visualization_msgs.msg import Marker1

predata=pd.read_csv('~/catkin_ws/src/kcctslam/config/waypointdata/wpdata.csv')
# get timestamp
from datetime import datetime as dt
tdatetime = dt.now()
tstr = tdatetime.strftime('%y%m%d_%H%M%S')
# output to excel file
predata.drop(predata.columns[[0]], axis=1,inplace=True)
predata.to_csv('~/catkin_ws/src/kcctslam/config/waypointdata/wpdata'+tstr+".csv")

df=pd.DataFrame(columns=['x', 'y','z','qx','qy','qz','qw','type','map'])
pub = rospy.Publisher("waypoint", Marker, queue_size = 10)
pub1 = rospy.Publisher("waypoint1", Marker, queue_size = 10)
i=0
map_num=0
path=Path

def mapcallback(map_num_row):
    global map_num
    map_num=map_num_row.data

def pathcallback(path_row):
    global path
    path=path_row

def listener():
    global path
    rospy.init_node('waypoint_manager', anonymous=True)

#    rospy.Subscriber("/move_base/goal", MoveBaseActionGoal, callback)
    rospy.Subscriber("/map_num", Int32, mapcallback)
    rospy.Subscriber("/Path",Path,pathcallback)
    rospy.spin()
    i=0
    for j in path.poses:
        df.loc[i] = [j.pose.position.x,j.pose.position.y,j.pose.position.z,j.pose.orientation.x,j.pose.orientation.y,j.pose.orientation.z,j.pose.orientation.w,1,map_num]
        df.to_csv('~/catkin_ws/src/kcctslam/config/waypointdata/wpdata.csv', header=True)
        i+=1
    print(" WayPoint SAVED!!")

if __name__ == '__main__':
    listener()
