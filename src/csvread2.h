//WayPointData(wpdata.csv) to nav_msgs/Path and Vector
//2020.7.25
//Shunya Hara

#pragma once
#include "Vector.h"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <ros/ros.h>
#include <nav_msgs/Path.h>
#include <tf/tf.h>
#include <tf/transform_broadcaster.h>
#include"wpdata.h"
using namespace std;

class csvread2{
    private:
    //static const int gyo=2000;
    //static const int retu=10;
    //std::string Score[gyo][retu];
    vector<vector<string> > Score;
    //double wpdata[gyo][retu];
    double geometry_quat_getyaw(geometry_msgs::Quaternion geometry_quat);
    public:
    csvread2(const char *st,nav_msgs::Path& path,vector<Vector>& vec);
    void print();
    //nav_msgs::Path wp_path;
    //vector<Vector> wp_vec;
     Wpdata wp;
     
};




csvread2::csvread2(const char *st,nav_msgs::Path& path,vector<Vector>& vec):Score(1,vector<string>(1)){

    ifstream ifs(st);
    // 開かなかったらエラー
    if (!ifs){
        cout << "Error! File can not be opened" << endl;
        //return 0;
    }
     /*  
     for(int k=0;k<gyo;k++){
    	for(int l=0;l<retu;l++){
    	    wpdata[k][l]=0;
    	}
    	
    }*/
    string str = "";
    int i = 0;  // Score[i][ ]のカウンタ。
    int j = 0;  // Score[ ][j]のカウンタ。

    // ファイルの中身を一行ずつ読み取る
    while(getline(ifs, str))
    {
        string tmp = "";
        istringstream stream(str);
 
        // 区切り文字がなくなるまで文字を区切っていく
        while (getline(stream, tmp, ','))
        {
            // 区切られた文字がtmpに入る
            //Score[i][j] = tmp;
            //cout<<tmp<<",";
            Score.at(i).push_back(tmp);
            j++;
            //Score.at(i).resize(j);
        }
          //cout<<endl;
        j = 0;
        i++;  
        Score.resize(i+1);
    }
     //先頭行のタグを消す
     Score.erase(Score.begin());
     //Score.resize(Score.size()-2);
    //サイズをwpの大きさに変更
    path.poses.resize(Score.size()-1);
    vec.resize(Score.size()-1);
    //csvの値をPathとVectorに代入
    for(int k=0;k<Score.size()-1;k++){
          path.poses[k].pose.position.x=atof(Score.at(k).at(1).c_str());
          path.poses[k].pose.position.y=atof(Score.at(k).at(2).c_str());
          path.poses[k].pose.position.z=atof(Score.at(k).at(3).c_str());
          path.poses[k].pose.orientation.x=atof(Score.at(k).at(4).c_str());
          path.poses[k].pose.orientation.y=atof(Score.at(k).at(5).c_str());
          path.poses[k].pose.orientation.z=atof(Score.at(k).at(6).c_str());
          path.poses[k].pose.orientation.w=atof(Score.at(k).at(7).c_str());
          vec.at(k).x=atof(Score.at(k).at(1).c_str());
          vec.at(k).y=atof(Score.at(k).at(2).c_str());
          vec.at(k).yaw=geometry_quat_getyaw(path.poses[k].pose.orientation);
          vec.at(k).type=atof(Score.at(k).at(8).c_str());
          cout<<"k="<<k<<endl;
     }
     path.header.frame_id="map";
     path.header.stamp=ros::Time::now();
     //cout<<"SCORE_SIZE="<<Score.size()<<endl;
     //cout<<"PATH_SIZE="<<path.poses.size()<<endl;
     //cout<<"VEC_SIZE="<<vec.size()<<endl;
     ifs.close();

}

void csvread2::print(){
     
    for(int k=0;k<Score.size();k++){
    	for(int l=0;l<Score.at(k).size();l++){
    	    cout <<Score.at(k).at(l).c_str()<<",";
    	}
    	cout <<endl;
    }
}

double csvread2::geometry_quat_getyaw(geometry_msgs::Quaternion geometry_quat){
    tf::Quaternion quat;
    double roll,pitch,yaw;
    quaternionMsgToTF(geometry_quat, quat);
    tf::Matrix3x3(quat).getRPY(roll, pitch, yaw);  //rpy are Pass by Reference
    return yaw;
}


