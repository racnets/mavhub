#include "array_1d.h"

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
namespace array_1d {	

#define VERBOSE 1

// little debug makro
#if VERBOSE >= 1
#define dout if(1) cout
#else
#define dout if(0) cout
#endif

using namespace std;

int Array1D::window;
list<uint16_t> Array1D::data;
int Array1D::fontBase;
int menuIncrementSummand;
int menuIncrement;

enum visibility_t {
	off,
	on,
	edit,
	postEdit
};

struct entry_t{
	const string name;
	const string unit;
	const Array1D::field_t field;
	visibility_t visibility;
	int value;
	int update;
	const bool editable;
} *entries;

struct menu_t{
	entry_t *entries;
	visibility_t visibility;
	unsigned int position;
	const int minUpdateRate;
};

entry_t _entries[] = {
	{"result","",Array1D::RESULT,off,0,0,true},
	{"exposure","us",Array1D::EXPOSURE,off,0,0,true},
	{"pixel clock","kHz",Array1D::PIX_CLK,off,0,0,true},
	{"gain","",Array1D::GAIN,off,0,0,true},
	{"average","",Array1D::AVG,off,0,0,true},
	{"algo","",Array1D::ALGO,off,0,0,true},
	{"debug","",Array1D::DEBUG,off,0,0,false},
	{"time","ms",Array1D::DEBUG,off,0,0,false},
	{"show raw","",Array1D::SHOW_RAW,off,0,0,true}
};

menu_t menu = {
	entries = _entries,
	off,
	0,
	50
};


//io
map<Array1D::field_t, Array1D::param_t> createParamsMap() {
  map<Array1D::field_t,Array1D::param_t> m;
  m[Array1D::RESULT] = Array1D::param_t(Array1D::CLEAR, 0);
  m[Array1D::EXPOSURE] = Array1D::param_t(Array1D::CLEAR, 0);
  m[Array1D::PIX_CLK] = Array1D::param_t(Array1D::CLEAR, 0);
  m[Array1D::GAIN] = Array1D::param_t(Array1D::CLEAR, 0);
  m[Array1D::AVG] = Array1D::param_t(Array1D::CLEAR, 0);
  m[Array1D::ALGO] = Array1D::param_t(Array1D::CLEAR, 0);
  m[Array1D::DEBUG] = Array1D::param_t(Array1D::CLEAR, 0);
  m[Array1D::TIME] = Array1D::param_t(Array1D::CLEAR, 0);
  m[Array1D::SHOW_RAW] = Array1D::param_t(Array1D::CLEAR, 0);
  return m;
}

map<Array1D::field_t,Array1D::param_t> Array1D::params = createParamsMap();

Array1D::Array1D(int *argc, char **argv, const int width, const int height) {
	glutInit(argc, argv);
  
	int screenWidth = glutGet(GLUT_SCREEN_WIDTH);
	int screenHeight = glutGet(GLUT_SCREEN_HEIGHT);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB );
	glutInitWindowSize(width, height); 
	glutInitWindowPosition((screenWidth - width) / 2, (screenHeight - height) / 2);
	window = glutCreateWindow("OpenGL 1D Array");

	buildFont();
	glutDisplayFunc(display);
 	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard_input);
	glutSpecialFunc(handleArrowpress);
}


void Array1D::addData(uint16_t *datap, uint8_t size) {
	data.assign(datap, datap + size);
}

void Array1D::displayMenu(float left, float top, float bottom) {
    stringstream oss;

	//save position
	glPushMatrix();
	//set new position
	glLoadIdentity();
	glTranslatef(left, top + 16.0f , 0.0f);
	//set color
	glColor3f(1.0f, 1.0f, 1.0f);
	
	switch (menu.visibility) {
		case on: {
			oss << "...";
			//for all entries
			for (unsigned int i=0; i < (sizeof(_entries) / sizeof(entry_t)); i++) {
				//which are visible
				if (menu.entries[i].visibility == on) {
					//show entry
					oss << "   " << menu.entries[i].name << ": " << menu.entries[i].value;
					//update value regularly
					if (menu.entries[i].update >= 1) menu.entries[i].update--;
					else if (menu.entries[i].update <= 1) {
						//update
						switch (params[menu.entries[i].field].status) {
							//request
							case CLEAR: {
								params[menu.entries[i].field].status = PENDING;
								break;
							}
							//WAITING - not responding?
							case WAITING: {
								static int waiting_time = menu.minUpdateRate;
								//after certain time repeat request
								if (!waiting_time--) {
									waiting_time = menu.minUpdateRate;
									params[menu.entries[i].field].status = PENDING;
								}
								break;
							}
							//got request response
							case COMPLETE: {
								params[menu.entries[i].field].status = CLEAR;
								menu.entries[i].value = params[menu.entries[i].field].value;
								menu.entries[i].update = menu.minUpdateRate;
								break;
							}
							default:;
						}
					}
				}
			}
			break;
		}
		case edit: {
			//compute increment
			if (menuIncrement >= 100 && menuIncrement < 1000)
				menuIncrementSummand = 10;
			else if (menuIncrement >= 1000)
				menuIncrementSummand = 100;
			else menuIncrementSummand = 1;
			
			//set other color
			glColor3f(0.0f, 1.0f, 0.0f);
			for (unsigned int i=0; i < (sizeof(_entries) / sizeof(entry_t)); i++) {
				if (((menu.position <= 3) && (i <= 3)) || ((menu.position > 3) && (i > 3))) {  
					if (menu.position == i) oss << " ->";
					else oss << "   ";
					switch (menu.entries[i].visibility) {
						case on: {
							oss << menu.entries[i].name << ": " << menu.entries[i].value;
							break;
						}
						case off: {
							oss << "(" << menu.entries[i].name << ": " << menu.entries[i].value << ")";
							break;
						}
						case edit: {
							//edit - no update - mark field
							oss << "< " << menu.entries[i].name << ": " << menu.entries[i].value << " >";				
							break;
						}
						case postEdit: {
							//ready with edit - send new value
							params[menu.entries[i].field].status = SENDING;
							params[menu.entries[i].field].value = menu.entries[i].value;
							menu.entries[i].visibility = on;
						}
					}
					if (menu.position == i) oss << "<- ";
					else oss << "   ";
				} else if ((i == 4) || (i == 3)) oss << "...";
			}
			break;
		}
		default:
			oss << "...";
	}
	glPrint(oss.str().c_str());

	//set new position
	glLoadIdentity();
	glTranslatef(left, bottom - 12.0f, 0.0f);
	//set color
	glColor3f(1.0f, 1.0f, 1.0f);
	oss.str("");
	oss << "time: " << params[TIME].value << "ms";
	glPrint(oss.str().c_str());
		
	//reload position
	glPopMatrix();
}

void Array1D::navigateMenu(int key) {
	static int old_key = 0;
	
	switch (menu.visibility) {
		case off: {
			switch (key) {
				case 'm': {
					menu.visibility = on;
					break;
				}
				case 'c': {
					menu.visibility = edit;
					break;
				}
				case ' ': {
					params[menu.entries[SHOW_RAW].field].status = SENDING;
					if (params[menu.entries[SHOW_RAW].field].value) params[menu.entries[SHOW_RAW].field].value = 0;
					else  params[menu.entries[SHOW_RAW].field].value = 1;
					break;
				}
			}
			break;
		}
		case on: {
			switch (key) {
				case 'm': {
					menu.visibility = off;
					break;
				}
				case 'c': {
					menu.visibility = edit;
					break;
				}
				case ' ': {
					params[menu.entries[SHOW_RAW].field].status = SENDING;
					if (params[menu.entries[SHOW_RAW].field].value) params[menu.entries[SHOW_RAW].field].value = 0;
					else  params[menu.entries[SHOW_RAW].field].value = 1;
					break;
				}
			}
			break;
		}
		case edit: {
			if (old_key != key) {
				menuIncrement = 0;
				menuIncrementSummand = 1;
			}
			old_key = key;
			
			switch (key) {
				case 'c':;
				case 27: {
					menu.visibility = on;
					break;
				}
				case '\r': {
					if (menu.entries[menu.position].visibility == on) menu.entries[menu.position].visibility = edit;
					else if (menu.entries[menu.position].visibility == edit) menu.entries[menu.position].visibility = postEdit;
					break;
				}
				case ' ': {
					if (menu.entries[menu.position].visibility == on) menu.entries[menu.position].visibility = off;
					else if (menu.entries[menu.position].visibility == off) menu.entries[menu.position].visibility = on;
					break;
				}
				case 1000 + GLUT_KEY_LEFT: {
					if (menu.position == 0) menu.position = sizeof(_entries) / sizeof(entry_t) - 1;
					else menu.position -= 1;
					break;
				}
				case 1000 + GLUT_KEY_RIGHT: {
					if (menu.position == (sizeof(_entries) / sizeof(entry_t) - 1)) menu.position = 0;
					else menu.position += 1;
					break;
				}
				case 1000 + GLUT_KEY_UP: {
					if (menu.entries[menu.position].visibility == edit) {
						if (menu.entries[menu.position].editable) {
							menu.entries[menu.position].value += menuIncrementSummand;
							menuIncrement += menuIncrementSummand;
						}									
					}
					break;
				}
				case 1000 + GLUT_KEY_DOWN: {
					if (menu.entries[menu.position].visibility == edit) {
						if (menu.entries[menu.position].editable) {
							menu.entries[menu.position].value -= menuIncrementSummand;
							menuIncrement += menuIncrementSummand;							
						}									
					}
					break;
				}
			}
		}
		default:;
	}
}

void Array1D::display() {
	static int sensorAbsent = 1;
	static int frame = 0;

	if (window && (sensorAbsent || !data.empty())) {
		/* setup 2D mode */
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		int window_width = glutGet(GLUT_WINDOW_WIDTH);
		int window_height = glutGet(GLUT_WINDOW_HEIGHT);
		int left = -10;
		int right = data.size() + 10;
		
		glOrtho(left, right, window_height, 0, 0, 1);
		glDisable(GL_DEPTH_TEST);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glClear(GL_COLOR_BUFFER_BIT);

		//display menu
		displayMenu(left + (right + abs(left))/50.0f, 0, window_height);

        //display sensor raw data
        if (!data.empty()) {
			sensorAbsent = 0;
		    // set camera view
			glTranslatef(0.0f, window_height / 2, 0.0f);

		    int pos = 0;
		    glBegin(GL_QUADS);
		    
		    while (!data.empty()) {
			    float color = static_cast<float>(data.front()) / 4096.0f;
			    data.pop_front();
			    if (params[menu.entries[GAIN].field].value) color *= params[menu.entries[GAIN].field].value;
				glColor3f(color, color, color);
			    glVertex3i(pos, -10, 0);
			    glVertex3f(pos + 1, -10, 0);
			    glVertex3f(pos + 1, 10, 0);
			    glVertex3f(pos, 10, 0);
			    pos++;
		    }
    		glEnd();
            frame++;
        } else {
			glTranslatef(-9.5f, window_height / 2, 0.0f);
            if (!frame) {
				glColor3f(1.0f, 0.0f, 0.0f);
                glPrint("no sensor data - check connection");
            } else {
				glColor3f(0.0f, 1.0f, 0.0f);
                glPrint("no sensor data - hit space to get");
			}
        }
		glutSwapBuffers();
	} else if (params[menu.entries[SHOW_RAW].field].value == 0) sensorAbsent = 1;

	glutMainLoopEvent();
}

void Array1D::keyboard_input(unsigned char key, int x, int y) {
	switch (key) {
		case 'q': {
			dout << "Got q,so quitting " << endl;
			glutLeaveMainLoop();
			glutDestroyWindow(window);
			window = 0; // needed because display function is called manually
			exit(0);
			break;
		}
		default: 
			navigateMenu(key);
	}
}

void Array1D::handleArrowpress(int key, int x, int y) {
	navigateMenu(key + 1000);
}


void Array1D::reshape(int width, int height) {
	glViewport(0, 0, width, height);
}

void Array1D::buildFont() 
{
    Display *dpy;
    XFontStruct *fontInfo;  // storage for our font.

    fontBase = glGenLists(224);                      // storage for 256 characters.
    
    // load the font.  what fonts any of you have is going
    // to be system dependent, but on my system they are
    // in /usr/X11R6/lib/X11/fonts/*, with fonts.alias and
    // fonts.dir explaining what fonts the .pcf.gz files
    // are.  in any case, one of these 2 fonts should be
    // on your system...or you won't see any text.
    
    // get the current display.  This opens a second
    // connection to the display in the DISPLAY environment
    // value, and will be around only long enough to load 
    // the font. 
    dpy = XOpenDisplay(NULL); // default to DISPLAY env.   

    fontInfo = XLoadQueryFont(dpy, "-adobe-helvetica-medium-r-normal--18-*-*-*-p-*-iso8859-1");
    if (fontInfo == NULL) {
	fontInfo = XLoadQueryFont(dpy, "fixed");
	if (fontInfo == NULL) {
	    printf("no X font available?\n");
	}
    }

    // after loading this font info, this would probably be the time
    // to rotate, scale, or otherwise twink your fonts.  

    // start at character 32 (space), get 224 characters (a few characters past z), and
    // store them starting at base.
    glXUseXFont(fontInfo->fid, 32, 224, fontBase);

    // free that font's info now that we've got the 
    // display lists.
    XFreeFont(dpy, fontInfo);

    // close down the 2nd display connection.
    XCloseDisplay(dpy);
}

void Array1D::killFont()                         // delete the font.
{
    glDeleteLists(fontBase, 224);                    // delete all 96 characters.
}

void Array1D::glPrint(const char *text, ...)                      // custom gl print routine.
{
	va_list ap;										// Pointer To List Of Arguments
	char txt[256];								// Holds Our String

	/* no text do nothing */
    if (text == NULL) return;

	va_start(ap, text);									// Parses The String For Variables
	    vsprintf(txt, text, ap);						// And Converts Symbols To Actual Numbers
	va_end(ap);											// Results Are Stored In Text
    
	glRasterPos2f(0.0f, 0.0f);
    glPushAttrib(GL_LIST_BIT);                  // alert that we're about to offset the display lists with glListBase
    glListBase(fontBase - 32);                      // sets the base character
    glCallLists(strlen(txt), GL_UNSIGNED_BYTE, txt); // draws the display list text.
    glPopAttrib();                              // undoes the glPushAttrib(GL_LIST_BIT);
}
} // namespace array_1d
} // namespace opengl
} // namespace hub
