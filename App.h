// Application infrastructure for platform-independent real-time CPU rendering.
// Requires the Glut library http://www.opengl.org/resources/libraries/glut/
//
// CS136 Support Code
// Morgan McGuire, Williams College Computer Science Department, 2014
// Released into the Public Domain
//
// Last modified 2014-08-01
#ifndef App_h
#define App_h

#include <vector>
#include <cmath>
#include <string>

// Methods are intentionally implemented in the header file to allow
// the compiler to inline them for performance.

class Color {
 public:
  float       r;
  float       g;
  float       b;
    
 Color() : r(0), g(0), b(0) {}

 Color(float r, float g, float b) : r(r), g(g), b(b) {}

  Color operator*(float k) const {
    return Color(r * k, g * k, b * k);
  }

  Color operator/(float k) const {
    return Color(r / k, g / k, b / k);
  }

  Color operator*(const Color& c) const {
    return Color(r * c.r, g * c.g, b * c.b);
  }

  Color operator/(const Color& c) const {
    return Color(r / c.r, g / c.g, b / c.b);
  }

  Color operator+(const Color& c) const {
    return Color(r + c.r, g + c.g, b + c.b);
  }

  Color operator-(const Color& c) const {
    return Color(r - c.r, g - c.g, b - c.b);
  }

  static Color black() { return Color(0, 0, 0); }
  static Color red()   { return Color(1, 0, 0); }
  static Color yellow(){ return Color(1, 1, 0); }
  static Color green() { return Color(0, 1, 0); }
  static Color cyan()  { return Color(0, 1, 1); }
  static Color blue()  { return Color(0, 0, 1); }
  static Color white() { return Color(1, 1, 1); }
};


inline Color pow(const Color& c, float k) {
  return Color(::pow(c.r, k), ::pow(c.g, k), ::pow(c.b, k));
}


inline Color operator*(float f, const Color& c) {
  return c * f;
}


/** Subclass this to create your own application */
class App {
 private:
    
  /** For glut callbacks */
  static App*        instance;

  // The glut callbacks
  static void staticOnGraphics();
  static void staticOnKeyPress(unsigned char key, int x, int y);
  static void staticOnMousePress(int button, int state, int x, int y);
  static void staticOnMouseMotion(int x, int y);
  static void staticReshape(int width, int height);
  static void timerCallback(int value);

  const std::string  m_windowCaption;
  const float        m_zoom;
  const float        m_exposureConstant;
  const float        m_imageGamma;
  const int          m_frameTimeMilliseconds;

  void savePPM(const std::string& filename) const;
  void saveTGA(const std::string& filename) const;

 protected:

  /** Row-major */
  std::vector<Color> m_imageData;
  const int          m_imageWidth;
  const int          m_imageHeight;

  int                m_mouseX;
  int                m_mouseY;

 public:

  /** To obtain typical "2D pixel linear brightness values" use the
        default arguments for exposureConstant and imageGamma. If you want
        your colors on the range [0, 255] instead of [0, 2], set exposureConstant = 1.0f / 255.0f.
  */
  App(const std::string& windowCaption, int imageWidth, int imageHeight, float zoom = 2.0f,
      float exposureConstant = 1.0f, float imageGamma = 2.1f);

  virtual ~App() {}

  /** Out-of-bounds behavior is undefined and deferred to the subclass. */
  virtual void setPixel(int x, int y, const Color& c) = 0;

  /** Out-of-bounds behavior is undefined and deferred to the subclass. */
  virtual Color pixel(int x, int y) const = 0;

  /** Saves the current image in PPM or TGA format. It must have a
      lower-case extension. */
  void saveImage(const std::string& filename);

  /** Call this to start the App executing. */
  void run();

  /** Called by App. Override with your image rendering code. */
  virtual void onGraphics() = 0;

  /** The default implementation quits the program when ESC is pressed */
  virtual void onKeyPress(unsigned char keyCode);

  /** Check m_mouseX and m_mouseY for the pixel coordinates.
        The button parameter is one of GLUT_LEFT_BUTTON, GLUT_MIDDLE_BUTTON, or GLUT_RIGHT_BUTTON. 
  */
  virtual void onMouseRelease(int button) {}

  /** \see onMouseRelease */
  virtual void onMousePress(int button) {}
};

#endif
