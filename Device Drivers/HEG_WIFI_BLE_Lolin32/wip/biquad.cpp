//Biquad filters for arduino based on this: https://arachnoid.com/phase_locked_loop/resources/biquad_module.py
     
#include <math.h>

class Biquad {
  public:

  float freq,sps,Q,dbGain,a0,a1,a2,b0,b1,b2,x1,x2,y1,y2,A,omega,sn,cs,alpha,beta;


  Biquad(const String& typein, float freqin,float spsin,float Qin=1/sqrt(2),float dbGainin=0) {
    //types = ['lowpass','highpass','bandpass','notch','peak','lowshelf','highshelf'];

    freq = freqin;
    sps = spsin;
    Q = Qin;
    dbGain = dbGainin;

    a0 = 0,a1 = 0,a2 = 0,
    b0 = 0,b1 = 0,b2 = 0;

    x1 = 0,x2 = 0,
    y1 = 0,y2 = 0;

    A = pow(10,dbGain/40);
    omega = 2*PI*freq/sps;
    sn = sin(omega);
    cs = cos(omega);
    alpha = sn/(2*Q);
    beta = sqrt(A+A);

    if(typein == "lowpass") {
      lowpass(A,sn,cs,alpha,beta);
    } else if (typein == "highpass") {
      highpass(A,sn,cs,alpha,beta);
    } else if (typein == "bandpass") {
      bandpass(A,sn,cs,alpha,beta);
    } else if (typein == "notch") {
      notch(A,sn,cs,alpha,beta);
    } else if (typein == "peak") {
      peak(A,sn,cs,alpha,beta);
    } else if (typein == "lowshelf") {
      lowshelf(A,sn,cs,alpha,beta);
    } else if (typein == "highshelf") {
      highshelf(A,sn,cs,alpha,beta);
    }

    //scale constants
    b0 /= a0;
    b1 /= a0;
    b2 /= a0;
    a1 /= a0;
    a2 /= a0;
    
  }

  void lowpass(float A,float sn,float cs,float alpha,float beta) { //Stop upper frequencies
    b0 = (1-cs)*.5;
    b1 = 1-cs;
    b2 = (1-cs)*.5;
    a0 = 1+alpha;
    a1 = -2*cs;
    a2 = 1-alpha;
  }

  void highpass(float A,float sn,float cs,float alpha,float beta) { //Stop lower frequencies 
    b0 = (1+cs)*.5;
    b1 = -(1+cs);
    b2 = (1+cs)*.5;
    a0 = 1 + alpha;
    a1 = -2*cs;
    a2 = 1-alpha;
  }

  void bandpass(float A,float sn,float cs,float alpha,float beta) { //Stop lower and upper frequencies. Q = frequency_resonant / Bandwidth(to 3db cutoff line); frequency_resonant = Math.sqrt(f_low * f_high); So for 60Hz with 0.5Hz bandwidth: Fr = Math.sqrt(59.5*60.5). Q = Fr/0.5 = 120;
    b0 = alpha;
    b1 = 0;
    b2 = -alpha;
    a0 = 1+alpha;
    a1 = -2*cs;
    a2 = 1-alpha;
  }

  void notch(float A,float sn,float cs,float alpha,float beta) { //Stop a specific frequency
    b0 = 1;
    b1 = -2*cs;
    b2 = 1;
    a0 = 1+alpha;
    a1 = -2*cs;
    a2 = 1-alpha;
  }

  void peak(float A,float sn,float cs,float alpha,float beta) { //Opposite of a notch filter, stop all but one frequency
    b0 = 1+(alpha*A);
    b1 = -2*cs;
    b2 = 1-(alpha*A);
    a0 = 1+(alpha/A);
    a1 = -2*cs;
    a2 = 1-(alpha/A);
  }

  void lowshelf(float A,float sn,float cs,float alpha,float beta) { //Amplify signals below the cutoff
    b0 = A*((A+1) - (A-1)*cs + beta*sn);
    b1 = 2*A*((A-1)-(A+1)*cs);
    b2 = A*((A+1) - (A-1)*cs - beta*sn);
    a0 = (A+1) + (A+1)*cs + beta*sn;
    a1 = 2*((A-1) + (A+1)*cs);
    a2 = (A+1) + (A-1)*cs - beta*sn;
  }

  void highshelf(float A,float sn,float cs,float alpha,float beta) { //Amplify signals above the cutoff
    b0 = A*((A+1) + (A-1)*cs + beta*sn);
    b1 = 2*A*((A-1) + (A+1)*cs);
    b2 = A*((A+1) - (A-1)*cs - beta*sn);
    a0 = (A+1) - (A+1)*cs - beta*sn;
    a1 = 2*((A-1) - (A+1)*cs);
    a2 = (A+1) - (A-1)*cs - beta*sn;
  }

  float applyFilter(float signal_step) { //Step the filter forward, return modulated signal amplitude
    float y = b0*signal_step + b1*x1 + b2*x2 - a1*y1 - a2*y2;
    x2 = x1;
    x1 = signal_step;
    y2 = y1;
    y1 = y;
    
    return y;
  }

  float zResult(float freq) { //This should return the z-transfer function values. Max freq = sps/2
    try{
      float phi = pow((sin(3.14159265359*freq*2/(2*sps))),2);
      float result = (pow(b0+b1+b2,2) - 
                  4*(b0*b1+4*b0*b2 + b1*b2)*phi + 16*b0*b2*phi*phi) / 
                  (pow(1+a1+a2,2) - 4*(a1 + 4*a2 + a1*a2)*phi + 16*a2*phi*phi);
      return result;
    } catch(...) {
      return -200;
    }
  }

  //Get the center frequency for your bandpass filter
  static float calcCenterFrequency(float freqStart,float freqEnd) {
    return (freqStart+freqEnd) / 2;
  }

  static float calcBandwidth(float freqStart,float freqEnd) {
    return (freqEnd-calcCenterFrequency(freqStart,freqEnd));
  }

  //Use for bandpass or peak filter //Q gets sharper as resonance approaches infinity. Set to 500 for example for a more precise filter. Recommended r: Math.floor(Math.log10(frequency))
  static float calcBandpassQ (float frequency, float bandwidth=1, float resonance=0) { //Use Math.sqrt(0.5) for low pass, high pass, and shelf filters
    float r = resonance; if (r == 0) r = pow(10,floor(log10(frequency))); 
    float Qout = r*sqrt((frequency-bandwidth)*(frequency+bandwidth))/(2*bandwidth); //tweaked
    return Qout;
  }

  static float calcNotchQ (float frequency, float bandwidth=1, float resonance=0) { //Q gets sharper as resonance approaches infinity. Recommended r: Math.floor(Math.log10(frequency))
      float r = resonance; if (r == 0) r = pow(10,floor(log10(frequency))); 
      float Qout = r*frequency*bandwidth/sqrt((frequency-bandwidth)*(frequency+bandwidth)); // bw/f
      return Qout;
  }

};

class DCBlocker {
  public:

    float r = 0;
    float x1,x2,y1,y2 = 0;

    DCBlocker(float res){
      r = res;
    }

    float applyFilter(float signal_step){
      x2=x1;
      x1 = signal_step;
      float y = x1 - x2 + r*y1;
      y2 = y1;
      y1 = y;
      
      return y;
    }
};

//Macros
Biquad makeNotchFilter(float frequency,float sps,float bandwidth) {
  return Biquad("notch",frequency,sps,Biquad::calcNotchQ(frequency,bandwidth),0);
}

Biquad makeBandpassFilter (float freqStart, float freqEnd, float sps, float resonance=0) {
  float r = resonance;
  if (r == 0) { r = pow(10,floor(log10(freqEnd))); }
  return Biquad("bandpass",
      Biquad::calcCenterFrequency(freqStart,freqEnd),
      sps,
      Biquad::calcBandpassQ(Biquad::calcCenterFrequency(freqStart,freqEnd),Biquad::calcBandwidth(freqStart,freqEnd),resonance),
      0);
}
