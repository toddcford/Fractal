// Application infrastructure for platform-independent real-time CPU rendering.
// Requires the Glut library http://www.opengl.org/resources/libraries/glut/
//
// CS136 Support Code
// Morgan McGuire, Williams College Computer Science Department, 2014
// Released into the Public Domain
//
// Last modified 2014-08-01

#include <GLUT/glut.h>
#include <cassert>
#include <stdio.h>
#include "App.h"

App* App::instance = NULL;
static const float deviceGamma = 2.1f;
static const float fps = 30.0f;

App::App(const std::string& windowCaption, int imageWidth, int imageHeight, float zoom, float exposureConstant, float imageGamma) : 
  m_windowCaption(windowCaption), m_zoom(zoom), m_exposureConstant(exposureConstant), m_imageGamma(imageGamma),
  m_frameTimeMilliseconds(int(1000.0f / fps + 0.5f)),
  m_imageWidth(imageWidth), m_imageHeight(imageHeight) {

  assert((imageWidth > 0) && (imageHeight > 0));
  assert(zoom > 0);
  m_imageData.resize(m_imageWidth * m_imageHeight);
  instance = this;
}


void App::run() {
  int argc = 0;
    
  // See glut specs at https://www.opengl.org/resources/libraries/glut/spec3/spec3.html

  // Initialize OpenGL
  glutInit(&argc, NULL);
  glutInitWindowSize(int(ceil(float(m_imageWidth) * m_zoom)), int(ceil(float(m_imageHeight) * m_zoom)));
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutCreateWindow(m_windowCaption.c_str());
  glutKeyboardFunc(&staticOnKeyPress);
  glutDisplayFunc(&staticOnGraphics);
  glutMotionFunc(&staticOnMouseMotion);
  glutPassiveMotionFunc(&staticOnMouseMotion);
  glutReshapeFunc(&staticReshape);

  // Set the color scale applied as textures are uploaded to be the exposure constant
  glMatrixMode(GL_COLOR);
  glLoadIdentity();
  glScalef(m_exposureConstant, m_exposureConstant, m_exposureConstant);

  // Create a gamma correction color table for texture load
  if (m_imageGamma != deviceGamma) {
    const float gamma = deviceGamma / m_imageGamma;
    std::vector<Color> gammaTable(256);
    for (unsigned int i = 0; i < gammaTable.size(); ++i) {
      gammaTable[i] = pow(Color::white() * i / (gammaTable.size() - 1.0f), 1.0f / gamma);
    }
    glColorTable(GL_POST_COLOR_MATRIX_COLOR_TABLE, GL_RGB, gammaTable.size(), GL_RGB, GL_FLOAT, &gammaTable[0]);
    glEnable(GL_POST_COLOR_MATRIX_COLOR_TABLE);
  }
    
  // Create a texture, upload our image, and bind it (assume a
  // version of GL that supports NPOT textures)
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_imageWidth, m_imageHeight, 0, GL_RGB, GL_FLOAT, &m_imageData[0]);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
  glEnable(GL_TEXTURE_2D);

  // The vertices of a 2D quad mesh containing a single CCW square
  static const float corner[] = {0,0, 0,1, 1,1, 1,0};

  // Bind the quad mesh as the active geometry
  glVertexPointer(2, GL_FLOAT, 0, corner);
  glTexCoordPointer(2, GL_FLOAT, 0, corner);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
  // Set orthographic projection that stretches the unit square to the
  // dimensions of the image
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, 1, 1, 0, 0, 2);

  // Kick off the timer
  timerCallback(0);
  glutMainLoop();
}


void App::timerCallback(int value) {
  // Request animation
  glutPostRedisplay();
  glutTimerFunc(instance->m_frameTimeMilliseconds, &timerCallback, value);
}


void App::staticOnKeyPress(unsigned char key, int x, int y) {
  instance->m_mouseX = x;
  instance->m_mouseY = y;
  instance->onKeyPress(key);
}


void App::staticOnMousePress(int button, int state, int x, int y) {
  instance->m_mouseX = x;
  instance->m_mouseY = y;
  if (state == GLUT_DOWN) {
    instance->onMousePress(button);
  } else {
    instance->onMouseRelease(button);
  }
}


void App::staticOnMouseMotion(int x, int y) {
  instance->m_mouseX = x;
  instance->m_mouseY = y;
}


void App::staticReshape(int width, int height) {
  // Restore the size originally requested
  glutReshapeWindow(int(ceil(float(instance->m_imageWidth)  * instance->m_zoom)), 
		    int(ceil(float(instance->m_imageHeight) * instance->m_zoom)));
}


void App::onKeyPress(unsigned char key) {
  if (key == 27) { ::exit(0); }
}


void App::staticOnGraphics() {
  instance->onGraphics();

  // Upload the image
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, instance->m_imageWidth, instance->m_imageHeight, 0,
	       GL_RGB, GL_FLOAT, &instance->m_imageData[0]);

  // Draw a full-screen quad of the image
  glClear(GL_COLOR_BUFFER_BIT);
  glDrawArrays(GL_QUADS, 0, 4);
  glutSwapBuffers();
}


void App::saveImage(const std::string& filename) {
  assert(filename.size() > 4);
  if (filename.substr(filename.length() - 4) == ".tga") {
    saveTGA(filename);
  } else if (filename.substr(filename.length() - 4) == ".ppm") {
    savePPM(filename);
  } else {
    // Bad file format
    assert(false);
  }
}


static int PPMGammaCorrect(float v, float e, float g) {
  return int(pow(std::min(1.0f, std::max(0.0f, v * e)), 1.0f / g) * 255.0f);
}


void App::savePPM(const std::string& filename) const {
  FILE* file = fopen(filename.c_str(), "wt");
  fprintf(file, "P3 %d %d 255\n", m_imageWidth, m_imageHeight); 

  const float gamma = deviceGamma / m_imageGamma;

  for (int y = 0, i = 0; y < m_imageHeight; ++y) {
    fprintf(file, "\n# y = %d\n", y);                                                               
    for (int x = 0; x < m_imageWidth; ++x, ++i) {
      const Color& c = m_imageData[i];
      fprintf(file, "%d %d %d\n", 
	      PPMGammaCorrect(c.r, m_exposureConstant, gamma), 
	      PPMGammaCorrect(c.g, m_exposureConstant, gamma),
	      PPMGammaCorrect(c.b, m_exposureConstant, gamma));
    }
  }
  fclose(file);
}


void App::saveTGA(const std::string& filename) const {
  // http://www.paulbourke.net/dataformats/tga/
  FILE* file = fopen(filename.c_str(), "wb");

  // Header
  putc(0, file);
  putc(0, file);
  putc(2, file);                         /* uncompressed RGB */
  putc(0, file); putc(0, file);
  putc(0, file); putc(0, file);
  putc(0, file);
  putc(0, file); putc(0, file);           /* X origin */
  putc(0, file); putc(0, file);           /* y origin */
  printf("%d\n", m_imageWidth); 
  putc((m_imageWidth & 0x00FF), file);
  putc((m_imageWidth & 0xFF00) >> 8, file);

  putc((m_imageHeight & 0x00FF), file);
  putc((m_imageHeight & 0xFF00) >> 8, file);

  putc(24, file);                        /* 32 bit bitmap */
  putc( (1 << 5), file);                 /* origin = top left */

  const float gamma = deviceGamma / m_imageGamma;

  for (int y = 0, i = 0; y < m_imageHeight; ++y) {
    for (int x = 0; x < m_imageWidth; ++x, ++i) {
      const Color& c = m_imageData[i];
      putc(PPMGammaCorrect(c.b, m_exposureConstant, gamma), file);
      putc(PPMGammaCorrect(c.g, m_exposureConstant, gamma), file);
      putc(PPMGammaCorrect(c.r, m_exposureConstant, gamma), file);
      //putc(0xFF, file); // alpha = 1
    }
  }
    
  fclose(file);
}
