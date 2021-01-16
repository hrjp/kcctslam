#!/usr/bin/env python
import pandas as pd
import rospy
import csv
import sys
from std_msgs.msg import Float32MultiArray
from datetime import datetime as dt

df=pd.DataFrame(columns=['error', 'area','veriance'])
i=0

def recordcallback(data_raw):
    global i
    #print(data_raw)
    df.loc[i] = [data_raw.data[0],data_raw.data[1],data_raw.data[2]]
    i+=1

def listener():
    rospy.init_node('data_recorder', anonymous=True)
    rospy.Subscriber("/data_record", Float32MultiArray, recordcallback)
    rospy.spin()
    tdatetime = dt.now()
    tstr = tdatetime.strftime('%y%m%d_%H%M%S')
    df.to_csv('~/catkin_ws/src/kcctslam/config/path_log/'+tstr+'data.csv', header=True)
    print('CSV Saved !!')

if __name__ == '__main__':
    listener()