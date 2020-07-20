//Include gpu-browser.min.js before this script
var kernels = ({
    edgeDetection: [
      -1, -1, -1,
      -1,  8, -1,
      -1, -1, -1
    ],
    boxBlur: [
      1/9, 1/9, 1/9,
      1/9, 1/9, 1/9,
      1/9, 1/9, 1/9
    ],
    sobelLeft: [
        1,  0, -1,
        2,  0, -2,
        1,  0, -1
    ],
    sobelRight: [
        -1, 0, 1,
        -2, 0, 2,
        -1, 0, 1
    ],
    sobelTop: [
        1,  2,  1,
        0,  0,  0,
       -1, -2, -1  
    ],
    sobelBottom: [
        -1, 2, 1,
         0, 0, 0,
         1, 2, 1
    ],
    identity: [
        0, 0, 0, 
        0, 1, 0, 
        0, 0, 0
    ],
    gaussian3x3: [
        1,  2,  1, 
        2,  4,  2, 
        1,  2,  1
    ],
    guassian7x7: [
        0, 0,  0,   5,   0,   0,  0,
        0, 5,  18,  32,  18,  5,  0,
        0, 18, 64,  100, 64,  18, 0,
        5, 32, 100, 100, 100, 32, 5,
        0, 18, 64,  100, 64,  18, 0,
        0, 5,  18,  32,  18,  5,  0,
        0, 0,  0,   5,   0,   0,  0,
    ],
    emboss: [
        -2, -1,  0, 
        -1,  1,  1, 
         0,  1,  2
    ],
    sharpen: [
        0, -1,  0,
       -1,  5, -1,
        0, -1,  0
    ]
  });


function includeGPUJS() {
    var link1 = document.createElement("script");
    link1.src = "https://raw.githubusercontent.com/gpujs/gpu.js/master/dist/gpu-browser.min.js"; // Can set this to be a nonlocal link like from cloudflare or a special script with a custom app
    link1.async = false; // Load synchronously
    link1.charset = "UTF-8";
    document.head.appendChild(link1); //Append script
}

function testGPUmath() {
    const gpu = new GPU();
    const multiplyMatrix = gpu.createKernel(function(a, b) {
        let sum = 0;
        for (let i = 0; i < 20; i++) {
            sum += this.thread.y * this.thread.x;//a[this.thread.y][i] * b[i][this.thread.x];
        }
        return sum;
    }).setOutput([20, 20]);

    var a = [], b = [];
    for(var i = 0; i < 20; i++){
        a.push([Math.floor(Math.random()*10),Math.floor(Math.random()*10)]);
        b.push([Math.floor(Math.random()*10),Math.floor(Math.random()*10)]);
    }

    var result = null;
    console.time('testGPUmath');
    result = multiplyMatrix(a, b);
    console.timeEnd('testGPUmath');
    console.info(result);
}

function testGPUrender() {
    const canvas = document.getElementById('c');
    const gpu = new GPU({
        canvas: canvas,
        mode: 'gpu'
    });
    
    const render = gpu.createKernel(function(time) {
        this.color(this.thread.x/(Math.abs(.5+Math.sin(time)*Math.cos(time))*500),this.thread.y/(1+Math.abs((.5+Math.sin(time)*Math.cos(time)))*500), 0.4, 1);
    })
    .setOutput([500, 500])
    .setGraphical(true);
    
    var tick = 0;
    var animate = () => {
        render(tick);
        tick+=0.01;
        setTimeout(requestAnimationFrame(animate),15);
    }

    //gpu.addNativeFunction('4DMean',
    //    `vec4`
    //);
    var result = null;
    console.time('square render');
    requestAnimationFrame(animate);
    console.timeEnd('square render');
}

function testGPUKernels() {
    var gpu = new GPU();
    const megaKernel = gpu.createKernelMap({
        addResult: function add(a, b) {
          return a + b + this.thread.x;
        },
        multiplyResult: function multiply(a, b) {
          return a * b;
        },
      }, function(a, b, c) {
        return multiply(add(a[this.thread.x], b[this.thread.x]), c[this.thread.x]);
      }, { output: [10] });
      
      console.info(megaKernel([...Array(10).fill(0)], [...Array(10).fill(10)], [...Array(10).fill(2)]));
}

function cov1D(arr1,arr2){ //Return covariance vector of two 1D vectors of length n
    var gpu = new GPU();
    function mean(arr,l){
        var sum = arr.reduce((prev,curr)=> curr += prev);
        return sum / l;
    }

    var cov = gpu.createKernel(function(arr1,arr1mean,arr2,arr2mean){
        var cov = (arr1[this.thread.x] - arr1mean) * (arr2[this.thread.x] - arr2mean);
        return cov;
    }).setOutput([arr1.length]);

    var cov1DKernel = gpu.combineKernels(cov, function(in1,in2,mean1,mean2){
        return cov(in1,mean1,in2,mean2);
    });

    var mean1 = mean(arr1,arr1.length);
    var mean2 = mean(arr2,arr2.length);

    //console.log(arr1);
    //console.info(mean(arr1,arr1.length));
    var result = null;
    console.time('cov1D');
    result = cov1DKernel(arr1,arr2,mean1,mean2);
    console.timeEnd('cov1D');
    console.info(result);
}

function cov2D(mat1,mat2){ //Uhhhhh
    var gpu = new GPU();

    var matmean = gpu.createKernel(function(mat,len){ // Input 2d matrix with format [[],[]]

    }).setOutput([1,1]);

    var cov = gpu.createKernel(function(mat1,mat1mean,mat2,mat2mean){

    }).setOutput([mat1[0].length,mat1[1].length])
}

function sumP(arr) { //Parallel Sum Operation
    var gpu = new GPU();
    var sum = gpu.createKernelMap({
        //Break array up into blocks
        //Compute sum of each block
        //Pass partial sums to new blocks until sum is totaled.
        // e.g. input [1,2,3,4,5,6] => [1+2],[3+4],[5+6] = [3],[7],[11] => [3+7],[11] => [10],[11] => [10+11] => output [21]
            addResult: function add(a, b) {
              return a + b;
            },
            multiplyResult: function multiply(a, b) {
              return a * b;
            },
          }, function(a, b) {
            return add(a[this.thread.x], b[this.thread.x]);
          }, { output: [arr.length] });
    
    
    var result = null;
    console.time('sumP');
    result = sum(arr,arr);
    console.timeEnd('sumP');
    console.info(result);
    //console.info(sum());
}

function testGPUthreading3D(){
    var gpu = new GPU();
    var kernel = gpu.createKernel(function(){
        return [this.thread.x,this.thread.y,this.thread.z] //z increments first, then y, then x e.g. first outputs are [x0,y0,z0],[x0,y0,z1]...[x0,y0,zn] then [x0,y1,z0]... etc.
    }).setOutput([5,5,5]) // Outputs X*Y*Z Array(3)'s
    
    console.time('3D threading');
    var result = kernel();
    console.timeEnd('3D threading');
    console.info(result);
}

function testGPUthreading2D(){
    var gpu = new GPU();
    var kernel = gpu.createKernel(function(){
        return [this.thread.x,this.thread.y] //y increments first, then x. So [x0,y(1->n)] then [x1,y(1->n)] etc. outputted in that order
    }).setOutput([5,5]) // Outputs X*Y Array(3)'s
    console.time('2D threading');
    var result = kernel();
    console.timeEnd('2D threading');
    console.info(result);
}

function testVideoConv(video=null, kernel){
    const canvas = document.getElementById('v');
    var gpu = new GPU({
        canvas: canvas,
        mode: 'gpu'
    });

    if(video == null){
        var htmlToAppend = `<video src="https://upload.wikimedia.org/wikipedia/commons/e/ec/Jellyfish_in_Vr%C3%A5ngo.webm" id="vid" controls width="337" height="599" crossorigin="anonymous" loop autoplay muted></video>`;
        var div = document.createElement("div");
        div.innerHTML = htmlToAppend;
        document.body.appendChild(div);
        video = document.getElementById("vid");
        console.log(video.width,video.height);
    }
    var kernelRadius = (Math.sqrt(kernel.length) - 1) / 2;

    var convolution = gpu.createKernel(function (src, width, height, kernel, kernelRadius) {
        const kSize = 2 * kernelRadius + 1;
        let r = 0, g = 0, b = 0;
    
        let i = -kernelRadius;
        let imgOffset = 0, kernelOffset = 0;
        while (i <= kernelRadius) {
        if (this.thread.x + i < 0 || this.thread.x + i >= width) {
            i++;
            continue;
        }
    
        let j = -kernelRadius;
        while (j <= kernelRadius) {
            if (this.thread.y + j < 0 || this.thread.y + j >= height) {
            j++;
            continue;
            }
    
            kernelOffset = (j + kernelRadius) * kSize + i + kernelRadius;
            const weights = kernel[kernelOffset];
            const pixel = src[this.thread.y + i][this.thread.x + j];
            r += pixel.r * weights;
            g += pixel.g * weights;
            b += pixel.b * weights;
            j++;
        }
        i++;
        }
        this.color(r, g, b);
    })
        .setOutput([video.width, video.height])
        .setGraphical(true);

    video.addEventListener('loadeddata',()=>{
        var result = null;
        var render = () => {
            result = convolution(video, video.width, video.height, kernel, kernelRadius);
            requestAnimationFrame(render);
        }
        console.time('vid conv2D');
        render();
        console.timeEnd('vid conv2D');
        console.info(result);
    },false);
    

    return convolution.canvas;

}
//Next apply multiple kernels in same operation
function multiConv2D(img=null, kernels=[]){
    const multiConv2D = gpu.createKernelMap({
        conv2D: function (src, width, height, kernel, kernelRadius) {
            const kSize = 2 * kernelRadius + 1;
            let r = 0, g = 0, b = 0;
        
            let i = -kernelRadius;
            let imgOffset = 0, kernelOffset = 0;
            while (i <= kernelRadius) {
            if (this.thread.x + i < 0 || this.thread.x + i >= width) {
                i++;
                continue;
            }
        
            let j = -kernelRadius;
            while (j <= kernelRadius) {
                if (this.thread.y + j < 0 || this.thread.y + j >= height) {
                j++;
                continue;
                }
        
                kernelOffset = (j + kernelRadius) * kSize + i + kernelRadius;
                const weights = kernel[kernelOffset];
                const pixel = src[this.thread.y + i][this.thread.x + j];
                r += pixel.r * weights;
                g += pixel.g * weights;
                b += pixel.b * weights;
                j++;
            }
            i++;
            }
            this.color(r, g, b);
        }, function(img, width, height, kernels, kernelLengths, nKernels) {
            for(var i = 0; i < nKernels; i++){
                var kernelLength = kernelLengths[i];            
                var kernelRadius = (Math.sqrt(kernelLength) - 1) / 2;
                conv2D(img, width, height, kernels[i], kernelRadius);
            }
        }
    },{output:[img.width,img.height],graphical:true});
    
    //finally render the result. requestAnimationFrame for video or canvas
}

function testGPUpipeline() {
    var gpu = new GPU();
    const conv2D = gpu.createKernel(function(v, kernel) {
        return v[this.thread.x];
    })
      .setPipeline(true)
      .setOutput([img.width,img.height]);
    
    const kernel2 = gpu.createKernel(function(v, kernel) {
        return v[this.thread.x];
    })
      .setOutput([img.width,img.height]);

    var result = null;

    console.time('testGPUpipeline');
    kernels.forEach((kern)=>{
        conv2D(result, kern);
    });
    console.timeEnd('testGPUpipeline');
    console.info(result);
}

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
       function icfft(amplitudes, size) //for use on a cfft array
       {
         var N = amplitudes.length;
         var iN = 1 / N;
       
         //conjugate if imaginary part is not 0
         for(var i = 0 ; i < N; ++i)
           if(amplitudes[i] instanceof Complex)
             amplitudes[i].im = -amplitudes[i].im;
       
         //apply fourier transform
         temp = cfft(amplitudes, size)
         amplitudes = temp[0]
       
         for(var i = 0 ; i < N; ++i)
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
         
         if((amplitudes.length == size) && (amplitudes.length/2 != Math.floor(amplitudes.length/2))){
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
       };

       function frequencyDomain(data, fs){ // Returns Frequency Domain object: [frequency Distribution, amplitude Distribution] with range based on input sample rate. Assumes constant sample rate.
         var tdat = [...data]; // Red
         var len = tdat.length;
         //var transformArr = cfft(tdat, len); //Returns Time Domain FFT
         var transform = icfft(tdat, len); //console.log(transform);
         var tranScaled = scaleTransform(transform, transform.length);
         
         var amplitudes = [];
         tranScaled.forEach(function(item, idx) { //Extract amplitude spectrum. Must be mapped based on frequency.
           amplitudes.push(Math.sqrt(Math.pow(item["re"],2)+Math.pow(item["im"],2))); //sqrt(real^2 + imag^2) = amplitude
         });

         var N = transform.length; // FFT size
         var df = fs/N; // frequency resolution
         
         var freqDist = [];
         for(var i=-fs; i<fs; i+=(2*df)) {
           freqDist.push(i);
         }

         return [freqDist,amplitudes];

       }

       /*
       With a complex DFT Array X (from Matlab tutorial)
         Freq:
         fs=20; %~20sps in our case
         N=tdat.length; %FFT size
         X = 1/N*fftshift(fft(x,N));%N-point complex DFT

         df=fs/N; %frequency resolution
         sampleIndex = -N/2:N/2-1; %ordered index for FFT plot
         f=sampleIndex*df; %x-axis index converted to ordered frequencies
         stem(f,abs(X)); %magnitudes vs frequencies
         xlabel('f (Hz)'); ylabel('|X(k)|');

         Phase:
         X2=X;%store the FFT results in another array
         %detect noise (very small numbers (eps)) and ignore them
         threshold = max(abs(X))/10000; %tolerance threshold
         X2(abs(X)<threshold) = 0; %maskout values that are below the threshold
         phase=atan2(imag(X2),real(X2))*180/pi; %phase information
         plot(f,phase); %phase vs frequencies
       */

       /*
       function fftShift(fftarr){ // Two sided FFT ordering. Assumes even-length input array
         var temp = [];
         var len = fftarr.length
         temp.length = len;
         fftarr.forEach(function(item,idx) {
           if(idx < len/2) {
             temp[idx+len/2] = item;
           }
           else{
             temp[idx] = item;
           }
         });
       }*/


class gpuUtils {
    constructor(){
        this.gpu = new GPU();
        this.kernel;
    }

    addFunctions() { //Use kernel map instead? or this.kernel.addfunction? Test performance!
        this.gpu.addFunction(function add(a, b) {
            return a + b;
            });
        
        this.gpu.addFunction(function sub(a, b) {
            return a - b;
            });

        this.gpu.addFunction(function mul(a, b) {
            return a * b;
            });

        this.gpu.addFunction(function div(a, b) {
            return a / b;
            });

        this.gpu.addFunction(function cadd(a_real,a_imag,b_real,b_imag) {
            return [a_real+b_real,a_imag+b_imag];
            });

        this.gpu.addFunction(function csub(a_real,a_imag,b_real,b_imag) {
            return [a_real-b_real,a_imag-b_imag];
            });

        this.gpu.addFunction(function cmul(a_real,a_imag,b_real,b_imag) {
            return [a_real*b_real - a_imag*b_imag, a_real*b_imag + a_imag*b_real];
            });

        this.gpu.addFunction(function cexp(a_real,a_imag) {
            var er = Math.exp(a_real);
            return [er * Math.cos(a_imag), er*Math.sin(a_imag)];
            });
            
            this.gpu.addFunction(function cScaleTransform(transform, iSize) {
                transform[this.thread.x][0] *= iSize;
                transform[this.thread.x][1] *= iSize;
                return transform;
              });
    }
}
