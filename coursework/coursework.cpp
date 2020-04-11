#include <ModelTriangle.h>
#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <RayTriangleIntersection.h>
#include <Utils.h>
#include <glm/glm.hpp>
#include <fstream>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

using namespace std;
using namespace glm;

#define WIDTH 680
#define HEIGHT 680
#define PI 3.14159

void draw();
void update();
void handleEvent(SDL_Event event);
void drawPoint(int x, int y, Colour colour);
void drawLine(CanvasPoint p1, CanvasPoint p2, uint32_t colour);
void drawStrokedTriangle(CanvasTriangle triangle);
void drawFilledTriangle(CanvasTriangle triangle);
void savePPM();
void readMTL();
void readOBJ(char fileName[50]);
void wireframe();
void filledRasterisedTriangles();
void filledRaytracedTriangles();
void fillTopTriangle(CanvasPoint ptop, CanvasPoint pmid, CanvasPoint pbottom, uint32_t colour);
void fillBottomTriangle(CanvasPoint ptop, CanvasPoint pmid, CanvasPoint pbottom, uint32_t colour);
void drawHorizontalLine(CanvasPoint p1, CanvasPoint p2, uint32_t colour);
std::vector<uint32_t> readPPM();
RayTriangleIntersection getClosestIntersection(vec3 rayDirection, vec3 raySource);
float diffuseLighting(RayTriangleIntersection currentTriangle);
bool hardShadow(RayTriangleIntersection currentTriangle);
Colour getClosestReflection(vec3 rayDirection, vec3 raySource);

//Defining global variables
DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
std::vector<Colour> colours;
std::vector<ModelTriangle> triangles;
std::vector<CanvasTriangle> projectedTriangles;
std::vector<RayTriangleIntersection> intersectedTriangles;
std::vector<float> depthBuffer;

//Variables that control camera and image
float imagePlaneDistance =HEIGHT/3;
mat3 camera_orientation(vec3(1.0,0.0,0.0),vec3(0.0,1.0,0.0),vec3(0.0,0.0,1.0));
vec3 camera(0,0,HEIGHT/30);
vec3 lightPosition(-0.2334011,4,-3.043968);
//-3.043968
vec3 lightColour = 14.f * vec3(1,1,1);
vec3 centreModel(0.5,-0.16,-2.5);

//Control variables for type of rendering
int drawType = 1;
int lightingType = 0;
int animationType = -1;
bool mirroredBox = false;


int main(int argc, char* argv[]){
  //Set depth buffer to infinity on load-up
  for (size_t i = 0; i < WIDTH*HEIGHT; i++) {
    depthBuffer.push_back(std::numeric_limits<float>::infinity());
  }
  SDL_Event event;

  readMTL();
  readOBJ("cornell-box.obj");
  // readOBJ("logo.obj");

  //Draw the wireframe on load-up
  wireframe();

  //Using the standard animation flow of event, update, draw.
  while(true){
    // We MUST poll for events - otherwise the window will freeze !
    if(window.pollForInputEvents(&event)) {
      handleEvent(event);
    }
    update();
    draw();
    window.renderFrame();
  }
}

void draw(){
  window.clearPixels();
  if (drawType == 1) {
    wireframe();
  }
  else if(drawType == 2){
    filledRasterisedTriangles();
  }
  else if(drawType == 3){
    filledRaytracedTriangles();
  }
  else if(drawType == 4){
    std::vector<uint32_t> pixels = readPPM();
  }
}

void update(){
  if (animationType == 0) {
    //Here we are implementing rotation of the object whilst potentially panning the camera out
    mat3 rotationMatrix(vec3(cos(PI/90),0,sin(PI/90)),vec3(0.0,1.0,0),vec3(-sin(PI/90),0,cos(PI/90)));
    for (size_t triangle_count = 22; triangle_count < triangles.size(); triangle_count++) {
      for (size_t i = 0; i < 3; i++) {
        triangles[triangle_count].vertices[i].x = (triangles[triangle_count].vertices[i].x - (-1.929011));
        triangles[triangle_count].vertices[i].y = (triangles[triangle_count].vertices[i].y - (0));
        triangles[triangle_count].vertices[i].z = (triangles[triangle_count].vertices[i].z - (-3.508598));

        triangles[triangle_count].vertices[i] = (triangles[triangle_count].vertices[i] * rotationMatrix);

        triangles[triangle_count].vertices[i].x = (triangles[triangle_count].vertices[i].x + (-1.929011));
        triangles[triangle_count].vertices[i].y = (triangles[triangle_count].vertices[i].y + (0));
        triangles[triangle_count].vertices[i].z = (triangles[triangle_count].vertices[i].z + (-3.508598));
      }

    }
  }
  animationType = -1;
}

void handleEvent(SDL_Event event)
{
  if(event.type == SDL_KEYDOWN) {
    if(event.key.keysym.sym == SDLK_DOWN){
      camera[1] -= 0.1;
    }
    else if(event.key.keysym.sym == SDLK_UP){
      camera[1] += 0.1;
    }
    else if(event.key.keysym.sym == SDLK_LEFT){
      camera[0] -= 0.1;
    }
    else if(event.key.keysym.sym == SDLK_RIGHT){
      camera[0] += 0.1;
    }
    else if(event.key.keysym.sym == SDLK_LSHIFT){
      camera[2] += 3;
    }
    else if(event.key.keysym.sym == SDLK_RSHIFT){
      camera[2] -= 3;
    }
    else if(event.key.keysym.sym == SDLK_1){
      drawType = 1;
    }
    else if(event.key.keysym.sym == SDLK_2){
      drawType = 2;
    }
    else if(event.key.keysym.sym == SDLK_3){
      lightingType = 0;
      drawType = 3;
    }
    else if(event.key.keysym.sym == SDLK_4){
      drawType = 3;
      lightingType = 1;
    }
    else if(event.key.keysym.sym == SDLK_5){
      drawType = 3;
      lightingType = 2;
    }
    else if(event.key.keysym.sym == SDLK_6){
      drawType = 3;
      lightingType = 2;
      mirroredBox = !mirroredBox;
    }
    else if(event.key.keysym.sym == SDLK_7){
      drawType = 3;
      lightingType = 3;
      mirroredBox = true;
    }
    else if(event.key.keysym.sym == SDLK_w){
      float theta = -PI/200;
      mat3 rotationMatrix(vec3(1.0,0.0,0.0),vec3(0.0,cos(theta),-sin(theta)),vec3(0.0,sin(theta),cos(theta)));
      camera_orientation =  camera_orientation * rotationMatrix;

    }
    else if(event.key.keysym.sym == SDLK_a){
      float theta = -PI/200;
      mat3 rotationMatrix(vec3(cos(theta),0,sin(theta)),vec3(0.0,1.0,0),vec3(-sin(theta),0,cos(theta)));
      camera_orientation =  camera_orientation * rotationMatrix;
    }
    else if(event.key.keysym.sym == SDLK_s){
      float theta = PI/200;
      mat3 rotationMatrix(vec3(1.0,0.0,0.0),vec3(0.0,cos(theta),-sin(theta)),vec3(0.0,sin(theta),cos(theta)));
      camera_orientation =  camera_orientation * rotationMatrix;
    }
    else if(event.key.keysym.sym == SDLK_d){
      float theta = PI/200;
      mat3 rotationMatrix(vec3(cos(theta),0,sin(theta)),vec3(0.0,1.0,0),vec3(-sin(theta),0,cos(theta)));
      camera_orientation =  camera_orientation * rotationMatrix;
    }
    else if(event.key.keysym.sym == SDLK_r){
      camera_orientation = mat3(vec3(1.0,0.0,0.0),vec3(0.0,1.0,0.0),vec3(0.0,0.0,1.0));
      camera = vec3(0,0,HEIGHT/30);
    }
    else if(event.key.keysym.sym == SDLK_m){
      //Write the screen to an image
      savePPM();
    }
    else if(event.key.keysym.sym == SDLK_n){
      //Read PPM file from current directory
      drawType = 4;
    }
    else if(event.key.keysym.sym == SDLK_l){
      //Look At
      vec3 forward = glm::normalize(camera-centreModel);
      vec3 right = (glm::cross(vec3(0,1,0), forward));
      vec3 up = (glm::cross(forward,right));
      camera_orientation[2] = forward;
      camera_orientation[0] = right;
      camera_orientation[1] = up;
    }
    else if(event.key.keysym.sym == SDLK_p){
      //Animation orbit and pan out
      animationType = 0;
    }
  }
  else if(event.type == SDL_MOUSEBUTTONDOWN) cout << "MOUSE CLICKED" << endl;
}

void drawPoint(int x, int y, Colour colour){
  //Bit pack the desired colour to uint_32 and then using SDL set colour of window
  uint32_t currentColour = (255<<24) + ((colour.red)<<16) + ((colour.green)<<8) + (colour.blue);
  window.setPixelColour(x,y,currentColour);
}

void drawLine(CanvasPoint p1, CanvasPoint p2, uint32_t colour){
  float xDiff = p2.x -p1.x;
  float yDiff = p2.y - p1.y;

  float numberOfSteps = std::max(abs(xDiff),abs(yDiff));

  float xStepSize = xDiff/numberOfSteps;
  float yStepSize = yDiff/numberOfSteps;

  for (float i=0.0; i<numberOfSteps; i++){
    float x = p1.x + (xStepSize*i);
    float y = p1.y + (yStepSize*i);
    window.setPixelColour(round(x), round(y), colour);
  }
}

void drawHorizontalLine(CanvasPoint p1, CanvasPoint p2, uint32_t colour){
  //Comparison for larger value
  if (p1.x > p2.x) {
      swap(p1.x, p2.x);
      swap(p1.depth, p2.depth);
  }

  //Find step size
  float zslope;
  if (p2.x - p1.x == 0) {
    zslope = 0;
  }
  else{
    zslope = (p2.depth - p1.depth)/(p2.x - p1.x);
  }

  //Implement the draw by using depthBuffer to make sure it is the closest point
  float currentZ = p1.depth;
  for (size_t i = p1.x; i <= p2.x; i++) {
    if(i + (p1.y)*WIDTH > 0 && i + (p1.y)*WIDTH < HEIGHT*WIDTH){
      currentZ = currentZ + zslope;
      if (currentZ != 0) {
        if (1/currentZ < depthBuffer[i + (p1.y)*WIDTH] ) {
          depthBuffer[i + (p1.y)*WIDTH] = 1/currentZ;
          window.setPixelColour(i, p1.y, colour);
        }
      }
    }
  }
}

void drawStrokedTriangle(CanvasTriangle triangle){
  uint32_t colour = (255<<24) + (int(triangle.colour.red)<<16) + (int(triangle.colour.green)<<8) + int(triangle.colour.blue);
  drawLine(triangle.vertices[0], triangle.vertices[1],colour);
  drawLine(triangle.vertices[1], triangle.vertices[2],colour);
  drawLine(triangle.vertices[0], triangle.vertices[2],colour);
}

void drawFilledTriangle(CanvasTriangle triangle){
  uint32_t colour = (255<<24) + (int(triangle.colour.red)<<16) + (int(triangle.colour.green)<<8) + int(triangle.colour.blue);

  CanvasPoint ptop = triangle.vertices[0];
  CanvasPoint pmid = triangle.vertices[1];
  CanvasPoint pbottom = triangle.vertices[2];

  //Swaps required so that we know the positioning of all three points
  if (pbottom.y < pmid.y){
    swap(pbottom.y, pmid.y);
    swap(pbottom.x, pmid.x);
    swap(pbottom.depth, pmid.depth);
    }

  if (pmid.y < ptop.y){
    swap(pmid.y, ptop.y);
    swap(pmid.x, ptop.x);
    swap(pmid.depth, ptop.depth);
  }

  if (pbottom.y < pmid.y){
    swap(pbottom.y, pmid.y);
    swap(pbottom.x, pmid.x);
    swap(pbottom.depth, pmid.depth);
    }

  if (pmid.y == ptop.y && ptop.y == pbottom.y) {
    drawHorizontalLine(ptop,pbottom,colour);
  }
  else if (pmid.y == pbottom.y) {
    if (pmid.x > pbottom.x) {
      swap(pbottom.x, pmid.x);
    }
    fillTopTriangle(ptop, pmid, pbottom, colour);
  }

  else if (pmid.y == ptop.y) {
    if (pmid.x < ptop.x) {
      swap(ptop.x, pmid.x);
    }
    fillBottomTriangle(pmid, ptop, pbottom, colour);
  }
  else{
    float psplitdepth = (ptop.depth + ((float)(pmid.y - ptop.y)/(float)(pbottom.y-ptop.y))*(pbottom.depth - ptop.depth));
    CanvasPoint psplit((int)(ptop.x + ((float)(pmid.y - ptop.y)/(float)(pbottom.y-ptop.y))*(pbottom.x - ptop.x)), pmid.y, psplitdepth);
    fillTopTriangle( ptop, pmid, psplit , colour);
    fillBottomTriangle(pmid, psplit, pbottom, colour);
  }
}

void fillTopTriangle(CanvasPoint ptop, CanvasPoint pmid, CanvasPoint pbottom, uint32_t colour){
  float slope1 = (pmid.x - ptop.x)/(pmid.y - ptop.y);
  float slope2 = (pbottom.x - ptop.x)/(pbottom.y - ptop.y);
  float zslope1 = (pmid.depth - ptop.depth)/(pmid.y - ptop.y);
  float zslope2 = (pbottom.depth - ptop.depth)/(pbottom.y - ptop.y);

  float x1 = ptop.x;
  float x2 = ptop.x;
  float z1 = ptop.depth;
  float z2 = ptop.depth;
  for (int i = ptop.y; i <= pmid.y; i++) {

    drawHorizontalLine(CanvasPoint(ceil(x1),i,z1),CanvasPoint(ceil(x2),i,z2), colour);
    x1 = x1 + slope1;
    x2 = x2 + slope2;
    z1 = z1 + zslope1;
    z2 = z2 + zslope2;
  }
}

void fillBottomTriangle(CanvasPoint ptop, CanvasPoint pmid, CanvasPoint pbottom, uint32_t colour){
  float slope1 = (pbottom.x - ptop.x)/(pbottom.y - ptop.y);
  float slope2 = (pbottom.x - pmid.x)/(pbottom.y - pmid.y);
  float zslope1 = (pbottom.depth - ptop.depth)/(pbottom.y - ptop.y);
  float zslope2 = (pbottom.depth - pmid.depth)/(pbottom.y - pmid.y);

  float x1 = ptop.x;
  float x2 = pmid.x;
  float z1 = ptop.depth;
  float z2 = pmid.depth;
  for (int i = ptop.y; i <= pbottom.y; i++) {
    drawHorizontalLine(CanvasPoint(ceil(x1),i,z1),CanvasPoint(ceil(x2),i,z2), colour);
    x1 = x1 + slope1;
    x2 = x2 + slope2;
    z1 = z1 + zslope1;
    z2 = z2 + zslope2;
  }
}

std::vector<uint32_t> readPPM(){
  char fileName[50] = "texture.ppm";
	string pSix;
	size_t width = 0;
	size_t height = 0;
	//size_t maximum = 255;
  string data;
  char red, green, blue;
  std::vector<uint32_t> packedPixels;


  ifstream infile;
  infile.open(fileName);

  getline(infile,pSix);
  getline(infile,data);

  string widthAndHeight;
  getline(infile,widthAndHeight);
  width = stoi(widthAndHeight.substr(0,widthAndHeight.find(" ")));
  height = stoi(widthAndHeight.substr(widthAndHeight.find(" ")));

  getline(infile,data);

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      red = infile.get();
      green = infile.get();
      blue = infile.get();
      uint32_t colour = (255<<24) + (red<<16) + (green<<8) + blue;
      window.setPixelColour(x,y,colour);
      packedPixels.push_back(colour);
    }
  }
  // close file
  infile.close();
  return packedPixels;
}

void savePPM(){
  const char *filename = "out.ppm";

  const int MaxColorComponentValue = 255;
  FILE * fp;
  const char *comment = "binary pgm file";

  fp = fopen(filename, "wb");

  fprintf(fp, "P6\n%s\n%d %d\n%d\n", comment, WIDTH, HEIGHT,
          MaxColorComponentValue);

  //Fill the array with colour values
  for (int y = 0; y < HEIGHT; ++y) {
    for (int x = 0; x < WIDTH; ++x) {
      uint32_t wholeColour = window.getPixelColour(x,y);
      uint8_t colour[3];
      char colour_char[3];

      //Reverse shift the colours to get uint8 values
      colour[0] = (wholeColour >> 16) & 255;
      colour[1] = (wholeColour >> 8) & 255;
      colour[2] = (wholeColour) & 255;

      colour_char[0] = (char)(colour[0]);
      colour_char[1] = (char)(colour[1]);
      colour_char[2] = (char)(colour[2]);
      fwrite(colour_char,1,3,fp);
    }
  }

  fclose(fp);
  printf("OK - file %s saved\n", filename);
}

//Read in the material file
void readMTL(){
  char fileName[50] = "cornell-box.mtl";
  string line;
  int red, green, blue;
  Colour currentColour;
  ifstream infile;

  infile.open(fileName);
  while (infile.is_open()) {
    getline(infile, line);
    string colour = line.substr(7,line.find("\n"));
    currentColour.name = colour;

    getline(infile, line);
    std::string* splitLine = split(line, char(32));
    red = stof(splitLine[1])*255;
    green = stof(splitLine[2])*255;
    blue = stof(splitLine[3])*255;

    currentColour.red = red;
    currentColour.green = green;
    currentColour.blue = blue;
    colours.push_back(currentColour);


    getline(infile, line);


    if (infile.eof()) {
      infile.close();
      break;
    }
  }
}

//Read in the object file
void readOBJ(char fileName[50]){
  string line;
  Colour currentColour;
  std::vector<vec3> tempPoints;
  ifstream infile;

  infile.open(fileName);

  //Intro lines to ignore
  getline(infile, line);
  getline(infile, line);

  while (infile.is_open()) {
    //First line is object name --Ignored for now
    getline(infile, line);
    getline(infile, line);
    string colour = line.substr(7,line.find("\n"));
    getline(infile, line);
    while (line.substr(0,1) == "v" && line.substr(0,2)!= "vt") { //If vertex then append new temporary point
      float x, y, z;
      std::string* splitLine = split(line, char(32));
      x = stof(splitLine[1]);
      y = stof(splitLine[2]);
      z = stof(splitLine[3]);

      if (strncmp(fileName,"logo.obj",8)==0){
        x = (x /250)-1;
        y = y/250+2;
        z = z/250-1;
      }

      vec3 temp(x,y,z);
      tempPoints.push_back(temp);
      getline(infile, line);
    }
    while (line.substr(0,1) == "f" || line.substr(0,2) =="vt") { //If face then create a new triangle with specified vertices
      if (line.substr(0,1) == "f"){
      int a,b,c;
      const char* chh=line.c_str();
      sscanf(chh, "f %i/ %i/ %i/", &a, &b, &c);
      ModelTriangle tempTriangle;
      tempTriangle.vertices[0] = tempPoints[a-1];
      tempTriangle.vertices[1] = tempPoints[b-1];
      tempTriangle.vertices[2] = tempPoints[c-1];
      for (size_t i = 0; i < colours.size(); i++) {
        if (colour == colours[i].name) {
          tempTriangle.colour = colours[i];
        }
      }
      triangles.push_back(tempTriangle);
    }
    else if (line.substr(0,2) =="vt") {
    }
      getline(infile, line);
    }
    if (infile.eof()) {
      infile.close();
      break;
    }
  }


}

void wireframe(){
  projectedTriangles.clear();
  for (size_t i = 0; i < WIDTH*HEIGHT; i++) {
    depthBuffer[i] = std::numeric_limits<float>::infinity();
  }

  for (size_t i = 0; i < triangles.size(); i++) {
    CanvasTriangle tempTriangle;
    for (size_t j = 0; j < 3; j++) {
      float x3D = (triangles[i].vertices[j].x);
      float y3D = triangles[i].vertices[j].y;
      float z3D = triangles[i].vertices[j].z;

      vec3 toVertex( x3D - camera.x, y3D - camera.y, z3D - camera.z);
      toVertex = toVertex*camera_orientation;
      float proportion = imagePlaneDistance/(-toVertex.z);
      float xImagePlane = round((proportion * toVertex.x) + WIDTH/2);
      float yImagePlane = round((proportion * (1-toVertex.y)) + HEIGHT/2);

      CanvasPoint temp(xImagePlane,yImagePlane, toVertex.z);
      tempTriangle.vertices[j] = temp;
    }
    tempTriangle.colour = triangles[i].colour;
    projectedTriangles.push_back(tempTriangle);
  }

   for (size_t i = 0; i < projectedTriangles.size(); i++) {
    drawStrokedTriangle(projectedTriangles[i]);

  }
}

void filledRasterisedTriangles(){
  projectedTriangles.clear();
  for (size_t i = 0; i < WIDTH*HEIGHT; i++) {
    depthBuffer[i] = std::numeric_limits<float>::infinity();
  }
  for (size_t i = 0; i < triangles.size(); i++) {

    //calculate triangle's surface normal
    vec3 v0v1 = triangles[i].vertices[1] - triangles[i].vertices[0];
    vec3 v0v2 = triangles[i].vertices[2] - triangles[i].vertices[0];
    vec3 normal = glm::cross(v0v2,v0v1);

    //calculate vector from triangle to camera
    vec3 triangle_position((triangles[i].vertices[0].x+triangles[i].vertices[1].x+triangles[i].vertices[2].x)/3,(triangles[i].vertices[0].y+triangles[i].vertices[1].y+triangles[i].vertices[2].y)/3,(triangles[i].vertices[0].z+triangles[i].vertices[1].z+triangles[i].vertices[2].z)/3);
    vec3 triangle_to_camera = camera - triangle_position;
    float magnitude = glm::length(triangle_to_camera);

    //if triangle faces away from camera -> ignore and move onto next triangle (use dot product of normal and vector to check if it is facing away)
    float triangle_orientation = glm::dot(normal,triangle_to_camera);

    //Basic Culling
    if (triangle_orientation > 0) {
      continue;
    }
    //Basic Clipping
    if (magnitude < 3 || magnitude > 30 ) {
      std::cout << "here" << '\n';
      continue;
    }

    // if triangle is really far away from camera -> ignore and move onto next triangle

    // if triangle is too close to camaera -> ignore and move onto next triangle



    CanvasTriangle tempTriangle;
    for (size_t j = 0; j < 3; j++) {
      float x3D = (triangles[i].vertices[j].x);
      float y3D = triangles[i].vertices[j].y;
      float z3D = triangles[i].vertices[j].z;

      vec3 toVertex( x3D - camera.x, y3D - camera.y, z3D - camera.z);
      toVertex = toVertex*camera_orientation;
      float proportion = floor(imagePlaneDistance/(-toVertex.z));
      float xOnImagePlane = round((proportion * toVertex.x) + WIDTH/2);
      float yOnImagePlane = round((proportion * (1-toVertex.y)) + HEIGHT/2);

      CanvasPoint temp(xOnImagePlane,yOnImagePlane, toVertex.z);
      tempTriangle.vertices[j] = temp;
    }
    tempTriangle.colour = triangles[i].colour;
    projectedTriangles.push_back(tempTriangle);
  }
  for (size_t i = 0; i < projectedTriangles.size(); i++) {
     drawFilledTriangle(projectedTriangles[i]);
   }
}

float diffuseLighting(RayTriangleIntersection currentTriangle){
  float brightness;
  if (currentTriangle.distanceFromLight != 0) {
    brightness = 3.5/(0.1f * PI * pow(currentTriangle.distanceFromLight,2));
  }else{
    brightness = 1;
  }

  if (brightness > 1) {
    brightness = 1;
  }else if (brightness < 0) {
    brightness = 0;
  }

  vec3 v0v1 = currentTriangle.intersectedTriangle.vertices[1] - currentTriangle.intersectedTriangle.vertices[0];
  vec3 v0v2 = currentTriangle.intersectedTriangle.vertices[2] - currentTriangle.intersectedTriangle.vertices[0];

  vec3 normal = glm::cross(v0v1,v0v2);
  vec3 intersectionToLight = lightPosition - currentTriangle.intersectionPoint;
  float angleIncidence = glm::dot(normal, intersectionToLight);

  if (angleIncidence <= 0) {
    brightness = 0;
  }
  else if (angleIncidence >= 1) {
    brightness = 1*brightness;
  }
  else{
    brightness = brightness*angleIncidence;
  }

  //Ambient lighting threshold
  if (brightness < 0.2) {
    brightness = 0.2;
  }
  return brightness;
}

bool hardShadow(RayTriangleIntersection currentTriangle){
  vec3 rayDirectionShadows = lightPosition - currentTriangle.intersectionPoint;
  rayDirectionShadows = glm::normalize(-rayDirectionShadows);
  RayTriangleIntersection closestIntersectionShadow = getClosestIntersection(rayDirectionShadows, currentTriangle.intersectionPoint);
  float distanceToIntersection = glm::length(currentTriangle.intersectionPoint - closestIntersectionShadow.intersectionPoint);
  float distanceToLight = glm::length(lightPosition - currentTriangle.intersectionPoint);
  if(distanceToIntersection < distanceToLight && closestIntersectionShadow.triangleIndex != currentTriangle.triangleIndex){
    return true;
  }
  else{
    return false;
  }
}

bool softShadow(RayTriangleIntersection currentTriangle, vec3 light_position){
  vec3 rayDirectionShadows = light_position - currentTriangle.intersectionPoint;
  rayDirectionShadows = glm::normalize(-rayDirectionShadows);
  RayTriangleIntersection closestIntersectionShadow = getClosestIntersection(rayDirectionShadows, currentTriangle.intersectionPoint);
  float distanceToIntersection = glm::length(currentTriangle.intersectionPoint - closestIntersectionShadow.intersectionPoint);
  float distanceToLight = glm::length(lightPosition - currentTriangle.intersectionPoint);
  if(distanceToIntersection < distanceToLight && closestIntersectionShadow.triangleIndex != currentTriangle.triangleIndex){
    return true;
  }
  else{
    return false;
  }
}

void filledRaytracedTriangles(){
  //Make sure we have the correct position of the image plane in the world co-ordinates
  vec3 topRightImagePlaneWorld(0,0,0);
  topRightImagePlaneWorld.z = floor(camera.z - imagePlaneDistance);
  topRightImagePlaneWorld.x = floor(camera.x + 0.5*WIDTH);
  topRightImagePlaneWorld.y = floor(camera.y - 0.5*HEIGHT - 10);

  for (size_t j = 0; j < HEIGHT; j++) {
    for (size_t i = 0; i < WIDTH; i++) {
      //Create 5 rays as we are anti-aliasing
      vec3 rayDirection(topRightImagePlaneWorld.x-i, topRightImagePlaneWorld.y+j, imagePlaneDistance);
      vec3 rayDirectionLEFT(topRightImagePlaneWorld.x-i-0.5, topRightImagePlaneWorld.y+j, imagePlaneDistance);
      vec3 rayDirectionRIGHT(topRightImagePlaneWorld.x-i+0.5, topRightImagePlaneWorld.y+j, imagePlaneDistance);
      vec3 rayDirectionTOP(topRightImagePlaneWorld.x-i, topRightImagePlaneWorld.y+j-0.5, imagePlaneDistance);
      vec3 rayDirectionBOTTOM(topRightImagePlaneWorld.x-i, topRightImagePlaneWorld.y+j+0.5, imagePlaneDistance);
      rayDirection = rayDirection*glm::inverse(camera_orientation);
      rayDirection = glm::normalize(rayDirection);
      rayDirectionLEFT = rayDirectionLEFT*glm::inverse(camera_orientation);
      rayDirectionLEFT = glm::normalize(rayDirectionLEFT);
      rayDirectionRIGHT = rayDirectionRIGHT*glm::inverse(camera_orientation);
      rayDirectionRIGHT = glm::normalize(rayDirectionRIGHT);
      rayDirectionTOP = rayDirectionTOP*glm::inverse(camera_orientation);
      rayDirectionTOP = glm::normalize(rayDirectionTOP);
      rayDirectionBOTTOM = rayDirectionBOTTOM*glm::inverse(camera_orientation);
      rayDirectionBOTTOM = glm::normalize(rayDirectionBOTTOM);

      //Get closest intersecting trinagles for all the rays
      RayTriangleIntersection currentTriangle = getClosestIntersection(rayDirection, camera);
      RayTriangleIntersection currentTriangleLEFT = getClosestIntersection(rayDirectionLEFT, camera);
      RayTriangleIntersection currentTriangleRIGHT = getClosestIntersection(rayDirectionRIGHT, camera);
      RayTriangleIntersection currentTriangleTOP = getClosestIntersection(rayDirectionTOP, camera);
      RayTriangleIntersection currentTriangleBOTTOM = getClosestIntersection(rayDirectionBOTTOM, camera);

      if (currentTriangle.distanceFromCamera != std::numeric_limits<float>::max()) {
        if (lightingType == 1 || lightingType == 2 || lightingType == 3) {
          //Apply diffuseLighting to all the triangles from the rays
          float brightness = diffuseLighting(currentTriangle);
          float brightnessLEFT = diffuseLighting(currentTriangleLEFT);
          float brightnessRIGHT = diffuseLighting(currentTriangleRIGHT);
          float brightnessTOP = diffuseLighting(currentTriangleTOP);
          float brightnessBOTTOM = diffuseLighting(currentTriangleBOTTOM);

          //Check if the intersected point returned is a hard shadow
          if (lightingType == 2) {
            if (hardShadow(currentTriangle)) {
              brightness = 0.1f;
            }
            if (hardShadow(currentTriangleLEFT)) {
              brightnessLEFT = 0.1f;
            }
            if (hardShadow(currentTriangleRIGHT)) {
              brightnessRIGHT = 0.1f;
            }
            if (hardShadow(currentTriangleTOP)) {
              brightnessTOP = 0.1f;
            }
            if (hardShadow(currentTriangleBOTTOM)) {
              brightnessBOTTOM = 0.1f;
            }
          }
          vec3 lp1(-0.2334011,4,-3.043968);
          vec3 lp2(-0.2,4,-3.043968);
          vec3 lp3(-0.262,4,-3.043968);
          vec3 lp4(-0.2334011,4,-2.74);
          vec3 lp5(-0.2334011,4,-3.343968);


          if (lightingType == 3) {
            if (softShadow(currentTriangle, lp1)) {
              brightness -= 0.1f;
            }
            if (softShadow(currentTriangle, lp2)) {
              brightness -= 0.1f;
            }
            if (softShadow(currentTriangle, lp3)) {
              brightness -= 0.1f;
            }
            if (softShadow(currentTriangle, lp4)) {
              brightness -= 0.1f;
            }
            if (softShadow(currentTriangle, lp5)) {
              brightness -= 0.1f;
            }
          }

          //Take an average of all the rays for a clearer & sharper render
          Colour adjusted;
          adjusted.red = (currentTriangle.intersectedTriangle.colour.red * brightness + currentTriangleLEFT.intersectedTriangle.colour.red * brightnessLEFT + currentTriangleRIGHT.intersectedTriangle.colour.red * brightnessRIGHT + currentTriangleTOP.intersectedTriangle.colour.red * brightnessTOP + currentTriangleBOTTOM.intersectedTriangle.colour.red * brightnessBOTTOM)/5;
          adjusted.green = (currentTriangle.intersectedTriangle.colour.green * brightness + currentTriangleLEFT.intersectedTriangle.colour.green * brightnessLEFT + currentTriangleRIGHT.intersectedTriangle.colour.green * brightnessRIGHT + currentTriangleTOP.intersectedTriangle.colour.green * brightnessTOP + currentTriangleBOTTOM.intersectedTriangle.colour.green * brightnessBOTTOM)/5;
          adjusted.blue = (currentTriangle.intersectedTriangle.colour.blue * brightness + currentTriangleLEFT.intersectedTriangle.colour.blue * brightnessLEFT + currentTriangleRIGHT.intersectedTriangle.colour.blue * brightnessRIGHT + currentTriangleTOP.intersectedTriangle.colour.blue * brightnessTOP + currentTriangleBOTTOM.intersectedTriangle.colour.blue * brightnessBOTTOM)/5;
          drawPoint(i,j,adjusted);
        }
        else{
          drawPoint(i,j,currentTriangle.intersectedTriangle.colour);
        }
      }
    }
  }
}

RayTriangleIntersection getClosestIntersection(vec3 rayDirection, vec3 raySource){
  RayTriangleIntersection currentClosestIntersection;

  //Set distance from camera to infinity for all triangles
  currentClosestIntersection.distanceFromCamera = std::numeric_limits<float>::max();
  for (size_t i = 0; i < triangles.size(); i++) {
    vec3 e0 = triangles[i].vertices[1] - triangles[i].vertices[0];
    vec3 e1 = triangles[i].vertices[2] - triangles[i].vertices[0];
    vec3 SPVector = vec3(raySource - triangles[i].vertices[0]);

    //Calculate intersecting triangles along the ray
    mat3 DEMatrix((rayDirection),e0,e1);
    vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;

    //If matrix above meets this criteria then it is a true intersection
    //Basic clipping contained in the if statement at the end. Too far and near not shown
    if (possibleSolution[1] > 0.0 && possibleSolution[2] > 0.0 && (possibleSolution[1] + possibleSolution[2] < 1.0) && possibleSolution[0] >= 1 && possibleSolution[0] <= 30) {
      if(possibleSolution[0] < currentClosestIntersection.distanceFromCamera && possibleSolution[0] > 1){

        vec3 intersection = triangles[i].vertices[0] + possibleSolution[1]*e0 + possibleSolution[2]*e1;
        currentClosestIntersection.distanceFromCamera = possibleSolution[0];
        currentClosestIntersection.intersectionPoint = intersection;
        currentClosestIntersection.intersectedTriangle = triangles[i];
        currentClosestIntersection.distanceFromLight = glm::length(lightPosition - intersection);
        currentClosestIntersection.triangleIndex = i;

        //If the mirror toggle is on, then for the tall box object reflect the incident rays.
        //Find new intersection on another triangle and use that colour.
        if (i >= 22 && mirroredBox) {
          vec3 v0v1 = triangles[i].vertices[1] - triangles[i].vertices[0];
          vec3 v0v2 = triangles[i].vertices[2] - triangles[i].vertices[0];
          vec3 incidentRay = raySource - intersection;
          incidentRay = glm::normalize(incidentRay);
          vec3 normal = glm::cross(v0v2,v0v1);
          vec3 reflectedRay = incidentRay -  2.0f*(glm::dot(normal, incidentRay) * normal);
          reflectedRay = glm::normalize(reflectedRay);


          currentClosestIntersection.intersectedTriangle.colour = getClosestReflection(reflectedRay, intersection);
        }

    }
  }
}

return currentClosestIntersection;
}

// Similar to function above, however ignores the tall box that is the mirrored material
// This is used to get the colour of the reflected triangle in the tall box
Colour getClosestReflection(vec3 rayDirection, vec3 raySource){

    RayTriangleIntersection currentClosestReflectedIntersection;
    currentClosestReflectedIntersection.distanceFromCamera = std::numeric_limits<float>::max();
    for (size_t i = 0; i<triangles.size(); i++) {
      if (i == 22) {
        i = 32;
        break;
      }
      vec3 e0 = triangles[i].vertices[1] - triangles[i].vertices[0];
      vec3 e1 = triangles[i].vertices[2] - triangles[i].vertices[0];
      vec3 SPVector = vec3(raySource - triangles[i].vertices[0]);

      mat3 DEMatrix((rayDirection),e0,e1);
      vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;

      if (possibleSolution[1] > 0.0 && possibleSolution[2] > 0.0 && (possibleSolution[1] + possibleSolution[2] < 1.0) && possibleSolution[0] >= 0) {
        if(possibleSolution[0] < currentClosestReflectedIntersection.distanceFromCamera && possibleSolution[0] > 1){
          vec3 intersection = triangles[i].vertices[0] + possibleSolution[1]*e0 + possibleSolution[2]*e1;
          currentClosestReflectedIntersection.distanceFromCamera = possibleSolution[0];
          currentClosestReflectedIntersection.intersectionPoint = intersection;
          currentClosestReflectedIntersection.intersectedTriangle = triangles[i];
          currentClosestReflectedIntersection.distanceFromLight = glm::length(lightPosition - intersection);
          currentClosestReflectedIntersection.triangleIndex = i;
      }
    }
  }
  //Only need to return the colour. If there is no intersection with a surface return background colour such as grey.
  if (currentClosestReflectedIntersection.distanceFromCamera == std::numeric_limits<float>::max()) {
    return Colour(202,204,206);
  }else{
    return currentClosestReflectedIntersection.intersectedTriangle.colour;
  }

}
