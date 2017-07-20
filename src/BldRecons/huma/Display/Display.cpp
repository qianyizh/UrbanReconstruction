// Display.cpp: implementation of the Display class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Display.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDisplay::CDisplay()
{
	m_Interface = DISPLAY_INTERFACE_OPENGL;
	m_BGcolor[0]=m_BGcolor[1]=m_BGcolor[2]=1.0;
}

CDisplay::~CDisplay()
{

}
//temp
/*
void CDisplay::createTexture(UINT textureArray[], LPSTR strFileName, int textureID)
{
	AUX_RGBImageRec *pBitmap = NULL;
	
	if(!strFileName)									// Return from the function if no file name was passed in
		return;
	
	// We need to load the texture data, so we use a cool API that the glaux.lib offers.
	
	pBitmap = NULL;
//	pBitmap = auxDIBImageLoad(strFileName);				// Load the bitmap and store the data
	
	if(pBitmap == NULL)									// If we can't load the file, quit!
		exit(0);

	// Generate a texture with the associative texture ID stored in the array
	glGenTextures(1, &textureArray[textureID]);

	// Bind the texture to the texture arrays index and init the texture
	glBindTexture(GL_TEXTURE_2D, textureArray[textureID]);

	// Build Mipmaps (builds different versions of the picture for distances - looks better)
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, pBitmap->sizeX, pBitmap->sizeY, GL_RGB, GL_UNSIGNED_BYTE, pBitmap->data);

	// Lastly, we need to tell OpenGL the quality of our texture map.  GL_LINEAR is the smoothest.	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexEnvf(GL_TEXTURE, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	// Now we need to free the bitmap data that we loaded since openGL stored it as a texture

	if (pBitmap)										// If we loaded the bitmap
	{
		if (pBitmap->data)								// If there is texture data
		{
			free(pBitmap->data);						// Free the texture data, we don't need it anymore
		}

		free(pBitmap);									// Free the bitmap structure
	}
}
*/

BOOL CDisplay::CreateOpenGL(HDC hDC)
{
	m_hDC=hDC;
	if(SetWindowPixelFormat(hDC)==FALSE)
		return FALSE;
	if(CreateViewGLContext(hDC)==FALSE)
		return FALSE;
	if(wglMakeCurrent(hDC,m_hGLContext)==FALSE)
		return FALSE;

	//Set object feature
//polygon mode
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);	//Polygon
	glPolygonOffset(1.0, 1.0);
//	glPolygonOffset(2.0,0.0);					//Offset
//	glEnable(GL_POLYGON_OFFSET_FILL);

//culling back face
//	glEnable(GL_CULL_FACE);//启用culling
//	glCullFace(GL_BACK);//default
//	glFrontFace(GL_CCW);//default


	//Set depth test
	glEnable(GL_DEPTH_TEST);					//Depth test
	glClearDepth(1.0f);							//clear z-buffer
	
	//Set color
//	COLORREF color = ::GetSysColor(COLOR_3DFACE);//Bg color
//	glClearColor((float)GetRValue(color)/255.0f,(float)GetGValue(color)/255.0f,(float)GetBValue(color)/255.0f,1.0);

//	glClearColor(0.f,0.f,0.f,1.f);

//antilizing
//	glEnable(GL_LINE_SMOOTH);
//	glEnable(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//	glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);

//shademodel
//	glShadeModel(GL_FLAT);						//color mode 比GL_SMOOTH速度快

	//Set Material properties
	//front face
	GLfloat matFrontSpecular[] = {0.478814f, 0.457627f, 0.5f};
	GLfloat matFrontAmbient[] =  {0.25f, 0.652647f, 0.254303f};
	GLfloat matFrontDiffuse[] =  {0.25f, 0.652647f, 0.254303f};
	GLfloat matFrontShininess = 25.f;

	glMaterialfv(GL_FRONT, GL_DIFFUSE, matFrontDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, matFrontSpecular);
	glMaterialfv(GL_FRONT, GL_AMBIENT, matFrontAmbient);
	glMaterialf(GL_FRONT, GL_SHININESS, matFrontShininess);
	//back face
	GLfloat matBackSpecular[] = {0.1596f,   0.1525f, 0.1667f};
	GLfloat matBackAmbient[] =  {0.175f,   0.3263f,   0.2772f};
	GLfloat matBackDiffuse[] =  {0.175f,   0.3263f,   0.2772f};
	GLfloat matBackShininess = 100.f;
	glMaterialfv(GL_BACK, GL_DIFFUSE, matBackDiffuse);
	glMaterialfv(GL_BACK, GL_SPECULAR, matBackSpecular);
	glMaterialfv(GL_BACK, GL_AMBIENT, matBackAmbient);
	glMaterialf(GL_BACK, GL_SHININESS, matBackShininess);

//	glEnable(GL_COLOR_MATERIAL);  	

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);

	//Set the lights
	//light0
	GLfloat	light_ambient[] = {0.3f,0.3f,0.3f,1.0f};//环境光
	GLfloat	light_diffuse[] = {0.8f,0.8f,0.8f,1.0f};//漫反射光
	GLfloat light_specular[]= {0.8f,0.8f,0.8f,1.0f};//镜面光
	GLfloat light_position[] =	{5.0f,3.0f,1.0f,1.0f};  //位置
	glLightfv( GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv( GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv( GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv( GL_LIGHT0, GL_POSITION, light_position);

//	GLfloat	light_ambient1[] = {0.2f,0.2f,0.2f,1.0f};//环境光
	GLfloat	light_diffuse1[] = {0.4f,0.4f,0.4f,1.0f};//漫反射光
	GLfloat light_specular1[]= {0.5f,0.5f,0.5f,1.0f};//镜面光
	GLfloat light_position1[] =	{-4.0f,-4.0f,-2.0f,1.0f};  //位置
	glLightfv( GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv( GL_LIGHT1, GL_DIFFUSE, light_diffuse1);
//	glLightfv( GL_LIGHT1, GL_SPECULAR, light_specular1);
//	glLightfv( GL_LIGHT1, GL_POSITION, light_position1);
	
	//Enable light
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);

	buildList();

	//for texture
//	glEnable(GL_TEXTURE_2D);
//	createTexture(m_Texture, "texture.bmp", 0);

	/*
	// Allocate and init memory for the image array and point to it from pTexture
	int size=32;
	unsigned int *pTexture = NULL;
	pTexture = new unsigned int [size * size * 3];
	memset(pTexture, 0, size * size * 3 * sizeof(unsigned int));
	for(int i=0;i<size;i+=2)
		for(int j=i%2;j<size;j+=2)
		{
			int k=(i*size+j)*3;
			pTexture[k]=pTexture[k+1]=pTexture[k+2]=4294967295;
		}


	// Register the texture with OpenGL and bind it to the texture ID
	glGenTextures(1, &m_Texture[0]);								
	glBindTexture(GL_TEXTURE_2D, m_Texture[0]);
		
	// Create the texture and store it on the video card
	glTexImage2D(GL_TEXTURE_2D, 0, 3, size, size, 0, GL_RGB, GL_UNSIGNED_INT, pTexture);						
	
	// Set the texture quality
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	// Since we stored the texture space with OpenGL, we can delete the image data
	delete [] pTexture;

*/
  
//	glDisable(GL_TEXTURE_2D);

	wglMakeCurrent(NULL,NULL);
	return TRUE;

}

BOOL CDisplay::SetWindowPixelFormat(HDC hDC)
{
	PIXELFORMATDESCRIPTOR pixelDesc;
	
	pixelDesc.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pixelDesc.nVersion = 1;
	
	pixelDesc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER | PFD_STEREO_DONTCARE;
	
	pixelDesc.iPixelType = PFD_TYPE_RGBA;
	pixelDesc.cColorBits = 32;
	pixelDesc.cRedBits = 8;
	pixelDesc.cRedShift = 16;
	pixelDesc.cGreenBits = 8;
	pixelDesc.cGreenShift = 8;
	pixelDesc.cBlueBits = 8;
	pixelDesc.cBlueShift = 0;
	pixelDesc.cAlphaBits = 0;
	pixelDesc.cAlphaShift = 0;
	pixelDesc.cAccumBits = 64;
	pixelDesc.cAccumRedBits = 16;
	pixelDesc.cAccumGreenBits = 16;
	pixelDesc.cAccumBlueBits = 16;
	pixelDesc.cAccumAlphaBits = 0;
	pixelDesc.cDepthBits = 32;
	pixelDesc.cStencilBits = 8;
	pixelDesc.cAuxBuffers = 0;
	pixelDesc.iLayerType = PFD_MAIN_PLANE;
	pixelDesc.bReserved = 0;
	pixelDesc.dwLayerMask = 0;
	pixelDesc.dwVisibleMask = 0;
	pixelDesc.dwDamageMask = 0;
	
	m_GLPixelIndex = ChoosePixelFormat(hDC,&pixelDesc);
	if(m_GLPixelIndex == 0) // Choose default
	{
		m_GLPixelIndex = 1;
		if(DescribePixelFormat(hDC,m_GLPixelIndex,
			sizeof(PIXELFORMATDESCRIPTOR),&pixelDesc)==0)
			return FALSE;
	}
	
	if(!SetPixelFormat(hDC,m_GLPixelIndex,&pixelDesc))
		return FALSE;
	
	return TRUE;
}

BOOL CDisplay::CreateViewGLContext(HDC hDC)
{
	m_hGLContext = wglCreateContext(hDC);
	if(m_hGLContext==NULL)
		return FALSE;
	return TRUE;
}

void CDisplay::buildList()
{
	//建一个三维坐标架
	m_ListXYZ=glGenLists(1);
	glNewList(m_ListXYZ,GL_COMPILE);
	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
//x轴
	glBegin(GL_LINES);
		// light red x axis arrow
		glColor3f(1.f,0.f,0.f);
		glVertex3f(0.0f,0.0f,0.0f);
		glVertex3f(1.0f,0.0f,0.0f);
		// x letter
		glVertex3f(1.1f,0.1f,0.0f);
		glVertex3f(1.3f,-0.1f,0.0f);
		glVertex3f(1.3f,0.1f,0.0f);
		glVertex3f(1.1f,-0.1f,0.0f);
//y轴
		glColor3f(0.f,1.f,0.f);
		glVertex3f(0.0f,0.0f,0.0f);
		glVertex3f(0.0f,1.0f,0.0f);
		// y letter
		glVertex3f(-0.1f,1.3f,0.0f);
		glVertex3f(0.f,1.2f,0.0f);
		glVertex3f(0.1f,1.3f,0.0f);
		glVertex3f(0.f,1.2f,0.0f);
		glVertex3f(0.f,1.2f,0.0f);
		glVertex3f(0.f,1.1f,0.0f);
//z
		glColor3f(0.f,0.f,1.f);
		glVertex3f(0.0f,0.0f,0.0f);
		glVertex3f(0.0f,0.0f,1.0f);
		// z letter 
		glVertex3f(0.0f,-0.1f,1.3f);
		glVertex3f(0.0f,0.1f,1.3f);
		glVertex3f(0.0f,0.1f,1.3f);
		glVertex3f(0.0f,-0.1f,1.1f);
		glVertex3f(0.0f,-0.1f,1.1f);
		glVertex3f(0.0f,0.1f,1.1f);
	glEnd();

	glEnable(GL_DEPTH_TEST);
	glPopAttrib();
	glEndList();

	//生成坐标网格m_ListNet
	m_ListNet=glGenLists(1);
	glNewList(m_ListNet,GL_COMPILE);
	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);
	
	glBegin(GL_LINES);
		glColor3f(0.1f,0.1f,0.1f);
		glLineWidth(1.5f);
		glVertex3f(1.0,0.0,0.0);
		glVertex3f(-1.0,0.0,0.0);
		glVertex3f(0.0,0.0,1.0);
		glVertex3f(0.0,0.0,-1.0);

		glColor3f(0.7f,0.7f,0.7f);
		glLineWidth(1.0f);
		glVertex3f(1.0,0.0,1.0);
		glVertex3f(-1.0,0.0,1.0);
		glVertex3f(1.0,0.0,0.8f);
		glVertex3f(-1.0,0.0,0.8f);
		glVertex3f(1.0,0.0,0.6f);
		glVertex3f(-1.0,0.0,0.6f);
		glVertex3f(1.0,0.0,0.4f);
		glVertex3f(-1.0,0.0,0.4f);
		glVertex3f(1.0,0.0,0.2f);
		glVertex3f(-1.0,0.0,0.2f);

		glVertex3f(1.0,0.0,-1.0);
		glVertex3f(-1.0,0.0,-1.0);
		glVertex3f(1.0,0.0,-0.8f);
		glVertex3f(-1.0,0.0,-0.8f);
		glVertex3f(1.0,0.0,-0.6f);
		glVertex3f(-1.0,0.0,-0.6f);
		glVertex3f(1.0,0.0,-0.4f);
		glVertex3f(-1.0,0.0,-0.4f);
		glVertex3f(1.0,0.0,-0.2f);
		glVertex3f(-1.0,0.0,-0.2f);
		
		glVertex3f(1.0,0.0,1.0);
		glVertex3f(1.0,0.0,-1.0);
		glVertex3f(0.8f,0.0,1.0);
		glVertex3f(0.8f,0.0,-1.0);
		glVertex3f(0.6f,0.0,1.0);
		glVertex3f(0.6f,0.0,-1.0);
		glVertex3f(0.4f,0.0,1.0);
		glVertex3f(0.4f,0.0,-1.0);
		glVertex3f(0.2f,0.0,1.0);
		glVertex3f(0.2f,0.0,-1.0);

		glVertex3f(-1.0,0.0,1.0);
		glVertex3f(-1.0,0.0,-1.0);
		glVertex3f(-0.8f,0.0,1.0);
		glVertex3f(-0.8f,0.0,-1.0);
		glVertex3f(-0.6f,0.0,1.0);
		glVertex3f(-0.6f,0.0,-1.0);
		glVertex3f(-0.4f,0.0,1.0);
		glVertex3f(-0.4f,0.0,-1.0);
		glVertex3f(-0.2f,0.0,1.0);
		glVertex3f(-0.2f,0.0,-1.0);

	glEnd();
	glPopAttrib();

	glEndList();
}

void CDisplay::LookAt(Vector3D vEye,Vector3D vGaze,Vector3D vUp)
{
	m_vEyePoint=vEye;
	m_vGazePoint=vGaze;
	m_vUp=vUp;
}
void CDisplay::Reshape(int cx, int cy)
{
	if((cx<=0)||(cy<=0))
		return;
	m_Width=cx;	m_Height=cy;
	if(m_Interface == DISPLAY_INTERFACE_OPENGL)
	{
		wglMakeCurrent(m_hDC,m_hGLContext);
		double aspect;
		aspect = (double)cx/(double)cy;
		glViewport(0,0,cx,cy);
//		glViewport(100,100,100,100);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(40,aspect,0.1f,100.0f);
		glMatrixMode(GL_MODELVIEW);
		wglMakeCurrent(NULL,NULL);
	}
}
void CDisplay::Lock()
{
	if(m_Interface == DISPLAY_INTERFACE_OPENGL)
	{
		wglMakeCurrent(m_hDC,m_hGLContext);
		glClearColor(m_BGcolor[0],m_BGcolor[1],m_BGcolor[2],1.f);
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();//因为使用gluLookAt，所以不用保留以前的View矩阵
		gluLookAt(m_vEyePoint.x,m_vEyePoint.y,m_vEyePoint.z,m_vGazePoint.x,m_vGazePoint.y,m_vGazePoint.z,m_vUp.x,m_vUp.y,m_vUp.z);
	//	glCallList(m_ListNet);
	}

}

void CDisplay::Unlock()
{
	if(m_Interface == DISPLAY_INTERFACE_OPENGL)
	{
		glPushMatrix();
		double d=m_vEyePoint.length()/30.0;
		glScaled(d,d,d);
//		glCallList(m_ListXYZ);
		glPopMatrix();
//		glFlush();
//		glFinish();
		SwapBuffers(m_hDC);
		wglMakeCurrent(NULL,NULL);
	}
}

void CDisplay::GLBegin(BYTE mode)
{
	if(m_Interface == DISPLAY_INTERFACE_OPENGL)
	{
		switch(mode)
		{
		case DISPLAY_POINTS:
			glBegin(GL_POINTS);
			break;
		case DISPLAY_LINES:
			glBegin(GL_LINES);
			break;
		case DISPLAY_TRIANGLES:
			glBegin(GL_TRIANGLES);
			break;
		case DISPLAY_QUADS:
			glBegin(GL_QUADS);
			break;
		case DISPLAY_POLYGON:
			glBegin(GL_POLYGON);
			break;
		}
		return;
	}
	if(m_Interface == DISPLAY_INTERFACE_DIRECT3D)
	{
		return;
	}
}

void CDisplay::GLEnd()
{
	if(m_Interface == DISPLAY_INTERFACE_OPENGL)
		glEnd();
}

void CDisplay::GLVertex(Vector3D v)
{
	if(m_Interface == DISPLAY_INTERFACE_OPENGL)
		glVertex3d(v.x,v.y,v.z);

}

void CDisplay::GLNormal(Vector3D v)
{
	glNormal3d(v.x,v.y,v.z);
}
void CDisplay::GLTexture(double s,double t)
{
	glTexCoord2d(s,t);
}
void CDisplay::GLPointSize(float f)
{
	glPointSize(f);
}

void CDisplay::GLLineWidth(float f)
{
	glLineWidth(f);
}

void CDisplay::GLColor3f(float r, float g, float b)
{
	glColor3f(r,g,b);
}

void CDisplay::GLColor3ub(BYTE r, BYTE g, BYTE b)
{
	glColor3ub(r,g,b);
}
void CDisplay::GLBindTexture(int iTexture)
{
	glBindTexture(GL_TEXTURE_2D,m_Texture[iTexture]);
}

void CDisplay::SetBGColor(float r, float g, float b)
{
	if(m_Interface == DISPLAY_INTERFACE_OPENGL)
	{
		m_BGcolor[0]=r;
		m_BGcolor[1]=g;
		m_BGcolor[2]=b;
	}
}

void CDisplay::SetMeterial(BYTE meterial)
{
	GLfloat matFrontAmbient[4];
	GLfloat matFrontDiffuse[4];
	GLfloat matFrontSpecular[4];
	GLfloat matFrontShininess;
	matFrontAmbient[3] = 1.f;
	matFrontDiffuse[3] = 1.f;
	matFrontSpecular[3] = 1.f;

//	wglMakeCurrent(m_hDC,m_hGLContext);
	

	switch(meterial)
	{
	case METERIAL_BRASS:
		matFrontAmbient[0] = 0.329412f;
		matFrontDiffuse[0] = 0.780392f;
		matFrontSpecular[0] = 0.992157f;
		matFrontAmbient[1] = 0.223529f;
		matFrontDiffuse[1] = 0.568627f;
		matFrontSpecular[1] = 0.941176f;
		matFrontAmbient[2] = 0.027451f;
		matFrontDiffuse[2] = 0.113725f;
		matFrontSpecular[2] = 0.807843f;
		matFrontShininess = 27.8974f;
		break;

	case METERIAL_BRONZE:
		matFrontAmbient[0] = 0.2125f;
		matFrontDiffuse[0] = 0.714f;
		matFrontSpecular[0] = 0.393548f;
		matFrontAmbient[1] = 0.1275f;
		matFrontDiffuse[1] = 0.4284f;
		matFrontSpecular[1] = 0.271906f;
		matFrontAmbient[2] = 0.054f;
		matFrontDiffuse[2] = 0.18144f;
		matFrontSpecular[2] = 0.166721f;
		matFrontShininess = 25.6f;
		break;

	case METERIAL_CHROME:
		matFrontAmbient[0] = 0.25f;
		matFrontDiffuse[0] = 0.4f;
		matFrontSpecular[0] = 0.774597f;
		matFrontAmbient[1] = 0.25f;
		matFrontDiffuse[1] = .4f;
		matFrontSpecular[1] = 0.774597f;
		matFrontAmbient[2] = 0.25f;
		matFrontDiffuse[2] = 0.4f;
		matFrontSpecular[2] = 0.774597f;
		matFrontShininess = 76.8f;
		break;

	case METERIAL_COPPER:
		matFrontAmbient[0] = 0.19125f;
		matFrontDiffuse[0] = 0.7038f;
		matFrontSpecular[0] = 0.256777f;
		matFrontAmbient[1] = 0.0735f;
		matFrontDiffuse[1] = 0.27048f;
		matFrontSpecular[1] = 0.137622f;
		matFrontAmbient[2] = 0.0225f;
		matFrontDiffuse[2] = 0.0828f;
		matFrontSpecular[2] = 0.086014f;
		matFrontShininess = 12.84f;
		break;

	case METERIAL_GOLD:
		matFrontAmbient[0] = 0.24725f;
		matFrontDiffuse[0] = 0.75164f;
		matFrontSpecular[0] = 0.628281f;
		matFrontAmbient[1] = 0.1995f;
		matFrontDiffuse[1] = 0.60648f;
		matFrontSpecular[1] = 0.555802f;
		matFrontAmbient[2] = 0.0745f;
		matFrontDiffuse[2] = 0.22648f;
		matFrontSpecular[2] = 0.366065f;
		matFrontShininess = 51.2f;
		break;

	case METERIAL_P_GOLD:
		matFrontAmbient[0] = 0.24725f;
		matFrontDiffuse[0] = 0.34615f;
		matFrontSpecular[0] = 0.797357f;
		matFrontAmbient[1] = 0.2245f;
		matFrontDiffuse[1] = 0.3143f;
		matFrontSpecular[1] = 0.723991f;
		matFrontAmbient[2] = 0.0645f;
		matFrontDiffuse[2] = 0.0903f;
		matFrontSpecular[2] = 0.208006f;
		matFrontShininess = 83.2f;
		break;

	case METERIAL_PEWTER:
		matFrontAmbient[0] = 0.105882f;
		matFrontDiffuse[0] = 0.427451f;
		matFrontSpecular[0] = 0.333333f;
		matFrontAmbient[1] = 0.058824f;
		matFrontDiffuse[1] = 0.470588f;
		matFrontSpecular[1] = 0.333333f;
		matFrontAmbient[2] = 0.113725f;
		matFrontDiffuse[2] = 0.541176f;
		matFrontSpecular[2] = 0.521569f;
		matFrontShininess = 9.84615f;
		break;

	case METERIAL_SILVER:
		matFrontAmbient[0] = 0.19225f;
		matFrontDiffuse[0] = 0.50754f;
		matFrontSpecular[0] = 0.508273f;
		matFrontAmbient[1] = 0.19225f;
		matFrontDiffuse[1] = 0.50754f;
		matFrontSpecular[1] = 0.508273f;
		matFrontAmbient[2] = 0.19225f;
		matFrontDiffuse[2] = 0.50754f;
		matFrontSpecular[2] = 0.508273f;
		matFrontShininess = 51.2f;
		break;

	case METERIAL_P_SILVER:
		matFrontAmbient[0] = 0.23125f;
		matFrontDiffuse[0] = 0.2775f;
		matFrontSpecular[0] = 0.773911f;
		matFrontAmbient[1] = 0.23125f;
		matFrontDiffuse[1] = 0.2775f;
		matFrontSpecular[1] = 0.773911f;
		matFrontAmbient[2] = 0.23125f;
		matFrontDiffuse[2] = 0.2775f;
		matFrontSpecular[2] = 0.773911f;
		matFrontShininess = 89.6f;
		break;

	case METERIAL_EMERALD:
		matFrontAmbient[0] = 0.0215f;
		matFrontDiffuse[0] = 0.07568f;
		matFrontSpecular[0] = 0.633f;
		matFrontAmbient[1] = 0.1745f;
		matFrontDiffuse[1] = 0.61424f;
		matFrontSpecular[1] = 0.727811f;
		matFrontAmbient[2] = 0.0215f;
		matFrontDiffuse[2] = 0.07568f;
		matFrontSpecular[2] = 0.633f;
		matFrontAmbient[3] = 0.55f;
		matFrontDiffuse[3] = 0.55f;
		matFrontSpecular[3] = 0.55f;
		matFrontShininess = 76.8f;
		break;

	case METERIAL_JADE:
		matFrontAmbient[0] = 0.135f;
		matFrontDiffuse[0] = 0.54f;
		matFrontSpecular[0] = 0.316228f;
		matFrontAmbient[1] = 0.2225f;
		matFrontDiffuse[1] = 0.89f;
		matFrontSpecular[1] = 0.316228f;
		matFrontAmbient[2] = 0.1575f;
		matFrontDiffuse[2] = 0.63f;
		matFrontSpecular[2] = 0.316228f;
		matFrontAmbient[3] = 0.95f;
		matFrontDiffuse[3] = 0.95f;
		matFrontSpecular[3] = 0.95f;
		matFrontShininess = 12.8f;
		break;

	case METERIAL_OBSIDIAN:
		matFrontAmbient[0] = 0.05375f;
		matFrontDiffuse[0] = 0.18275f;
		matFrontSpecular[0] = 0.332741f;
		matFrontAmbient[1] = 0.05f;
		matFrontDiffuse[1] = 0.17f;
		matFrontSpecular[1] = 0.328634f;
		matFrontAmbient[2] = 0.06625f;
		matFrontDiffuse[2] = 0.22525f;
		matFrontSpecular[2] = 0.346435f;
		matFrontAmbient[3] = 0.82f;
		matFrontDiffuse[3] = 0.82f;
		matFrontSpecular[3] = 0.82f;
		matFrontShininess = 38.4f;
		break;

	case METERIAL_PEARL:
		matFrontAmbient[0] = 0.25f;
		matFrontDiffuse[0] = 1.0f;
		matFrontSpecular[0] = 0.296648f;
		matFrontAmbient[1] = 0.20725f;
		matFrontDiffuse[1] = 0.829f;
		matFrontSpecular[1] = 0.296648f;
		matFrontAmbient[2] = 0.20725f;
		matFrontDiffuse[2] = 0.829f;
		matFrontSpecular[2] = 0.296648f;
		matFrontAmbient[3] = 0.922f;
		matFrontDiffuse[3] = 0.922f;
		matFrontSpecular[3] = 0.922f;
		matFrontShininess = 11.264f;
		break;

	case METERIAL_RUDY:
		matFrontAmbient[0] = 0.1745f;
		matFrontDiffuse[0] = 0.61424f;
		matFrontSpecular[0] = 0.727811f;
		matFrontAmbient[1] = 0.01175f;
		matFrontDiffuse[1] = 0.04136f;
		matFrontSpecular[1] = 0.626959f;
		matFrontAmbient[2] = 0.01175f;
		matFrontDiffuse[2] = 0.04136f;
		matFrontSpecular[2] = 0.626959f;
		matFrontAmbient[3] = 0.55f;
		matFrontDiffuse[3] = 0.55f;
		matFrontSpecular[3] = 0.55f;
		matFrontShininess = 76.8f;
		break;

	case METERIAL_TURQUOISE:
		matFrontAmbient[0] = 0.1f;
		matFrontDiffuse[0] = 0.396f;
		matFrontSpecular[0] = 0.297254f;
		matFrontAmbient[1] = 0.18725f;
		matFrontDiffuse[1] = 0.74151f;
		matFrontSpecular[1] = 0.30829f;
		matFrontAmbient[2] = 0.1745f;
		matFrontDiffuse[2] = 0.69102f;
		matFrontSpecular[2] = 0.306678f;
		matFrontAmbient[3] = 0.8f;
		matFrontDiffuse[3] = 0.8f;
		matFrontSpecular[3] = 0.8f;
		matFrontShininess = 12.8f;
		break;

	default:
		matFrontAmbient[0] = 0.25f;
		matFrontDiffuse[0] = 0.25f;
		matFrontSpecular[0] = 0.478814f;
		matFrontAmbient[1] = 0.652647f;
		matFrontDiffuse[1] = 0.652647f;
		matFrontSpecular[1] = 0.457627f;
		matFrontAmbient[2] = 0.254303f;
		matFrontDiffuse[2] = 0.254303f;
		matFrontSpecular[2] = 0.5f;
		matFrontShininess = 25.f;
		break;
	}

	glMaterialfv(GL_FRONT, GL_DIFFUSE, matFrontDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, matFrontSpecular);
	glMaterialfv(GL_FRONT, GL_AMBIENT, matFrontAmbient);
	glMaterialf(GL_FRONT, GL_SHININESS, matFrontShininess);

//	wglMakeCurrent(NULL,NULL);
}

void CDisplay::RenderWireFrame()
{
	if(m_Interface == DISPLAY_INTERFACE_OPENGL)
	{
		glShadeModel(GL_FLAT); 
		glDisable(GL_LIGHTING);
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	}
}

void CDisplay::RenderHidenLine(bool bBack)
{
	if(m_Interface == DISPLAY_INTERFACE_OPENGL)
	{
		if(bBack)
		{
			glShadeModel(GL_FLAT); 
			glDisable(GL_LIGHTING);
			glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);	//Polygon
			glEnable(GL_POLYGON_OFFSET_FILL);
			glColor3f(m_BGcolor[0],m_BGcolor[1],m_BGcolor[2]);
		}
		else
		{
			glDisable(GL_LIGHTING);
			glDisable(GL_POLYGON_OFFSET_FILL);
			glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);	//Polygon
		}
	}
}
void CDisplay::RenderFlatShade(bool bLine)
{
	if(m_Interface == DISPLAY_INTERFACE_OPENGL)
	{
		if(bLine)
			glEnable(GL_POLYGON_OFFSET_FILL);
		glShadeModel(GL_FLAT); 
		glEnable(GL_LIGHTING);
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	}
}

void CDisplay::RenderSmoothShade()
{
	if(m_Interface == DISPLAY_INTERFACE_OPENGL)
	{
		glShadeModel(GL_SMOOTH); 
		glEnable(GL_LIGHTING);
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	}
}

void CDisplay::BeginPick()
{
	wglMakeCurrent(m_hDC,m_hGLContext);
	glPushAttrib(GL_COLOR_BUFFER_BIT);//保留以前的背景色
	glClearColor(0.0,0.0,0.0,1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPopAttrib();
	glLoadIdentity();//因为使用gluLookAt，所以不用保留以前的View矩阵
	gluLookAt(m_vEyePoint.x,m_vEyePoint.y,m_vEyePoint.z,m_vGazePoint.x,m_vGazePoint.y,m_vGazePoint.z,m_vUp.x,m_vUp.y,m_vUp.z);

	glPushAttrib(GL_LIGHTING_BIT);//拾取的绘制过程不需要光照（glPolygonMode是GL_FILL还是GL_LINE以及是否glDisable(GL_DEPTH_TEST)有待考虑）
	glDisable(GL_LIGHTING);
}

int CDisplay::EndPick(int x,int y)//返回值为被选中物体的个数
{
	glPopAttrib();
	if((x<HALFPICKRANGE)||(y<HALFPICKRANGE)||(x>m_Width-HALFPICKRANGE)||(y>m_Height-HALFPICKRANGE))
		return 0;
	glFinish();
	glReadPixels(x-HALFPICKRANGE,m_Height-y-HALFPICKRANGE,PICKRANGE,PICKRANGE,GL_RGB,GL_UNSIGNED_BYTE,m_Pick);
	int i,j,k,n;
	UINT l;
	n=0;
	for(i=0;i<PICKRANGE;i++)
		for(j=0;j<PICKRANGE;j++)
		{
			l=m_Pick[i][j][0]*65536+m_Pick[i][j][1]*256+m_Pick[i][j][2];
			if(l>0)
			{
				if(n==0)
					m_PickNum[n++]=l;
				else
				{
					for(k=0;k<n;k++)
					{
						if(m_PickNum[k]==l)
							break;
					}
					if(m_PickNum[k]!=l)
						m_PickNum[n++]=l;
				}

			}
		}

//	SwapBuffers(m_hDC);//测试时保留，可以看到效果，最终将删除
	wglMakeCurrent(NULL,NULL);
	return n;
}