/*****************************************************************************/
/* This is the program skeleton for homework 2 in CS 184 by Ravi Ramamoorthi */
/* Extends HW 1 to deal with shading, more transforms and multiple objects   */
/*****************************************************************************/


#include "stdafx.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <GL/glew.h>
#include <GL/glut.h>
#include "shaders.h"
#include "Transform.h"
#include <string>
#include <stack>
#include <vector>
#include "FreeImage.h" 
#include "SOIL.h"
#include <cmath>

//Added following line for error checking in (Ricardo) setCommand
using namespace std ; 

int amount; // The amount of rotation for each arrow press
int yBounds; //Manage the keyboard rotation
int xBounds;
float rotyBounds; //Manage the mouse rotation
float rotxBounds;
GLint animate = 0 ; // ** NEW ** whether to animate or not
bool isBumpMapping;
bool TextureOn;

GLfloat ourFovy = 30.0; // Our Camera fovy

vec3 ourLookAt; //Since we declare our own camera we need to know were we are lookint at
vec3 strafeVec;
vec3 eye; // The (regularly updated) vector coordinates of the eye location 
vec3 up;  // The (regularly updated) vector coordinates of the up location 
const vec3 eyeinit(0, 2, 9) ; // Initial eye position, also for resets
const vec3 upinit(0, 3, 9) ; // Initial up position, also for resets
const vec3 ourLookAtInit(0, 2, 10);
const vec3 strafeVecInit(1, 2, 9);
bool useGlu; // Toggle use of "official" opengl/glm transform vs user code
int w, h; // width and height 
GLuint vertexshader, fragmentshader, shaderprogram ; // shaders
static enum {view, translate, scale, lightRotation } transop ; // which operation to transform by 
float sx, sy ; // the scale in x and y 
float tx, ty ; // the translation in x and y
int mouseoldx, mouseoldy ; // For mouse motion
bool myMouse;
bool LightsOn = true;
bool Wire;
// Constants to set up lighting on the teapot
const GLfloat light_position[] = {0, 4, -7, 1};    // Position of light 0
const GLfloat light_position1[] = {0, 4, 0, 0};  // Position of light 1
const GLfloat light_specular[] = {1, 0.5, 0, 1 };    // Specular of light 0
const GLfloat light_specular1[] = {0.5, 0.5, 1, 1 };   // Specular of light 1
const GLfloat one[] = {1, 1, 1, 1};                 // Specular on teapot
const GLfloat medium[] = {0.5, 0.5, 0.5, 1};        // Diffuse on teapot
const GLfloat small[] = {0.2, 0.2, 0.2, 1};         // Ambient on teapot
const GLfloat high[] = {100} ;                      // Shininess of teapot
GLfloat light0[4], light1[4] ; 

GLfloat	xrot;				// X Rotation ( NEW )
GLfloat	yrot;				// Y Rotation ( NEW )
GLfloat	zrot;				// Z Rotation ( NEW )

// Variables to set uniform params for lighting fragment shader 
GLuint islight ; 
GLuint light0posn ; 
GLuint light0color ; 
GLuint light1posn ; 
GLuint light1color ; 
GLuint ambient ; 
GLuint diffuse ; 
GLuint specular ; 
GLuint shininess ; 
GLuint emission;
GLuint istex;
GLuint isBM;
GLuint eyePos;
GLuint TexOn;
GLint texsampler ; 

//Trimans extra vars to handle window size, default 600x400
GLfloat windowSize[2]={600,400};

//static GLuint texture;
FIBITMAP* bitmap;
GLuint	texture[4];			// Storage For One Texture ( NEW )

// New helper transformation function to transform vector by modelview 
// May be better done using newer glm functionality.
void transformvec (const GLfloat input[4], GLfloat output[4]) {
  GLfloat modelview[16] ; // in column major order
  glGetFloatv(GL_MODELVIEW_MATRIX, modelview) ; 
  
  for (int i = 0 ; i < 4 ; i++) {
    output[i] = 0 ; 
    for (int j = 0 ; j < 4 ; j++) 
      output[i] += modelview[4*j+i] * input[j] ; 
  }
}

// Uses the Projection matrices (technically deprecated) to set perspective 
// We could also do this in a more modern fashion with glm.  
void reshape(int width, int height){
	w = width;
	h = height;
        mat4 mv ; // just like for lookat

	glMatrixMode(GL_PROJECTION);
        float fovy = 90.0, aspect = w / (float) h, zNear = 0.1, zFar = 45.0 ;
        // I am changing the projection stuff to be consistent with lookat
        //if (useGlu) mv = glm::perspective(ourFovy,aspect,zNear,zFar) ; 
		if (useGlu) mv = glm::perspective(fovy,aspect,zNear,zFar) ; 
        else {
	      //mv = Transform::perspective(ourFovy,aspect,zNear,zFar) ; 
          mv = Transform::perspective(fovy,aspect,zNear,zFar) ; 
          mv = glm::transpose(mv) ; // accounting for row major 
        }
        glLoadMatrixf(&mv[0][0]) ; 

	glViewport(0, 0, w, h);
}

void animation(void) {
	xrot+=0.6f;
	yrot+=0.4f;
	zrot+=0.8f;
  glutPostRedisplay() ;  
}

void printHelp() {
  std::cout << "\npress 'h' to print this message again.\n" 
       << "press '+' or '-' to change the amount of rotation that\noccurs with each arrow press.\n" 
       << "press 'g' to switch between using glm::lookAt and glm::Perspective or your own LookAt.\n"       
       << "press 'r' to reset the transformations.\n"
	   << "press capital 'L' to turn Lights on and off.\n"
       << "press 'v' 't' 'c' to do view [default], translate, scale.\n" 
       << "press capital 'P' to animate the objects.\n"
	   << "press capital 'W' to swicth to wireframe.\n"
	   << "First Person Controls.\n"
		<< "press 'a' to move Forward.\n"
		<< "press 'd' to move Right.\n"
		<< "press 'w' to move Back.\n"
		<< "press 's' to move Left.\n"
		<< "press 'z' to move Up.\n"
		<< "press 'x' to move Down.\n"
		<< "Use arrow keys to rotate camera.\n"
		<< "press ESC to quit.\n" ;
    
}

void keyboard(unsigned char key, int x, int y) {
	switch(key) {
	case '+':
		amount++;
		std::cout << "amount set to " << amount << "\n" ;
		break;
	case '-':
		amount--;
		std::cout << "amount set to " << amount << "\n" ; 
		break;
	case 'g':
		useGlu = !useGlu;
        reshape(w,h) ; 
		std::cout << "Using glm::LookAt and glm::Perspective set to: " << (useGlu ? " true " : " false ") << "\n" ; 
		break;
	case 'h':
		printHelp();
		break;
    case 27:  // Escape to quit
        exit(0) ;
        break ;
    case 'r': // reset eye and up vectors, scale and translate. 
		eye = eyeinit ; 
		up = upinit ; 
        sx = sy = 1.0 ; 
        tx = ty = 0.0 ;
		ourLookAt = ourLookAtInit;
		strafeVec = strafeVecInit;
		yBounds = 0;
		xBounds = 0;
		rotyBounds = 0;
		rotxBounds = 0;
		break ; 
	case 'M':
		myMouse = !myMouse;
		if(myMouse) cout << "Moving controls turned on." << endl;
		if(!myMouse) cout << "Moving controls turned off." << endl;
		break;
    case 'v': 
		transop = view ;
        std::cout << "Operation is set to View\n" ; 
        break ; 
    case 't':
        transop = translate ; 
        std::cout << "Operation is set to Translate\n" ; 
        break ; 
    case 'c':
        transop = scale ; 
        std::cout << "Operation is set to Scale\n" ; 
		break;
	case 'P': // ** NEW ** to pause/restart animation
		animate = !animate ;
		if (animate) glutIdleFunc(animation) ;
		else glutIdleFunc(NULL) ;
		break ;
	case 'L': // Swicth Lights on and off
		LightsOn = !LightsOn ;
		break ;
	case 'W': // Turns Wire rendering on and off
		Wire = !Wire;
		break ;
	case 'B': // Turns BumpMapping on and off
		isBumpMapping = !isBumpMapping;
		break ;
	case 'I': // Turns Textures on and off
		TextureOn = !TextureOn;
		break ;
	case 'w': //Moving Forward
		{
		//float mx = WalkAmount;
		mat4 mr(1.0) ; 
		vec3 movingD = ourLookAt - eye;
		movingD = -glm::normalize(movingD);
		vec3 movingDir(movingD[0], movingD[1], movingD[2]) ;
        mr = Transform::translate(movingDir[0],movingDir[1],movingDir[2]) ; 
		vec4 eye4(eye[0],eye[1],eye[2],1);
		vec4 ourLookAt4(ourLookAt[0],ourLookAt[1],ourLookAt[2],1);
		vec4 up4(up[0],up[1],up[2],1);
		vec4 strafeVec4(strafeVec[0],strafeVec[1],strafeVec[2],1);
		eye4 = eye4 * mr;
		ourLookAt4 = ourLookAt4 * mr;
		up4 = up4 * mr;
		strafeVec4 = strafeVec4 * mr;
		eye[0]=eye4[0]/eye4[3];
		eye[1]=eye4[1]/eye4[3];
		eye[2]=eye4[2]/eye4[3];
		ourLookAt[0] = ourLookAt4[0]/ourLookAt4[3];
		ourLookAt[1] = ourLookAt4[1]/ourLookAt4[3];
		ourLookAt[2] = ourLookAt4[2]/ourLookAt4[3];
		up[0] = up4[0]/up4[3];
		up[1] = up4[1]/up4[3];
		up[2] = up4[2]/up4[3];
		strafeVec[0] = strafeVec4[0]/strafeVec4[3];
		strafeVec[1] = strafeVec4[1]/strafeVec4[3];
		strafeVec[2] = strafeVec4[2]/strafeVec4[3];
		gluLookAt(eye[0],eye[1],eye[2],ourLookAt[0],ourLookAt[1],ourLookAt[2],up[0],up[1],up[2]);
		break;
	    }
    case 's': //Moving Back
		{
		mat4 mr(1.0) ; 
		vec3 movingD = ourLookAt - eye;
		movingD = glm::normalize(movingD);
		vec3 movingDir(movingD[0], movingD[1], movingD[2]) ;
        mr = Transform::translate(movingDir[0],movingDir[1],movingDir[2]) ; 
		vec4 eye4(eye[0],eye[1],eye[2],1);
		vec4 ourLookAt4(ourLookAt[0],ourLookAt[1],ourLookAt[2],1);
		vec4 up4(up[0],up[1],up[2],1);
		vec4 strafeVec4(strafeVec[0],strafeVec[1],strafeVec[2],1);
		eye4 = eye4 * mr;
		ourLookAt4 = ourLookAt4 * mr;
		up4 = up4 * mr;
		strafeVec4 = strafeVec4 * mr;
		eye[0]=eye4[0]/eye4[3];
		eye[1]=eye4[1]/eye4[3];
		eye[2]=eye4[2]/eye4[3];
		ourLookAt[0] = ourLookAt4[0]/ourLookAt4[3];
		ourLookAt[1] = ourLookAt4[1]/ourLookAt4[3];
		ourLookAt[2] = ourLookAt4[2]/ourLookAt4[3];
		up[0] = up4[0]/up4[3];
		up[1] = up4[1]/up4[3];
		up[2] = up4[2]/up4[3];
		strafeVec[0] = strafeVec4[0]/strafeVec4[3];
		strafeVec[1] = strafeVec4[1]/strafeVec4[3];
		strafeVec[2] = strafeVec4[2]/strafeVec4[3];
		break;
		}
		case 'd': // Moving Right
		{
		mat4 mr(1.0) ; 
		vec3 movingD = strafeVec - eye;
		movingD = glm::normalize(movingD);
		vec3 movingDir(movingD[0], movingD[1], movingD[2]) ;
        mr = Transform::translate(movingDir[0],movingDir[1],movingDir[2]) ; 
		vec4 eye4(eye[0],eye[1],eye[2],1);
		vec4 ourLookAt4(ourLookAt[0],ourLookAt[1],ourLookAt[2],1);
		vec4 up4(up[0],up[1],up[2],1);
		vec4 strafeVec4(strafeVec[0],strafeVec[1],strafeVec[2],1);
		eye4 = eye4 * mr;
		ourLookAt4 = ourLookAt4 * mr;
		strafeVec4 = strafeVec4 * mr;
		eye[0]=eye4[0]/eye4[3];
		eye[1]=eye4[1]/eye4[3];
		eye[2]=eye4[2]/eye4[3];
		ourLookAt[0] = ourLookAt4[0]/ourLookAt4[3];
		ourLookAt[1] = ourLookAt4[1]/ourLookAt4[3];
		ourLookAt[2] = ourLookAt4[2]/ourLookAt4[3];
		strafeVec[0] = strafeVec4[0]/strafeVec4[3];
		strafeVec[1] = strafeVec4[1]/strafeVec4[3];
		strafeVec[2] = strafeVec4[2]/strafeVec4[3];
		break;
			}
		case 'a': // Moving Left
			{
		mat4 mr(1.0) ; 
		vec3 movingD = strafeVec - eye;
		movingD = -glm::normalize(movingD);
		vec3 movingDir(movingD[0], movingD[1], movingD[2]) ;
        mr = Transform::translate(movingDir[0],movingDir[1],movingDir[2]) ; 
		vec4 eye4(eye[0],eye[1],eye[2],1);
		vec4 ourLookAt4(ourLookAt[0],ourLookAt[1],ourLookAt[2],1);
		vec4 up4(up[0],up[1],up[2],1);
		vec4 strafeVec4(strafeVec[0],strafeVec[1],strafeVec[2],1);
		eye4 = eye4 * mr;
		ourLookAt4 = ourLookAt4 * mr;
		strafeVec4 = strafeVec4 * mr;
		eye[0]=eye4[0]/eye4[3];
		eye[1]=eye4[1]/eye4[3];
		eye[2]=eye4[2]/eye4[3];
		ourLookAt[0] = ourLookAt4[0]/ourLookAt4[3];
		ourLookAt[1] = ourLookAt4[1]/ourLookAt4[3];
		ourLookAt[2] = ourLookAt4[2]/ourLookAt4[3];
		strafeVec[0] = strafeVec4[0]/strafeVec4[3];
		strafeVec[1] = strafeVec4[1]/strafeVec4[3];
		strafeVec[2] = strafeVec4[2]/strafeVec4[3];
			break;
			}
	case 'z': // Moving Down
		{
		mat4 mr(1.0) ; 
		vec3 movingD = up - eye;
		movingD = -glm::normalize(movingD);
		vec3 movingDir(movingD[0], movingD[1], movingD[2]) ;
        mr = Transform::translate(movingDir[0],movingDir[1],movingDir[2]) ; 
		vec4 eye4(eye[0],eye[1],eye[2],1);
		vec4 ourLookAt4(ourLookAt[0],ourLookAt[1],ourLookAt[2],1);
		vec4 up4(up[0],up[1],up[2],1);
		vec4 strafeVec4(strafeVec[0],strafeVec[1],strafeVec[2],1);
		eye4 = eye4 * mr;
		ourLookAt4 = ourLookAt4 * mr;
		up4 = up4 * mr;
		strafeVec4 = strafeVec4 * mr;
		eye[0]=eye4[0]/eye4[3];
		eye[1]=eye4[1]/eye4[3];
		eye[2]=eye4[2]/eye4[3];
		ourLookAt[0] = ourLookAt4[0]/ourLookAt4[3];
		ourLookAt[1] = ourLookAt4[1]/ourLookAt4[3];
		ourLookAt[2] = ourLookAt4[2]/ourLookAt4[3];
		up[0] = up4[0]/up4[3];
		up[1] = up4[1]/up4[3];
		up[2] = up4[2]/up4[3];
		strafeVec[0] = strafeVec4[0]/strafeVec4[3];
		strafeVec[1] = strafeVec4[1]/strafeVec4[3];
		strafeVec[2] = strafeVec4[2]/strafeVec4[3];
		break;
			}
	case 'x': // Moving Up
		{
		mat4 mr(1.0) ; 
		vec3 movingD = up - eye;
		movingD = glm::normalize(movingD);
		vec3 movingDir(movingD[0], movingD[1], movingD[2]) ;
        mr = Transform::translate(movingDir[0],movingDir[1],movingDir[2]) ; 
		vec4 eye4(eye[0],eye[1],eye[2],1);
		vec4 ourLookAt4(ourLookAt[0],ourLookAt[1],ourLookAt[2],1);
		vec4 up4(up[0],up[1],up[2],1);
		vec4 strafeVec4(strafeVec[0],strafeVec[1],strafeVec[2],1);
		eye4 = eye4 * mr;
		ourLookAt4 = ourLookAt4 * mr;
		up4 = up4 * mr;
		strafeVec4 = strafeVec4 * mr;
		eye[0]=eye4[0]/eye4[3];
		eye[1]=eye4[1]/eye4[3];
		eye[2]=eye4[2]/eye4[3];
		ourLookAt[0] = ourLookAt4[0]/ourLookAt4[3];
		ourLookAt[1] = ourLookAt4[1]/ourLookAt4[3];
		ourLookAt[2] = ourLookAt4[2]/ourLookAt4[3];
		up[0] = up4[0]/up4[3];
		up[1] = up4[1]/up4[3];
		up[2] = up4[2]/up4[3];
		strafeVec[0] = strafeVec4[0]/strafeVec4[3];
		strafeVec[1] = strafeVec4[1]/strafeVec4[3];
		strafeVec[2] = strafeVec4[2]/strafeVec4[3];
		break;
		}
	
	}
	gluLookAt(eye[0],eye[1],eye[2],ourLookAt[0],ourLookAt[1],ourLookAt[2],up[0],up[1],up[2]) ;
	glutPostRedisplay();
}

//  You will need to enter code for the arrow keys 
//  When an arrow key is pressed, it will call your transform functions

void specialKey(int key, int x, int y) {
	switch(key) {
	case 100: //Left
          if (transop == view){
			  Transform::left(amount, ourLookAt,  up, eye, strafeVec);
		  }
          else if (transop == scale) sx -= amount * 0.01 ; 
          else if (transop == translate) tx -= amount * 0.01 ; 
          break;
	case 101: //Up
          if (transop == view){ 
			yBounds += amount;
			if(yBounds > 18){;}
			else{Transform::up(amount, ourLookAt,  up, eye);}		
		  }
          else if (transop == scale) sy += amount * 0.01 ; 
          else if (transop == translate) ty += amount * 0.01 ; 
          break;
	case 102: //Right
          if (transop == view){
			  Transform::left(-amount, ourLookAt,  up, eye, strafeVec);
		  }
          else if (transop == scale) sx += amount * 0.01 ; 
          else if (transop == translate) tx += amount * 0.01 ; 
          break;
	case 103: //Down
          if (transop == view) { 
			yBounds -= amount;
			if(yBounds < -18){;}
			else{Transform::up(-amount, ourLookAt,  up, eye);}
		}
          else if (transop == scale) sy -= amount * 0.01 ; 
          else if (transop == translate) ty -= amount * 0.01 ; 
          break;
	}
	glutPostRedisplay();
	glFlush();
}

// Defines a Mouse callback to zoom in and out 
// This is done by modifying gluLookAt         
// The actual motion is in mousedrag           
// mouse simply sets state for mousedrag       
void mouse(int button, int state, int x, int y) 
{
  if (button == GLUT_LEFT_BUTTON) {
    if (state == GLUT_UP) {
      // Do Nothing ;
    }
    else if (state == GLUT_DOWN) {
		if(myMouse){
			y = y-200; //Tunrs our screen into a coordinate system with center as zero
			x = x-300;
			if (abs(x)<abs(y)){
				if(rotyBounds + y  > -900 && rotyBounds + y  < 350){
					Transform::up(y*.05, ourLookAt,  up, eye);
					rotyBounds += y;
				}
				else{;}
			}else if (abs(x) >abs(y)){
				if(rotxBounds + x  > -1200 && rotxBounds + x  < 1200){
					Transform::left(-x*.02, ourLookAt,  up, eye, strafeVec);
					rotxBounds += x;
				}
				else{;}
			}
		glutPostRedisplay() ;
		}
	}
  }
  else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) 
    { // Reset gluLookAt
      	eye = eyeinit ; 
		up = upinit ; 
        sx = sy = 1.0 ; 
        tx = ty = 0.0 ;
		ourLookAt = ourLookAtInit;
		strafeVec = strafeVecInit;
		yBounds = 0;
		xBounds = 0;
		rotyBounds = 0;
		rotxBounds = 0; 
      glMatrixMode(GL_MODELVIEW) ;
      glLoadIdentity() ;
      gluLookAt(eye[0],eye[1],eye[2],ourLookAt[0],ourLookAt[1],ourLookAt[2],up[0],up[1],up[2]) ;
      glutPostRedisplay() ;
    }
}


void mousedrag(int x, int y) {
	//At the moment not used
	if(myMouse){
		int yloc = y - mouseoldy  ; // We will use the y coord to zoom in/out
		int xloc = x - mouseoldx  ; 
		if(xloc < 0){ //right
			if (transop == view) ;//Transform::left((0.010*xloc), eye,  up);
			else if (transop == scale) sx += (0.010*xloc) * 0.01 ; 
			else if (transop == translate) tx += (0.010*xloc) * 0.01 ; 
		}else if (xloc > 0){ //Left
			if (transop == view) ;//Transform::left((0.010*xloc), eye,  up);
			else if (transop == scale) sx -= (0.010*xloc) * 0.01 ; 
			else if (transop == translate) tx -= (0.010*xloc) * 0.01 ; 
		}else if (yloc > 0){ //Up
			if (transop == view){
				rotyBounds += yloc;
				if(yBounds > 1.10){;}
				else{Transform::up(yloc*.01, ourLookAt,  up, eye);}
			}
			else if (transop == scale) sy += (0.010*yloc)* 0.01 ; 
			else if (transop == translate) ty += (0.010*yloc) * 0.01 ; 
		}else if (yloc < 0){ //Down
			if (transop == view){ 
				rotyBounds += yloc;
				if(yBounds < -1.10){;}
				else{Transform::up(yloc*.01, ourLookAt,  up, eye);}
			}
			else if (transop == scale) sy -= (0.010*yloc) * 0.01 ; 
			else if (transop == translate) ty -= (0.010*yloc) * 0.01 ; 
		} 
  /* Set the eye location */
  glMatrixMode(GL_MODELVIEW) ;
  glLoadIdentity() ;
 // gluLookAt(eye[0],eye[1],eye[2],0,0,0,0,1,1) ;
  
  glutPostRedisplay() ;
  }
  //Do nothing mouse not being used ;	  
}


void printGLfloatArray(const GLfloat OurArray[10]){
	for(int i=0;i<10;i++){
		cout << "OurArray[" << i << "] " <<OurArray[i] <<endl;
	}
}

void printMatrix (mat4  mat){
	cout<< " "<< endl;
	cout << mat[0][0]<<" "<<mat[0][1]<<" "<<mat[0][2] <<" "<<mat[0][3]<<endl;
	cout << mat[1][0]<<" "<<mat[1][1]<<" "<<mat[1][2] <<" "<<mat[1][3]<<endl;
	cout << mat[2][0]<<" "<<mat[2][1]<<" "<<mat[2][2] <<" "<<mat[2][3]<<endl;
	cout << mat[3][0]<<" "<<mat[3][1]<<" "<<mat[3][2] <<" "<<mat[3][3]<<endl;
}

bool parseLine(string line){
	string cmd;

	if (line.empty())
		return true;
	stringstream ss(stringstream::in | stringstream::out);
	ss.str(line);
	ss >> cmd;
	if(cmd[0] == '#'){
		return true;
	} else if (cmd.compare("v") == 0){
		float x, y, z;
		ss >>x >>y >>z;
	} else if (cmd.compare("f") == 0){
		unsigned short a, b, c;
        ss >> a >> b >> c;
	if(ss.fail())
		return false;
	return true;
	}
}

struct obj_struct {
	vector<float> tempVertices;
	vector<float> tempNormals;
	vector<int> tempVecNorms;
};

struct raw_struct {
	vector<float> tempVertices;
};

obj_struct parseObjFile(const char * filename) {
	ifstream inFile;
	inFile.open(filename);
	vector<float> tempVertices;
	vector<float> tempNormals;
	vector<int> tempVecNorms;
	string Line, Name;

	while(std::getline(inFile, Line)){
	  if(Line == "" || Line[0] == '#') // Skip everything and continue with the next line
		continue;

	  std::istringstream LineStream(Line);
	  LineStream >> Name;

	  if(Name == "v"){  // Vertex
		float vertex[3];
		sscanf(Line.c_str(), "%*s %f %f %f", &vertex[0], &vertex[1], &vertex[2]);
		tempVertices.push_back(vertex[0]);
		tempVertices.push_back(vertex[1]);
		tempVertices.push_back(vertex[2]);
		//cout << "v " << vertex[0] << " " << vertex[1] << " " <<vertex[2] <<endl;
	  }else if(Name == "vn"){
		float normal[3];
		sscanf(Line.c_str(), "%*s %f %f %f", &normal[0], &normal[1], &normal[2]);
		tempNormals.push_back(normal[0]);
		tempNormals.push_back(normal[1]);
		tempNormals.push_back(normal[2]);
		//cout << "vn " << normal[0] << " " << normal[1] << " " <<normal[2] <<endl;
	  } else if(Name == "f"){//still gotta figure this out
		  int preVecNorm[6];
		  sscanf(Line.c_str(), "%*s %d//%d %d//%d %d//%d", 
			  &preVecNorm[0],&preVecNorm[1],&preVecNorm[2],
			  &preVecNorm[3],&preVecNorm[4],&preVecNorm[5]); 
		  //cout<< "f " << preVecNorm[0]<<"/"<<preVecNorm[1]<<"/"<<preVecNorm[2]<<" " <<preVecNorm[3]<<"/"<<preVecNorm[4]<<"/"<<preVecNorm[5]<<" " << preVecNorm[6]<<"/"<<preVecNorm[7]<<"/"<<preVecNorm[8]<<endl; 
				tempVecNorms.push_back(preVecNorm[0]);
				tempVecNorms.push_back(preVecNorm[1]);
				tempVecNorms.push_back(preVecNorm[2]);
				tempVecNorms.push_back(preVecNorm[3]);
				tempVecNorms.push_back(preVecNorm[4]);
				tempVecNorms.push_back(preVecNorm[5]);

	  }
	}

	obj_struct result;
	result.tempVertices=tempVertices;
	result.tempNormals=tempNormals;
	result.tempVecNorms=tempVecNorms;

	return result;
}

raw_struct parseRawFile(const char * filename) {
	ifstream inFile;
	inFile.open(filename);
	vector<float> tempVertices;
	string Line;

	while(std::getline(inFile, Line)){
	  if(Line == "" || Line[0] == '#') // Skip everything and continue with the next line
		continue;

	  std::istringstream LineStream(Line);

	  // Vertex
		float vertex[9];
		sscanf(Line.c_str(), "%*s %f %f %f %f %f %f %f %f %f", &vertex[0], &vertex[1], &vertex[2], &vertex[3], &vertex[4], &vertex[5], &vertex[6], &vertex[7], &vertex[8]);
		tempVertices.push_back(vertex[0]);
		tempVertices.push_back(vertex[1]);
		tempVertices.push_back(vertex[2]);
		tempVertices.push_back(vertex[3]);
		tempVertices.push_back(vertex[4]);
		tempVertices.push_back(vertex[5]);
		tempVertices.push_back(vertex[6]);
		tempVertices.push_back(vertex[7]);
		tempVertices.push_back(vertex[8]);
		//cout << "v " << vertex[0] << " " << vertex[1] << " " <<vertex[2] <<endl;
	  
	}
	raw_struct result;
	result.tempVertices=tempVertices;

	return result;
}

void renderRawFile(const char * filename) {
	raw_struct raw=parseRawFile(filename);
	vector<float> v= raw.tempVertices;

	glBegin(GL_TRIANGLES);

	for(int i=0;i<v.size();i=i+9) {
		glVertex3f(v[i],v[i+1],v[i+2]);
		glVertex3f(v[i+3],v[i+4],v[i+5]);
		glVertex3f(v[i+6],v[i+7],v[i+8]);
	}
	glEnd();
}

void renderObjFile(const char * filename) {
	obj_struct obj = parseObjFile(filename);
	vector<float> v = obj.tempVertices;
	vector<float> vn = obj.tempNormals;
	vector<int> f = obj.tempVecNorms;

	glBegin(GL_TRIANGLES);

	for (int i=0; i< (f.size())/6; i++ ){
		int first_index=i*6;
		int normloc= (f[first_index+1]-1)*3; //first look into f to get index in vn, then look in vn
		int v1loc= (f[first_index]-1)*3 ; //first look into f to get index in v, then look in v
		int v2loc= (f[first_index+2]-1)*3 ;
		int v3loc= (f[first_index+4]-1)*3 ;
		
			glNormal3f(vn[normloc], vn[normloc+1], vn[normloc+2]);
			glVertex3f(v[v1loc], v[v1loc+1], v[v1loc+2]);
			glVertex3f(v[v2loc], v[v2loc+1], v[v2loc+2]);
			glVertex3f(v[v3loc], v[v3loc+1], v[v3loc+2]);
			//cout<<"glNormal3f("<<vn[normloc]<<", "<<vn[normloc+1]<<", "<<vn[normloc+2]<<");"<<endl;
			//cout<<"glVertex3f("<<v[v1loc]<<", "<<v[v1loc+1]<<", "<<v[v1loc+2]<<");"<<endl;
			//cout<<"glVertex3f("<<v[v2loc]<<", "<<v[v2loc+1]<<", "<<v[v2loc+2]<<");"<<endl;
			//cout<<"glVertex3f("<<v[v3loc]<<", "<<v[v3loc+1]<<", "<<v[v3loc+2]<<");"<<endl;
	}
	glEnd();
}

void parseObjFile(string filename){
	char line[1024];
	ifstream inFile(filename.c_str(), ifstream::in);
	if(!inFile){
		cout << "Could not open given obj file." << filename;
	}
	while(inFile.good()){
		inFile.getline(line,1023); //Read line into storage
		if(!parseLine(string(line)))
		continue;
	}
	inFile.close();
}

GLuint raw_texture_load(const char *filename, int width, int height)
 {
     GLuint texture;
     unsigned char *data;
     FILE *file;
 
     // open texture data
     file = fopen(filename, "rb");
     if (file == NULL) return 0;
 
	 cout << "opened: " << filename <<  endl;
     // allocate buffer
     data = (unsigned char*) malloc(width * height * 4);
 
     // read texture data
     fread(data, width * height * 4, 1, file);
     fclose(file);
 
     // allocate a texture name
     glGenTextures(1, &texture);
 
     // select our current texture
     glBindTexture(GL_TEXTURE_2D, texture);
 
     // select modulate to mix texture with color for shading
     glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
 
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_DECAL);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_DECAL);
 
     // when texture area is small, bilinear filter the closest mipmap
     glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
     // when texture area is large, bilinear filter the first mipmap
     glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 
     // texture should tile
     glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
     glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
 
     // build our texture mipmaps
     gluBuild2DMipmaps(GL_TEXTURE_2D, 4, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
 
     // free buffer
     free(data);
 
     return texture;
 }

// load a bitmap with freeimage
bool loadBitmap(string filename, FIBITMAP* &bitmap) {
  // get the file format
  FREE_IMAGE_FORMAT format = FreeImage_GetFileType(filename.c_str(), 0);
  if (format == FIF_UNKNOWN)
    format = FreeImage_GetFIFFromFilename(filename.c_str());
  if (format == FIF_UNKNOWN)
    return false;

  // load the image
  bitmap = FreeImage_Load(format, filename.c_str());
  if (!bitmap)
    return false;

  return true;
}

// load a texture into opengl with freeimage
bool loadTexture(string filename, GLuint &texture) {
  FIBITMAP *bitmap = NULL;
  if (!loadBitmap(filename, bitmap)) {
    return false;
  }

  // convert to 32 bit bit-depth
  FIBITMAP *bitmap32 = FreeImage_ConvertTo32Bits(bitmap);
  FreeImage_Unload(bitmap);
  if (!bitmap32) {
    return false;
  }
  bitmap = bitmap32;

  // get bits and dimensions
  BYTE *bits = FreeImage_GetBits(bitmap);
  int w = FreeImage_GetWidth(bitmap);
  int h = FreeImage_GetHeight(bitmap);

  // get bit order
  int order = GL_BGRA;

  // upload texture to opengl
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  gluBuild2DMipmaps(GL_TEXTURE_2D, 4, w, h, order, GL_UNSIGNED_BYTE, (GLvoid*)bits);

  // forget our copy of the bitmap now that it's stored the card
  FreeImage_Unload(bitmap);

  return true;
}

int LoadGLTextures()									// Load Bitmaps And Convert To Textures
{
	/* load an image file directly as a new OpenGL texture */
	texture[0] = SOIL_load_OGL_texture
		(
		"WoodTxt.bmp",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);

	if(texture[0] == 0)
		return false;

	// Typical Texture Generation Using Data From The Bitmap
	//glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

		texture[1] = SOIL_load_OGL_texture
		(
		"NeHe.bmp",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	//Load Bump Mapping Textures
	texture[2] = SOIL_load_OGL_texture
		(
		"Brick.bmp",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);

	texture[3] = SOIL_load_OGL_texture
		(
		"BrickNormalMap.bmp",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);
	if(texture[3] == 0) cout<<"NormalMapTextureNot loaded"<<endl;

	return true;										// Return Success
}

void init() {
	
    eye = eyeinit ; 
	up = upinit ; 
	ourLookAt = ourLookAtInit;
	strafeVec = strafeVecInit;
	amount = 3;
    sx = sy = 1.0 ; 
    tx = ty = 0.0 ;
	useGlu = false;
	istex = 0;
	Wire = false;
	isBumpMapping = false;
	TextureOn = true;
	
	//glGenTextures(4, &texture[0]);  // Create The Texture
		if (!LoadGLTextures())								// Jump To Texture Loading Routine ( NEW )
	{
		cout << "Texture Didnt Load" << endl;									// If Texture Didn't Load Return FALSE
	}

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);

  // The lighting is enabled using the same framework as in mytest 3 
  // Except that we use two point lights
  // For now, lights and materials are set in display.  Will move to init 
  // later, per update lights

	  vertexshader = initshaders(GL_VERTEX_SHADER, "shaders/light.vert.glsl") ;
	  fragmentshader = initshaders(GL_FRAGMENT_SHADER, "shaders/light.frag.glsl") ;
	  shaderprogram = initprogram(vertexshader, fragmentshader) ; 
      islight = glGetUniformLocation(shaderprogram,"islight") ;        
      light0posn = glGetUniformLocation(shaderprogram,"light0posn") ;       
      light0color = glGetUniformLocation(shaderprogram,"light0color") ;       
      light1posn = glGetUniformLocation(shaderprogram,"light1posn") ;       
      light1color = glGetUniformLocation(shaderprogram,"light1color") ;       
	  ambient = glGetUniformLocation(shaderprogram,"ambient") ;       
      diffuse = glGetUniformLocation(shaderprogram,"diffuse") ;       
      specular = glGetUniformLocation(shaderprogram,"specular") ;       
      shininess = glGetUniformLocation(shaderprogram,"shininess") ; 

	  eyePos = glGetUniformLocation(shaderprogram,"eyepos") ;  //Eye position for fragshader

	 emission = glGetUniformLocation(shaderprogram,"emission") ; 
	 isBM = glGetUniformLocation(shaderprogram,"isBumpMapping") ;
	 istex = glGetUniformLocation(shaderprogram,"istex") ;

	 TexOn = glGetUniformLocation(shaderprogram,"istexOn") ;
	 glUniform1i(TexOn,1);
	 glUniform1i(isBM,false);

	// GLint texsampler ; 
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture[0]);
	 	texsampler = glGetUniformLocation(shaderprogram, "tex") ; 
		glUniform1i(texsampler,0); // Could also be GL_TEXTURE0 
	    glActiveTexture(GL_TEXTURE1);
	    glBindTexture(GL_TEXTURE_2D, texture[1]);
		texsampler = glGetUniformLocation(shaderprogram, "CubeText") ; 
		glUniform1i(texsampler,1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, texture[2]);
		texsampler = glGetUniformLocation(shaderprogram, "Base") ; 
		glUniform1i(texsampler,2);
		glActiveTexture(GL_TEXTURE3);
	    glBindTexture(GL_TEXTURE_2D, texture[3]);
		texsampler = glGetUniformLocation(shaderprogram, "NormalMap") ; 
		glUniform1i(texsampler,3);

}

void display() {
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Making our own viewModel 
	glMatrixMode(GL_MODELVIEW);
	mat4 mv ; 
	//Using our camera parameters to set up our Modelview matrix
    if (useGlu) mv = glm::lookAt(eye,ourLookAt,up) ; 
	else {
          mv = Transform::lookAt(eye,ourLookAt,up) ; 
          mv = glm::transpose(mv) ; // accounting for row major
        }
    glLoadMatrixf(&mv[0][0]) ;

	if(!TextureOn){
		glUniform1i(TexOn,0);
	}else if(TextureOn){
		glUniform1i(TexOn,1);
	}
	//Turns on lights
	glUniform1i(islight,LightsOn);
        // Set Light and Material properties for the teapot
        // Lights are transformed by current modelview matrix. 
        // The shader can't do this globally. 
        // So we need to do so manually.  
		transformvec(light_position, light0) ; 
        transformvec(light_position1, light1) ; 
 
		glUniform4fv(light0posn, 1, light0) ; 
		glUniform4fv(light0color, 1, light_specular) ; 
        glUniform4fv(light1posn, 1, light1) ; 
		glUniform4fv(light1color, 1, light_specular1) ; 
  
		glUniform4fv(ambient,1,small) ; 
        glUniform4fv(diffuse,1,small) ; 
        glUniform4fv(specular,1,one) ; 
        glUniform1fv(shininess,1,high) ; 

		GLfloat eyeV[] = {eye[0],eye[1],eye[2]};
		glUniform3fv(eyePos,1,eyeV);

		glUniform1i(istex,0) ;
		//glUniform1fv(emission,1, small);

        // Transformations for Teapot, involving translation and scaling 
        mat4 sc(1.0) , tr(1.0) ; 
        sc = Transform::scale(sx,sy,1.0) ; 
        tr = Transform::translate(tx,ty,0.0) ; 
        // Multiply the matrices, accounting for OpenGL and GLM.
        sc = glm::transpose(sc) ; tr = glm::transpose(tr) ; 
        mat4 transf  = mv * tr * sc ; // scale, then translate, then lookat.
        //glLoadMatrixf(&transf[0][0]) ; 


	glPushMatrix() ; 
		glTranslatef(0,0.8,0);
		glColor3f(0.1, 0.45, 0.54);
		glUniform4f(ambient,0.0, 0.045, 0.054, 1) ; 
		glUniform4f(diffuse,0.3, 0.5, 0.8, 1) ; 
        glUniform4f(specular,1, 1, 1, 1) ; 
		glUniform1f(shininess,100) ;
		glutSolidTeapot(.4);
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glUniform1i(istex,1) ;
	glPushMatrix() ; 
		glTranslatef(0,-1,0);
		glUniform4f(ambient,0.3, 0.3, 0.3, 1) ; 
        glUniform4f(diffuse,0.8, 0.8, 0.8, 1) ; 
        glUniform4f(specular,1, 1, 1, 1) ; 
		glUniform1f(shininess,100) ; 
	glBegin(GL_POLYGON); // Makes floor
		glColor3f(1,1,1);
		glNormal3f(0.0, 1.0, 0.0);
		glTexCoord2d(0, 0); glVertex3f(10,0,18);
		glTexCoord2d(1, 0); glVertex3f(10,0,-18);
		glTexCoord2d(1, 1); glVertex3f(-10,0,-18);
		glTexCoord2d(0, 1); glVertex3f(-10,0,18);
	glEnd();
	glPopMatrix();
	glUniform1i(istex,0) ;

	glPushMatrix();
		glColor3f(1,1,1);
		glUniform4f(ambient,0.3, 0.3, 0.3, 1) ; 
        glUniform4f(diffuse,0.8, 0.8, 0.8, 1) ; 
        glUniform4f(specular,1, 1, 1, 1) ; 
		glUniform1f(shininess,100) ; 
		glTranslatef(8,-1,0);
		glRotatef(-90, 0,1,0);
		//glScalef(0.005,0.005,0.005);
		renderObjFile("Desk2.obj");
	glPopMatrix();

	glPushMatrix();
		glColor3f(1,1,1);
		glUniform4f(ambient,0.3, 0.3, 0.3, 1) ; 
        glUniform4f(diffuse,0.8, 0.8, 0.8, 1) ; 
        glUniform4f(specular,1, 1, 1, 1) ; 
		glUniform1f(shininess,100) ; 
		glTranslatef(-8,-1,0);
		glRotatef(90, 0,1,0);
		renderRawFile("DeskDraws.RAW");
	glPopMatrix();

	glPushMatrix() ; 
		glTranslatef(0,5,0);
		glRotatef(180,0,0,-1);
		glUniform4f(ambient,0.3, 0.3, 0.3, 1) ; 
        glUniform4f(diffuse,0.8, 0.8, 0.8, 1) ; 
        glUniform4f(specular,1, 1, 1, 1) ; 
		glUniform1f(shininess,100) ; 
	glBegin(GL_POLYGON); // Makes Roof
		glColor3f(1,1,1);
		glNormal3f(0.0, 1.0, 0.0);
		glVertex3f(10,0,18);
		glVertex3f(10,0,-18);
		glVertex3f(-10,0,-18);
		glVertex3f(-10,0,18);
	glEnd();
	glPopMatrix();

	glUniform1i(isBM,true);
	//glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, texture[3]);
	glPushMatrix() ; 
		glTranslatef(10,0,0);
		glRotatef(-90,0,0,-1);
		glUniform4f(ambient,0.3, 0.3, 0.3, 1) ; 
        glUniform4f(diffuse,0.8, 0.8, 0.8, 1) ; 
        glUniform4f(specular,1, 1, 1, 1) ; 
		glUniform1f(shininess,100) ; 
		glEnable(GL_TEXTURE_2D);
		glBegin(GL_QUADS); //Makes Right wall
		glColor3f(1,1,1);
		glNormal3f(0.0, 1.0, 0.0);
		glTexCoord2d(0, 0); glVertex3f(5,0,18);
		glTexCoord2d(1, 0); glVertex3f(5,0,-18);
		glTexCoord2d(1, 1); glVertex3f(-5,0,-18);
		glTexCoord2d(0, 1); glVertex3f(-5,0,18);
	glEnd();
	glPopMatrix();
	//glUniform1i(isBM,false);

	glPushMatrix() ; 
		glTranslatef(-10,0,0);
		glRotatef(90,0,0,-1);
		glUniform4f(ambient,0.3, 0.3, 0.3, 1) ; 
        glUniform4f(diffuse,0.8, 0.8, 0.8, 1) ; 
        glUniform4f(specular,1, 1, 1, 1) ; 
		glUniform1f(shininess,100) ; 
		glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS); //Makes left wall
		glColor3f(0,1,1);
		glNormal3f(0.0, 1.0, 0.0);
		glTexCoord2d(0, 0); glVertex3f(5,0,18);
		glTexCoord2d(1, 0); glVertex3f(5,0,-18);
		glTexCoord2d(1, 1); glVertex3f(-5,0,-18);
		glTexCoord2d(0, 1); glVertex3f(-5,0,18);
	glEnd();
	glPopMatrix();
	glUniform1i(isBM,false);

	glPushMatrix() ; 
		glTranslatef(0,0,-10);
		glRotatef(90,1,0,0);
		glUniform4f(ambient,0.3, 0.3, 0.3, 1) ; 
        glUniform4f(diffuse,0.8, 0.8, 0.8, 1) ; 
        glUniform4f(specular,1, 1, 1, 1) ; 
		glUniform1f(shininess,100) ; 
	glBegin(GL_QUADS); //Makes Back wall
		glColor3f(1,1,1);
		glNormal3f(0.0, 1.0, 0.0);
		glVertex3f(7,0,5);
		glVertex3f(7,0,-5);
		glVertex3f(-7,0,-5);
		glVertex3f(-7,0,5);
	glEnd();
	glPopMatrix();

	glPushMatrix() ; 
		glTranslatef(0,0,-18);
		glRotatef(90,1,0,0);
		glUniform4f(ambient,0.3, 0.3, 0.3, 1) ; 
        glUniform4f(diffuse,0.8, 0.8, 0.8, 1) ; 
        glUniform4f(specular,1, 1, 1, 1) ; 
		glUniform1f(shininess,100) ; 
	glBegin(GL_QUADS); //Makes the second Back wall
		glColor3f(1,1,1);
		glNormal3f(0.0, 1.0, 0.0);
		glVertex3f(12,0,5);
		glVertex3f(12,0,-5);
		glVertex3f(-12,0,-5);
		glVertex3f(-12,0,5);
	glEnd();
	glPopMatrix();

	glPushMatrix() ; 
		glTranslatef(0,0,18);
		glRotatef(90,1,0,0);
		glUniform4f(ambient,0.3, 0.3, 0.3, 1) ; 
        glUniform4f(diffuse,0.8, 0.8, 0.8, 1) ; 
        glUniform4f(specular,1, 1, 1, 1) ; 
		glUniform1f(shininess,100) ; 
	glBegin(GL_QUADS); //Makes the Back wall that is behind the camera
 		glColor3f(1,1,1);
		glNormal3f(0.0, -1.0, 0.0);
		glVertex3f(12,0,5);
		glVertex3f(12,0,-5);
		glVertex3f(-12,0,-5);
		glVertex3f(-12,0,5);
	glEnd();
	glPopMatrix();

	glPushMatrix() ; //Right Cube
		glColor3f(1,1,1);
		glTranslatef(5,0,-9); 
		glUniform4f(ambient,0.4, 0.08, 0.08, 1) ; 
        glUniform4f(diffuse,0.6, 0.6, 0.6, 1) ; 
        glUniform4f(specular,1, 1, 1, 1) ; 
		glUniform1f(shininess,100) ; 
		glutSolidCube(2);
    glPopMatrix() ;

	glPushMatrix() ; //Left Sphere
		glColor3f(1,1,1);
		glTranslatef(-5,3,-9);
		glUniform4f(ambient,0.03, 0.06, 0.5, 1) ; 
        glUniform4f(diffuse,0.7, 0.7, 0.7, 1) ; 
        glUniform4f(specular, 1, 1, 1, 1) ; 
		glUniform1f(shininess,100) ; 
		glutSolidSphere(2,20,20);
    glPopMatrix() ;

	glPushMatrix() ; // Left Cube
		glColor3f(1,1,1);
		glTranslatef(-5,0,-9);
		glUniform4f(ambient,0.4, 0.08, 0.08, 1) ; 
        glUniform4f(diffuse,0.6, 0.6, 0.6, 1) ; 
        glUniform4f(specular,.9, .9, .9, .9) ; 
		glUniform1f(shininess,100) ; 
		glutSolidCube(2);
    glPopMatrix() ;

	glPushMatrix() ; // Right Sphere
		glColor3f(1,1,1);
		glTranslatef(5,3,-9);
		glUniform4f(ambient,0.03, 0.06, 0.5, 1) ; 
        glUniform4f(diffuse,0.7, 0.7, 0.7, 1) ; 
        glUniform4f(specular,.2, .2, .2, .2) ; 
		glUniform1f(shininess,10) ; 
		glutSolidSphere(2,20,20);
    glPopMatrix() ;

	glPushMatrix() ; //Right Cone
		glColor3f(0,0,0);
		glTranslatef(4,2.2,3);
		glRotatef(-270,-1,0,0);
		glUniform4f(ambient,0.4, 0.1, 0.1, 1) ; 
        glUniform4f(diffuse,0.7, 0.7, 0.8, 1) ; 
        glUniform4f(specular,1, 1, 1, 1) ; 
		glUniform1f(shininess,200) ; 
		glutSolidCone(1,3,20,20);
    glPopMatrix() ;

	glPushMatrix() ; //Left Cone
		glColor3f(0,0,0);
		glTranslatef(-4,2.2,3);
		glRotatef(-270,-1,0,0);
		glUniform4f(ambient,0.4, 0.1, 0.1, 1) ; 
        glUniform4f(diffuse,0.7, 0.7, 0.8, 1) ; 
        glUniform4f(specular,1, 1, 1, 1) ; 
		glUniform1f(shininess,200) ; 
		if(!Wire)glutSolidCone(1,3,20,20);
		else if (Wire)glutWireCone(1,3,20,20);
    glPopMatrix() ;

	glPushMatrix() ; //Left Octa
		glColor3f(0,0,1);
		glTranslatef(-4,0,3);
		glRotatef(yrot,0.0f,1.0f,0.0f);
		glUniform4f(ambient,0.5, 0.5, 0.5, 1) ; 
        glUniform4f(diffuse,0.2, 0.2, 0.8, 1) ; 
        glUniform4f(specular, 1, 1, 1, 1) ; 
		glUniform1f(shininess,100) ; 
		glutSolidOctahedron();
    glPopMatrix() ;

	glPushMatrix() ; //Right Octa
		glColor3f(0,0,1);
		glTranslatef(4,0,3);
		glRotatef(yrot,0.0f,1.0f,0.0f);
		glUniform4f(ambient,0.5, 0.5, 0.5, 1) ; 
        glUniform4f(diffuse,0.2, 0.2, 0.8, 1) ; 
        glUniform4f(specular, 1, 1, 1, 1) ; 
		glUniform1f(shininess,100) ; 
		glutSolidOctahedron();
    glPopMatrix() ;

	glPushMatrix() ; 
		glTranslatef(0,1,-9);
		glRotatef(xrot,1.0f,0.0f,0.0f);
		glRotatef(yrot,0.0f,1.0f,0.0f);
		glRotatef(zrot,0.0f,0.0f,1.0f);

	
	//glBindTexture(GL_TEXTURE_2D, texture[1]);
	glUniform1i(istex,2) ;
	//Making our Spining Cube
	glBegin(GL_QUADS);
		// Front Face
		glNormal3f(0.0, 0.0, 1.0);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
		// Back Face
		glNormal3f(0.0, 0.0, -1.0);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
		// Top Face
		glNormal3f(0.0, 1.0, 0.0);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
		// Bottom Face
		glNormal3f(0.0, -1.0, 0.0);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
		// Right face
		glNormal3f(1.0, 0.0, 0.0);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
		// Left Face
		glNormal3f(-1.0, 0.0, 0.0);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
	glEnd();
	glPopMatrix() ;
	glUniform1i(istex,0); 
	//Refresh Buffers

	glPushMatrix() ; 
	glTranslatef(-1.65,-0.5,-1.3);
	//Making a table using long blocks
	glPushMatrix() ; 
		glTranslatef(3,0,0);
		glUniform4f(ambient,0.1, 0.1, 0.1, 1) ; 
        glUniform4f(diffuse,0.8, 0.8, 0.8, 1) ; 
        glUniform4f(specular,1, 1, 1, 1) ; 
		glUniform1f(shininess,100) ;
		glColor3f(1,1,1);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
		// Back Face
		glNormal3f(0.0, 0.0, 1.0);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.0f,  0.0f,  0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.2f,  0.0f,  0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.2f,  1.0f,  0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.0f,  1.0f,  0.0f);
		// Right Face
		glNormal3f(-1.0, 0.0, 0.0);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.0f,  0.0f,  0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.0f,  0.0f,  0.2f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.0f,  1.0f,  0.2f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.0f,  1.0f,  0.0f);
		// Left face
		glNormal3f(1.0, 0.0, 0.0);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.2f,  0.0f,  0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.2f,  0.0f,  0.2f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.2f,  1.0f,  0.2f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.2f,  1.0f,  0.0f);
		// Front Face
		glNormal3f(0.0, 0.0, -1.0);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.0f,  0.0f,  0.2f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.2f,  0.0f,  0.2f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.2f,  1.0f,  0.2f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.0f,  1.0f,  0.2f);
		// Bottom Face
		glNormal3f(0.0, -1.0, 0.0);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.0f,  0.0f,  0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.2f,  0.0f,  0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.2f,  0.0f,  0.2f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.0f,  0.0f,  0.2f);
		// Top Face
		glNormal3f(0.0, 1.0, 0.0);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.0f,  1.0f,  0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.2f,  1.0f,  0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.2f,  1.0f,  0.2f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.0f,  1.0f,  0.2f);
	glEnd();
	glPopMatrix() ;
	glPushMatrix() ; 
		glTranslatef(3,0,2);
		glUniform4f(ambient,0.1, 0.1, 0.1, 1) ;  
        glUniform4f(diffuse,0.8, 0.8, 0.8, 1) ; 
        glUniform4f(specular,1, 1, 1, 1) ; 
		glUniform1f(shininess,100) ;
		glColor3f(1,1,1);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
		// Back Face
		glNormal3f(0.0, 0.0, 1.0);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.0f,  0.0f,  0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.2f,  0.0f,  0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.2f,  1.0f,  0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.0f,  1.0f,  0.0f);
		// Right Face
		glNormal3f(-1.0, 0.0, 0.0);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.0f,  0.0f,  0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.0f,  0.0f,  0.2f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.0f,  1.0f,  0.2f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.0f,  1.0f,  0.0f);
		// Left face
		glNormal3f(1.0, 0.0, 0.0);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.2f,  0.0f,  0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.2f,  0.0f,  0.2f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.2f,  1.0f,  0.2f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.2f,  1.0f,  0.0f);
		// Front Face
		glNormal3f(0.0, 0.0, -1.0);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.0f,  0.0f,  0.2f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.2f,  0.0f,  0.2f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.2f,  1.0f,  0.2f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.0f,  1.0f,  0.2f);
		// Bottom Face
		glNormal3f(0.0, -1.0, 0.0);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.0f,  0.0f,  0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.2f,  0.0f,  0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.2f,  0.0f,  0.2f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.0f,  0.0f,  0.2f);
		// Top Face
		glNormal3f(0.0, 1.0, 0.0);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.0f,  1.0f,  0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.2f,  1.0f,  0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.2f,  1.0f,  0.2f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.0f,  1.0f,  0.2f);
	glEnd();
	glPopMatrix() ;
	glPushMatrix() ; 
		glTranslatef(0,0,0);
		glUniform4f(ambient,0.1, 0.1, 0.1, 1) ; 
        glUniform4f(diffuse,0.8, 0.8, 0.8, 1) ; 
        glUniform4f(specular,1, 1, 1, 1) ; 
		glUniform1f(shininess,100) ;
		glColor3f(1,1,1);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
		// Back Face
		glNormal3f(0.0, 0.0, 1.0);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.0f,  0.0f,  0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.2f,  0.0f,  0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.2f,  1.0f,  0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.0f,  1.0f,  0.0f);
		// Right Face
		glNormal3f(-1.0, 0.0, 0.0);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.0f,  0.0f,  0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.0f,  0.0f,  0.2f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.0f,  1.0f,  0.2f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.0f,  1.0f,  0.0f);
		// Left face
		glNormal3f(1.0, 0.0, 0.0);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.2f,  0.0f,  0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.2f,  0.0f,  0.2f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.2f,  1.0f,  0.2f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.2f,  1.0f,  0.0f);
		// Front Face
		glNormal3f(0.0, 0.0, -1.0);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.0f,  0.0f,  0.2f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.2f,  0.0f,  0.2f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.2f,  1.0f,  0.2f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.0f,  1.0f,  0.2f);
		// Bottom Face
		glNormal3f(0.0, -1.0, 0.0);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.0f,  0.0f,  0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.2f,  0.0f,  0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.2f,  0.0f,  0.2f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.0f,  0.0f,  0.2f);
		// Top Face
		glNormal3f(0.0, 1.0, 0.0);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.0f,  1.0f,  0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.2f,  1.0f,  0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.2f,  1.0f,  0.2f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.0f,  1.0f,  0.2f);
	glEnd();
	glPopMatrix() ;

	glPushMatrix() ; 
	glTranslatef(0,0,2);
		glUniform4f(ambient,0.1, 0.1, 0.1, 1) ; 
        glUniform4f(diffuse,0.8, 0.8, 0.8, 1) ; 
        glUniform4f(specular,1, 1, 1, 1) ; 
		glUniform1f(shininess,100) ;
		glColor3f(1,1,1);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
		// Back Face
		glNormal3f(0.0, 0.0, 1.0);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.0f,  0.0f,  0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.2f,  0.0f,  0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.2f,  1.0f,  0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.0f,  1.0f,  0.0f);
		// Right Face
		glNormal3f(-1.0, 0.0, 0.0);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.0f,  0.0f,  0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.0f,  0.0f,  0.2f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.0f,  1.0f,  0.2f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.0f,  1.0f,  0.0f);
		// Left face
		glNormal3f(1.0, 0.0, 0.0);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.2f,  0.0f,  0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.2f,  0.0f,  0.2f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.2f,  1.0f,  0.2f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.2f,  1.0f,  0.0f);
		// Front Face
		glNormal3f(0.0, 0.0, -1.0);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.0f,  0.0f,  0.2f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.2f,  0.0f,  0.2f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.2f,  1.0f,  0.2f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.0f,  1.0f,  0.2f);
		// Bottom Face
		glNormal3f(0.0, -1.0, 0.0);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.0f,  0.0f,  0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.2f,  0.0f,  0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.2f,  0.0f,  0.2f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.0f,  0.0f,  0.2f);
		// Top Face
		glNormal3f(0.0, 1.0, 0.0);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.0f,  1.0f,  0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.2f,  1.0f,  0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.2f,  1.0f,  0.2f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.0f,  1.0f,  0.2f);
	glEnd();
	glPopMatrix() ;

	glPushMatrix() ; 
		glColor3f(1,1,1);
		glUniform4f(ambient,0.3, 0.3, 0.3, 1) ; 
        glUniform4f(diffuse,0.8, 0.8, 0.8, 1) ; 
        glUniform4f(specular,1, 1, 1, 1) ; 
		glUniform1f(shininess,100) ; 
	glBegin(GL_QUADS); //Makes table top
		glNormal3f(0.0, 1.0, 0.0);
		glVertex3f(0,1,0);
		glVertex3f(0,1,2.2);
		glVertex3f(3.2,1,2.2);
		glVertex3f(3.2,1,0);
	glEnd();
	glPopMatrix();

	glPopMatrix() ;

	glutSwapBuffers();
	
}

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutCreateWindow("HW3: Open Project");

	//triman added this, argv[1] is name of txt file
	//std::string testString = textFileRead(argv[1]);
	//std::cout << testString;
	//parseFile(argv[1]);

	GLenum err = glewInit() ; 
	if (GLEW_OK != err) { 
		std::cerr << "Error: " << glewGetString(err) << std::endl; 
	} 

	init();
	glutDisplayFunc(display);
	glutSpecialFunc(specialKey);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
    //glutMotionFunc(mousedrag);
	myMouse = false;
	glutReshapeFunc(reshape);
	glutReshapeWindow(600, 400);

	printHelp();
	glutMainLoop();
	return 0;
}
