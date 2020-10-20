//Enable web gl 2.0 compute in chrome://flags or edge://flags and set the ANGLE backend to OpenGL
class computeJS {
    constructor(canvasId, res=[500,500], src){
        // Create WebGL2ComputeRenderingContext
        this.src = src;
        this.renderProgram = null;
        this.canvas = document.getElementById(canvasId);
        this.res = res;
        this.canvas.width = res[0];
        this.canvas.height = res[1];
        this.context = this.canvas.getContext('webgl2-compute', { antialias: false });
        if (!this.context) {
            console.error("Cannot start webgl2 compute context. Check that WebGL2.0 compute is enabled and the ANGLE backend set to OpenGL");
            return false;
        }

        this.init();
    }

    async init(src){
        const computeShaderSource = await getShaderSource(this.src);
        const computeShader = this.context.createShader(this.context.COMPUTE_SHADER);
        this.context.shaderSource(computeShader,computeShaderSource);
        this.context.compileShader(computeShader);

        if (!this.context.getShaderParameter(computeShader, this.context.COMPILE_STATUS)) {
            console.error("Main compute shader build failed.");
            console.error(this.context.getShaderInfoLog(computeShader));
            this.context = null;
            return;
        }

        //=> Create the program.
        this.renderProgram = this.context.createProgram();
        this.context.attachShader(this.renderProgram, computeShader);
        this.context.linkProgram(this.renderProgram);

        if (!this.context.getProgramParameter(this.renderProgram, this.context.LINK_STATUS)) {
            console.error("Main compute shader program initialization failed.");
            console.error(this.context.getProgramInfoLog(this.renderProgram));
            this.context = null;
            return;
        }

        return this.context != null;
    }

    render = () => {
        this.context.useProgram(this.renderProgram);
        
        /* 
            //=> Bind the buffers to the rendering shader.
            this.bindBuffer(this.context, this.renderProgram, this.vertices_buffer_id, "Vertices");
            this.bindBuffer(this.context, this.renderProgram, this.triangles_buffer_id, "Triangles");
            this.bindBuffer(this.context, this.renderProgram, this.meshes_buffer_id, "Meshes");

            //=> Fill the rendering shader uniforms.
            this.context.uniform1f(this.uniform_locations.rng, this.RENDER_SEED);
            this.context.uniform1i(this.uniform_locations.spp, this.SPP);
            this.context.uniformMatrix4fv(this.uniform_locations.camera_inverse_projection, false, inverse_camera_perspective.elements);
            this.context.uniformMatrix4fv(this.uniform_locations.camera_to_world, false, camera_world_matrix.elements);

        */

        this.context.dispatchCompute(this.res[0] / 16,this.res[1] / 16, 1);

        this.context.memoryBarrier(this.context.TEXTURE_FETCH_BARRIER_BIT);
        var result;
        gl.getBufferSubData(gl.SHADER_STORAGE_BUFFER, 0, result)
    }

}