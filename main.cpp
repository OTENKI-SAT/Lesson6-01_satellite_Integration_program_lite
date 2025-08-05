#include "mbed.h"
#include "LITE_CDH.h"
#include "LITE_EPS.h"
#include "LITE_SENSOR.h"
#include "LITE_COM.h"

// センサまわりのインスタンス
LITE_CDH    cdh(PB_5, PB_4, PB_3, PA_8, "sd", PA_3);
LITE_EPS    eps(PA_0, PA_4);
LITE_SENSOR sensor(PA_7, PB_7, PB_6);
LITE_COM    com(PA_9,PA_10,9600); 
RawSerial   sat(USBTX, USBRX, 9600); 
DigitalOut condition(PB_1);
Timer sattime;


int main()
{
    sat.printf("From Sat : Nominal Operation\r\n");
    com.printf("From Sat : Nominal Operation\r\n");
    int flag = 0; //condition flag
    float batvol, temp; //voltage, temperature 
    int rcmd=0,cmdflag=0;  //command variable
    sattime.start();
    eps.turn_on_regulator();//turn on 3.3V conveter
    cdh.turn_on_analogSW();//turn on transceiver
    com.printf("Hello\r\n");
    while(1){
        com.xbee_receive(&rcmd,&cmdflag);
        wait(1);
        //senssing HK data
        eps.vol(&batvol);
        sensor.temp_sense(&temp);
        
        //Transmitting HK data to Ground Station(GS)
        if(cmdflag == 1){
            if(rcmd == 'a'){
                sat.printf("rcmd=%c,cmdflag=%d\r\n",rcmd,cmdflag);   
                com.printf("Hepta-Sat Lite Uplink Ok\r\n");
                for(int j=0;j<5;j++){
                    eps.vol(&batvol);
                    sensor.temp_sense(&temp);
                    com.printf("Time = %f [s], batvol = %2f [V], temp = %2f [deg C]\r\n",sattime.read(),batvol,temp);
                    wait_ms(1000);
                }
            }else if(rcmd == 'b') {
                sat.printf("rcmd=%c,cmdflag=%d\r\n",rcmd,cmdflag);
                com.printf("Hepta-Sat Lite Uplink Ok\r\n");
                char str[100];
                mkdir("/sd/mydir", 0777);
                FILE *fp = fopen("/sd/mydir/satdata.txt","w");
                if(fp == NULL) {
                    error("Could not open file for write\r\n");
                }
                for(int i = 0; i < 10; i++) {
                    eps.vol(&batvol);
                    fprintf(fp,"%f\r\n",batvol);
                    wait_ms(500);
                }
                fclose(fp);
                fp = fopen("/sd/mydir/satdata.txt","r");
                for(int i = 0; i < 10; i++) {
                    fgets(str,100,fp);
                    com.puts(str);
                }
                fclose(fp);
            }else if(rcmd == 'c'){
                sat.printf("rcmd=%c,cmdflag=%d\r\n",rcmd,cmdflag);
                com.printf("Hepta-Sat Lite Uplink Ok\r\n");
                float ax,ay,az,gx,gy,gz,mx,my,mz;
                for(int i = 0; i < 10; i++) {
                    sensor.sen_acc(&ax,&ay,&az);
                    sensor.sen_gyro(&gx,&gy,&gz);
                    sensor.sen_mag(&mx,&my,&mz);
                    com.printf("ax:%f,ay:%f,az:%f\r\n",ax,ay,az);
                    com.printf("gx:%f,gy:%f,gz:%f\r\n",gx,gy,gz);
                    com.printf("mx:%f,my:%f,mz:%f\r\n",mx,my,mz);
                    wait_ms(500); 
                }
            }
             com.initialize(); //initializing
        }
       
    }
    sattime.stop();
    sat.printf("From Sat : End of operation\r\n");
    com.printf("From Sat : End of operation\r\n");
    
}
