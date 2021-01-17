#!/usr/bin/env python
import pandas as pd
import rospy
import csv
import sys
#from move_base_msgs.msg import MoveBaseActionGoal
#from 
from visualization_msgs.msg import Marker
from std_msgs.msg import Int32
#from visualization_msgs.msg import Marker1
from geometry_msgs.msg import PoseStamped
predata=pd.read_csv('~/catkin_ws/src/kcctslam/config/waypointdata/wpdata.csv')
# get timestamp
from datetime import datetime as dt
tdatetime = dt.now()
tstr = tdatetime.strftime('%y%m%d_%H%M%S')
# output to excel file
predata.drop(predata.columns[[0]], axis=1,inplace=True)
predata.to_csv('~/catkin_ws/src/kcctslam/config/waypointdata/wpdata'+tstr+".csv")
print(predata)
df=pd.DataFrame(columns=['x', 'y','z','qx','qy','qz','qw','type','map'])
#pub = rospy.Publisher("waypoint", Marker, queue_size = 10)
#pub1 = rospy.Publisher("waypoint1", Marker, queue_size = 10)
i=0
map_num=0
def callback(data):
    global i
    global map_num
    pos = data.pose
    print "[({0},{1},0.0),(0.0,0.0,{2},{3})],".format(pos.position.x,pos.position.y,pos.orientation.z,pos.orientation.w)
    df.loc[i] = [pos.position.x,pos.position.y,0,pos.orientation.x,pos.orientation.y,pos.orientation.z,pos.orientation.w,pos.position.z,map_num]
#    df.to_csv('~/catkin_ws/src/nakanoshima/scripts/sample.csv', header=True)
    df.to_csv('~/catkin_ws/src/kcctslam/config/waypointdata/wpdata.csv', header=True)
    i=i+1
    #print(df)
    
#    with open('~/catkin_ws/src/nakanoshima/scripts/sample2.csv', 'r') as f:
    counter = 0
    j=0

            


def mapcallback(map_num_row):
    global map_num
    map_num=map_num_row.data










def listener():

    rospy.init_node('waypoint_manager', anonymous=True)

    rospy.Subscriber("/move_base_simple/goal", PoseStamped, callback)
    rospy.Subscriber("/map_num", Int32, mapcallback)
        
    rospy.spin()
        
        
        

if __name__ == '__main__':
#    df.iat[1,1]=10
#    print(df.iat[1,1])
    listener()