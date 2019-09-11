#!/usr/bin/env python

import rospy
import actionlib
import tf
from nav_msgs.msg import Odometry
import math
from move_base_msgs.msg import MoveBaseAction, MoveBaseGoal


waypoints = [
    [(229.966918945,115.991737366,0.0),(0.0,0.0,0.998713073099,-0.0507168376415)],
    [(344.102478027,-130.938964844,0.0),(0.0,0.0,0.681444056207,0.731870205883)]
]


def goal_pose(pose): 
    goal_pose = MoveBaseGoal()
    goal_pose.target_pose.header.frame_id = 'map'
    goal_pose.target_pose.pose.position.x = pose[0][0]
    goal_pose.target_pose.pose.position.y = pose[0][1]
    goal_pose.target_pose.pose.position.z = pose[0][2]
    goal_pose.target_pose.pose.orientation.x = pose[1][0]
    goal_pose.target_pose.pose.orientation.y = pose[1][1]
    goal_pose.target_pose.pose.orientation.z = pose[1][2]
    goal_pose.target_pose.pose.orientation.w = pose[1][3]

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
