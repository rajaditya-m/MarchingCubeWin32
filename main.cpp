#include "Oracle.h"
#include "globalTables.h"
#include "pointmanip.h"
#include "meshrep.h"
#include <vector>
#include <ctime>

using namespace marching;

#define MAXNUMVERT 5000000
#define PERTURB 0.001

int numTriangles = 0 ;
int degTriangles = 0;

//const int numVertices = numTriangles * 3;

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;

point4 pointsT[MAXNUMVERT];
vec3 normalsT[MAXNUMVERT];

/*
point4 pointsT[numVertices];
vec3 normalsT[numVertices];
*/
point4* points;
vec3* normals;

typedef struct 
{
  point4 pointData[8];
  GLfloat scalarValues[8];
}GridCell;

typedef struct GridData
{
	GLuint xR_,yR_,zR_;
	GLfloat scalarVal_[MAXPOINT][MAXPOINT][MAXPOINT];
};

//Uniform Locations for the ModelView and Projection Matrices in the shaders
GLuint ModelView,Projection;

int idx = 0;



void triangle(const point4& a,const point4& b,const point4& c,int cubeindex)
{
	vec3 normal = normalize(cross(b-a,c-b));
	if(equalityPoints(a,b) || equalityPoints(b,c) || equalityPoints(a,c))
	{
		//std::cout << "Ignoring degenerate triangle";
		degTriangles++;
		return;
	}	
	normalsT[idx] = normal; pointsT[idx] = a; idx++;
	normalsT[idx] = normal; pointsT[idx] = b; idx++;
	normalsT[idx] = normal; pointsT[idx] = c; idx++;
	numTriangles++;
}

GLfloat euclidDist(const point4& p1, const point4& p2)
{
  return length(p1-p2);
}

GridData* generateGrids(int xR,int yR,int zR)
{
	GridData* gData = new GridData();
	gData->xR_ = xR;
	gData->yR_ = yR;
	gData->zR_ = zR;

	int i,j,k;

	GLfloat radius = 45.0;

  	point4 center(50.0,50.0,50.0,1.0);
	

	for(i=0;i<xR;i++)
	{
		for(j=0;j<yR;j++)
		{
			for(k=0;k<zR;k++)
			{
				point4 po(i,j,k,1.0);
				GLfloat val_ = euclidDist(po,center)-radius;
				if(equalityInLimits(val_,0))
					val_ += PERTURB;
				gData->scalarVal_[i][j][k] = val_;
			}
		}
	}

	return gData;
}

GridData* readMRIFILE(const char* fileName,int x,int y,int z)
{
	int i,j,k;
	FILE* ptr ;
	GridData* gData = new GridData();

	gData->xR_ = x;
	gData->yR_ = y;
	gData->zR_ = z;

	ptr = fopen(fileName,"rb");

	for(k=0;k<z;k++)
	{
		for(j=0;j<y;j++)
		{
			for(i=0;i<x;i++)
			{

				gData->scalarVal_[i][j][k] = fgetc(ptr);
			}
		}
	}

	fclose(ptr);

	return gData;

}

point4 unit (const point4& p)
{
	float len = p.x*p.x + p.y*p.y + p.z*p.z;

	point4 t;
	if(len>DivideByZeroTolerance)
	{
		t = p/sqrt(len);
		t.w = 1.0;
	}
	return t;
}

void trimArrays()
{
	points = new point4[idx];
	normals = new vec3[idx];

	for(int i =0;i<idx;i++)
	{
		points[i] = pointsT[i];
		normals[i] = normalsT[i];
	}
}


point4 VertexInterp(GLfloat isoLevel,const point4& p1,const point4& p2,GLfloat v1,GLfloat v2)
{

	if(equalityInLimits(isoLevel,v1))
		return p1;
	if(equalityInLimits(isoLevel,v2))
		return p2;
	if(equalityInLimits(v1,v2))
		return p1;

	GLfloat mu = (isoLevel-v1)/(v2-v1);

	if(mu!=mu)
		std::cout<< "NaN value found in interpolation.\n";

	point4 p;
	p = p1 + (mu * (p2-p1));
	return p;
}

void polygonise(const GridCell& grid,GLfloat isolevel,MeshRep* mesh)
{
   int i,ntriang;
   int cubeindex;
   point4* vertlist = new point4[12];
   //std::vector<point4> vertlist ;
   //vertlist.resize(12);
   cubeindex = 0;

   if (grid.scalarValues[0] < isolevel) cubeindex |= 1;
   if (grid.scalarValues[1] < isolevel) cubeindex |= 2;
   if (grid.scalarValues[2] < isolevel) cubeindex |= 4;
   if (grid.scalarValues[3] < isolevel) cubeindex |= 8;
   if (grid.scalarValues[4] < isolevel) cubeindex |= 16;
   if (grid.scalarValues[5] < isolevel) cubeindex |= 32;
   if (grid.scalarValues[6] < isolevel) cubeindex |= 64;
   if (grid.scalarValues[7] < isolevel) cubeindex |= 128;

   // Cube is entirely in/out of the surface 
   if (edgeTable[cubeindex] == 0)
      return;

   // Find the vertices where the surface intersects the cube 
   if (edgeTable[cubeindex] & 1)
      vertlist[0] =
         VertexInterp(isolevel,grid.pointData[0],grid.pointData[1],grid.scalarValues[0],grid.scalarValues[1]);
   if (edgeTable[cubeindex] & 2)
      vertlist[1] =
         VertexInterp(isolevel,grid.pointData[1],grid.pointData[2],grid.scalarValues[1],grid.scalarValues[2]);
   if (edgeTable[cubeindex] & 4)
      vertlist[2] =
         VertexInterp(isolevel,grid.pointData[2],grid.pointData[3],grid.scalarValues[2],grid.scalarValues[3]);
   if (edgeTable[cubeindex] & 8)
      vertlist[3] =
         VertexInterp(isolevel,grid.pointData[3],grid.pointData[0],grid.scalarValues[3],grid.scalarValues[0]);
   if (edgeTable[cubeindex] & 16)
      vertlist[4] =
         VertexInterp(isolevel,grid.pointData[4],grid.pointData[5],grid.scalarValues[4],grid.scalarValues[5]);
   if (edgeTable[cubeindex] & 32)
      vertlist[5] =
         VertexInterp(isolevel,grid.pointData[5],grid.pointData[6],grid.scalarValues[5],grid.scalarValues[6]);
   if (edgeTable[cubeindex] & 64)
      vertlist[6] =
         VertexInterp(isolevel,grid.pointData[6],grid.pointData[7],grid.scalarValues[6],grid.scalarValues[7]);
   if (edgeTable[cubeindex] & 128)
      vertlist[7] =
         VertexInterp(isolevel,grid.pointData[7],grid.pointData[4],grid.scalarValues[7],grid.scalarValues[4]);
   if (edgeTable[cubeindex] & 256)
      vertlist[8] =
         VertexInterp(isolevel,grid.pointData[0],grid.pointData[4],grid.scalarValues[0],grid.scalarValues[4]);
   if (edgeTable[cubeindex] & 512)
      vertlist[9] =
         VertexInterp(isolevel,grid.pointData[1],grid.pointData[5],grid.scalarValues[1],grid.scalarValues[5]);
   if (edgeTable[cubeindex] & 1024)
      vertlist[10] =
         VertexInterp(isolevel,grid.pointData[2],grid.pointData[6],grid.scalarValues[2],grid.scalarValues[6]);
   if (edgeTable[cubeindex] & 2048)
      vertlist[11] =
         VertexInterp(isolevel,grid.pointData[3],grid.pointData[7],grid.scalarValues[3],grid.scalarValues[7]);

   // Create the triangle 
   for (i=0;triTable[cubeindex][i]!=-1;i+=3) {

	   unsigned long i1 = mesh->addOrRetrieve(vertlist[triTable[cubeindex][i]]);
	   unsigned long i2 = mesh->addOrRetrieve(vertlist[triTable[cubeindex][i+1]]);
	   unsigned long i3 = mesh->addOrRetrieve(vertlist[triTable[cubeindex][i+2]]);
	   mesh->addAnEdge(i1,i2);
	   mesh->addAnEdge(i2,i3);
	   mesh->addAnEdge(i1,i3);
	   mesh->addAFace(i1,i2,i3);
	   //std::cout << "[" << i1 << "," << i2 << "," << i3 << "]\n";
	   //std::cout << "[" << vertlist[triTable[cubeindex][i]] << ", " << vertlist[triTable[cubeindex][i+1]] << "," << vertlist[triTable[cubeindex][i+2]] << "]\n";
   	  triangle(vertlist[triTable[cubeindex][i]],vertlist[triTable[cubeindex][i+1]],vertlist[triTable[cubeindex][i+2]],cubeindex);
   }

   delete[] vertlist;


}

MeshRep* generateData(GridData* grid,GLfloat isoLevel)
{
	//Create a mesh representation object
	MeshRep* mesh = new MeshRep();



  int i,j,k;
  //Creating the grid cell
  for(i=0;i<grid->xR_-1;i++)
  {
    for(j=1;j<grid->yR_;j++)
    {
      for(k=0;k<grid->zR_-1;k++)
      {
        GridCell* temp = (GridCell*)malloc(sizeof(GridCell));

        temp->pointData[0].x = (GLfloat)i;
        temp->pointData[0].y = (GLfloat)j;
        temp->pointData[0].z = (GLfloat)k;
        temp->pointData[0].w = 1.0;
        temp->scalarValues[0] = grid->scalarVal_[i][j][k];

        temp->pointData[1].x = (GLfloat)i+1;
        temp->pointData[1].y = (GLfloat)j;
        temp->pointData[1].z = (GLfloat)k;
        temp->pointData[1].w = 1.0;
        temp->scalarValues[1] = grid->scalarVal_[i+1][j][k];

        temp->pointData[2].x = (GLfloat)i+1;
        temp->pointData[2].y = (GLfloat)j-1;
        temp->pointData[2].z = (GLfloat)k;
        temp->pointData[2].w = 1.0;
        temp->scalarValues[2] = grid->scalarVal_[i+1][j-1][k];

        temp->pointData[3].x = (GLfloat)i;
        temp->pointData[3].y = (GLfloat)j-1;
        temp->pointData[3].z = (GLfloat)k;
        temp->pointData[3].w = 1.0;
        temp->scalarValues[3] = grid->scalarVal_[i][j-1][k];

        temp->pointData[4].x = (GLfloat)i;
        temp->pointData[4].y = (GLfloat)j;
        temp->pointData[4].z = (GLfloat)k+1;
        temp->pointData[4].w = 1.0;
        temp->scalarValues[4] = grid->scalarVal_[i][j][k+1];

        temp->pointData[5].x = (GLfloat)i+1;
        temp->pointData[5].y = (GLfloat)j;
        temp->pointData[5].z = (GLfloat)k+1;
        temp->pointData[5].w = 1.0;
        temp->scalarValues[5] = grid->scalarVal_[i+1][j][k+1];

        temp->pointData[6].x = (GLfloat)i+1;
        temp->pointData[6].y = (GLfloat)j-1;
        temp->pointData[6].z = (GLfloat)k+1;
        temp->pointData[6].w = 1.0;
        temp->scalarValues[6] = grid->scalarVal_[i+1][j-1][k+1];

        temp->pointData[7].x = (GLfloat)i;
        temp->pointData[7].y = (GLfloat)j-1;
        temp->pointData[7].z = (GLfloat)k+1;
        temp->pointData[7].w = 1.0;
        temp->scalarValues[7] = grid->scalarVal_[i][j-1][k+1];

        //Spew out the triangle coordinates
        polygonise(*temp,isoLevel,mesh);
		free(temp);

      }
    }
  }

  std::cout <<"Finished generating Data\n";
  std::cout <<"Total number of triangles for OPENGL:"<<numTriangles<<"\n";
  std::cout <<"Total number of vertices for OPENGL:"<<idx<<"\n";
  std::cout <<"Total number of degenerate triangles:"<<degTriangles<<"\n";
  std::cout << "Total number of unique vertices processed:"<< mesh->nosVertices() <<"\n";
  std::cout << "Total number of raw edges processed:"<< mesh->nosRawEdges() <<"\n";
  std::cout << "Average number of triangles per vertex :"<< mesh->nosAvgTrianglesPerVertex() <<"\n";

  return mesh;
}


// comparison function
int compareRawEdges(const void* a, const void* b)
{
    RawEdge arg1 = *reinterpret_cast<const RawEdge*>(a);
    RawEdge arg2 = *reinterpret_cast<const RawEdge*>(b);
    if(arg1 < arg2) return -1;
    if(arg1 > arg2) return 1;
    return 0;
}



void init()
{
	clock_t startTime,endTime;
	MeshRep* genM;
	startTime = clock();
	GridData* grid = generateGrids(150,150,150);
	//GridData* grid = readMRIFILE("mri.raw",200,160,160);
	//generateData(grid,128);
	genM = generateData(grid,0);
	delete grid;
	endTime = clock();
	double totTime = endTime - startTime;
	double timeInSecs = totTime/(double)CLOCKS_PER_SEC;
	std::cout << "Total time elapsed for all non-OPENGL mesh generation operation:" << timeInSecs << " secs \n";

	//Start the sanity checker 

	//Sort all raw edges in their correct order
	startTime = clock();
	std::qsort(genM->getRawEdgeArray(),genM->nosRawEdges(),sizeof(RawEdge),compareRawEdges);
	endTime = clock();
	totTime = endTime - startTime;
	timeInSecs = totTime/(double)CLOCKS_PER_SEC;
	std::cout << "Total time elapsed for all non-OPENGL raw edge sorting operation:" << timeInSecs << " secs \n";
	
	//Check for 2 edge property
	startTime = clock();
	if(!(genM->checkEdgeCorrectness()))
		std::cout << "[ERROR] Generated Mesh is non-manifold intersections.\n";
	endTime = clock();
	totTime = endTime - startTime;
	timeInSecs = totTime/(double)CLOCKS_PER_SEC;
	std::cout << "Total time elapsed for non-OPENGL adjacency face checking operation:" << timeInSecs << " secs \n";

	//Check for triangle fan 
	startTime = clock();
	if(!(genM->checkFanProperty()))
		std::cout << "[ERROR] Generated Mesh has some vertex that doesn't form an open or closed fan.\n";
	endTime = clock();
	totTime = endTime - startTime;
	timeInSecs = totTime/(double)CLOCKS_PER_SEC;
	std::cout << "Total time elapsed for non-OPENGL fan property checking operation:" << timeInSecs << " secs \n";


	delete genM;

	//Generate Vertex Array Objects
	GLuint vao;
	glGenVertexArrays(1,&vao);
	glBindVertexArray(vao);

	//Generate Vertex Buffer Objects
	GLuint buffer;
	glGenBuffers(1,&buffer);
	glBindBuffer(GL_ARRAY_BUFFER,buffer);
	glBufferData(GL_ARRAY_BUFFER,sizeof(pointsT)+sizeof(normalsT),NULL,GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(pointsT),pointsT);
	glBufferSubData(GL_ARRAY_BUFFER,sizeof(pointsT),sizeof(normalsT),normalsT);

	//Load shader setups
	GLuint program = InitShader("vshader_marching.glsl","fshader_marching.glsl");
	glUseProgram(program);

	//setup vertex arrays for data transmissions
	GLuint vPosition = glGetAttribLocation(program,"vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition,4,GL_FLOAT,GL_FALSE,0,BUFFER_OFFSET(0));

	GLuint vNormal = glGetAttribLocation(program,"vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal,3,GL_FLOAT,GL_FALSE,0,BUFFER_OFFSET(sizeof(pointsT)));

	point4 lightPosition(300.0,300.0,300.0,1.0);
	color4 lightAmbient(0.2,0.2,0.2,1.0);
	color4 lightDiffuse(1.0,1.0,1.0,1.0);
	color4 lightSpecular(1.0,1.0,1.0,1.0);

	color4 materialAmbient(1.0,0.0,1.0,1.0);
	color4 materialDiffuse(1.0,0.8,0.0,1.0);
	color4 materialSpecular(1.0,0.0,1.0,1.0);
	float materialShininess = 0.5;

	color4 ambientPdk = lightAmbient*materialAmbient;
	color4 diffusePdk = lightDiffuse*materialDiffuse;
	color4 specularPdk = lightSpecular*materialSpecular;

	glUniform4fv(glGetUniformLocation(program,"ambientPdk"),1,ambientPdk);
	glUniform4fv(glGetUniformLocation(program,"diffusePdk"),1,diffusePdk);
	glUniform4fv(glGetUniformLocation(program,"specularPdk"),1,specularPdk);

	glUniform4fv(glGetUniformLocation(program,"lightPos"),1,lightPosition);

	glUniform1f(glGetUniformLocation(program,"shininess"),materialShininess);

	ModelView = glGetUniformLocation(program,"modelView");
	Projection = glGetUniformLocation(program,"projection");

	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0,1.0,1.0,1.0);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//Model Matrix
	mat4 model_ = mat4(1.0);
	//mat4 model_ = Scale(0.5,0.5,0.5);


	//Viewing Matrix
	point4 at(0.0,0.0,0.0,1.0);
	point4 eye(15.0,15.0,20.0,1.0);
	vec4 up(0.0,1.0,0.0,0.0);
	mat4 view_ = LookAt(eye,at,up);

	mat4 modelView_ = model_*view_;
	glUniformMatrix4fv(ModelView,1,GL_TRUE,modelView_);

	glDrawArrays(GL_TRIANGLES,0,idx);
	
	glutSwapBuffers();
}

void kbdFunc(unsigned char key,int x,int y)
{
	switch(key)
	{
		case 'q' : case 'Q' : exit(EXIT_SUCCESS); break;
	}
}

void reshapeFunc(int width,int height)
{
	glViewport(0,0,width,height);

	GLfloat left = -120.0,right = 120.0;
	GLfloat top = 120.0, bottom = -120.0;
	GLfloat hither = -120.0,yon = 120.0;

	GLfloat aspectRatio = GLfloat(width)/height;

	if(aspectRatio > 1.0)
	{
		left *= aspectRatio;
		right *= aspectRatio;
	} 
	else
	{
		top /= aspectRatio;
		bottom /= aspectRatio;
	}

	mat4 projection_ = Ortho(left,right,bottom,top,hither,yon);

	glUniformMatrix4fv(Projection,1,GL_TRUE,projection_);
}

int main(int argc, char** argv)
{
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DEPTH);
	glutInitWindowSize(512,512);
	 glutInitContextVersion( 3, 2 );
    glutInitContextProfile( GLUT_CORE_PROFILE );
	glutCreateWindow("Marching Cube with MRI");

	glewExperimental = GL_TRUE;
	glewInit();

	init();

	glutReshapeFunc(reshapeFunc);
	glutDisplayFunc(display);
	glutKeyboardFunc(kbdFunc);

	glutMainLoop();
	return 0;
}