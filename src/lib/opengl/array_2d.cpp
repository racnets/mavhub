#include "array_2d.h"

#include <iostream>	//cout
#include <cstring>	//strlen
#include <cstdio>	//vfprintf
#include <cstdarg>	//va_list
#include <sstream>  //stringstream
#include <map>      //map


#include <GL/glut.h>
#include <GL/freeglut_ext.h>	//glutMainLoopEvent
#include <GL/glx.h>     // Header file fot the glx libraries.

namespace hub {
namespace opengl {
namespace array_2d {

#define VERBOSE 1

// little debug makro
#if VERBOSE >= 1
#define dout if(1) cout
#else
#define dout if(0) cout
#endif

using namespace std;

Array2D::status_t Array2D::status;
int Array2D::window;
Array2D::image_t Array2D::image;
unsigned int Array2D::texture;
int Array2D::data_packets_to_receive;

Array2D::Array2D(int *argc, char **argv, const int width, const int height) {
	
	glutInit(argc, argv);
  
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB );
	glutInitWindowSize(width, height); 

	window = glutCreateWindow("OpenGL 2D Array");

	glutDisplayFunc(display);
 	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard_input);
	glutSpecialFunc(handleArrowpress);
	
	status = CLEAR;
	//~ glutTimerFunc(100, update, 0);
	
	//~ glutMainLoop();
}

void Array2D::update(int value) {
	glutPostRedisplay();
	glutTimerFunc(100, update, 0);
}

void Array2D::display() {
	/* setup 2D mode */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	int window_width = glutGet(GLUT_WINDOW_WIDTH);
	int window_height = glutGet(GLUT_WINDOW_HEIGHT);
	int left = (image.width == window_width) ? 0 : (image.width - window_width) / 2;
	int right = left + window_width;
	int top = (image.height == window_height) ? 0: (image.height - window_height) / 2;
	int bottom = top + window_height;
	
	glOrtho(left, right, bottom, top, 0, 1);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClear(GL_COLOR_BUFFER_BIT);

	/* draw image */
	/* generate texture */
	if (image.not_textured_yet) {
		imageToTexture(image, &texture);
		image.not_textured_yet = 0;
		status = COMPLETE;
	}
	/* show texture */
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_QUADS);
	//~ glColor3f(0.0f, 1.0f, 0.0f);
	glTexCoord2i(0,0); glVertex2i(0, 0);
	glTexCoord2i(0,1); glVertex2i(0, image.height);
	glTexCoord2i(1,1); glVertex2i(image.width, image.height);
	glTexCoord2i(1,0); glVertex2i(image.width, 0);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	glutSwapBuffers(); 
	glutMainLoopEvent();
}

void Array2D::keyboard_input(unsigned char key, int x, int y) {
	switch (key) {
		case 'q': {
			dout << "Got q,so quitting " << endl;
			glutLeaveMainLoop();
			glutDestroyWindow(window);
			window = 0; // needed because display function is called manually
			exit(0);
			break;
		}
	}
}

void Array2D::handleArrowpress(int key, int x, int y) {
	switch (key) {
		case GLUT_KEY_LEFT: {
			image.width--;
			image.not_textured_yet = 1;
			break;
		}
		case GLUT_KEY_RIGHT: {
			image.width++;
			image.not_textured_yet = 1;
			break;
		}
		case GLUT_KEY_UP: {
			image.height++;
			image.not_textured_yet = 1;
			break;
		}
		case GLUT_KEY_DOWN: {
			image.height--;
			image.not_textured_yet = 1;
			break;
		}
	}
}

void Array2D::reshape(int width, int height) {
	glViewport(0, 0, width, height);
}

void Array2D::setupImage(int width, int height, int packets_per_image) {
	image.width = width;
	image.height = height;
	image.data.clear();
	data_packets_to_receive = packets_per_image;
}

void Array2D::addImageData(uint8_t *data, uint8_t size, int sequence) {
	/* add new elements */
	image.data.insert(image.data.end(), data, data+size); 		
	vector<uint8_t>::iterator it = image.data.begin();

	if (sequence >= data_packets_to_receive) {
		image.not_textured_yet = 1;
	}
	
	status = WAITING;
}

/* Makes the image into a mipmapped texture, and returns the id of the texture */
void Array2D::imageToTexture(image_t &image, unsigned int* texture) {
	static int init = 0;
	
	/* setup the texture id once */
	if (init == 0) {
		glGenTextures(1, texture);
		glBindTexture(GL_TEXTURE_2D, *texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		init = 1;
	}

	/* load image data into texture buffer */
	uint8_t *data = &(*image.data.begin());
	glTexImage2D(GL_TEXTURE_2D,
				 0,
				 1,
				 image.width, image.height,
				 0,
				 GL_RED,
				 GL_UNSIGNED_BYTE,
				 data);
					  
}

} // namespace array_2d
} // namespace opengl
} // namespace hub
