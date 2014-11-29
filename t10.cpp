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
long double tstep   = .15;			// gap between two points on the curve
int px[100], py[100];				// for storing the points	
float polyx[100], polyy[100];		// control points or the points on polygon in case of bezier curve
int polycount = 0;					// no. of points in the bezeir polygon
int drawpoly  = 1;					// get points of the curve polygon
int drawpoly1 = 0;
int donepoly  = 0;					// stop taking points for curve polygon
int changepoint = 0;				// for changing  control point
int count     = 0;
int minindex  = 0;
int finsertpoint = 0;				// 1 when a new point is to be inserted
int fdeletepoint = 0;				// 1 when a new point is to be deleted

float **Tmatinv;					// inverse of Tmat be used for 2nd order continous cubic spline
float **Tmat;						// to be used while drawing the 2nd order continous cubic spline
float **Pmat;						// to be used while drawing the 2nd order continous cubic spline
float **Pdmat;						// to be used while drawing the 2nd order continous cubic spline
float Fmat[4];						// to be used while drawing the 2nd order continous cubic spline

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

// calculate the cofactor of element (row,col)
int GetMinor(float **src, float **dest, int row, int col, int order){
    // indicate which col and row is being copied to dest
    int colCount=0,rowCount=0;
 
    for(int i = 0; i < order; i++ )
    {
        if( i != row )
        {
            colCount = 0;
            for(int j = 0; j < order; j++ )
            {
                // when j is not the element
                if( j != col )
                {
                    dest[rowCount][colCount] = src[i][j];
                    colCount++;
                }
            }
            rowCount++;
        }
    }
 
    return 1;
}

// Calculate the determinant recursively.
double CalcDeterminant( float **mat, int order){
    // order must be >= 0
    // stop the recursion when matrix is a single element
    if( order == 1 )
        return mat[0][0];
 
    // the determinant value
    float det = 0;
 
    // allocate the cofactor matrix
    float **minor;
    minor = new float*[order-1];
    for(int i=0;i<order-1;i++)
        minor[i] = new float[order-1];
 
    for(int i = 0; i < order; i++ )
    {
        // get minor of element (0,i)
        GetMinor( mat, minor, 0, i , order);
        // the recusion is here!
 
        det += (i%2==1?-1.0:1.0) * mat[0][i] * CalcDeterminant(minor,order-1);
        //det += pow( -1.0, i ) * mat[0][i] * CalcDeterminant( minor,order-1 );
    }
 
    // release memory
    for(int i=0;i<order-1;i++)
        delete [] minor[i];
    delete [] minor;
 
    return det;
}

// matrix inversioon
// the result is put in Y
void MatrixInversion(float **A, int order, float **Y){
    // get the determinant of a
    double det = 1.0/CalcDeterminant(A,order);
 
    // memory allocation
    float *temp = new float[(order-1)*(order-1)];
    float **minor = new float*[order-1];
    for(int i=0;i<order-1;i++)
        minor[i] = temp+(i*(order-1));
 
    for(int j=0;j<order;j++)
    {
        for(int i=0;i<order;i++)
        {
            // get the co-factor (matrix) of A(j,i)
            GetMinor(A,minor,j,i,order);
            Y[i][j] = det*CalcDeterminant(minor,order-1);
            if( (i+j)%2 == 1)
                Y[i][j] = -Y[i][j];
        }
    }
 
    // release memory
    //delete [] minor[0];
    delete [] temp;
    delete [] minor;
}
 
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
	else if(value == 2){		// change the control point
		changepoint = 1;
	}
	else if(value == 1){		// draw the Bezeir Curve
		drawpoly = 1;
		donepoly = 0;
		drawpoly1 = 5;		
	}
	else if(value == 3){		// draw the Cubic B Spline
		drawpoly = 1;
		donepoly = 0;		
		drawpoly1 = 6;		
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
	glutAddMenuEntry("Draw Bezeir", 1);
	//glutAddMenuEntry("Draw 2nd Order Continuos", 2);
	glutAddMenuEntry("Draw Cubic B Spline", 3);
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

void drawpolygon(){									// draws line joining the control points
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

void drawcubicBspline(){	
	float mymat[4][4] = {-1,3, -3, 1, 3, -6, 3, 0, -3, 0, 3, 0, 1, 4, 1, 0};	
	float temp1[4], temp2[4], temp3[4];
	
	long double  t = 0;
	tstep    = 1/(pointsonbez-1.0);
	
	glColor3f(0, 1, 1);
	glBegin(GL_LINE_STRIP);
	for(int i=1;i<=polycount-3;++i){
		t = 0;		
		for(int j=0;j<pointsonbez;++j){
			temp1[0] = pow(t, 3);
			temp1[1] = pow(t, 2);
			temp1[2] = pow(t, 1);
			temp1[3] = 1;
			
			t = t+tstep;
			temp2[0] = (-1*polyx[i-1]+3*polyx[i]-3*polyx[i+1]+polyx[i+2])/6.0;
			temp2[1] = (3*polyx[i-1]-6*polyx[i]+3*polyx[i+1])/6.0;
			temp2[2] = (-3*polyx[i-1]+3*polyx[i+1])/6.0;
			temp2[3] = (1*polyx[i-1]+4*polyx[i]+1*polyx[i+1])/6.0;			
			
			temp3[0] = (-1*polyy[i-1]+3*polyy[i]-3*polyy[i+1]+polyy[i+2])/6.0;
			temp3[1] = (3*polyy[i-1]-6*polyy[i]+3*polyy[i+1])/6.0;
			temp3[2] = (-3*polyy[i-1]+3*polyy[i+1])/6.0;
			temp3[3] = (1*polyy[i-1]+4*polyy[i]+1*polyy[i+1])/6.0;
			
			float sumx = 0;
			float sumy = 0;
			for(int k=0;k<4;++k){
				sumx = sumx+temp1[k]*temp2[k];
				sumy = sumy+temp1[k]*temp3[k];
			}			
			glVertex2f(sumx, sumy);			
		}					
	}
	glEnd();
	glutSwapBuffers();	
}

// draws the bezier curve using the control points contained in the array
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

// makes T matrix and Tmatinv for the cubic spline
void makecubicTmatrix(){
	Tmat    = (float **)malloc(sizeof(float *)*polycount);
	Tmatinv = (float **)malloc(sizeof(float *)*polycount);
	
	for(int i=0;i<polycount;++i){
		Tmat[i]    = (float *)malloc(sizeof(float)*polycount);
		Tmatinv[i] = (float *)malloc(sizeof(float)*polycount);
	}
	
	for(int i=0;i<polycount;++i)					// initialize the matrix to zero
		for(int j=0;j<polycount;++j)
					Tmat[i][j] =0;	
				
	for(int i=1;i<=polycount-2;++i){				// make the matrix for taking inverse		
		Tmat[i][i] = 4;
		Tmat[i][i-1] = 1;
		Tmat[i][i+1] = 1;
	}	
	Tmat[0][0] = 1;
	Tmat[polycount-1][polycount-1] = 1;
	
	printf("PRINTING T matrix\n");
	for(int i=0;i<polycount;++i){
		for(int j=0;j<polycount;++j){
			printf("%f ", Tmat[i][j]);
		}
		printf("\n");
	}
	return;
}

// makes P matrix for the cubic spline
void makecubicPmatrix(){
	Pmat  = (float **)malloc(sizeof(float *)*polycount);
	Pdmat = (float **)malloc(sizeof(float *)*polycount);	
	
	for(int i=0;i<polycount;++i){
		Pmat[i]  = (float *)malloc(sizeof(float)*2);
		Pdmat[i] = (float *)malloc(sizeof(float)*2);
	}
		
	Pmat[0][0] 		     = 0;				// at present make it zero
	Pmat[polycount-1][0] = 0;				
	Pmat[0][1] 		     = 0;				
	Pmat[polycount-1][1] = 0;				
	
	for(int i=1;i<=polycount-2;++i){
		Pmat[i][0] = 3*(polyx[i+2]-polyx[i]);
		Pmat[i][1] = 3*(polyy[i+2]-polyy[i]);
	}	
	return;
}

// multiplies the inverse matrix with the P matrix to get the derivatives into Pdmat
void multcubicinversemat(){
	MatrixInversion(Tmat, polycount, Tmatinv);			// get the matrix inverse in the Tmatinv
	
	printf("INVERSE MATRIX\n");
	for(int i=0;i<polycount;++i){
		for(int j=0;j<polycount;++j){
			printf("%f ", Tmatinv[i][j]);
		}	
		printf("\n");
	}
	
	return;
	for(int i=0;i<polycount;++i){
		float tempsum1 = 0;
		float tempsum2 = 0;
		for(int j=0;j<polycount;++j){
			tempsum1 = tempsum1+Tmatinv[i][j]*Pmat[j][0];
			tempsum2 = tempsum2+Tmatinv[i][j]*Pmat[j][1];
		}
		Pdmat[i][0] = tempsum1;
		Pdmat[i][1] = tempsum2;
	}
	return;
}

// takes value of t and makes the F matrix
void makeFmatrix(float t){	
	float temp[4], sum;
	for(int i=0;i<4;++i)
		temp[i] = pow(t, 3-i);
		
	float B[4][4] = {2, -2, 1, 1, -3, 3, -2, -1, 0, 0, 1, 0, 1, 0, 0, 0};
	
	for(int i=0;i<4;++i){		
		sum = 0;
		for(int j=0;j<4;++j){
			sum = sum+temp[i]*B[j][i];
		}
		Fmat[i] = sum;
	}
	return;
}

// draws the cubic spline which is 2nd order continous
void drawcubic(){
	makecubicTmatrix();					// make the Tmat
	makecubicPmatrix();					// make the Pmat
	multcubicinversemat();				// take inverse of Tmat and multiply Pmat with Tmatinv to get Pdmat
	return;
	
	long double  t = 0;
	tstep    = 1/(pointsonbez-1.0);
	float mymat[4][2];	
	
	glColor3f(0, 1, 1);
	glBegin(GL_LINE_STRIP);
		for(int i=0;i<1;++i){		// for each segment
			t = 0;
			mymat[0][0] = polyx[i];			mymat[0][1] = polyy[i];
			mymat[1][0] = polyx[i+1];		mymat[1][1] = polyy[i+1];
			mymat[2][0] = Pdmat[i][0];		mymat[2][1] = Pdmat[i][1];
			mymat[3][0] = Pdmat[i+1][0];	mymat[3][1] = Pdmat[i+1][1];
			
			for(int j=0;j<pointsonbez;++j){
				makeFmatrix(t);				
				float sumx = 0, sumy = 0;				
				for(int k=0;k<4;++k){
					sumx = sumx+Fmat[k]*mymat[i][0];
					sumy = sumy+Fmat[k]*mymat[i][1];
				}				
				glVertex2f(sumx, sumy);
				printf("%f %f\n", sumx, sumy);
				t = t+tstep;				
			}
		}		
	glEnd();
	glutSwapBuffers();	
}

// makes the queue into a array for easy access
void makeintoarray(){
	int tx1, ty1, size;	
	size 	  = (int)poly1.size();	
	polycount = size;							// update the count of points on the curve polygon
	
	for(int i=0;i<size;++i){
		tx1  = poly1.front()->x;
		ty1  = poly1.front()->y;
		polyx[i] = -1+2*tx1/(w+0.0);
		polyy[i] = -1+2*ty1/(h+0.0);
		poly1.pop_front();
	}	
}

// removes the point from the array and redraws the curve
void removepoint(){	
	for(int i=minindex;i<polycount-1;++i){
		polyx[i] = polyx[i+1];
		polyy[i] = polyy[i+1];
	}
	--polycount;
	drawpolygon();
	if(drawpoly1 == 5)
		drawbez();
	else if(drawpoly1 == 6)	
		drawcubicBspline();
	//drawcubicBspline();
	return;
}

//inserts the given point just after the selected point in the array
void putpoint(float nx, float ny){
	for(int i=polycount+1;i>=minindex+2;--i){
		polyx[i] = polyx[i-1];
		polyy[i] = polyy[i-1];
	}
	++polycount;
	polyx[minindex+1] = nx;
	polyy[minindex+1] = ny;
	drawpolygon();
	if(drawpoly1 == 5)
		drawbez();
	else if(drawpoly1 == 6)
		drawcubicBspline();
	//drawcubicBspline();
	
}

//updates the control point
void contmotion(int x, int y){
	if(changepoint ==2){						// if the change point is activated then carry out the operations
		y = h-y;
		float te1 = -1+2*x/(w+0.0);
		float te2 = -1+2*y/(h+0.0);		   
		   
		polyx[minindex] = te1;								
		polyy[minindex] = te2;
		  
		drawpolygon();
		if(drawpoly1 == 5)
			drawbez();
		else if(drawpoly1 == 6)
			drawcubicBspline();
		//drawcubicBspline();
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
	if(changepoint == 1){									// gets the control point which is to be changed
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
	   makeintoarray();										// put the queue elements into an array
	   
	   // testing
		drawpolygon();	   
		if(drawpoly1 == 5)
			drawbez();		
		else if(drawpoly1 == 6)
			drawcubicBspline();
	   //drawcubicBspline();	   
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
				drawpoly = 2;					// ignore the point .....bcoz it just released point				
				return;
		   }		   
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
					   if(drawpoly1 == 5)
							drawbez();
					   else if(drawpoly1 == 6)
							drawcubicBspline();
					   //drawcubicBspline();
					   
					   break;
			case 'o' : --pointsonbez; 					   
					   drawpolygon();
					   if(drawpoly1 == 5)
							drawbez();
					   else if(drawpoly1 == 6)
							drawcubicBspline();
					   //drawcubicBspline();
					   
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
