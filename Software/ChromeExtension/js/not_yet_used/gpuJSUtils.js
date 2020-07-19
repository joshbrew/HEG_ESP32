//Include gpu-browser.min.js before this script

function includeGPUJS() {
    var link1 = document.createElement("script");
    link1.src = "https://raw.githubusercontent.com/gpujs/gpu.js/master/dist/gpu-browser.min.js"; // Can set this to be a nonlocal link like from cloudflare or a special script with a custom app
    link1.async = false; // Load synchronously
    link1.charset = "UTF-8";
    document.head.appendChild(link1); //Append script
}

function testGPUmath() {
    const gpu = new GPU();
    var gpuKernel = gpu.createKernel(function() {
        return (this.thread.x + this.thread.y);
    }).setOutput([2,2]);

    console.info(gpuKernel());
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
    
    requestAnimationFrame(animate);
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
    console.info(cov1DKernel(arr1,arr2,mean1,mean2));
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
          }, function(a, b, c) {
            return multiply(add(a[this.thread.x], b[this.thread.x]), c[this.thread.x]);
          }, { output: [arr.length] });
    console.info(sum(arr,arr.length));
}


var kernels = ({
    edgeDetection: [
      -1, -1, -1,
      -1, 8, -1,
      -1, -1, -1
    ],
    boxBlur: [
      1/9, 1/9, 1/9,
      1/9, 1/9, 1/9,
      1/9, 1/9, 1/9
    ],
    sobelX: [
        1,  0,  -1,
        2,  0,  -2,
        1,  0,  -1
    ],
    sobelY: [
        1,  2,   1,
        0,  0,   0,
       -1, -2,  -1  
    ]
  });

function testGPUthreading3D(){
    var gpu = new GPU();
    var kernel = gpu.createKernel(function(){
        return [this.thread.x,this.thread.y,this.thread.z] //z increments first, then y, then x
    }).setOutput([5,5,5])

    console.info(kernel());
}

function testGPUthreading2D(){
    var gpu = new GPU();
    var kernel = gpu.createKernel(function(){
        return [this.thread.x,this.thread.y]
    }).setOutput([5,5])

    console.info(kernel());
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
        var render = () => {
            convolution(video, video.width, video.height, kernel, kernelRadius);
            requestAnimationFrame(render);
        }
        render();
    },false);
    

    return convolution.canvas;

}
