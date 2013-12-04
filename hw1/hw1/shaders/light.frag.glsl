# version 120 

// Mine is an old machine.  For version 130 or higher, do 
// in vec4 color ;  
// in vec4 mynormal ; 
// in vec4 myvertex ;
// That is certainly more modern

varying vec4 color ;
varying vec3 mynormal ; 
varying vec4 myvertex ; 
varying vec3 eyepos;
uniform int istex ; //Texture command
uniform int islight ; // are we lighting. 

// Assume light 0 and light 1 are both point lights
// The actual light values are passed from the main OpenGL program. 
// This could of course be fancier.  My goal is to illustrate a simple idea. 

uniform vec4 light0posn ; 
uniform vec4 light0color ; 
uniform vec4 light1posn ; 
uniform vec4 light1color ; 


// Now, set the material parameters.  These could be varying and/or bound to 
// a buffer.  But for now, I'll just make them uniform.  
// I use ambient, diffuse, specular, shininess as in OpenGL.  
// But, the ambient is just additive and doesn't multiply the lights.  

uniform vec4 ambient ; 
uniform vec4 diffuse ; 
uniform vec4 specular ; 
uniform vec4 emission ;
uniform float shininess ;

uniform sampler2D tex;
uniform sampler2D CubeText;
uniform sampler2D Base; //Variables added for bump mapping
uniform sampler2D NormalMap;
uniform vec3 CAMERA_POSITION;
varying vec3 position;
varying vec3 lightvec;
uniform bool isBumpMapping;
uniform int istexOn;

//Changed Formula to that in spec
vec4 ComputeLight (const in vec3 direction, const in vec4 lightcolor, const in vec3 normal, const in vec3 halfvec, const in vec4 mydiffuse, const in vec4 myspecular, const in float shine) {

        float nDotL = dot(normal, direction)  ;         
        vec4 lambert = mydiffuse * max (nDotL, 0.0) ;  

        float nDotH = dot(normal, halfvec) ; 
        vec4 phong = myspecular * pow (max(nDotH, 0.0), shine) ; 

        vec4 retval = lightcolor * (lambert + phong) ; 
        return retval ;            
}


void main (void) 
{       
    if (islight == 0) gl_FragColor = color ;  
    else { 
        // They eye is always at (0,0,0) looking down -z axis 
        // Also compute current fragment position and direction to eye 

        //const vec3 eyepos = vec3(0,0,0) ; 
        vec4 _mypos = gl_ModelViewMatrix * myvertex ; 
        vec3 mypos = _mypos.xyz / _mypos.w ; // Dehomogenize current location 
        vec3 eyedirn = normalize(eyepos - mypos) ; 

        // Compute normal, needed for shading. 
        // Simpler is vec3 normal = normalize(gl_NormalMatrix * mynormal) ; 
        vec3 _normal = (gl_ModelViewMatrixInverseTranspose*vec4(mynormal,0.0)).xyz ; 
        vec3 normal = normalize(_normal) ; 

        // Light 0, point
        vec3 position0 = light0posn.xyz / light0posn.w ; 
        vec3 direction0 = normalize (position0 - mypos) ; // no attenuation 
        vec3 half0 = normalize (direction0 + eyedirn) ;  
        vec4 col0 = ComputeLight(direction0, light0color, normal, half0, diffuse, specular, shininess) ;

		//Light 1, directional
        //vec3 position1 = light1posn.xyz / light1posn.w ; 
        //vec3 direction1 = normalize (position1 - mypos) ; // no attenuation 
		vec3 direction1 = normalize (light1posn.xyz) ;
        vec3 half1 = normalize (direction1 + eyedirn) ;  
        vec4 col1 = ComputeLight(direction1, light1color, normal, half1, diffuse, specular, shininess) ;

        
		if (istex == 1){
			gl_FragColor = texture2D(tex, gl_TexCoord[0].st) * (col0 + col1 + ambient) ; 
			if(istexOn == 0) gl_FragColor = (col0 + col1 + ambient) ; 
		}else if(istex == 2){
			gl_FragColor = texture2D(CubeText, gl_TexCoord[0].st) * (col0 + col1 + ambient) ; 
			if(istexOn == 0) gl_FragColor = (col0 + col1 + ambient) ; 
		}
        else if(isBumpMapping){
			vec3 norm = texture2D(NormalMap, gl_TexCoord[0].st).rgb * 2.0 - 1.0;
			vec3 baseColor = texture2D(Base, gl_TexCoord[0].st).rgb;
			//float dist = length(lightvec);
			//vec3 lightVector = normalize(lightvec);

			//float nxDir = max(0.0, dot(norm, lightVector));
			//vec4 diffuse = diffuse * nxDir;
			//float specularPower = 0.0;
			//if(nxDir != 0.0)
			//{
			//vec3 cameraVector = normalize(eyepos - position.xyz);
			//vec3 halfVector = normalize(lightVector + cameraVector);
			//float nxHalf = max(0.0,dot(norm, halfVector));
			//specularPower = pow(nxHalf, shininess);
			//}
			//vec4 specular = specular * specularPower;

			vec4 _mypos = gl_ModelViewMatrix * myvertex ; 
			vec3 mypos = _mypos.xyz / _mypos.w ; // Dehomogenize current location 
			vec3 eyedirn = normalize(eyepos - mypos) ; 

			 // Light 0, point
			vec3 position0 = light0posn.xyz / light0posn.w ; 
			vec3 direction0 = normalize (position0 - mypos) ; // no attenuation 
			vec3 half0 = normalize (lightvec + eyedirn) ;  
			vec4 col0 = ComputeLight(lightvec, light0color, norm, half0, diffuse, specular, shininess) ;

			gl_FragColor = (ambient + diffuse) * texture2D(Base, gl_TexCoord[0].st); //vec4(baseColor.rgb,1.0);

			if(istexOn == 0) gl_FragColor = (col0 + col1 + ambient) ; 

		}else{ gl_FragColor = ambient + col0 + col1 ;
			} 
		}
}
