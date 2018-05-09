import processing.serial.*;
Serial myPort;
String data;
int sampleSize = 512;
int opacity = 60;
float[] eq;
int framerate = 110;

volatile float[] dataArray = new float[sampleSize];

void setup(){
  //size(1024, 1024);
  fullScreen();
  frameRate(framerate);
  smooth(8);
  colorMode(RGB, 100);
  myPort = new Serial(this, Serial.list()[1], 115200);
  myPort.clear();
  
  eq = new float[sampleSize];
  for (int c = 0; c < sampleSize; c++){
    eq[c] = 1.15;
  }
  
  //Bass band increase
  float bass = 0.4;
  float sub = 2;
  for (int c = 1; c < 20; c++){
    if (c == 1 || c == 2) {
      eq[c] += sub;
    }
    eq[c] += bass;
  }
  
  float mid = -0.4;
  for (int c = 20; c < 100; c++){
    eq[c] += mid;
  }
  float midhigh = -0.8;
  for (int c = 100; c < 200; c++){
    eq[c] += midhigh;
  }
  float high = -2;
  for (int c = 200; c < sampleSize; c++){
    eq[c] += high;
  }
}

int t =0;
int threshold = 300;
float rot = 6;
int spacing = 2;
boolean layer1 = true;
void draw(){
  byte[] inBuffer = new byte[16];
  while(myPort.available() > 0){
    myPort.readBytesUntil(0xFF, inBuffer);
    if (inBuffer != null){
      int ID = unhex(hex(inBuffer[0]));
      int MSB = unhex(hex(inBuffer[1] << 8));
      int LSB = unhex(hex(inBuffer[2]));
      int amplitude = LSB + MSB;
      if (ID != 0){dataArray[ID] = amplitude * eq[ID];}
    }
  }
  //Calculate average value of the data
  float sum = 0;
  for (int c = 0; c < sampleSize / 2; c++){
    sum = sum + dataArray[c];
  }
  float average = sum / (sampleSize / 2);
  
  t++;
  
  //Update threshold
  threshold = (int)(300 + 100*sin(0.001*t));
  
  background(50 + 70*sin(0.001*t), 0.0001);
  //background(100 - 115 * noise(0.002*t), 0.0001);
  
  if (frameRate > 0.5 * framerate && layer1){
    for (int c = 0; c < sampleSize / 3; c++){
    
    if (dataArray[c] > average + threshold){
      fill(dataArray[c], dataArray[c + 3], dataArray[c + 6], dataArray[c] - threshold);
    }
    else { noFill(); }
    
    //Increase bass opacity
    if (c < 10){
      stroke(dataArray[2], dataArray[c], dataArray[20], map(average, 0, 200, opacity, 100) + 50);
    }
    else {
      stroke(dataArray[5], dataArray[c], dataArray[10], map(average, 0, 200, opacity, 100));
    }
    
    stroke(dataArray[5], dataArray[c], dataArray[20], opacity);
    ellipse(width / 2, height / 2, 6*dataArray[c] + random(-0.7*average, 0.7*average), 6*dataArray[c] + random(-0.7*average, 0.7*average));
  }
}
  
  
  
  float rotate = 0;
  for (int c = 0; c < (sampleSize / 3); c++){
    pushMatrix();
    translate((width / 2), (height / 2));
    //rotate = rotate + (average*PI) / (subaverage);
    
    rotate = rotate + (PI) / (rot);
    
    rotate(rotate);
    translate(spacing*(c) + 8*average + 100, 0);
    //Draw circle...
    
    if (dataArray[c] > average + threshold){
      fill(dataArray[c + (int)random(0,10)], dataArray[100], dataArray[50], dataArray[c] - threshold);
      //Chance to change the angle
      if (random(0,1) > 0.97){
        //Change angle
        rot = random(0,32);
        if (random(0,1) > 0.5){
          rot = (int)rot;
        }
        
        if (random(0,1) > 0.7){
          //Change spacing
          spacing = (int)random(1,10);
        }
        
        //Toggle layer 1
          if (layer1){
          layer1 = false;
          } else {
            if (random(0,1) > 0.6){
              layer1 = true;
            }
            
          }
      }
    }
    else { noFill(); }
    
    stroke(dataArray[c], dataArray[100], dataArray[50], opacity * 3);
    ellipse(0, 0, 2*dataArray[c], 2*dataArray[c]);
    popMatrix();
  }
}
