#ifndef _HUB_OPENGL_ARRAY_1D_H_
#define _HUB_OPENGL_ARRAY_1D_H_

#include <list>
#include <map>

#include <GL/glut.h>


namespace hub {
namespace opengl {
namespace array_1d {	

class Array1D  {    
	public:
		Array1D(int *argc, char **argv, const int width = 640, const int height = 120);

		static void display();
		static void addData(uint16_t *datap, uint8_t size);
        
		enum field_t{
			RESULT,
			EXPOSURE,
			PIX_CLK,
			GAIN,
			AVG,
			ALGO,
			DEBUG,
			TIME,
			SHOW_RAW
		};

		enum status_t {
			CLEAR,
			PENDING, // marked for requesting(getter)
			WAITING, // sended request, but no answer so far
			COMPLETE,
			SENDING // marked for sending(setter)
		};

		struct param_t{
			param_t(status_t status = CLEAR, int value = 0) : status(status), value(value) {}
			status_t status;
/*			union {
				int i;
				unsigned int u;
				float f;
			} value;
			*/
			int value;
			
		};
		
		static std::map<field_t, param_t> params;

	protected:

	private:
		Array1D(const Array1D &);
		void operator=(const Array1D &);

		static int window;
		static int fontBase;
		static std::list<uint16_t> data;
		static void camera_view();
		static void keyboard_input(unsigned char key, int x, int y);
        static void handleArrowpress(int key, int x, int y);
 		static void reshape(int width, int height);
		static void buildFont(); 
		static void killFont();
		static void glPrint(const char *text, ...);
		static void displayMenu(float left, float top, float bottom);
		static void navigateMenu(int key);
};

} // namespace array_1d
} // namespace opengl
} // namespace hub

#endif
