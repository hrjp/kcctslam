//PID制御クラス
//2018.12.27
//原 俊哉
#ifndef PID_LIB_H
#define PID_LIB_H
#include"Vector.h"
class PID{
    public:

    //コンストラクタ
    //kp:比例定数
    //ki:積分定数
    //kd:微分定数
    PID(float kp,float ki,float kd);

    //空コンストラクタ
    PID(){}

    //初期化関数
    //コンストラクタで定数を設定しなかった場合や
    //プログラムの途中で定数を変更する場合に使う。
    //kp:比例定数
    //ki:積分定数
    //kd:微分定数
    void set(float kp,float ki,float kd);

    //更新関数
    //1ループに一つ入れる。
    //制御周期を固定しない場合。
    //local_val:現在地
    //target_val:目標値
    //戻り値:PID計算結果
    float update(float local_val,float target_val);
    float update(Vector local_val,Vector target_val);
    //更新関数
    //1ループに一つ入れる。
    //制御周期を固定する場合。
    //local_val:現在地
    //target_val:目標値
    //loop_cycle_ms:制御周期(ミリ秒)
    //戻り値:PID計算結果
    float update(float local_val,float target_val,unsigned long loop_cycle_ms);

    //計算結果を返す関数
    //戻り値:PID計算結果
    float result_val();

    //積分値をリセットする関数。
    void reset_i();


    private:

    float kpp,kii,kdd;
    float dt;
    float res_p,res_i,res_d,res_prep;
    float result_value;
    unsigned long pretime;

};



//example
//可変抵抗の値を使ってPID制御
/*

#include"PID_lib.h"
#include"Servo.h"

PID pid(1.0,0.1,0.01);//比例定数1.0,積分定数0.1,微分定数0.01に設定
Servo m;
int terget=500;//目標値を500に設定
void setup(){
  m.attach(3);
}

void loop(){
  pid.update(analogRead(A0),terget);//現在値に可変抵抗のアナログ値,目標値にtergetを渡す
  m.writeMicroseconds(1500+pid.result_val());//PID制御の計算結果をモーターの出力にする
}
 
 */










/*
*
*以下関数の実装部
*
*/

PID::PID(float kp,float ki,float kd){
    kpp=kp;
    kii=ki;
    kdd=kd;
}

void PID::set(float kp,float ki,float kd){
    kpp=kp;
    kii=ki;
    kdd=kd;
}

float PID::update(float local_val,float target_val,unsigned long loop_cycle_ms){
    dt=loop_cycle_ms/1000.0;

    res_p=target_val-local_val;
    res_i+=res_p*dt;
    res_d=(res_p-res_prep)/dt;

    res_prep=res_p;

    result_value=kpp*res_p+kii*res_i+kdd*res_d;
    return result_value;
}

float PID::update(float local_val,float target_val){
    dt=(micros()-pretime)/1000000.0;
    pretime=micros();

    res_p=target_val-local_val;
    res_i+=res_p*dt;
    res_d=(res_p-res_prep)/dt;

    res_prep=res_p;

    result_value=kpp*res_p+kii*res_i+kdd*res_d;
    return result_value;
}

float PID::update(Vector local_val,Vector target_val){
    dt=(micros()-pretime)/1000000.0;
    pretime=micros();

    res_p=(target_val-local_val).size();
    res_i+=res_p*dt;
    res_d=(res_p-res_prep)/dt;

    res_prep=res_p;

    result_value=kpp*res_p+kii*res_i+kdd*res_d;
    return result_value;
}

void PID::reset_i(){
    res_i=0;
}

float PID::result_val(){
    return result_value;
}


#endif
