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
int main()
{
    model = new Model("obj/african_head.obj");
    TGAImage image(width, height, TGAImage::RGB);
    // for (int i = 0; i < model->nfaces(); i++)
    // {
    //     vector<int> face = model->face(i);
    //     for (int j = 0; j < 3; j++)
    //     {
    //         Vec3f v0 = model->vert(face[j]);
    //         Vec3f v1 = model->vert(face[(j + 1) % 3]);
    //         int x0 = (v0.x + 1.) * width / 2.;
    //         int y0 = (v0.y + 1.) * height / 2.;
    //         int x1 = (v1.x + 1.) * width / 2.;
    //         int y1 = (v1.y + 1.) * height / 2.;
    //         line(x0, y0, x1, y1, image, white);
    //     }
    // }
    Vec2i t0[3] = {Vec2i(10, 70), Vec2i(50, 160), Vec2i(70, 80)};
    Vec2i t1[3] = {Vec2i(180, 50), Vec2i(150, 1), Vec2i(70, 180)};
    Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};

    triangle(t0[0], t0[1], t0[2], image, red);
    image.flip_vertically();
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}