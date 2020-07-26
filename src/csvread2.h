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
#include"wpdata.h"
using namespace std;

class csvread2{
    private:
    //static const int gyo=2000;
    //static const int retu=10;
    //std::string Score[gyo][retu];
    vector<vector<string> > Score;
    //double wpdata[gyo][retu];
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
    
    //サイズをwpの大きさに変更
    path.poses.resize(Score.size()+1);
    vec.resize(Score.size()+1);
    //csvの値をPathとVectorに代入
    for(int k=0;k<Score.size()-1;k++){
          path.poses[k].pose.position.x=atof(Score.at(k).at(1).c_str());
          path.poses[k].pose.position.y=atof(Score.at(k).at(2).c_str());
          path.poses[k].pose.position.z=atof(Score.at(k).at(3).c_str());
          path.poses[k].pose.orientation.x=atof(Score.at(k).at(4).c_str());
          path.poses[k].pose.orientation.y=atof(Score.at(k).at(5).c_str());
          path.poses[k].pose.orientation.z=atof(Score.at(k).at(6).c_str());
          path.poses[k].pose.orientation.w=atof(Score.at(k).at(7).c_str());
          cout<<"k="<<k<<endl;
     }
     path.header.frame_id="map";
     path.header.stamp=ros::Time::now();
    //cout<<"RETSU="<<Score.size()<<endl;
    //cout<<"GYOU="<<Score.at(0).size()<<endl;
    /*
    for(int k=1;k<Score.size();k++){
    	for(int l=1;l<Score.at(0).size();l++){
    	istringstream is;        // cinの親戚？
        is.str(Score.at(k).at(l));
    	is>>wp.data[l-1][k-1];
        //wp.data[l-1][k-1]=wpdata[k-1][l-1];
    	}
    	
    }
    wp.atov();
    */
    ifs.close();

}

void csvread2::print(){
     cout<<"RETSU="<<Score.size()<<endl;
    cout<<"GYOU="<<Score.at(0).size()<<endl;
    for(int k=0;k<Score.size();k++){
    	for(int l=0;l<Score.at(k).size();l++){
    	    cout <<Score.at(k).at(l).c_str()<<",";
    	}
    	cout <<endl;
    }
}



