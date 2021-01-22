//2nd order IIR filters for arduino
  //https://github.com/mariusrubo/Unity-IIR-Realtime-Filtering/blob/master/FilterData.cs
    class IIRnotch {

        public:

        float frequency, sps;
        float a0, a1, a2, b0, b1, b2;
        float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
        float apply(float signal_step);
        IIRnotch(float freq, float sr, float bandwidth);
    };

    IIRnotch::IIRnotch(float freq, float sr, float bandwidth) {
        frequency = freq;
        sps = sr;

        float f0 = frequency/sps;
        float wc = TWO_PI/f0;
        float bw = bandwidth/sps;
        float r = 1-3*bw;
        float k = (1-2*r*cos(wc) + r*r)/(2-2*cos(wc));

        a0 = k; a1 = -2*cos(wc); a2 = k;
        b1 = 2*r*cos(wc); b2=-r*r;

    }

    float IIRnotch::apply(float signal_step) {
        float y = a0*signal_step + a1*x1 + a2*x2 + b1*y1 + b2*y2;

        x2 = x1;
        x1 = signal_step;
        y2 = y1;
        y1 = y;

        return y; //Output filtered time step
    }


    class IIRlowpass {

        public:

        float frequency, sps;
        float a0, a1, a2, b0, b1, b2;
        float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
        float apply(float signal_step);
        IIRlowpass(float freq, float sr);

    };

    IIRlowpass::IIRlowpass(float freq, float sr) {
        frequency = freq;
        sps = sr;

        float f0 = frequency/sps;
        float wc = tan(TWO_PI/f0);
        float k1 = 1.414213562*wc;
        float k2 = wc*wc;

        a0 = k2/(1+k1+k2);
        a1 = 2*a0;
        a2 = a0;

        float k3 = a1/k2;
        
        b1 = -2*a0+k3;
        b2 = 1-(2*a0)-k3;

    }

    float IIRlowpass::apply(float signal_step) {
      float y = a0*signal_step + a1*x1 + a2*x2 + b1*y1 + b2*y2;

      x2 = x1;
      x1 = signal_step;
      y2 = y1;
      y1 = y;

      return y; //Output filtered time step
    }

    class DCBlocker {
      public:

      float r = 0;
      float x1,x2,y1,y2 = 0;
      float apply(float signal_step);
      DCBlocker(float res);
    };

    DCBlocker::DCBlocker(float res){
      r = res;
    }

    float DCBlocker::apply(float signal_step) {
      x2=x1;
      x1 = signal_step;
      float y = x1 - x2 + r*y1;
      y2 = y1;
      y1 = y;
      
      return y;
    }



    /*
      //This one needs work
    export  class IIRHighPassFilter { // https://github.com/ruohoruotsi/Butterworth-Filter-Design/
        constructor (sps, frequency) {
          this.sps = sps; this.frequency = frequency; this.TWO_PI = 2*Math.PI; 
          let w0 = Math.PI*2*frequency/sps;
          let Q = 1/2;
          let α = Math.sin(w0)/(2*Q);

          let k0 = 1+α
          let k1 = -2*Math.cos(w0);
          let k2 = 1-α;
          let j0 = (1+Math.cos(w0))*.5;
          let j1 = -(1+Math.cos(w0))*.5;
          let j2 = (1+Math.cos(w0))*.5;

          this.b1 = (-k1)/k0;
          this.b2 = (-k2)/k0;
          this.a0 = j0/k0;
          this.a1 = j1/k0;
          this.a2 = j2/k0;
          console.log(this.a0, this.a1, this.b0, this.b1, this.b2)

          this.x1=this.x2=this.y1=this.y2=0;
        }
      }//More biquad filters https://github.com/ruohoruotsi/Butterworth-Filter-Design/blob/master/MATLAB/biquad_coeff_tests.m

    class IIRhighpass {

        public:

        float frequency, sps;
        float a0, a1, a2, b0, b1, b2;
        float x1 = 0, x2 = 0, y1 = 0, y2 = 0;

    }

    IIRhighpass::IIRhighpass(float freq, float sr) {
        frequency = freq;
        sps = sr;

        float TWO_PI = 3.1411592653*2;
        float f0 = frequency/sps;
        float wc = TWO_PI/f0;
        float bw = bandwidth/sps;
        float r = 1-3*bw;
        float k = (1-2*r*cos(wc) + r*r)/(2-2*cos(wc));

        a0 = k; a1 = -2*cos(wc); a2 = k;
        b1 = 2*r*cos(wc); b2=-r*r;

    }

    export class DCBlocker {
      constructor(r=0.995) {
        this.r = r;
        this.y1=this.y2=this.x1=this.x2=0;
      }

      step(newAmplitude) {
        this.x2=this.x1;
        this.x1 = newAmplitude
        let y = this.x1 - this.x2 + this.r*this.y1;
        
        this.y2 = this.y1;
        this.y1 = y;
        
        return y;
      }
    }

      //Apply the filter for each time step as they are cumulative
    export  const apply = (signal_step, filter) => {
        let y = a0*signal_step +
          a1*x1 +
          a2*x2 +
          b1*y1 +
          b2*y2;

        x2 = x1;
        x1 = signal_step;
        y2 = y1;
        y1 = y;

        return y; //Output filtered time step
      }

      */
