#include <18F4620.h>
#device adc = 10
#include <stdio.h>
#include <stdlib.h>
#include <DHT11.h>
#include <MQ8.h>
#include <math.h>

#fuses HS, NOFCMEN, NOIESO, PUT, NOBROWNOUT, NOWDT
#fuses NOPBADEN, NOMCLR, STVREN, NOLVP, NODEBUG
#use delay(clock=16000000)
#define TX_232        PIN_C6
#define RX_232        PIN_C7
#use RS232(BAUD=9600, XMIT=TX_232, RCV=RX_232, stream = uart, BITS=8, PARITY=N, STOP=1)
#define lcd_rs_pin               PIN_D0
#define lcd_rw_pin               PIN_D1
#define lcd_enable_pin           PIN_D2
#define lcd_data4                PIN_D4
#define lcd_data5                PIN_D5
#define lcd_data6                PIN_D6
#define lcd_data7                PIN_D7 
#include <lcd.c>
#define alarmatem 30
#define alarmarh 25
//contador de anillo respecto a los activadores de los displays para el control del uso y mostrado de los displays
//timer0 con el despliegue de la memoria de video(displays)

int16 valor_leer = 0, sensor1 = 0, sensor2 = 0;
float concentration;
int  flag_canal1 = 0, flag_canal2 = 0, adc_read_done = 0 ;
int cambio = 1;
int selector_conf = 1;
int flag = 1, flag_conf = 0;
int flag_interrupcion = 0, flag_interrupcion_push = 0;
int dato [9]={0};
int flag_conf_timer0 = 0, flag_conf_serial = 0, flag_enviar_tem = 0, flag_enviar_rh = 0, flag_enviar_sen1_2 = 0;
int16 ms = 100, envio_cont=0, borrame=0, velocidad_serial_save = 100, velocidad_serial = 100;
int16  velocidad_lcd = 200, velocidad_lcd_save = 100; 
int alarma_tem = 30, alarma_tem_save = 30;
int alarma_rh = 12, alarma_rh_save = 12, alarma_s1 = 50, alarma_s1_save = 50, alarma_s2 = 60, alarma_s2_save = 60;
int flag_conf_lcd = 0, flag_conf_alarma = 0, selector_alarma = 1;
int flag_conf_alarma_tem = 0, flag_conf_alarma_rh = 0, flag_conf_alarma_s1 = 0, flag_conf_alarma_s2 = 0;
int16 flag_enviar_serial = 100, flag_mostrar_lcd = 100;

#INT_TIMER0
void sti_TIMER0(){
   flag_enviar_serial+=100;
   flag_mostrar_lcd +=100;
}

#INT_AD
void isr_adc(){
   valor_leer = read_adc(adc_read_only);
   adc_read_done = 1;
}

#INT_RB
void isr_RB(){
   if(input(PIN_B7) == 1){
      flag_interrupcion_push = 1;
   }
   if(input(PIN_B6) == 1){
      flag_interrupcion_push = 2;
   }
   if(input(PIN_B5) == 1){
      flag_interrupcion_push = 3;
   }
   if(input(PIN_B4) == 1){
      flag_interrupcion_push = 4;
   }
   flag_interrupcion = 1;
}

void main(void) {
   
   set_tris_a(0x02);
   set_tris_b(0xf0);   
   setup_timer_0(RTCC_INTERNAL | RTCC_DIV_8);
   enable_interrupts(INT_TIMER0);
   setup_adc(ADC_CLOCK_INTERNAL | ADC_TAD_MUL_4);
   enable_interrupts(INT_RB);
   enable_interrupts(INT_AD);
 
   enable_interrupts(GLOBAL);
   setup_adc_ports(AN0_TO_AN1);
   
   flag_canal1 = 1;
   set_adc_channel(0);
   read_adc(ADC_START_ONLY);
   
   lcd_init();
   
   while (1) {
      //establecer la lectura del adc 0 y 1
      if(flag_canal1 == 1 && adc_read_done == 1){
         sensor1 = valor_leer;
         flag_canal1 = 0;
         flag_canal2 = 1;
         adc_read_done = 0;
         set_adc_channel(1);
         read_adc(ADC_START_ONLY);
         concentration = getConcentration_8(sensor1,R0);
      }
      if(flag_canal2 == 1 && adc_done() == 1){
         sensor2 = valor_leer;
         flag_canal1 = 1;
         flag_canal2 = 0;
         adc_read_done = 0;
         set_adc_channel(0);
         read_adc(ADC_START_ONLY);
      }
      
      if(flag_interrupcion){
         flag_interrupcion = 0;
         if(flag_interrupcion_push == 1){
            if(flag_conf_alarma && flag_conf_alarma_tem == 0){
               if(flag_conf_alarma && flag_conf_alarma_rh == 0){
                  if(flag_conf_alarma && flag_conf_alarma_s1 == 0){
                     if(flag_conf_alarma && flag_conf_alarma_s2 == 0){
                     selector_alarma+=1;
                     if(selector_alarma>4){
                        selector_alarma = 4;
                     }
                  }
               }
            }
         }

         if(flag_conf_serial){
            velocidad_serial-=100;
            if(velocidad_serial<100){
               velocidad_serial = 100;
            }
         }
         if(flag_conf_lcd){
            velocidad_lcd-=100;
            if(velocidad_lcd<100){
               velocidad_lcd = 100;
            }
         }
         if(flag == 1){
            cambio++;
            //printf(lcd_putc,"\f");
            if(cambio>=4){
               cambio=3;
            }
         }
         if(flag_conf){
            selector_conf++;
            if(selector_conf>=4){
               selector_conf=3;
            }
         }
         if(flag_conf_alarma_tem){
            alarma_tem-=1;
            if(alarma_tem<1){
               alarma_tem = 1;
            }
         }
         if(flag_conf_alarma_rh){
            alarma_rh-=1;
            if(alarma_rh<1){
               alarma_rh = 1;
            }
         }
         if(flag_conf_alarma_s1){
            alarma_s1-=1;
            if(alarma_s1<1){
               alarma_s1 = 1;
            }
         }
         if(flag_conf_alarma_s2){
            alarma_s2-=1;
            if(alarma_s2<1){
               alarma_s2 = 1;
            }
         }
         printf(lcd_putc,"\f");
      }

      if(flag_interrupcion_push == 2){
         if(flag){
            cambio--;
            //printf(lcd_putc,"\f");
         if(cambio<=0){
            cambio=1;
         }
         }
         if(flag_conf){
            selector_conf--;
            if(selector_conf<=0){
               selector_conf=1;
            }
         }
         if(flag_conf_serial){
            velocidad_serial+=100;
            if(velocidad_serial>5000){
               velocidad_serial = 5000;
            }
         }
         if(flag_conf_lcd){
            velocidad_lcd+=100;
            if(velocidad_lcd>5000){
               velocidad_lcd = 5000;
            }
         }
         if(flag_conf_alarma && flag_conf_alarma_tem == 0){
            if(flag_conf_alarma && flag_conf_alarma_rh == 0){
               if(flag_conf_alarma && flag_conf_alarma_s1 == 0){
                  if(flag_conf_alarma && flag_conf_alarma_s2 == 0){
                     selector_alarma-=1;
                     if(selector_alarma<1){
                        selector_alarma = 1;
                     }
                  }      
               }
            }
         }

         if(flag_conf_alarma_tem){
            alarma_tem+=1;
            if(alarma_tem>90){
               alarma_tem = 90;
            }
         }
         if(flag_conf_alarma_rh){
            alarma_rh+=1;
            if(alarma_rh>90){
               alarma_rh = 90;
            }
         }
         if(flag_conf_alarma_s1){
            alarma_s1+=1;
            if(alarma_s1>90){
               alarma_s1 = 90;
            }
         }
         if(flag_conf_alarma_s2){
            alarma_s2+=1;
            if(alarma_s2>90){
               alarma_s2 = 90;
            }
         }
         printf(lcd_putc,"\f");
      }

      if(flag_interrupcion_push == 3){
         if(flag_conf_serial == 1 && flag_conf == 0){
            flag_conf = 1;
            flag_conf_serial = 0;
            velocidad_serial = velocidad_serial_save;
         }
         else if(flag_conf == 1 && flag == 0){
            flag = 1;
            flag_conf = 0;
         } 
         else if(flag_conf_lcd == 1 && flag_conf == 0){
            flag_conf = 1;
            flag_conf_lcd = 0;
            velocidad_lcd = velocidad_lcd_save;
         }
         else if(flag_conf_alarma_tem == 1 && flag_conf_alarma == 1){
            alarma_tem = alarma_tem_save;
            flag_conf_alarma_tem = 0;
         }
         else if(flag_conf_alarma_rh == 1 && flag_conf_alarma == 1){//selector humedad
            alarma_rh = alarma_rh_save;
            flag_conf_alarma_rh = 0;
         }
         else if(flag_conf_alarma_s1 == 1 && flag_conf_alarma == 1){//selector s1
            alarma_s1 = alarma_s1_save;
            flag_conf_alarma_s1 = 0;
         }
         else if(flag_conf_alarma_s2 == 1 && flag_conf_alarma == 1){//selector s2
            alarma_s2 = alarma_s2_save;
            flag_conf_alarma_s2 = 0;
         }
         else if(flag_conf_alarma == 1 && flag_conf_alarma_tem == 0){
            if(flag_conf_alarma == 1 && flag_conf_alarma_rh == 0){
               if(flag_conf_alarma == 1 && flag_conf_alarma_s1 == 0){
                  if(flag_conf_alarma == 1 && flag_conf_alarma_s2 == 0){
                     flag_conf_alarma = 0;
                     flag_conf = 1;
                  }
               }
            }
         }
      }

      if(flag_interrupcion_push == 4){
         if(flag_conf == 1 && selector_conf == 1){
            flag_conf_serial = 1;
            flag_conf = 0;
         }
         else if(flag_conf_alarma_tem == 1){
            flag_conf_alarma_tem = 0;
            alarma_tem_save = alarma_tem;
         }
         else if(selector_alarma == 1 && flag_conf_alarma == 1){
            alarma_tem = alarma_tem_save;
            flag_conf_alarma_tem = 1;
         }
         //slector humedad
         else if(flag_conf_alarma_rh == 1){
            flag_conf_alarma_rh = 0;
            alarma_rh_save = alarma_rh;
         }
         else if(selector_alarma == 2 && flag_conf_alarma == 1){
            alarma_rh = alarma_rh_save;
            flag_conf_alarma_rh = 1;
         }
         //slector s1
         else if(flag_conf_alarma_s1 == 1){
            flag_conf_alarma_s1 = 0;
            alarma_s1_save = alarma_s1;
         }
         else if(selector_alarma == 3 && flag_conf_alarma == 1){
            alarma_s1 = alarma_s1_save;
            flag_conf_alarma_s1 = 1;
         }
         //slector s2
         else if(flag_conf_alarma_s2 == 1){
            flag_conf_alarma_s2 = 0;
            alarma_s2_save = alarma_s2;
         }
         else if(selector_alarma == 4 && flag_conf_alarma == 1){
            alarma_s2 = alarma_s2_save;
            flag_conf_alarma_s2 = 1;
         }

         else if(flag_conf_serial == 1 && flag_conf == 0){
            velocidad_serial_save = velocidad_serial;
            flag_conf = 1;
            flag_conf_serial = 0;
         }
         else if(flag_conf_lcd == 1 && flag_conf == 0){
            velocidad_lcd_save = velocidad_lcd;
            flag_conf = 1;
            flag_conf_lcd = 0;
         }
         else if(flag == 1 && flag_conf == 0){
            flag_conf = 1;
            flag = 0;
         }
         else if(flag_conf == 1 && selector_conf == 2){
            flag_conf_lcd = 1;
            flag_conf = 0;
         }
         else if(flag_conf_lcd){
            flag_conf_lcd = 0;
            flag_conf = 1;
         }
         else if(selector_conf == 3){
            flag_conf_alarma = 1;
            flag_conf = 0;
         }
      }
      printf(lcd_putc,"\f");
      flag_interrupcion_push = 0;
   }    
      //si es esc al momento de conf se aborta
      //alarma tiempo de actualizacion es de 100 a 2 seg
      //conf de las alarmas es de 01% en temperaturas y 100ppm para los gases 
      //lectura del sensor dht11 mas alarmas
   Start_signal();
   if(check_response()){                    
      RhByte1 = Read_Data();          
      RhByte2 = Read_Data();
      TemByte1 = Read_Data();                   
      TemByte2 = Read_Data();                     
      CheckSum = Read_Data();                     
      if(CheckSum == ((RhByte1 + RhByte2 + TemByte1 + TemByte2) )){
         
         dato[1]=(int)TemByte1/10;
         dato[2]=(int)TemByte1%10;
         dato[3]=(int)TemByte2/10;
         dato[4]=(int)TemByte2%10;
         if(TemByte1 >= alarma_tem_save){
            output_high(pin_b0);
            //printf("\fAlarma temperatura");
         }
         else{
            output_low(pin_b0);
         }
         dato[5]=(int)RhByte1/10;
         dato[6]=(int)RhByte1%10;
         dato[7]=(int)RhByte2/10;
         dato[8]=(int)RhByte2%10;
         if(RhByte1 <= alarma_rh_save){
            output_high(pin_b1);
            //printf("\fAlarma humedad");
         }
         else{
            output_low(pin_b1);
         } 
      }
   } 

   if(flag_mostrar_lcd == velocidad_lcd_save){
      printf(lcd_putc,"\f");
      if(flag_conf){
         switch(selector_conf){
            case 1:
               lcd_gotoxy(1,1);
               printf(lcd_putc,"->conf serial");
               lcd_gotoxy(1,2);
               printf(lcd_putc,"  conf act lcd");
               velocidad_serial_save = velocidad_serial;
            break;
            case 2:
               lcd_gotoxy(1,1);
               printf(lcd_putc,"->conf act lcd");
               lcd_gotoxy(1,2);
               printf(lcd_putc,"  conf alarma");
            break;
            case 3:
               lcd_gotoxy(1,1);
               printf(lcd_putc,"  conf act lcd");
               lcd_gotoxy(1,2);
               printf(lcd_putc,"->conf alarma");
            break;
         }
      }
     
     //desplegado del menu principal
      if(flag){
         switch(cambio){
            case 1:
               lcd_gotoxy(1,1);
               printf(lcd_putc,"\f%d%d.%d%d C",dato[1],dato[2],dato[3],dato[4]);
               lcd_gotoxy(1,2);
               printf(lcd_putc,"%d%d.%d%d %c",dato[5],dato[6],dato[7],dato[8],37);
            break;
            case 2:
               lcd_gotoxy(1,1);
               printf(lcd_putc,"\f%d%d.%d%d %c",dato[5],dato[6],dato[7],dato[8],37);
               lcd_gotoxy(1,2);
               printf(lcd_putc,"%0.9f",(float)concentration);
            break;
            case 3:
               lcd_gotoxy(1,1);
               printf(lcd_putc,"\f%li",concentration);
               lcd_gotoxy(1,2);
               printf(lcd_putc,"%li",sensor2);
            break;
         }
      }
      if(flag_conf_serial){
         lcd_gotoxy(1,1);
         printf(lcd_putc,"\fTiempo");
         lcd_gotoxy(1,2);
         printf(lcd_putc,"%li",velocidad_serial);  
      }
      if(flag_conf_lcd){
         lcd_gotoxy(1,1);
         printf(lcd_putc,"\fTiempo");
         lcd_gotoxy(1,2);
         printf(lcd_putc,"%li",velocidad_lcd);
      }
      if(flag_conf_alarma){
         switch(selector_alarma){
            case 1:
               lcd_gotoxy(1,1);
               printf(lcd_putc,"\f->%d C",alarma_tem);
               lcd_gotoxy(1,2);
               printf(lcd_putc,"  %d %c",alarma_rh,37);
            break;
            case 2:
               lcd_gotoxy(1,1);
               printf(lcd_putc,"\f->%d %c",alarma_rh,37);
               lcd_gotoxy(1,2);
               printf(lcd_putc,"  %d ppm",alarma_s1);
            break;
            case 3:
               lcd_gotoxy(1,1);
               printf(lcd_putc,"\f->%d ppm",alarma_s1);
               lcd_gotoxy(1,2);
               printf(lcd_putc,"  %d ppm",alarma_s2);
            break;
            case 4:
               lcd_gotoxy(1,1);
               printf(lcd_putc,"\f  %d ppm",alarma_s1);
               lcd_gotoxy(1,2);
               printf(lcd_putc,"->%d ppm",alarma_s2);
               break;
         }
      }
      flag_mostrar_lcd = 0;
   }
   if(velocidad_serial_save == flag_enviar_serial){
      printf("\n1");
      printf("\n%d%d.%d%d",dato[5],dato[6],dato[7],dato[8]);
      printf("\n0");
      printf("\n%d%d.%d%d",dato[1],dato[2],dato[3],dato[4]);
      printf("\n2");
      printf("\n%li",sensor1);
      printf("\n3");
      printf("\n%li",sensor2);
      

      /*printf("\n0");
      printf("\n%d%d.%d%d %c",dato[5],dato[6],dato[7],dato[8],37);
      printf("\n1");
      printf("\n%d%d.%d%d C",dato[1],dato[2],dato[3],dato[4]);
      printf("\n2");
      printf("\n%li",sensor1);
      printf("\n3");
      printf("\n%li",sensor2);
      */
      flag_enviar_serial = 0;
   }
}
}
