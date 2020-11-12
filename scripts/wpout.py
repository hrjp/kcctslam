#!/usr/bin/env python
import pandas as pd
import rospy
import csv
import sys
from move_base_msgs.msg import MoveBaseActionGoal
from visualization_msgs.msg import Marker
from std_msgs.msg import Int32
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
def callback(data):
    global i
    global map_num
    pos = data.goal.target_pose.pose
    print "[({0},{1},0.0),(0.0,0.0,{2},{3})],".format(pos.position.x,pos.position.y,pos.orientation.z,pos.orientation.w)
    df.loc[i] = [pos.position.x,pos.position.y,0,pos.orientation.x,pos.orientation.y,pos.orientation.z,pos.orientation.w,pos.position.z,map_num]
#    df.to_csv('~/catkin_ws/src/nakanoshima/scripts/sample.csv', header=True)
    df.to_csv('~/catkin_ws/src/kcctslam/config/waypointdata/wpdata.csv', header=True)
    i=i+1
#    print(df)
    
#    with open('~/catkin_ws/src/nakanoshima/scripts/sample2.csv', 'r') as f:
    counter = 0
    j=0
#        reader = csv.reader(f)
#        header = next(reader)

    for j in range(i):
            # Mark arrow
            marker_data = Marker()
            marker_data.header.frame_id = "map"
            marker_data.header.stamp = rospy.Time.now()

            marker_data.ns = "basic_shapes"
            marker_data.id = j

            marker_data.action = Marker.ADD

            marker_data.pose.position.x =df.iat[j,0]
            marker_data.pose.position.y =df.iat[j,1]
            marker_data.pose.position.z =0

            marker_data.pose.orientation.x=df.iat[j,3]
            marker_data.pose.orientation.y=df.iat[j,4]
            marker_data.pose.orientation.z=df.iat[j,5]
            marker_data.pose.orientation.w=df.iat[j,6]

            marker_data.color.r = 1.0
            marker_data.color.g = 0.0
            marker_data.color.b = 0.0
            marker_data.color.a = 1.0
            marker_data.scale.x = 1
            marker_data.scale.y = 0.15
            marker_data.scale.z = 0.15

            marker_data.lifetime = rospy.Duration()

            marker_data.type = 0
            
            
            
            
            # Mark number
            marker_data1 = Marker()
            marker_data1.header.frame_id = "map"
            marker_data1.header.stamp = rospy.Time.now()

            marker_data1.ns = "basic_shapes"
            marker_data1.id = j

            marker_data1.action = Marker.ADD

            marker_data1.pose.position.x =df.iat[j,0]
            marker_data1.pose.position.y =df.iat[j,1]
            marker_data1.pose.position.z =1

            marker_data1.pose.orientation.x=df.iat[j,3]
            marker_data1.pose.orientation.y=df.iat[j,4]
            marker_data1.pose.orientation.z=df.iat[j,5]
            marker_data1.pose.orientation.w=df.iat[j,6]

            marker_data1.color.r = 0.0
            marker_data1.color.g = 0.0
            marker_data1.color.b = 0.0
            marker_data1.color.a = 1.0
            marker_data1.scale.x = 1
            marker_data1.scale.y = 1
            marker_data1.scale.z = 1

            marker_data1.lifetime = rospy.Duration()
            marker_data1.text=str(j)
            marker_data1.type = 9

            pub1.publish(marker_data1)
            pub.publish(marker_data)
            


def mapcallback(map_num_row):
    global map_num
    map_num=map_num_row.data










def listener():

    rospy.init_node('waypoint_manager', anonymous=True)

    rospy.Subscriber("/move_base/goal", MoveBaseActionGoal, callback)
    rospy.Subscriber("/map_num", Int32, mapcallback)
        
    rospy.spin()
        
        
        

if __name__ == '__main__':
#    df.iat[1,1]=10
#    print(df.iat[1,1])
    listener()
