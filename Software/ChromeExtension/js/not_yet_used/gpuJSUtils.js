
class gpuUtils {
  constructor(){
      this.gpu = new GPU();
      this.kernel;
      this.PI = 3.141592653589793;
      this.SQRT1_2 = 0.7071067811865476

      this.addFunctions();
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
        const er = Math.exp(a_real);
        return [er * Math.cos(a_imag), er*Math.sin(a_imag)];
      });
          
      this.gpu.addFunction(function cScaleTransform(transform, iSize) {
        transform[this.thread.x][0] *= iSize;
        transform[this.thread.x][1] *= iSize;
        return transform;
      });

      this.gpu.addFunction(function mag(a,b){ // Returns magnitude
        return Math.sqrt(a*a + b*b);
      });

      this.gpu.addFunction(function conj(imag){ //Complex conjugate of x + iy is x - iy
        return 0-imag;
      });

      this.gpu.addFunction(function lof(n){ //Lowest odd factor
        const sqrt_n = Math.sqrt(n);
        var factor = 3;

        while(factor <= sqrt_n) {
          if (n % factor === 0) return factor;
          factor += 2;
        }
      });

      this.gpu.addFunction(function DFT(signal,len,freq){ //Extract a particular frequency
        var real = 0;
        var imag = 0;
        for(var i = 0; i<len; i++){
          var shared = 6.28318530718*freq*i/len; //this.thread.x is the target frequency
          real = real+signal[i]*Math.cos(shared);
          imag = imag-signal[i]*Math.sin(shared);
        }
        //var mag = Math.sqrt(real[k]*real[k]+imag[k]*imag[k]);
        return [real,imag]; //mag(real,imag)
      });

      this.gpu.addFunction(function DFTlist(signals,len,freq,n){ //Extract a particular frequency
        var real = 0;
        var imag = 0;
        for(var i = 0; i<len; i++){
          var shared = 6.28318530718*freq*i/len; //this.thread.x is the target frequency
          real = real+signals[i+len*n]*Math.cos(shared);
          imag = imag-signals[i+len*n]*Math.sin(shared);
        }
        //var mag = Math.sqrt(real[k]*real[k]+imag[k]*imag[k]);
        return [real,imag]; //mag(real,imag)
      });

      //Return frequency domain based on DFT
      this.dft = this.gpu.createKernel(function (signal,len){
        var result = DFT(signal,len,this.thread.x);
          return mag(result[0],result[1]);
      })
      .setDynamicOutput(true)
      .setDynamicArguments(true);
      //.setOutput([signal.length]) //Call before running the kernel
      //.setLoopMaxIterations(signal.length);

      // Takes a 2D array input [signal1[],signal2[],signal3[]]; does not work atm
      this.listdft2D = this.gpu.createKernel(function (signals){
        var len = this.output.x;
        var result = DFT(signals[this.thread.y],len,this.thread.x);
        //var mag = Math.sqrt(real[k]*real[k]+imag[k]*imag[k]);
        return mag(result[0],result[1]); //mag(real,imag)
      })
      .setDynamicOutput(true)
      .setDynamicArguments(true);

      //More like a vertex buffer list to chunk through lists of signals
      this.listdft1D = this.gpu.createKernel(function(signals,len){
        var result = [0,0];
        if(this.thread.x <= len){
          result = DFT(signals,len,this.thread.x);
        }
        else{
          var n = Math.floor(this.thread.x/len);
          result = DFTlist(signals,len,this.thread.x-n*len,n);
        }
        return mag(result[0],result[1]);
      })
      .setDynamicOutput(true)
      .setDynamicArguments(true)

      //TO DO
      //BitReverseIndex(index,n)
      //BitReverseComplexArray(array)

      /*
        function BitReverseIndex(index, n) {
          let bitreversed_index = 0;

          while (n > 1) {
            bitreversed_index <<= 1;
            bitreversed_index += index & 1;
            index >>= 1;
            n >>= 1;
          }
          return bitreversed_index;
        }

        function BitReverseComplexArray(array) {
          const n = array.length;
          const flips = new Set();

          for(let i = 0; i < n; i++) {
            const r_i = BitReverseIndex(i, n);

            if (flips.has(i)) continue;

            [array.real[i], array.real[r_i]] = [array.real[r_i], array.real[i]];
            [array.imag[i], array.imag[r_i]] = [array.imag[r_i], array.imag[i]];

            flips.add(r_i);
          }

          return array;
        }
      */
      /*
      this.getRecursive = this.gpu.createKernel(function (input,p){ //Not sure about this yet.
        var recursive_result = new Array(2);
        recursive_result[0] = input[0][this.thread.x*p + this.thread.y]; //this.thread.y does not work on a 1D output. Need to solve this
        recursive_result[1] = input[1][this.thread.x*p + this.thread.y];
        return recursive_result
      })
      .setDynamicOutput(true)
      .setDynamicArguments(true); //setDynamic output allows setOutput to be called for different sized arrays
      //.setOutput([100])
      //.setLoopMaxIterations(1000) //Set to input length if greater than 1000 (default)
      

      //UNFINISHED. This needs to be broken up properly in separate kernels then combined. Reference fft.js
      this.FFT_Recursive = this.gpu.createKernel(function(input, len, inverse){ //This needs to be done with a combined kernel so that getRecursive can be called
        */
        /* OG CODE
          const n = input.length;

          if (n === 1) {
            return input;
          }

          const output = new ComplexArray(n, input.ArrayType);

          // Use the lowest odd factor, so we are able to use FFT_2_Iterative in the
          // recursive transforms optimally.
          const p = LowestOddFactor(n);
          const m = n / p;
          const normalisation = 1 / Math.sqrt(p);
          let recursive_result = new ComplexArray(m, input.ArrayType);

        */
        /*
        if(len === 1){
          return input;
        }

        const p = lof(len);
        const m = en/p;
        const normalize = 1 / Math.sqrt(p);
        var recursive_result = [];
        */
        /* OG CODE
          // Loops go like O(n Σ p_i), where p_i are the prime factors of n.
          // for a power of a prime, p, this reduces to O(n p log_p n)
          for(let j = 0; j < p; j++) { //this.thread.y
            for(let i = 0; i < m; i++) { //this.thread.x
              recursive_result.real[i] = input.real[i * p + j];
              recursive_result.imag[i] = input.imag[i * p + j];
            }
            // Don't go deeper unless necessary to save allocs.
            if (m > 1) {
              recursive_result = fft(recursive_result, inverse);
            }

        */

        /*
        //Get Recursive Result
        //Need to put this into a combined kernel
        this.getRecursive.setOutput([len]);
        recursive_result = this.getRecursive(input,p);

        if(m > 1){
          recursive_result = this.fft(recursive_result,len,inverse); //This is a scoping issue, need to do this in the combined kernel and have it call itself to not pass things between CPU and GPU at all unless absolutely necessary (only for input and output ideally)
        }
        */
        /*
        
            const del_f_r = Math.cos(2*PI*j/n);
            const del_f_i = (inverse ? -1 : 1) * Math.sin(2*PI*j/n);
            let f_r = 1;
            let f_i = 0;

            for(let i = 0; i < n; i++) { //this.thread.y
              const _real = recursive_result.real[i % m];
              const _imag = recursive_result.imag[i % m];

              output.real[i] += f_r * _real - f_i * _imag;
              output.imag[i] += f_r * _imag + f_i * _real;

              [f_r, f_i] = [
                f_r * del_f_r - f_i * del_f_i,
                f_i = f_r * del_f_i + f_i * del_f_r,
              ];
            }
          }
        */

        /*
        //This section should be scoped into its own kernel as it is its own loop
        const del_f_r = Math.cos(2*PI*this.thread.y/len);
        var del_f_i = 0;
        if(inverse === true){
          del_f_i = -1*Math.sin(2*PI*this.thread.y/len);
        }
        else{
          del_f_i = Math.sin(2*PI*this.thread.y/len);
        }
        var f_r = 1;
        var f_i = 0;

        var output = new Array(2);
        
        var real = recursive_result[0][this.thread.x % m];
        var imag = recursive_result[1][this.thread.x % m];

        output[0] += f_r * output[0] - f_i * output[1];
        output[1] += f_r * output[1] + f_i * output[0];

        f_r = f_r * del_f_r - f_i * del_f_i;
        f_i = f_r * del_f_i + f_i * del_f_r;

        return output;

      }).setOutput([100]) // Output array of vec2's representing real and complex components. Call setOutput with input array size.
      .setLoopMaxIterations(1000) //Set to input length if greater than 1000 (default)
      .setDynamicOutput(true)
      .setDynamicArguments(true); //setDynamic output allows setOutput to be called for different sized arrays
      */
      //WIP
      /*
      this.FFT_Iterative = this.gpu.createKernel(function(input, len, inverse){
        var output = new Array(2);
      
          const n = len;

          const noutput = BitReverseComplexArray(input);
          const output_r = noutput[0];
          const output_i = noutput[1];


          /*
          // Loops go like O(n log n):
          //   width ~ log n; i,j ~ n
          let width = 1;
          while (width < n) {
            const del_f_r = Math.cos(PI/width);
            const del_f_i = (inverse ? -1 : 1) * Math.sin(PI/width);
            for (let i = 0; i < n/(2*width); i++) {
              let f_r = 1;
              let f_i = 0;
              for (let j = 0; j < width; j++) {
                const l_index = 2*i*width + j;
                const r_index = l_index + width;

                const left_r = output_r[l_index];
                const left_i = output_i[l_index];
                const right_r = f_r * output_r[r_index] - f_i * output_i[r_index];
                const right_i = f_i * output_r[r_index] + f_r * output_i[r_index];

                output_r[l_index] = SQRT1_2 * (left_r + right_r);
                output_i[l_index] = SQRT1_2 * (left_i + right_i);
                output_r[r_index] = SQRT1_2 * (left_r - right_r);
                output_i[r_index] = SQRT1_2 * (left_i - right_i);

                [f_r, f_i] = [
                  f_r * del_f_r - f_i * del_f_i,
                  f_r * del_f_i + f_i * del_f_r,
                ];
              }
            }
            width <<= 1;
          }

          return output;
        */
       /*
       return noutput;


      }).setOutput([100])
      .setLoopMaxIterations(1000) //Set to input length if greater than 1000 (default)
      .setDynamicOutput(true)
      .setDynamicArguments(true);
      */
      /*
      this.fft = this.gpu.combineKernels(this.getRecursive,this.FFT_Recursive,this.FFT_Iterative, function(input,len){
        if (len & (len - 1)) {
          return FFT_Recursive(input, inverse); // Faster
        } else {
          return FFT_2_Iterative(input, inverse); // Slower
        }
      }).setOutput([100])
      .setDynamicArguments(true)
      .setDynamicOutput(true)
      */
      //TODO, abstract this to handle 2D input, i.e. each row is one channel of data, so the whole input stream will be decoded one one kernel and results outputted altogether. Saves memory big time
  }
}





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
//Next apply multiple kernels in same operation (untested)
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

function testGPUCameraWobble() {
    var camera = true;
    var image = () => {
        if (camera) {
          const stream = navigator.mediaDevices
              ? navigator.mediaDevices.getUserMedia({video: true})
              : new Promise((y, n) => navigator.getUserMedia({video: true}, y, n));
          var html = document.createElement('div');
          html.innerHTML = `<video autoplay=true playsinline=true></video>`;
          const video =  document.body.appendChild(html);
          video.style.maxWidth = "100%";
          stream.then((stream) => {
            if ("srcObject" in video) video.srcObject = stream;
            else video.src = URL.createObjectURL(stream);
            invalidation.then(() => {
              stream.getTracks().forEach(t => t.stop());
              URL.revokeObjectURL(video.src);
            });    
            draw();
            return getimagedata(video);
          });
          //while (true) {
          //  requestAnimationFrame(getimagedata(video));
          //}
        } 
        else {
          return new Promise((resolve, reject) => {
            const image = new Image;
            image.onload = () => resolve(getimagedata(image));
            image.onerror = reject;
            image.crossOrigin = "anonymous";
            image.src = "https://raw.githubusercontent.com/tensorflow/tfjs-examples/master/mobilenet/cat.jpg";
          });
        }
      }
    
    function getimagedata(v) {
        const w = Math.min(400, v.width || Infinity), h = Math.min(300, v.height || Infinity);
        const context = DOM.context2d(w, h, 1);
        context.drawImage(v, 0, 0, w, h);
        return context.getImageData(0, 0, w, h);
      }

    var kernel = function(data, wobble) {
        var x = this.thread.x,
            y = this.thread.y;
      
        //var data = this.constants.data;
        // wouldn't be fun if the kernel did _nothing_
        x = Math.floor(x + wobble * Math.sin(y / 10));
        y = Math.floor(y + wobble * Math.cos(x / 10));
        
        var n = 4 * ( x + this.constants.w * (this.constants.h - y) );
        this.color(data[n]/256, data[n+1]/256,data[n+2]/256,1);
      }

    var render = new GPU({ mode: "gpu" })
            .createKernel(kernel)
            .setConstants({ w: image.width, h: image.height })
            .setOutput([image.width, image.height])
            .setGraphical(true);

    var draw = () => {
        //var fpsTime = performance.now(); var fps = 60;
        render(image.data, 14 * Math.sin(Date.now() / 400));
        render.getCanvas();
        //fps = (1 + fps) * (1 + 0.000984 * (fpsTime - (fpsTime = performance.now())));
        //console.log(fps);
        setTimeout(requestAnimationFrame(draw),15);
    }

    image();

}

var mandebrotFrag = 
`
uniform sampler1D tex;
uniform vec2 center;
uniform float scale;
uniform int iter;

void main() {
    vec2 z, c;

    c.x = 1.3333 * (gl_TexCoord[0].x - 0.5) * scale - center.x;
    c.y = (gl_TexCoord[0].y - 0.5) * scale - center.y;

    int i;
    z = c;
    for(i=0; i<iter; i++) {
        float x = (z.x * z.x - z.y * z.y) + c.x;
        float y = (z.y * z.x + z.x * z.y) + c.y;

        if((x * x + y * y) > 4.0) break;
        z.x = x;
        z.y = y;
    }

    gl_FragColor = texture1D(tex, (i == iter ? 0.0 : float(i)) / 100.0);
}
`;

var juliaSetFrag =
`
uniform sampler1D tex;
uniform vec2 c;
uniform int iter;

void main() {
    vec2 z;
    z.x = 3.0 * (gl_TexCoord[0].x - 0.5);
    z.y = 2.0 * (gl_TexCoord[0].y - 0.5);

    int i;
    for(i=0; i<iter; i++) {
        float x = (z.x * z.x - z.y * z.y) + c.x;
        float y = (z.y * z.x + z.x * z.y) + c.y;

        if((x * x + y * y) > 4.0) break;
        z.x = x;
        z.y = y;
    }

    gl_FragColor = texture1D(tex, (i == iter ? 0.0 : float(i)) / 100.0);
}
`;
