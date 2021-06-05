#include <vector>
#include <cmath>
#include "model.h"
#include "geometry.h"
#include "tgaimage.h"

using namespace std;

Model *model = NULL;
const int width = 800;
const int height = 800;
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
    bool steep = false;
    if (abs(x0 - x1) < abs(y0 - y1))
    {
        steep = true;
        swap(x0, y0);
        swap(x1, y1);
    }

    if (x0 > x1)
    {
        swap(x0, x1);
        swap(y0, y1);
    }
    int xLen = x1 - x0;
    for (int x = x0; x <= x1; x++)
    {
        float t = (x - x0) / (float)xLen;
        int y = y0 * (1 - t) + y1 * t;
        if (steep)
            image.set(y, x, color);
        else
            image.set(x, y, color);
    }
}
void line(Vec2i t0, Vec2i t1, TGAImage &image, TGAColor color)
{
    int x0 = t0.x;
    int y0 = t0.y;
    int x1 = t1.x;
    int y1 = t1.y;
    bool steep = false;
    if (abs(x0 - x1) < abs(y0 - y1))
    {
        steep = true;
        swap(x0, y0);
        swap(x1, y1);
    }

    if (x0 > x1)
    {
        swap(x0, x1);
        swap(y0, y1);
    }
    int xLen = x1 - x0;
    for (int x = x0; x <= x1; x++)
    {
        float t = (x - x0) / (float)xLen;
        int y = y0 * (1 - t) + y1 * t;
        if (steep)
            image.set(y, x, color);
        else
            image.set(x, y, color);
    }
}
void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color)
{
    line(t0, t1, image, color);
    line(t2, t1, image, color);
    line(t0, t2, image, color);
}
void FillTriangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color)
{
    if (t1.y <= t2.y)
    {
        Vec2i t;
        t = t1;
        t1 = t2;
        t2 = t;
    }
    if (t0.y <= t1.y)
    {
        Vec2i t;
        t = t0;
        t0 = t1;
        t1 = t;
    }
    if (t1.y <= t2.y)
    {
        Vec2i t;
        t = t1;
        t1 = t2;
        t2 = t;
    }
    int lenY1 = t0.y - t1.y;
    int lenY2 = t1.y - t2.y;
    int lenY = t0.y - t2.y;
    int lenX = t2.x - t0.x;
    int lenX1 = t1.x - t0.x;
    int lenX2 = t2.x - t1.x;
    Vec2i pos1;
    Vec2i pos2;
    pos1.x = t0.x;
    pos2.x = t0.x;
    for (int y = t0.y; y >= t1.y; y--)
    {

        pos1.y = y;
        pos2.y = y;
        pos1.x = (t0.y - y) / (float)lenY * lenX + t0.x;
        pos2.x = (t0.y - y) / (float)lenY1 * lenX1 + t0.x;
        line(pos1, pos2, image, color);
    }
    Vec2i t3 = pos1;
    for (int y = t1.y; y >= t2.y; y--)
    {
        pos1.y = y;
        pos2.y = y;
        pos1.x = (t0.y - y) / (float)lenY * lenX + t0.x;
        pos2.x = (t1.y - y) / (float)lenY2 * lenX2 + t1.x;
        line(pos1, pos2, image, color);
    }
}
int main()
{
    model = new Model("obj/african_head.obj");
    TGAImage image(width, height, TGAImage::RGB);
    Vec2i t0[3] = {Vec2i(110, 70), Vec2i(150, 160), Vec2i(170, 80)};
    Vec2i t1[3] = {Vec2i(280, 50), Vec2i(150, 1), Vec2i(70, 180)};
    Vec2i t2[3] = {Vec2i(280, 150), Vec2i(120, 160), Vec2i(130, 180)};
    
    FillTriangle(t0[0], t0[1], t0[2], image, red);
    triangle(t0[0], t0[1], t0[2], image, green);
    image.flip_vertically();
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}