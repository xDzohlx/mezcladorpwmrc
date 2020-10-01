#include"p30f4013.h"
#include"stdbool.h"
#include"dsp.h"
#include "delay.h"
#include <math.h>
#define INT1 0
#define INT0 0
#define alpha 0.5


//#define FCY 
//dos entradas int 0, int 1
//int0 accelerador
//int1 volante
//dos salidas portb 0,1
//Ya funciona bien el pwm
// se tiene que cambiar por pwm de 8 bits para control de motores mas pequeño 
//lectura funciona bien 



volatile static unsigned int motor1 = 0xBB8;//señal de salida de motor 1
volatile static unsigned int motor2 = 0xBB8;//señal de salida de motor 2
volatile static unsigned int acelerador = 0xBB8;//señal de acelerador
volatile static unsigned int acelerador1 = 0xBB8;//variable temporal de acelerador
volatile static unsigned int acelerador_cero = 2700;//offset acelerador
volatile static unsigned int volante = 2200;//señal de volante
volatile static unsigned int volante1 = 0x00;//variable temporal de acelerador
volatile static unsigned int volante_cero = 2700;//offset volante
volatile static unsigned int tiempo1 = 0x00;//variable de control para medir tiempo
volatile static unsigned int tiempo2 = 0x00;//variable de control para medir tiempo
int volante_ref_pos = 0x00;//varible diferencial
int volante_ref_neg = 0x00;//varible diferencial
int contador = 0x00;//sincronizador
float volante_ref_motor1 = 0x00;//variable para diferencial
float volante_ref_motor2 = 0x00;//variable para diferencial
//variables para filtro
float signal_in = 0;
float last_ema = 0;
float last_ema1 = 0;
//booleanos para sincronizacion
bool primero1 = false;
bool primero2 = false;
bool lectura = false;
bool lectura1 = false;
int main(){
//int i = exp(6.9); //exponencial 6.9 es mas o menos 1000
setup();
//interrupciones
IEC1 |= (1<<0);
IEC0 |= (1<<0);//|(1<<6);
_T1IF = 0;
_T1IE = 1;
//interrupciones externas
IFS1 &= ~(1<<INT1);
IFS0 &= ~(1<<INT0);
IEC1 |= (1<<0);
IEC0 |= (1<<0);//|(1<<6);
//TIMER 1 A  2MHZ PARA TEOREMA DE NIQUIST
_T1IF = 0;
_T1IE = 1;
__delay32(8000);
__delay32(8000);
__delay32(8000);
__delay32(8000);
__delay32(8000);
__delay32(8000);
volante_cero = volante;//PRIMER VALOR DE VOLANTE
//volante_cero = volante;


while(1){

if(!primero1&&!primero2){//SINCRONIZACIÓN
volante_ref_pos = volante - volante_cero;
volante_ref_neg = volante_cero - volante;
if(volante>=volante_cero){//SELECCION DE SENTIDO DE GIRO
//EXPONENCIALES
volante_ref_motor1 = (((volante_ref_pos/100)*(volante_ref_pos/100)*(volante_ref_pos/100)*(volante_ref_pos/100))-((volante_ref_pos/100)*(volante_ref_pos/100)))/8;
volante_ref_motor2 = -volante_ref_motor1;
}else{
volante_ref_motor2 = (((volante_ref_neg/100)*(volante_ref_neg/100)*(volante_ref_neg/100)*(volante_ref_neg/100))-((volante_ref_neg/100)*(volante_ref_neg/100)))/8;
volante_ref_motor1 = -volante_ref_motor2;
}
//VARIABLES DE MOTOR DE SALIDA
motor1 = acelerador + volante_ref_motor1;//el compilador no puede multiplicar por 0.5 se tiene que dividir entre 2
motor2 = acelerador + volante_ref_motor2;
}
//saturacion de canales

//pwm de motor a 50 hz negado porque esta optoacoplao
if(motor1<=motor2){
if((TMR1>=motor1)&&primero1&&(TMR1>=2000)){
	LATD |= (1<<0);
	primero1 = false;
}
if((TMR1>=motor2)&&primero2&&(TMR1>=2000)){
	LATB |= (1<<1);
	primero2 = false;
}
}else{
if((TMR1>=motor2)&&primero2&&(TMR1>=2000)){
	LATB |= (1<<1);
	primero2 = false;
}
if((TMR1>=motor1)&&primero1&&(TMR1>=2000)){
	LATD |= (1<<0);
	primero1 = false;
}
}

}
return 0;
}

void setup(){
// Entradas
_TRISA11=1;
_TRISD8=1;
//Salidas
_TRISB0=0;
_TRISB1=0;
_TRISB2=0;
_TRISD0=0;
//TIMER
	_TON = 1;			//Enciende el timer, Timer ON	
	//_TCKPS = 1;	
	TMR1 = 0;
	PR1 = 0x9000;//50 HZ
//PR1 = 0xFF;//50 HZ
//interrupciones timer
	

}
void _ISR _T1Interrupt(void)
{//segunda parte de pwm 
	_LATB1 = 0;
	_LATD0 = 0;
	primero1 = true;
	primero2 = true;
 	_T1IF = 0;	
}

void _ISR _INT0Interrupt(void){

if(!(PORTA & 0x800)){//lectura del canal
	if(lectura){
	if(TMR1>tiempo1)
	acelerador = TMR1 - tiempo1;
	//acelerador = (alpha * acelerador) + ((1 - alpha) * last_ema);//filtro digital
	//last_ema = acelerador;
	lectura = false;
	_INT0EP = 0;//cambiamos flanco de interrupcion
	}
}else{
	if(!lectura){
	tiempo1 = TMR1;
	lectura = true;
	_INT0EP = 1;//cambiamos flanco de interrupcion
	}
}

IFS0 &= ~(1<<0);
}

void _ISR _INT1Interrupt(void){
if(!(PORTD & 0x100)){//lectura del canal
	if(lectura1){
	if(TMR1>tiempo2)
	volante = TMR1 - tiempo2;
	//volante = (alpha * volante) + ((1 - alpha) * last_ema1);//filtro
  	//last_ema1 = volante;
	lectura1 = false;
	_INT1EP = 0;
	}
}else{
	if(!lectura1){
	tiempo2 = TMR1;
	lectura1 = true;
	_INT1EP = 1;
	}
}
IFS1 &= ~(1<<0);
}
