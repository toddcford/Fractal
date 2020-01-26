#include "App.cpp"
#include <math.h>
#include <cassert>
#include <iostream>
#include <vector>
#include <cmath>

#define PI 3.14159265

/* 

compile with this comamand:
g++ -std=c++17 main.cpp  -Wno-deprecated-declarations -framework GLUT -framework OpenGL

*/

class Fractal : public App {
  
public:

  Fractal(const std::string& windowCaption, int imageWidth, int imageHeight): App(windowCaption, imageWidth, imageHeight) {}
  void setPixel(int x, int y, const Color& c) override;
  Color pixel(int x, int y) const override;
  void onGraphics() override;
  void drawLine(int x0, int x1, int y0, int y1, Color c);
  void drawLine2(int x, int y, int r, float theta, Color c);
  void drawCircle(int r, float n, int x, int y, Color c);
  void drawGradient(Color c);
  void drawGradient2(Color a, Color b);
  void drawFunction(float cubed, float squared, float linear, float constant, float leftBound, float rightBound, Color c);
  void drawFractal(int x, int y, float r, float theta, Color c);
  int m_frameNumber = 0;
  float m_clockAngle = 0.0;


};
	       
void Fractal::setPixel(int x, int y, const Color& color)  {
  /* still need to deal with out of bounds coordinates*/
  m_imageData[x + (m_imageWidth * y)] = Color(color.r, color.g, color.b);;
}

Color Fractal::pixel(int x, int y) const {
  return m_imageData[x + (m_imageWidth*y)];
}

void Fractal::onGraphics()  {
  drawGradient2(Color(1,1,1), Color(.565, .933,.565));

  drawFunction(1,0 , 0, 25, -4, 4, Color::red());
  drawFractal(3 * m_imageWidth / 4, m_imageHeight - 1, 60, (3.0*PI) / 2.0, Color::white());
  drawCircle(m_imageHeight / 10, 5000, m_imageWidth / 8, m_imageHeight / 8, Color::black());
  
  /*draw second hand on clock. User frame number to make it move at around the right speed. */
  drawLine2( m_imageWidth / 8, m_imageHeight / 8, m_imageHeight/ 12, m_clockAngle + 2.0 * PI / 60, Color::black()); 
  if(m_frameNumber % 60 == 0) {
    m_clockAngle += 2.0 * PI / 60;
  }
  ++m_frameNumber;

}

/* Gradient is from black to the inputted color */
void Fractal::drawGradient(Color c) {
  float scale = 1 / float(m_imageHeight);
  
  for(int i = 0; i < m_imageHeight; ++i) 
    drawLine(0, i, m_imageWidth, i, c * scale * i);  


}

/* Gradient is from color a to color b */
void Fractal::drawGradient2(Color a, Color b) {

  float scaleR = float((b.r - a.r)) / float(m_imageHeight);
  float scaleG = float((b.g - a.g)) / float(m_imageHeight);
  float scaleB = float((b.b - a.b)) / float(m_imageHeight);

  for(int i = 0; i < m_imageHeight; ++i) 
    drawLine(0, i, m_imageWidth, i, Color(a.r + (scaleR*i), 
					  a.g + (scaleG*i),
					  a.b + (scaleB*i)));

}

void Fractal::drawLine(int x0, int y0, int x1, int y1, Color c){
  
  float m = float(y1 - y0) / float(x1 - x0);

  if( (y1 - y0) < 0 && (x1 - x0) <= 0 ) {
    drawLine(x1, y1, x0, y0, c);
    return;
  }
  
  if( (x1 - x0) < 0 && (y1 - y0) >= 0) {
    drawLine(x1, y1, x0, y0, c);
  }
  
  if(m < 1) {
    float y = y0;
    for(int x = x0; x < x1; ++x) {
      setPixel(x, int(y+0.5f), c);
      y += m;
    }
  }
  else {
    float x = x0;
    for(int y = y0; y <= y1; ++y) {
      setPixel(int(x+0.5f), y, c);
      x += (1/m);
    }
  }
      
}

/* Use start point, length, and angle off horizontal to calculate end point. Then use drawLine to draw */
void Fractal::drawLine2(int x, int y, int r, float theta, Color c) {

  int x1 = x + r * cos(theta);
  int y1 = y + r * sin(theta);

  drawLine(x, y, x1, y1, c);

}

void Fractal::drawCircle(int r, float n, int x, int y, Color c) {
  for(int i = 0; i < n; ++i) {
   
    drawLine(x + r * cos((2 * PI * i)/n), y + r * sin((2 * PI * i) / n),
	     x + r * cos(((2 * PI)*(i + 1)) / n), y + r * sin(((2 * PI) * (i+1) )/ n), 
	     c);
  }
}

void Fractal::drawFunction(float cubed, float squared, float linear, float constant, float leftBound, float rightBound, Color c) {
  /* Has to be a way to make this more efficient, but it will do for now. 
     Issue is with determining how to scale y in order to fit the function on
     the window. I'm pretty sure you need to know y_max or y_min for entire range 
     before setting any of the pixels, but that means need to loop through bounds twice.
   */
  drawLine(0, m_imageHeight / 2, m_imageWidth, m_imageHeight / 2, Color::black());
  drawLine(m_imageWidth / 2, 0, m_imageWidth / 2, m_imageHeight-1, Color::black());

  float y = 0.0;
  float x = 0.0;
  float y_max = 0.0;
  float y_min = 0.0;
  std::vector<float> range;

  float increment = (rightBound - leftBound) / float(m_imageWidth);
  
  /* calculate range of function */
  for(float i = leftBound; i <= rightBound; i += increment) {  
    y = cubed*pow(i, 3) + squared*pow(i, 2) + linear*i + constant;
    if(y > y_max) y_max = y;
    if(y < y_min) y_min = y;
    range.push_back(y);
  }
  /* plot the function, using point with largest absolute value to scale */
  for(int i = 0; i < range.size(); ++i) {
    
    /*last pixel on the right for each row is actually 1 pixel less than the window width*/
    if (x == m_imageWidth) x = m_imageWidth - 1;
    y = range[i];
    y *= (m_imageHeight / (2*(abs(y_max) > abs(y_min) ? abs(y_max):abs(y_min))));    
    setPixel(x, -int(y+0.5f) + (m_imageHeight / 2), c);
    x += 1;
  }
    
}


void Fractal::drawFractal(int x, int y, float r, float theta, Color c) {
  
  if ( r < 2 ) return;
  drawLine2(x, y, r, theta, c);
  int x1 = x + r * cos(theta);
  int y1 = y + r * sin(theta);
  float thetaPlus = theta + (PI / 5.0);
  float thetaMinus = theta - (PI / 7.0);



  drawFractal(x1, y1, r/1.5, thetaPlus,  Color(1, 1, 1));
  drawFractal(x1, y1, r/1.5, thetaMinus, Color(1, 1, 1));


}

int main() {


  Fractal myFractal("My Fractal", 450, 450);
  myFractal.run();




  return 0;
}
