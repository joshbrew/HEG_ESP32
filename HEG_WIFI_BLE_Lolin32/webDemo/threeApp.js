//ThreeJS App(s)
var link1 = document.createElement("script");
link1.src = "https://cdnjs.cloudflare.com/ajax/libs/three.js/110/three.min.js"; // Can set this to be a nonlocal link like from cloudflare or a special script with a custom app
link1.async = false;
document.head.appendChild(link1); //Append script

var link2 = document.createElement("script");
link2.src = "https://cdn.jsdelivr.net/npm/postprocessing@6.10.0/build/postprocessing.min.js"; // Can set this to be a nonlocal link like from cloudflare or a special script with a custom app
link2.async = false;
document.head.appendChild(link2); //Append script



class ThreeGlobe {
    constructor() {
        var rendererHTML = '<div id="threeContainer" class="canvasContainer"></div>';
            HEGwebAPI.appendFragment(rendererHTML,'main_body');

        this.scene = new THREE.Scene();
        this.camera = new THREE.PerspectiveCamera( 75, (window.innerWidth - 20) / 435, 0.1, 1000 );
        this.renderer = new THREE.WebGLRenderer();
        this.renderer.setSize(window.innerWidth - 20, 435);
        document.getElementById("threeContainer").appendChild(this.renderer.domElement);

        var vertices = [];

        for ( var i = 0; i < 10000; i ++ ) {

            var x = THREE.Math.randFloatSpread( 1000 );
            var y = THREE.Math.randFloatSpread( 1000 );
            var z = THREE.Math.randFloatSpread( 1000 );

            vertices.push( x, y, z );
        }
        
        var geometry = new THREE.BufferGeometry();
        geometry.setAttribute( 'position', new THREE.Float32BufferAttribute( vertices, 3 ) );

        var pointmat = new THREE.PointsMaterial( { color: 0x888888 } );

        this.points = new THREE.Points( geometry, pointmat );
        this.scene.add( this.points );

        var sunmat = new THREE.MeshBasicMaterial( {
            wireframe: false,
        } );
        
        var sphere = new THREE.SphereBufferGeometry( 0.5, 20, 20 );
        this.sunMesh = new THREE.Mesh( sphere, sunmat );

        this.sunMesh.position.set(-5, 0, -10);

        this.scene.add( this.sunMesh );

        //var myUrl = 'https://i.imgur.com/1RXQSUL.jpg'

        //var textureLoader = new THREE.TextureLoader()
        //textureLoader.crossOrigin = "Anonymous"
        //var myTexture = textureLoader.load(myUrl);

        //material
        var globemat = new THREE.MeshPhysicalMaterial( {
            wireframe: false
        } );
        //globemat.map = myTexture;

        //sphere
        var sphere = new THREE.SphereBufferGeometry(2,50,50);
        this.sphereMesh = new THREE.Mesh( sphere, globemat );

        this.scene.add( this.sphereMesh );

        this.pointLight = new THREE.PointLight(0xFFFFFF);
        this.pointLight.position.set( 0, 0, -10 );

        this.pointLight.castShadow = true;
        this.pointLight.intensity = 3;

        this.pointLight.shadow.mapSize.width = 1024;
        this.pointLight.shadow.mapSize.height = 1024;

        this.pointLight.shadow.camera.fov = 90;

        //var sphere = new THREE.SphereBufferGeometry( 0.1, 20, 20 );
        //this.pointLight.add(new THREE.Mesh( sphere, new THREE.MeshBasicMaterial( { color: 0xff0040 } ) ));

        this.scene.add( this.pointLight );

        this.sphereMesh.rotation.z += 1;
        this.points.rotation.z += 1;
        this.camera.position.x = -2.3;
        this.camera.position.y = 0;
        this.camera.position.z = 0;

        this.camera.rotation.y = -0.35;
        this.camera.rotation.z = 0.3;

        this.begin = 0;
        this.ticks = 0;
        this.change = 0.001;
        this.threeAnim;
        this.threeWidth = window.innerWidth - 20;

        this.composer = new POSTPROCESSING.EffectComposer(this.renderer);
        this.renderPass = new POSTPROCESSING.RenderPass( this.scene, this.camera )
        
        this.composer.addPass( this.renderPass );

        var godRaysEffect = new POSTPROCESSING.GodRaysEffect(this.camera, this.sunMesh, {
			height: 720,
			density: 3,
			decay: 0.92,
			weight: 0.5,
			exposure: 0.6,
			samples: 60,
			clampMax: 1.0
		});

        godRaysEffect.dithering = true;
        this.effect = godRaysEffect;
        
        this.effectpass = new POSTPROCESSING.EffectPass(this.camera, this.effect);
        this.composer.addPass(this.effectpass)
        
        this.renderPass.renderToScreen = false;
        this.effectpass.renderToScreen = true;

        this.render();
    }

    destroyThreeApp = () => {
        cancelAnimationFrame(this.threeAnim);
        this.renderer.domElement.addEventListener('dblclick', null, false); //remove listener to render
        this.composer = null;
        this.scene = null;
        this.projector = null;
        this.camera = null;
        this.controls = null;

        this.sphereMesh = null;
        this.points = null;

        document.getElementById("threeContainer").remove();
    }

    onData(score) {
        this.change = score * 0.04;
    }

    render = () => {
        if(this.threeWidth != window.innerWidth - 20) {
            this.threeWidth = window.innerWidth - 20;
            this.renderer.setSize(this.threeWidth, 435);
            this.camera.aspect = this.threeWidth / 435;
            this.camera.updateProjectionMatrix();
            }

        this.ticks += this.change*1000;
        //this.sphereMesh.rotation.y += this.change;
        this.points.rotation.y += this.change;

        var theta = (this.ticks + 2800) * 0.001;
        this.pointLight.position.x = Math.sin(theta) * 20;
        //this.pointLight.position.y = Math.cos( time * 7 ) * 3;
        this.pointLight.position.z = Math.cos(theta) * 20;
        this.sunMesh.position.x = Math.sin(theta - 0.2) * 20;
        this.sunMesh.position.z = Math.cos(theta - 0.2) * 20;
        
        this.composer.render();
        this.threeAnim = requestAnimationFrame(this.render);

    }  
} 