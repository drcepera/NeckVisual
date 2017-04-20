#pragma once

#include <3dsReader/3dsReader.h>
#include <mat/mat.h>

void rotate(model_type_ptr model, double angle, double x, double y, double z);

void translate(model_type_ptr model, double x, double y, double z);

// функция для нахождения угла поворота маяка вокруг вертикальной оси таким образом,
// чтобы он смотрел в сторону КИМа
float FindBeaconAngle(float x, float y, float z);
