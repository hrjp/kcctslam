#pragma once

class wpdata{
    public:
    wpdata();
    static const int width=10;
    static const int height=100;
    double data[width][height];
    double x(int num){return data[0][num];}
    double y(int num){return data[1][num];}
    double z(int num){return data[2][num];}
    double qx(int num){return data[3][num];}
    double qy(int num){return data[4][num];}
    double qz(int num){return data[5][num];}
    double qw(int num){return data[6][num];}
    int size();
    private:
};

wpdata::wpdata(){
    for(int i;i<width;i++){
        for(int j=0;j<height;j++){
            data[i][j]=0;
        }
    }
}

int wpdata::size(){
    int data_size=0;
    while(x(data_size)||y(data_size)){
        data_size++;
    }
    return data_size;
}

