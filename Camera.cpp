#include "Camera.h"

Camera::Camera(){
   bounds.x =0;
   bounds.y =0;
   bounds.w = WINDOW_WIDTH;
   bounds.h = WINDOW_HEIGHT;
}

Camera::Camera(SDL_Rect rect){
   bounds.x = rect.x;
   bounds.y = rect.y;
   bounds.w = rect.w;
   bounds.h = rect.h;
}

SDL_Rect* Camera::get_bounds(){
  return &bounds;
}

void Camera::center_on(SDL_Rect box)
{
  bounds.x = box.x + box.w/2 - bounds.w/2;
  bounds.y = box.y + box.h/2 - bounds.h/2;
}

int Camera::get_x(){
  return bounds.x;
}

int Camera::get_y(){
  return bounds.y;
}

int Camera::get_w(){
  return bounds.w;
}

int Camera::get_h(){
  return bounds.h;
}

void Camera::set_x(int x){
  bounds.x = x;
}

void Camera::set_y(int y){
  bounds.y = y;
}

void Camera::scroll(Direction dir, int dist)
{
  switch(dir)
  {
    case UP: bounds.y -= dist; break;
    case DOWN: bounds.y += dist; break;
    case LEFT: bounds.x -= dist; break;
    case RIGHT: bounds.x += dist; break;
  }
}
