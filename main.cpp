#include "mbed.h"
#include "LITE_EPS.h"
#include "LITE_SENSOR.h"
#include "LITE_CDH.h"
#include "LITE_COM.h"
LITE_EPS eps(PA_0,PA_4);
LITE_SENSOR sensor(PA_7,PB_7,PB_6);
RawSerial pc(USBTX,USBRX,9600);
LITE_CDH cdh(PB_5, PB_4, PB_3, PA_8, "sd", PA_3);
LITE_COM com(PA_9,PA_10,9600);
Timer sattime;
int main()
{   
    sattime.start();
    mkdir("/sd/mydir", 0777);
    FILE *fp = fopen("/sd/mydir/test.txt","w");
    if(fp == NULL) {
        error("Could not open file for write\r\n");
    }
    char str[100];
    float ax,ay,az,gx,gy,gz,mx,my,mz;
    float temp;
    float batvol;
    eps.turn_on_regulator(); //Turn on 3.3V converter
    sensor.set_up();
    cdh.turn_on_analogSW();//turn on transceiver
    int rcmd=0,cmdflag=0;
    while(1){
        //各センサーの値を取得
        eps.vol(&batvol);
        sensor.temp_sense(&temp);
        sensor.sen_acc(&ax,&ay,&az);
        sensor.sen_gyro(&gx,&gy,&gz);
        sensor.sen_mag(&mx,&my,&mz);
        //SDにデータを保存
        fprintf(fp,"%f,%f,%f,",sattime.read(),batvol,temp);
        fprintf(fp,"%f,%f,%f,",ax,ay,az);
        fprintf(fp,"%f,%f,%f,",gx,gy,gz);
        fprintf(fp,"%f,%f,%f\r\n",mx,my,mz);
        //地上局にデータを送信
        com.printf("time %f,batvol %f,temp %f\r\n",sattime.read(),batvol,temp);
        com.printf("accel %f,%f,%f\r\n",ax,ay,az);
        com.printf("gyro %f,%f,%f\r\n",gx,gy,gz);
        com.printf("magnet %f,%f,%f\r\n",mx,my,mz);
        //1秒待機（1秒間隔でデータを取得する）
        wait(1);
        
        //地上局からのコマンドを受信
        com.xbee_receive(&rcmd,&cmdflag);
        
        if (cmdflag == 1) {
            if (rcmd == 'a') {//コマンドが"a"かを識別
                //"a"だったら下記を実行
                for(int i = 0; i < 100; i++){
                    //各センサーの値を取得
                    eps.vol(&batvol);
                    sensor.temp_sense(&temp);
                    sensor.sen_acc(&ax,&ay,&az);
                    sensor.sen_gyro(&gx,&gy,&gz);
                    sensor.sen_mag(&mx,&my,&mz);
                    //SDにデータを保存
                    fprintf(fp,"%f,%f,%f",sattime.read(),batvol,temp);
                    fprintf(fp,"%f,%f,%f",ax,ay,az);
                    fprintf(fp,"%f,%f,%f",gx,gy,gz);
                    fprintf(fp,"%f,%f,%f\r\n",mx,my,mz);
                    //地上局にデータを送信
                    com.printf("time %f,batvol %f,temp %f\r\n",sattime.read(),batvol,temp);
                    com.printf("accel %f,%f,%f\r\n",ax,ay,az);
                    com.printf("gyro %f,%f,%f\r\n",gx,gy,gz);
                    com.printf("magnet %f,%f,%f\r\n",mx,my,mz);
                }
            }
            com.initialize();
        }
    }
}

