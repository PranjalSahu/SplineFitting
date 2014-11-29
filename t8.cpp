#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <math.h>
#include <queue>
#include <list>
#include <vector>

void *font = GLUT_BITMAP_8_BY_13;

void drawpolygon();
void drawbez();

unsigned long dp[101][101];			// for storing the value of C
double 		  jni[100][100];		// Jni matrix
unsigned int w = 500;
unsigned int h = 500;
int godisp 	   = 1;

static int winmenu;

int pointsonbez = 11;				// no. of parameter points on bezier curve		
long double tstep   = .15;		// gap between two points on the curve
int px[100], py[100];			// for storing the points	
float polyx[100], polyy[100];	// polygon points
int polycount = 0;				// no. of points in the bezeir polygon
int drawpoly  = 1;			// get points of the curve polygon
int donepoly  = 0;			// stop taking points for curve polygon
int changepoint = 0;		// for changing  control point
int count     = 0;
int minindex  = 0;
int finsertpoint = 0;		// 1 when a new point is to be inserted
int fdeletepoint = 0;		// 1 when a new point is to be deleted

class mypoint{           
public:
   int x;
   int y;
   mypoint(int a, int b);
   mypoint(mypoint *t);
   mypoint();
};
mypoint::mypoint(int a,int b){
   x = a;
   y = b;
}
mypoint::mypoint(mypoint *t){
	x = t->x;
	y = t->y;
}
mypoint::mypoint(){
	x = 0;
	y = 0;
}

std::list<mypoint*> poly1;			// for storing the points of polygon

unsigned long nCr(int n, int r){
    if(n==r) return dp[n][r] = 1;
    if(r==0) return dp[n][r] = 1;
    if(r==1) return dp[n][r] = (unsigned long)n;
    if(dp[n][r]) return dp[n][r];
    return dp[n][r] = nCr(n-1,r) + nCr(n-1,r-1);
}

void maketable(){
	for(int i=1;i<=100;++i){
		dp[i][0] = 1;
		dp[i][1] = i;
	}
	nCr(100, 50);
	dp[1][1] = 1;
	return;
}

void drawpoint(float x, float y, int color){
	glPointSize(10);
	if(color == 1)
		glColor3f(0, 1, 0);
	else
	    glColor3f(1, 0, 0);
	    
	glBegin(GL_POINTS);       
      glVertex2f(x, y);
    glEnd();
    glutSwapBuffers();
}

void myreset(){
	//polycount = 0;
	glClear (GL_COLOR_BUFFER_BIT);
    glutSwapBuffers();
	return;
}

void savecurve(){
	FILE *fp;
	fp = fopen("curve", "w");
	fprintf(fp, "%d\n", polycount);			// no. of control points
	for(int i=0;i<polycount;++i){
		fprintf(fp, "%f %f\n", polyx[i], polyy[i]);
	}
	fprintf(fp, "%d", pointsonbez);			// no. of points on bezier curve
	fclose(fp);
	printf("Curve Saved\n");
	return;	
}

void opencurve(){
	FILE *fp;
	fp = fopen("curve", "r");
	int p, b;
	fscanf(fp, "%d", &p); 				// no. of control points
	float ppx[100], ppy[100];
	for(int i=0;i<p;++i){				// values of the control points
		fscanf(fp, "%f %f", &ppx[i], &ppy[i]);
	}	
	fscanf(fp, "%d", &b); 				// no of points on the bezier curve
	fclose(fp);
	
	polycount   = p;
	pointsonbez = b;	
	for(int i=0;i<polycount;++i){
		polyx[i] = ppx[i];
		polyy[i] = ppy[i];
	}
	drawpolygon();
	drawbez();
	printf("File opened successfully\n");
}

void outputCharacter(float x, float y, char *string) {
  int len, i;
  glRasterPos2f(x, y);  
  len = (int) strlen(string);
  for (i = 0; i < len; i++) {
    glutBitmapCharacter(font, string[i]);
  }  
}

void menufun(int value){
	count = 0;
	if(value == 0){
		glutDestroyWindow(winmenu);
		exit(0);
	}
	else if(value == 2){
		changepoint = 1;
	}
	else if(value == 1){		// draw the polygon points
		drawpoly = 1;
		donepoly = 0;		
	}
	else if(value == 9){		// done polygon	
		drawpoly = 0;
		donepoly = 1;		
	}
	else if(value == 5){		// save curve
		savecurve();
	}
	else if(value == 6){		// open curve
		opencurve();
	}
	else if(value == 4){		// reset
		myreset();
	}
}

void createmymenu(void){		
	glutCreateMenu(menufun);	
	glutAddMenuEntry("Draw Polygon", 1);
	glutAddMenuEntry("Save Curve", 5);
	glutAddMenuEntry("Open Curve", 6);
	//glutAddMenuEntry("Change Control Point", 2);
	glutAddMenuEntry("Done", 9);
	glutAddMenuEntry("Reset", 4);
	glutAddMenuEntry("Exit", 0);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int findclosestpoint(float x, float y){
	float min    = 100000;
	int minindex = 0;
	for(int i=0;i<polycount;++i){
		float dx = (x-polyx[i])>0?(x-polyx[i]):-(x-polyx[i]);
		float dy = (y-polyy[i])>0?(y-polyy[i]):-(y-polyy[i]);		
		if(dx+dy<min){
			min      = dx+dy;
			minindex = i; 
		}
	}
	//printf("Returning closest points as %d\n", minindex);
	return minindex;
}

void drawpolygon(){	
	glClear (GL_COLOR_BUFFER_BIT);
	
	glColor3f(0, 0, 1);	
	glBegin(GL_LINE_STRIP);
		for(int i=0;i<polycount;++i){			
			glVertex2f(polyx[i], polyy[i]);			
		}			
	glEnd();
	
	
	for(int i=0;i<polycount;++i){			
		drawpoint(polyx[i], polyy[i], 1);
	}
	
	glutSwapBuffers();
}

void drawbez(){
	char s[100];
	sprintf(s, "CONTROL POINTS  %d", polycount);
	outputCharacter(0.4, 0.9, s);
	sprintf(s, "POINTS ON CURVE %d", pointsonbez);
	outputCharacter(0.4, 0.8, s);	
		
	int N = polycount;
	
	double bx[100];		// parameter points for bezeir curve
	double by[100];	
	
	long double  t = 0;
	tstep    = 1/(pointsonbez-1.0);
	 
	for(int j=0;j<pointsonbez;j++){				
		bx[j] = 0;
		by[j] = 0;		
		for(int I=1;I<=N;++I){
			double term1, term2;
			term1 = pow(t, I-1);
			term2 = pow(1.0-t, N-I);
			
			jni[N][I]  = term1*term2*dp[N-1][I-1];
			
			bx[j] 	   = bx[j]+polyx[I-1]*jni[N][I];
			by[j] 	   = by[j]+polyy[I-1]*jni[N][I];
		}		
		t = t+tstep;
	}
	
	glColor3f(0, 1, 1);
	glBegin(GL_LINE_STRIP);
		for(int i=0;i<pointsonbez;++i){
			glVertex2f(bx[i], by[i]);
		}			
	glEnd();
	glutSwapBuffers();	
	return;
}

void makeintoarray(){
	int tx1, ty1, size;	
	size 	  = (int)poly1.size();	
	polycount = size;			// update the count of points on the curve polygon
	
	for(int i=0;i<size;++i){
		tx1  = poly1.front()->x;
		ty1  = poly1.front()->y;		
			
		polyx[i] = -1+2*tx1/(w+0.0);
		polyy[i] = -1+2*ty1/(h+0.0);
				
		poly1.pop_front();
	}	
}

void removepoint(){	
	for(int i=minindex;i<polycount-1;++i){
		polyx[i] = polyx[i+1];
		polyy[i] = polyy[i+1];
	}
	--polycount;
	drawpolygon();
	drawbez();
	return;
}

void putpoint(float nx, float ny){
	for(int i=polycount+1;i>=minindex+2;--i){
		polyx[i] = polyx[i-1];
		polyy[i] = polyy[i-1];
	}
	++polycount;
	polyx[minindex+1] = nx;
	polyy[minindex+1] = ny;
	drawpolygon();
	drawbez();
}

void contmotion(int x, int y){
	if(changepoint ==2){
		y = h-y;
		float te1 = -1+2*x/(w+0.0);
		float te2 = -1+2*y/(h+0.0);		   
		   
		polyx[minindex] = te1;								//update the control point
		polyy[minindex] = te2;
		  
		drawpolygon();
		drawbez();
		drawpoint(polyx[minindex], polyy[minindex], 2);
	}
		return;	   
}

void mousemotion(int button, int state, int x, int y){ 	
	if(finsertpoint == 2){
		if(state ==  GLUT_DOWN){		
		   y = h-y;
		   float te1    = -1+2*x/(w+0.0);
		   float te2    = -1+2*y/(h+0.0);		   		   
		   finsertpoint = 0;
		   putpoint(te1, te2);
		 }
	}
	if(finsertpoint == 1){
		if(state ==  GLUT_DOWN){		
		   y = h-y;
		   float te1    = -1+2*x/(w+0.0);
		   float te2    = -1+2*y/(h+0.0);		   
		   minindex     =  findclosestpoint(te1, te2);
		   finsertpoint = 2;		   
		}
		return;
	}
	if(fdeletepoint){
		if(state ==  GLUT_DOWN){		
		   y = h-y;
		   float te1    = -1+2*x/(w+0.0);
		   float te2    = -1+2*y/(h+0.0);		   
		   minindex     =  findclosestpoint(te1, te2);
		   fdeletepoint = 0;
		   removepoint();
		}
		return;
	}
	if(changepoint == 1){
		if(state ==  GLUT_DOWN){		
		   y = h-y;
		   float te1 = -1+2*x/(w+0.0);
		   float te2 = -1+2*y/(h+0.0);		   
		   minindex    = findclosestpoint(te1, te2);
		   changepoint = 2;
		}
		return;
   }
   if(changepoint == 2){	   
	    if(state ==  GLUT_DOWN){
			changepoint = 0;
			drawpoint(polyx[minindex], polyy[minindex], 1);
		}
		return;	   
   }
   if(donepoly){	   
	   donepoly = 0;
	   drawpoly = 0;
	   makeintoarray();
	   drawpolygon();	   
	   drawbez();
	   return;
   }		
   if(state == GLUT_DOWN){	   
		px[count] = x;
		py[count] = y;
		++count;
   }
   else{
	   if(drawpoly && button == 0){
		   if(drawpoly == 1){					// taking points of polygon
				drawpoly = 2;					// ignore the point				
				return;
		   }
		   //printf("taking point drawpoly = %d\n", drawpoly);
		   if(drawpoly == 2){
			    y = h-y;
			    float ttx = -1+(x*2)/(w+0.0);
			    float tty = -1+(2*y)/(h+0.0);
				drawpoint(ttx, tty, 1);
				poly1.push_back(new mypoint(x, y));					
		   }
		   return;
	   }
   }
}

void changeSize(int w, int h){
    //width = w;
    //height = h;
    
}

void display(void){
	if(!godisp)
		return;
	glClear (GL_COLOR_BUFFER_BIT);
	glutSwapBuffers();	
	godisp = 0;
}

void insertpoint(){
	finsertpoint = 1;
	printf("Click the point after which you have to insert the new point then click the position of the new point\n");
	return;
}

void deletepoint(){
	fdeletepoint = 1;
}

void inputKey(unsigned char c, int x, int y){
    switch (c) {			
			case 'c' : changepoint = 1; break;
			case 'd' : changepoint = 0; 
					   drawpoint(polyx[minindex], polyy[minindex], 1);	
					   break;  
			case 'i' : ++pointsonbez; 					   
					   drawpolygon();
					   drawbez();
					   break;
			case 'o' : --pointsonbez; 					   
					   drawpolygon();
					   drawbez();
					   break;
			case 'r' : deletepoint();break;
			case 'p' : insertpoint();break;			
    }    
}

int main(int argc, char** argv){	
	printf("Instructions to use this program\n\n");
	printf("\tTo increase the number of points on Bezier curve press 'i'\n");
	printf("\tTo decrease the number of points on Bezier curve press 'o'\n");
	printf("\tTo change a control point press 'c'\n");
	printf("\tAfter completion of change control point opeartion press 'd'\n");
	printf("\tTo insert a control point press 'p'\n");
	printf("\tTo remove a control point press 'r'\n\n\n\n");	
	
	maketable();
	
	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize(w, h);
    winmenu = glutCreateWindow("Draw bezier Curve");
	createmymenu(); 
	
    glutDisplayFunc(display);
    glutMouseFunc(mousemotion);
    glutPassiveMotionFunc(contmotion);
    glutKeyboardFunc(inputKey);    
	glutMainLoop();
		
	return 0;
}
