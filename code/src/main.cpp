#include <Adafruit_MPU6050.h> 
#include <Adafruit_Sensor.h>

#include <PID_v1.h>

Adafruit_MPU6050 mpu;

#define m1 12 //-x +y
#define m2 14 //+x +y
#define m3 27 //-x -y
#define m4 26 //+x -y

double clamp(double d, double min, double max) {
  const double t = d < min ? min : d;
  return t > max ? max : t;
}

double AzSetpoint, AzInput, AzOutput;
double RxSetpoint, RxInput, RxOutput;
double RySetpoint, RyInput, RyOutput;

PID AzPID(&AzInput, &AzOutput, &AzSetpoint, 2,5,1, DIRECT);
PID RxPID(&RxInput, &RxOutput, &RxSetpoint, 2,5,1, DIRECT);
PID RyPID(&RyInput, &RyOutput, &RySetpoint, 2,5,1, DIRECT);

void setup() {

  Serial.begin(115200);

  while (!mpu.begin()) { //Initialize the MPU-6050 sensor.

    Serial.println("Sensor init failed for now");
    delay(1000);
  }

  Serial.println("found!");
  
  pinMode(m1, OUTPUT);
  pinMode(m2, OUTPUT);
  pinMode(m3, OUTPUT);
  pinMode(m4, OUTPUT);
  
  AzPID.SetMode(AUTOMATIC);
  RxPID.SetMode(AUTOMATIC);
  RyPID.SetMode(AUTOMATIC);

  AzSetpoint = -10.4; // g to maintain
  RxSetpoint = 0.0; //rotx
  RySetpoint = 0.0; //roty
}

double t1 = 0.0, t2 = 0.0, t3 = 0.0, t4 = 0.0;
double RxGain = 0.1, RyGain = 0.1;
int Dir = 1;


void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  Serial.println("Acceleration Z: ");
  Serial.println(a.acceleration.z);

  Serial.println("Rotation X,Y: ");

  Serial.println(g.gyro.x);
  Serial.println(g.gyro.y);

  AzInput = a.acceleration.z;
  RxInput = g.gyro.x;
  RyInput = g.gyro.y;

  AzPID.Compute();
  RxPID.Compute();
  RyPID.Compute();

  Serial.print("Output PIDs: %d");

  Serial.println(AzOutput);
  Serial.println(RxOutput);
  Serial.println(RyOutput);
  
  //altitude control

  t1 = AzOutput;
  t2 = AzOutput;
  t3 = AzOutput;
  t4 = AzOutput;

  //Rx gyro control
  if (RxInput >= 0) Dir = 1;
  else Dir = -1;

  t1 += RxOutput * RxGain * Dir;
  t2 += RxOutput * RxGain * Dir;
  t3 += RxOutput * RxGain * -Dir;
  t4 += RxOutput * RxGain * -Dir;

  
  //Ry gyro control
  if (RyInput >= 0) Dir = 1;
  else Dir = -1;

  t1 += RyOutput * RxGain * -Dir;
  t2 += RyOutput * RxGain * Dir;
  t3 += RyOutput * RxGain * -Dir;
  t4 += RyOutput * RxGain * Dir;

  analogWrite(m1, t1);
  analogWrite(m2, t2);
  analogWrite(m3, t3);
  analogWrite(m4, t4);
  
  delay(500); // MPU SAMPLE LIMIT
}
