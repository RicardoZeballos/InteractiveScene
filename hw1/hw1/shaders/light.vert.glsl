# version 120 


// Mine is an old machine.  For version 130 or higher, do 
// out vec4 color ;  
// out vec4 mynormal ; 
// out vec4 myvertex ;
// That is certainly more modern

varying vec4 color ; 
varying vec3 mynormal ; 
varying vec4 myvertex ; 
varying int istex ;
//varying int eyepos ;
uniform bool isBumpMapping;


//Following variables used for bump mapping
attribute vec3 tangent;
attribute vec3 binormal;
varying vec3 position;
varying vec3 lightvec; 

void main() {

	gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex ; 
    mynormal = gl_Normal ; 
    myvertex = gl_Vertex ; 
    color = gl_Color ;  
	gl_TexCoord[0] = gl_MultiTexCoord0;

	//Added for bump mapping
	if(isBumpMapping){
	position = gl_Vertex.xyz;
	mat3 TBNMatrix = mat3(tangent, binormal, gl_Normal);
	lightvec = gl_LightSource[0].position.xyz - position;
	lightvec *= TBNMatrix;
	}
}


