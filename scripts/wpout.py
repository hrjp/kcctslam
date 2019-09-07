#!/usr/bin/env python
import pandas as pd
import rospy
import csv
from move_base_msgs.msg import MoveBaseActionGoal
from visualization_msgs.msg import Marker
df=pd.DataFrame(columns=['x', 'y','z','qx','qy','qz','qw'])
pub = rospy.Publisher("waypoint", Marker, queue_size = 10)
def callback(data):
    i=0
    pos = data.goal.target_pose.pose
    print "[({0},{1},0.0),(0.0,0.0,{2},{3})],".format(pos.position.x,pos.position.y,pos.orientation.z,pos.orientation.w)
    df.loc[callback.i] = [pos.position.x,pos.position.y,pos.position.z,pos.orientation.x,pos.orientation.y,pos.orientation.z,pos.orientation.w]
#    df.to_csv('~/catkin_ws/src/nakanoshima/scripts/sample.csv', header=True)
    df.to_csv('~/catkin_ws/src/kcctslam/config/waypointdata/wpdata.csv', header=True)
    callback.i=callback.i+1
    print(df)
    
#    with open('~/catkin_ws/src/nakanoshima/scripts/sample2.csv', 'r') as f:
    counter = 0
    j=0
#        reader = csv.reader(f)
#        header = next(reader)

    for j in range(callback.i):
            # Mark arrow
            marker_data = Marker()
            marker_data.header.frame_id = "map"
            marker_data.header.stamp = rospy.Time.now()

            marker_data.ns = "basic_shapes"
            marker_data.id = j

            marker_data.action = Marker.ADD

            marker_data.pose.position.x =df.iat[j,0]
            marker_data.pose.position.y =df.iat[j,1]
            marker_data.pose.position.z =df.iat[j,2]

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

            pub.publish(marker_data)
            
            
            
            
            
callback.i=0

def listener():

    rospy.init_node('waypoint_manager', anonymous=True)

    rospy.Subscriber("/move_base/goal", MoveBaseActionGoal, callback)

    rospy.spin()

if __name__ == '__main__':
#    df.iat[1,1]=10
#    print(df.iat[1,1])
    listener()
