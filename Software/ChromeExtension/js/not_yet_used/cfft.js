/*
        basic complex number arithmetic from 
        http://rosettacode.org/wiki/Fast_Fourier_transform#Scala
        */
       function Complex(re, im) 
       {
         this.re = re;
         this.im = im || 0.0;
       }
       Complex.prototype.add = function(other, dst)
       {
         dst.re = this.re + other.re;
         dst.im = this.im + other.im;
         return dst;
       }
       Complex.prototype.sub = function(other, dst)
       {
         dst.re = this.re - other.re;
         dst.im = this.im - other.im;
         return dst;
       }
       Complex.prototype.mul = function(other, dst)
       {
         //cache re in case dst === this
         var r = this.re * other.re - this.im * other.im;
         dst.im = this.re * other.im + this.im * other.re;
         dst.re = r;
         return dst;
       }
       Complex.prototype.cexp = function(dst)
       {
         var er = Math.exp(this.re);
         dst.re = er * Math.cos(this.im);
         dst.im = er * Math.sin(this.im);
         return dst;
       }
       Complex.prototype.log = function()
       {
         /*
         although 'It's just a matter of separating out the real and imaginary parts of jw.' is not a helpful quote
         the actual formula I found here and the rest was just fiddling / testing and comparing with correct results.
         http://cboard.cprogramming.com/c-programming/89116-how-implement-complex-exponential-functions-c.html#post637921
         */
         if( !this.re )
           console.log(this.im.toString()+'j');
         else if( this.im < 0 )
           console.log(this.re.toString()+this.im.toString()+'j');
         else
           console.log(this.re.toString()+'+'+this.im.toString()+'j');
       }

       /*
       complex fast fourier transform and inverse from
       http://rosettacode.org/wiki/Fast_Fourier_transform#C.2B.2B
       */
       function icfft(amplitudes, size)
       {

         var N = size;
         var iN = 1 / N;
       
         //conjugate if imaginary part is not 0
         for(var i = 0 ; i < N; ++i)
           if(amplitudes[i] instanceof Complex)
             amplitudes[i].im = -amplitudes[i].im;
       
         //apply fourier transform
         temp = cfft(amplitudes, size)
         amplitudes = temp[0]
       
         for(var i = 0 ; i < amplitudes.length; ++i)
         {
           //conjugate again
           amplitudes[i].im = -amplitudes[i].im;
           //scale
           amplitudes[i].re *= iN;
           amplitudes[i].im *= iN;
         }
         return amplitudes;
       }
        
       
       function cfft(amplitudes, size)
       {
         var N = amplitudes.length;
         
         if(amplitudes.length % 2 != 0){
           amplitudes.pop(); //Make the initial array even to make the rest work.
           size = size-1;
         }
         if( N <= 1 )
           return [amplitudes,size];
       
         var hN = N / 2;
         var even = [];
         var odd = [];
         if((amplitudes.length != size) && (Math.floor(hN) != hN)) { return [amplitudes,size];} // If the sub-index is not an integer stop
         even.length = Math.floor(hN);
         odd.length = Math.floor(hN);
         for(var i = 0; i < hN; ++i)
         {
           even[i] = amplitudes[i*2];
           odd[i] = amplitudes[i*2+1];
         }
         var evenfft = cfft(even, size);
         var oddfft = cfft(odd, size);
         even = evenfft[0];
         odd = oddfft[0];
       
         var a = -2*Math.PI;
         for(var k = 0; k < hN; ++k)
         {
           if(!(even[k] instanceof Complex))
             even[k] = new Complex(even[k], 0);
           if(!(odd[k] instanceof Complex))
             odd[k] = new Complex(odd[k], 0);
           var p = k/N;
           var t = new Complex(0, a * p);
           t.cexp(t).mul(odd[k], t);
           amplitudes[k] = even[k].add(t, odd[k]);
           amplitudes[k + hN] = even[k].sub(t, even[k]);
         }
         return [amplitudes,size];
       }
       
       function scaleTransform(trans, size) {
         var i = 0,
             bSi = 1.0 / size,
             x = trans;
         while(i < x.length) {
           x[i]["re"] *= bSi;
           x[i]["im"] *= bSi; i++;
         }
         return x;
       }

       function frequencyDomain(data, fs){ // Returns Frequency Domain object: [frequency Distribution, amplitude Distribution] with range based on input sample rate. Assumes constant sample rate.
         var tdat = [...data]; // Red
         var len = tdat.length;
         var transformArr = cfft(tdat, len); //Returns Time Domain FFT
         var transform = icfft(transformArr[0], transformArr[0].length); //transformArr[0] //console.log(transform);
         var tranScaled = scaleTransform(transform, transform.length);
         
         var amplitudes = [];
         tranScaled.forEach(function(item, idx) { //Extract amplitude spectrum. Must be mapped based on frequency.
           amplitudes.push(Math.sqrt(Math.pow(item["re"],2)+Math.pow(item["im"],2))); //sqrt(real^2 + imag^2) = amplitude
         });

         var N = transform.length; // FFT size
         var df = fs/N; // frequency resolution
         
         var freqDist = [];
         for(var i=(-N/2); i<(N/2); i++) {
           var freq = i*df;
           freqDist.push(freq);
         }

         return [freqDist,amplitudes];

       }