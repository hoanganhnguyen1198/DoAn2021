#include "ServerLib.h"
#include <wiringPi.h>
#include <errno.h>
#include <math.h>

/* Raspberry Pi pin connection */
#define SOURCE_ENCODER_LEFT 18  //3.3V BCMpin 15
#define GND_ENCODER_LEFT 15     //GND BCMpin 13
#define SOURCE_ENCODER_RIGHT 22 //3.3V BCMpin 15
#define GND_ENCODER_RIGHT 27    //GND BCMpin 13
#define ENCODER_PIN_LEFT 14     //PIN A_LEFT BCMpin 11
#define ENCODER_PIN_RIGHT 17    //PIN A_RIGHT BCMpin 8
#define IN1 13                  //LEFT_WHEEL BCMpin 33
#define IN2 19                  //GND BCMpin 35
#define IN3 12                  //RIGHT_WHEEL BCMpin 32
//#define IN4 GND BCMpin 34 (GND Pin)

/* Motor Specifications */
// Pulses in the encoder before transmission: number_of_pulse = 11 (pulses)
#define I_PULSE_OVER_ITV 1.8182 // inverted from pulses over interval: 1/(number_of_pulse*interval/1000) 
#define TRANSMIT_RATIO 34  // Transmission ratio 34 : 1

/* WMR Specifications */
#define LENGTH 0.185   // Distance between 2 main wheels (m)
			//wheel's radius: RADIUS = 0.03 (m)
#define I_RADIUS 33.333 // 1/RADIUS
#define I_2RADIUS 16.667 // 1/(2*RADIUS)
			// Distance between main shaft and center of mass: DISTANCE = 0.05 (m)
#define I_DISTANCE 20 // 1/DISTANCE

#define PI 3.14159
#define interval 50 // Interval of inner loop: interval = 50 (ms)
#define I_ITV_OVER_TIME 20 // 1/(interval/1000)

/* Outer loop gains */
#define Kp_phi 0.75
#define Kd_phi 0.03
#define Kp_v 1.5
#define Kd_v 0.05

/* Inner loop gains */
#define Kp_l 0.25
#define Kd_l 0.015
#define Kp_r 0.25
#define Kd_r 0.015



using namespace std;

volatile int pulse_count_left = 0;
volatile int pulse_count_right = 0;


float *act_coor = new float[3]; // Toa do thuc cua WMR (x, y, phi)
float *xRef = new float[21];
float *yRef = new float[21];

float dxRef;
float dyRef;
double phiRef = 0.00;

double e_x = 0.00;
double e_y = 0.00;
double e_phi = 0.00;

double e_x_1 = 0.0;
double e_y_1 = 0.0;
double e_phi_1 = 0.0;

double alpha = 0.00;
double v = 0.00;
double w = 0.00;

double w_left_ref = 0;
double w_right_ref = 0;

double w_left = 0; // Van toc motor banh trai: (rad/s)
double w_right = 0; // Van toc motor banh phai: (rad/s)

double e_w_left = 0; // Sai lech van toc goc motor banh trai
double e_w_right = 0; // Sai lech van toc goc motor banh phai

double e_w_left_1 = 0; // Sai lech van toc goc motor banh trai vong lap truoc
double e_w_right_1 = 0; // Sai lech van toc goc motor banh phai vonglap truoc

double de_w_left = 0;
double de_w_right = 0;
 
int level_left = 180; // PWM < 180 -> Banh k chay
int level_right= 200;       

int inner_count = 0;
int outer_count = 0;

int signum(double number);

double ConvertTransmissionW(int round_w);

void ReadPulseLeft(void) {
	pulse_count_left++;	
}
void ReadPulseRight(void) {
	pulse_count_right++;
}

double CalAngularVelocity(int pulse_count) {
	double Angular_velocity = 0.00;
	Angular_velocity = (double)pulse_count*I_PULSE_OVER_ITV*2*PI;
	return Angular_velocity;
}

int main()
{
	if (wiringPiSetupGpio() < 0) 
	{
	fprintf(stderr, "Unable to setup: %s\n", strerror(errno));
	return 1;
	}	
	
	if (wiringPiISR(ENCODER_PIN_LEFT, INT_EDGE_FALLING, &ReadPulseLeft) < 0) 
	{
	fprintf(stderr, "Unable to setup ISR Left: %s\n", strerror(errno));
	return 1;
	}
	
	if (wiringPiISR(ENCODER_PIN_RIGHT, INT_EDGE_FALLING, &ReadPulseRight) < 0) 
	{
	fprintf(stderr, "Unable to setup ISR Right: %s\n", strerror(errno));
	return 1;
	}
	wiringPiSetup();
			
	pinMode(IN1, PWM_OUTPUT);
	pinMode(IN2, OUTPUT);
	pinMode(IN3, PWM_OUTPUT);
	
	pinMode(SOURCE_ENCODER_LEFT, OUTPUT);
	pinMode(GND_ENCODER_LEFT, OUTPUT);
	
	pinMode(SOURCE_ENCODER_RIGHT, OUTPUT);
	pinMode(GND_ENCODER_RIGHT, OUTPUT);
	
	digitalWrite(SOURCE_ENCODER_LEFT, HIGH);
	digitalWrite(GND_ENCODER_LEFT, LOW);	
	digitalWrite(SOURCE_ENCODER_RIGHT, HIGH);
	digitalWrite(GND_ENCODER_RIGHT, LOW);	
    
    /* Generate Trajectory here */
     for (int i = 0; i < 21; i++)
        {
	    if (i == 0)
	    {
                xRef[i]=0.2;
            }
            else  if (i == 20)
            {
                dxRef = 0;
		xRef[i] = xRef[i - 1] + dxRef * 0.5;
            }
            else if (i <= 5 || i > 18)
            {
                dxRef = 0.12;
		xRef[i] = xRef[i - 1] + dxRef * 0.5;
            }
            else if (6 <= i && i <= 12)
            {
                dxRef = 0.1432;
		xRef[i] = xRef[i - 1] + dxRef * 0.5;
            }
	    else
	    {
		dxRef = 0.1484;
		xRef[i] = xRef[i - 1] + dxRef * 0.5;
	    }

            //dyRef = 0.000;

            if (i == 0)
            {
                yRef[i] = 0.5;
            }
	    else if (i <= 5 || i > 18)
	    {
		dyRef = 0;
		yRef[i] = yRef[i - 1] + dyRef * 0.25;
	    }
	    else if (6 <= i && i <= 12)
            {
                dyRef = 0.0445;
		yRef[i] = yRef[i - 1] + dyRef * 0.25;
            }
            else
            {
		dyRef = -0.0721;
                yRef[i]= yRef[i - 1] + dyRef * 0.25;                
            }
        }
    /* Generate Trajectory end */
        
    /* Initilize Socket Server */
    ServerSocket sv = ServerSocket();
    sv.ServerInit();
    sv.start();
    /* Socket Server starts running */
	
	digitalWrite(IN2, LOW);
	while(outer_count < 21) {
		
		pwmWrite(IN3, 0);
		pwmWrite(IN1, 0);
		
		e_x_1 = e_x;
		e_y_1 = e_y;
		e_phi_1 = e_phi;
		
		act_coor = sv.GetCoordinates();
		cout << "act x: " << act_coor[0] << "; ref x: " << xRef[outer_count] << endl << "act y: " << act_coor[1] << "; ref y: " << yRef[outer_count] << endl << endl;
		
		
		phiRef = (double)atan2((double)act_coor[1] - yRef[outer_count], (double)act_coor[2] - xRef[outer_count]);
		//cout << "phiRef: " << phiRef << endl;

		e_x = (double)xRef[outer_count] - act_coor[0];
		e_y = (double)yRef[outer_count] - act_coor[1];
		e_phi = (double)phiRef - act_coor[2];

		e_phi = atan2(sin(e_phi), cos(e_phi)); // Wrap To Pi

		//Control		
		v = Kp_v * (double)sqrt(e_x * e_x + e_y * e_y) + Kd_v * 2 * (e_x * (double)(e_x - e_x_1) + e_y * (double)(e_y - e_y_1)) * I_ITV_OVER_TIME * 0.1 * signum(cos(e_phi));
		e_phi = atan(tan(e_phi));
		alpha = Kp_phi * e_phi + Kd_phi * (double)(e_phi - e_phi_1) * I_ITV_OVER_TIME * 0.1;
		if (abs(alpha) > 0.5026)
		{
			alpha = 0.5026 * signum(alpha);
		}
		if (abs(v) > 0.24)
		{
			v = (double)0.24 * signum(v);
		}
		w = (double)v * I_DISTANCE * tan(alpha);
		//cout << "v: " << v << endl << "w: " << w << endl << "alpha: " << alpha << endl << endl;
		
		w_right_ref = (double)v * I_RADIUS + (double)(LENGTH * w) * I_2RADIUS; // Temporary wR
		w_left_ref = (double)v * I_RADIUS - (double)(LENGTH * w) * I_2RADIUS; // Temporary wL

		int round_wR = (int)round(w_right_ref);
		int round_wL = (int)round(w_left_ref);
		cout << "Rounded wR: " << round_wR << endl << "Rounded wL: " << round_wL << endl << endl;
		if (round_wR > 10) round_wR = 10;
		else if (0 < round_wR && round_wR < 3) round_wR = 3;
		else if (round_wR <= 0) round_wR = 0;
		if (round_wL > 10) round_wL = 10;
		else if (0 < round_wL && round_wL < 3) round_wL = 3;
		else if (round_wL <= 0) round_wL = 0;
		
		w_right_ref = ConvertTransmissionW(round_wR);
		w_left_ref = ConvertTransmissionW(round_wL);
		//cout << "Reference wR: " << w_right_ref << endl << "Reference wL: " << w_left_ref << endl << endl;
		//w_right_ref = ConvertTransmissionW(4);
		//w_left_ref = ConvertTransmissionW(4);
				
		for(inner_count = 0; inner_count < 10; inner_count++) 
		{	
			
			pwmWrite(IN3, level_left); //level_left
			pwmWrite(IN1, level_right); //level_right
			delay(interval);
			
			w_left =  CalAngularVelocity(pulse_count_left);
			w_right=  CalAngularVelocity(pulse_count_right);
			
			pulse_count_left = 0;
			pulse_count_right= 0;
			
			e_w_left = w_left_ref- w_left;
			
			de_w_left = (double)(e_w_left - e_w_left_1)*I_ITV_OVER_TIME;

			level_left += (Kp_l*e_w_left + Kd_l*de_w_left);

			e_w_right = w_right_ref - w_right;
			
			de_w_right = (double)(e_w_right - e_w_right_1)*I_ITV_OVER_TIME;

			level_right += (Kp_r*e_w_right + Kd_r*de_w_right);
			
			e_w_left_1 = e_w_left;
			e_w_right_1= e_w_right;
		
		}
		//s_real_left = s_real_left + w_left*interval*RADIUS/(100*TRANSMIT_RATIO);
		//s_real_right = s_real_right + w_right*interval*RADIUS/(100*TRANSMIT_RATIO);
		//printf("x:%.2f\n", p[0]);
		//printf("y:%.2f\n", p[1]);  
		//printf("phi:%.2f\n", p[2]);
		//printf("Left: %f (rad/s) Right: %f (rad/s)\n\n", w_left, w_right);
		outer_count++;		
    }
    pwmWrite(IN1, 0);
    pwmWrite(IN3,0);
    return 0;
}

int signum(double number)
{
    if (signbit(number) == true)
    {
        return (-1);
    }
    else
    {
        return 1;
    }
}

double ConvertTransmissionW(int round_w)
{
    double converted;
    switch (round_w)
    {	
    case 0:
	converted = 0;
    case 3:
        converted = 102.816;
        break;
    case 4:
        converted = 137.087;
        break;
    case 5:
        converted = 171.359;
        break;
    case 6:
        converted = 205.631;
        break;
    case 7:
        converted = 239.903;
        break;
    case 8:
        converted = 274.175;
        break;
    case 9:
        converted = 308.447;
        break;
    case 10:
        converted = 342.719;
        break;
    default:
        converted = PI;
        //cout << "Error Calculating reference w.\n";
        break;
    }
    return converted;
}

  

