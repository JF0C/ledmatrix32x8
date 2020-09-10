int testfunction(float Time){
  float Pi = 3.14159265359;
  return (int) 50*(cos(2*Pi*100*Time)+(1+0.5*cos(2*Pi*100*Time))*sin(2*Pi*300*Time)+1);
}

byte sine_data [91]=  {0,  
                        4,    9,    13,   18,   22,   27,   31,   35,   40,   44, 
                        49,   53,   57,   62,   66,   70,   75,   79,   83,   87, 
                        91,   96,   100,  104,  108,  112,  116,  120,  124,  127,  
                        131,  135,  139,  143,  146,  150,  153,  157,  160,  164,  
                        167,  171,  174,  177,  180,  183,  186,  189,  192,  195,
                        198,  201,  204,  206,  209,  211,  214,  216,  219,  221,  
                        223,  225,  227,  229,  231,  233,  235,  236,  238,  240,  
                        241,  243,  244,  245,  246,  247,  248,  249,  250,  251,  
                        252,  253,  253,  254,  254,  254,  255,  255,  255,  255};

int amps[64] = {2, 2, 2, 2, 2, 2, 2, 2,
                4, 2, 2, 8, 2, 2, 2, 2,
                2, 2, 2, 2, 2, 2, 2, 2,
                2, 2, 2, 2, 2, 3, 2, 2,
                2, 2, 2, 2, 2, 2, 2, 2,
                4, 2, 2, 8, 2, 2, 2, 2,
                2, 2, 2, 2, 2, 2, 2, 2,
                2, 2, 2, 2, 2, 3, 2, 2};

int N = 32;

void render_fourier(){  
  if(!conf.fouriermode) return;
  int maxamps = getMax(amps, N);
  //Serial.print("Max amplitude: ");
  //Serial.println(maxamps);

  for(int i=0; i<N/2; i++){
    int nrleds = floor((float) amps[i]/ (float) maxamps * 8);
    for(int j = 0; j<nrleds; j++){
      drawxy(N/2-i-1, 7-j, red, 0.9, false);
    }
  }
  for(int i=0; i<N/2; i++){
    int nrleds = floor((float) amps[i]/ (float) maxamps * 8);
    for(int j = 0; j<nrleds; j++){
      drawxy(N/2+i, 7-j, red, 0.9, false);
    }
  }
}

void call_FFT(){
  float fs = 1000; //Hz
  
  float tmax = 0.1;
  float dt_test = 1/fs;
  int samples = 100;//(int) (tmax * fs) +1;

  
  int testsignal[samples];
  Serial.print("Nr of testarray samples: ");
  Serial.println(samples);
  Serial.print("Sampling frequency: ");
  Serial.println(1/dt_test);

  
  for(int i = 0; i < samples; i++){
    testsignal[i] = testfunction((float)i * dt_test);
  }

  Full_FFT(testsignal, N, fs);
}

float Q_FFT(int in[],int N,float Frequency)
{ 
  unsigned int Pow2[13]={1,2,4,8,16,32,64,128,256,512,1024,2048}; // declaring this as global array will save 1-2 ms of time
  int a,c1,f,o,x;         
  byte check=0;
  a=N;  
                                 
  for(int i=0;i<12;i++){ if(Pow2[i]<=a){o=i;} } //calculating levels
      
  int out_r[Pow2[o]];   //real part of transform
  int out_im[Pow2[o]];  //imaginory part of transform

  for(int i = 0; i < Pow2[o]; i++){
    out_r[i] = 0;
    out_im[i] = 0;
  }
           
  x=0;  
  
  for(int b=0;b<o;b++){                     // bit reversal
    c1=Pow2[b];
    f=Pow2[o]/(c1+c1);
    
    for(int j=0;j<c1;j++){ 
      x=x+1;
      out_im[x]=out_im[j]+f;
    }
  }

  for(int i=0;i<Pow2[o];i++){            // update input array as per bit reverse order
    out_r[i]=in[out_im[i]]; 
    out_im[i]=0;
  }

  int i10,i11,n1,tr,ti;
  float e;
  int c,s;
  for(int i=0;i<o;i++){                                    //fft
    i10=Pow2[i];              // overall values of sine/cosine  
    i11=Pow2[o]/Pow2[i+1];    // loop with similar sine cosine
    e=360/Pow2[i+1];
    e=0-e;
    n1=0;

    for(int j=0;j<i10;j++){
      c=e*j;
      while(c<0){c=c+360;}
      while(c>360){c=c-360;}
      n1=j;
          
      for(int k=0;k<i11;k++){
        if(c==0) { tr=out_r[i10+n1]; ti=out_im[i10+n1];}
        else if(c==90){ tr= -out_im[i10+n1]; ti=out_r[i10+n1];}
        else if(c==180){tr=-out_r[i10+n1]; ti=-out_im[i10+n1];}
        else if(c==270){tr=out_im[i10+n1]; ti=-out_r[i10+n1];}
        else if(c==360){tr=out_r[i10+n1]; ti=out_im[i10+n1];}
        else if(c>0  && c<90)   {tr=out_r[i10+n1]-out_im[i10+n1]; ti=out_im[i10+n1]+out_r[i10+n1];}
        else if(c>90  && c<180) {tr=-out_r[i10+n1]-out_im[i10+n1]; ti=-out_im[i10+n1]+out_r[i10+n1];}
        else if(c>180 && c<270) {tr=-out_r[i10+n1]+out_im[i10+n1]; ti=-out_im[i10+n1]-out_r[i10+n1];}
        else if(c>270 && c<360) {tr=out_r[i10+n1]+out_im[i10+n1]; ti=out_im[i10+n1]-out_r[i10+n1];}
          
        out_r[n1+i10]=out_r[n1]-tr;
        out_r[n1]=out_r[n1]+tr;
        if(out_r[n1]>15000 || out_r[n1]<-15000){check=1;}
        out_im[n1+i10]=out_im[n1]-ti;
        out_im[n1]=out_im[n1]+ti;
        if(out_im[n1]>15000 || out_im[n1]<-15000){check=1;}          
        n1=n1+i10+i10;
      }       
    }

    if(check==1){                                             // scale the matrics if value higher than 15000 to prevent varible from overloading
      for(int i=0;i<Pow2[o];i++){
        out_r[i]=out_r[i]/100;
        out_im[i]=out_im[i]/100;    
      }
      check=0;  
    }           
  }

  /*
  for(int i=0;i<Pow2[o];i++)
  {
  Serial.print(out_r[i]);
  Serial.print("\t");                                     // un comment to print RAW o/p    
  Serial.print(out_im[i]); Serial.println("i");      
  }
  */

  //---> here onward out_r contains amplitude and our_in conntains frequency (Hz)
  int fout,fm,fstp;
  float fstep;
  fstep=Frequency/N;
  fstp=fstep;
  fout=0;fm=0;

  for(int i=1;i<Pow2[o-1];i++){              // getting amplitude from compex number
    if((out_r[i]>=0) && (out_im[i]>=0)){out_r[i]=out_r[i]+out_im[i];}
    else if((out_r[i]<=0) && (out_im[i]<=0)){out_r[i]=-out_r[i]-out_im[i];}
    else if((out_r[i]>=0) && (out_im[i]<=0)){out_r[i]=out_r[i]-out_im[i];}
    else if((out_r[i]<=0) && (out_im[i]>=0)){out_r[i]=-out_r[i]+out_im[i];}
    // to find peak sum of mod of real and imaginery part are considered to increase speed
        
    out_im[i]=out_im[i-1]+fstp;
    if (fout<out_r[i]){fm=i; fout=out_r[i];}
     
   Serial.print(out_im[i]);Serial.print("Hz");
   Serial.print("\t");                                
   Serial.println(out_r[i]); 
   amps[i] = out_r[i];
     
  }
  
  float fa,fb,fc;
  fa=out_r[fm-1];
  fb=out_r[fm]; 
  fc=out_r[fm+1];
  fstep=(fa*(fm-1)+fb*fm+fc*(fm+1))/(fa+fb+fc);
  
  return (fstep*Frequency/N);
}

float sine(int i)
{
  int j=i;
  float out;
  while(j<0){j=j+360;}
  while(j>360){j=j-360;}
  if(j>-1   && j<91){out= sine_data[j];}
  else if(j>90  && j<181){out= sine_data[180-j];}
  else if(j>180 && j<271){out= -sine_data[j-180];}
  else if(j>270 && j<361){out= -sine_data[360-j];}
  return (out/255);
}

float cosine(int i)
{
  int j=i;
  float out;
  while(j<0){j=j+360;}
  while(j>360){j=j-360;}
  if(j>-1   && j<91){out= sine_data[90-j];}
  else if(j>90  && j<181){out= -sine_data[j-90];}
  else if(j>180 && j<271){out= -sine_data[270-j];}
  else if(j>270 && j<361){out= sine_data[j-270];}
  return (out/255);
}

void Full_FFT(int in[],byte N,float Frequency)
{
    /*
    Code to perform FFT on ESP2866,
    Iputs:
    1. in[]     : Data array, 
    2. N        : Number of sample (recommended sample size 2,4,8,16,32,64,128...)
    3. Frequency: sampling frequency required as input (Hz)
    Quicker, but less accurate: Q_FFT() on top
    Documentation:https://www.instructables.com/member/abhilash_patel/instructables/
    */

    unsigned int data[13]={1,2,4,8,16,32,64,128,256,512,1024,2048};
    int a,c1,f,o,x;
    a=N;
                                     
    for(int i=0;i<12;i++){ if(data[i]<=N){o=i;} }
    
    int in_ps[data[o]];     //input for sequencing
    float out_r[data[o]];   //real part of transform
    float out_im[data[o]];  //imaginory part of transform

    for(int i = 0; i < data[o]; i++){
      in_ps[i]  = 0;
      out_r[i] = 0;
      out_im[i] = 0;
    }
               
    x=0;  

    for(int b=0;b<o;b++){                     // bit reversal
      c1=data[b];
      f=data[o]/(c1+c1);
      for(int j=0;j<c1;j++)
      {
        x=x+1;
        in_ps[x]=in_ps[j]+f;
      }
    }

    for(int i=0;i<data[o];i++){            // update input array as per bit reverse order
      if(in_ps[i]<a){out_r[i]=in[in_ps[i]];}
      if(in_ps[i]>a){out_r[i]=in[in_ps[i]-a];} 
    }
    
    int i10,i11,n1;
    float e,c,s,tr,ti;

    //fft
    for(int i=0;i<o;i++){
      i10=data[i];              // overall values of sine/cosine  :
      i11=data[o]/data[i+1];    // loop with similar sine cosine:
      e=360/data[i+1];
      e=0-e;
      n1=0;
      
      for(int j=0;j<i10;j++){
        c=cosine(e*j);
        s=sine(e*j);    
        n1=j;
          
        for(int k=0;k<i11;k++){
          tr=c*out_r[i10+n1]-s*out_im[i10+n1];
          ti=s*out_r[i10+n1]+c*out_im[i10+n1];
          
          out_r[n1+i10]=out_r[n1]-tr;
          out_r[n1]=out_r[n1]+tr;
          
          out_im[n1+i10]=out_im[n1]-ti;
          out_im[n1]=out_im[n1]+ti;          
          
          n1=n1+i10+i10;
        }       
      }
    }
    /*
    for(int i=0;i<data[o];i++){
      Serial.print(out_r[i]);
      Serial.print("\t");
      Serial.print(out_im[i]); Serial.println("i");      
    }
    */
    //---> here onward out_r contains amplitude and out_im conntains frequency (Hz)
    for(int i=0;i<data[o-1];i++){               // getting amplitude from compex number
      out_r[i]=sqrt(out_r[i]*out_r[i]+out_im[i]*out_im[i]); // to  increase the speed delete sqrt
      out_im[i]=i*Frequency/N;

      /*
      Serial.print(out_im[i]); Serial.print("Hz");
      Serial.print("\t");
      Serial.println(out_r[i]); 
      */
      if(i==0){ amps[i] = out_r[i];}
      else{ amps[i] = 2*out_r[i];}
    }
}
