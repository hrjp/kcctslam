#!/usr/bin/env python
import pandas as pd
import rospy
import actionlib
import tf
from nav_msgs.msg import Odometry
import math
from move_base_msgs.msg import MoveBaseAction, MoveBaseGoal

import csv
from visualization_msgs.msg import Marker
#df=pd.DataFrame(columns=['x', 'y','z','qx','qy','qz','qw'])
df = pd.read_csv('~/catkin_ws/src/nakanoshima/scripts/sample.csv', header=0,index_col=0)
#pub = rospy.Publisher("waypoint", Marker, queue_size = 10)
print(df)

waypoints = df.values.tolist()

def goal_pose(pose): 
    goal_pose = MoveBaseGoal()
    goal_pose.target_pose.header.frame_id = 'map'
    goal_pose.target_pose.pose.position.x = pose[0]
    goal_pose.target_pose.pose.position.y = pose[1]
    goal_pose.target_pose.pose.position.z = pose[2]
    goal_pose.target_pose.pose.orientation.x = pose[3]
    goal_pose.target_pose.pose.orientation.y = pose[4]
    goal_pose.target_pose.pose.orientation.z = pose[5]
    goal_pose.target_pose.pose.orientation.w = pose[6]

    return goal_pose


if __name__ == '__main__':
    rospy.init_node('patrol')
    listener = tf.TransformListener()

    client = actionlib.SimpleActionClient('move_base', MoveBaseAction) 
    client.wait_for_server()
    listener.waitForTransform("map", "base_link", rospy.Time(), rospy.Duration(4.0))
    while True:
        for pose in waypoints: 
            goal = goal_pose(pose)
            print(goal)
            client.send_goal(goal)
            while True:
                now = rospy.Time.now()
                listener.waitForTransform("map", "base_link", now, rospy.Duration(4.0))
                

                position, quaternion = listener.lookupTransform("map", "base_link", now)
                

                if(math.sqrt((position[0]-goal.target_pose.pose.position.x)**2 + (position[1]-goal.target_pose.pose.position.y)**2 ) <= 1):
                    print "next!!"
                    break

                else:
                    rospy.sleep(0.5)
