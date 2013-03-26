#ifndef _HUB_OPENGL_ARRAY_2D_H_
#define _HUB_OPENGL_ARRAY_2D_H_

#include <vector>

#include <GL/glut.h>


namespace hub {
namespace opengl {
namespace array_2d {
	
class Array2D  {    
	public:
		Array2D(int *argc, char **argv, const int width = 640, const int height = 480);

		static void display();

		static void setupImage(int width, int height, int packets_per_image);
		static void addImageData(uint8_t *data, uint8_t size, int sequence);
		static void finalizeImage(void);
		
		enum status_t {
			CLEAR,
			PENDING, // marked for requesting(getter)
			WAITING, // sended request, but no answer so far
			COMPLETE,
			SENDING // marked for sending(setter)
		};
		static status_t status;

	protected:

	private:
		Array2D(const Array2D &);
		void operator=(const Array2D &);

		struct image_t{
			image_t(int width = 160, int height = 120) : width(width), height(height), not_textured_yet(0), data() {}
			int width;
			int height;
			int not_textured_yet;
			std::vector<uint8_t> data;
		};
		static image_t image;
		static unsigned int texture;
		static int data_packets_to_receive;
		
		static int window;
		static void keyboard_input(unsigned char key, int x, int y);
        static void handleArrowpress(int key, int x, int y);
 		static void reshape(int width, int height);
 		static void update(int value);
 		
 		static void imageToTexture(image_t &image, unsigned int *texture);
 		
};

} // namespace array_2d
} // namespace opengl
} // namespace hub

#endif
