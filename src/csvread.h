#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include"wpdata.h"

class csvread{
    private:
    static const int gyo=50;
    static const int retu=9;
    std::string Score[gyo][retu];
    
    public:
    csvread(const char *st);
    void print();
    wpdata wp;

};




csvread::csvread(const char *st){
using namespace std;
cout<<"INIT!!";
    ifstream ifs(st);
 
    // 開かなかったらエラー
    if (!ifs)
    {
        cout << "Error! File can not be opened" << endl;
        //return 0;
    }
 
     cout<<"INITOK!!";
    //  仮に百人分の配列を作っておく。
    //　名前・国語・英語・数学の順に値が入る。
    /*const int gyo=50;
    const int retu=9;
    string Score[gyo][retu];
    double wpdata[gyo][retu];*/
     for(int k=0;k<gyo;k++){
    	for(int l=0;l<retu;l++){
    	    Score[k][l]="0";
    	}
    	
    }
    string str = "";
    int i = 0;  // Score[i][ ]のカウンタ。一人分のデータを入れる
    int j = 0;  // Score[ ][j]のカウンタ。一人分の名前、国語、英語、数学を入れていく。（０～３）
    cout<<"INITOK!!";
    // ファイルの中身を一行ずつ読み取る
    while(getline(ifs, str))
    {
        string tmp = "";
        istringstream stream(str);
 
        // 区切り文字がなくなるまで文字を区切っていく
        while (getline(stream, tmp, ','))
        {
            // 区切られた文字がtmpに入る
            Score[i][j] = tmp;
            j++;
        }
 
        j = 0;
        i++;  // 次の人の配列に移る
    }
    cout<<"FILEdataOK!!";
    for(int k=1;k<wp.height;k++){
    	for(int l=1;l<wp.width;l++){
    	istringstream is;        // cinの親戚？
        is.str(Score[k][l]);
    	is>>wp.data[k-1][l-1];
    	}
    	
    }
     


}

void csvread::print(){
using namespace std;
    for(int k=0;k<wp.height-1;k++){
    	for(int l=0;l<wp.width-1;l++){
    	    cout <<wp.data[k][l]<<",";
    	}
    	cout <<endl;
    }
}



