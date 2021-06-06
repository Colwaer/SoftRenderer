#include <cmath>
#include <vector>
#include <time.h>
#include <limits>
#include "geometry.h"
#include "tgaimage.h"
#include "model.h"

using namespace std;

Model *model = NULL;

const int width = 800;
const int height = 800;
const int depth = 255;

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);
const TGAColor yellow = TGAColor(255, 255, 0, 255);

Vec3f light_dir(0, 0, -1);
Vec3f camera(0, 0, 3);

Vec3f Matrix2Vector(Matrix m)
{
    return Vec3f(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
}
Matrix Vector2Matrix(Vec3f v)
{
    Matrix m(4, 1);
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
    m[3][0] = 1.f;
    return m;
}
// x和y为左下角的坐标
Matrix viewport(int x, int y, int w, int h)
{
    Matrix m = Matrix::identity(4);
    m[0][3] = x + w / 2.f;
    m[1][3] = y + h / 2.f;
    m[2][3] = depth / 2.f;

    m[0][0] = w / 2.f;
    m[1][1] = h / 2.f;
    m[2][2] = depth / 2.f;
    return m;
}
Vec3f cross(Vec3f p0, Vec3f p1)
{
    return Vec3f(p0.y * p1.z - p0.z * p1.y, p0.z * p1.x - p0.x * p1.z, p0.x * p1.y - p0.y * p1.x);
}
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
Vec3f Barycentric(Vec2i *pts, Vec2i P)
{
    Vec3f u = cross(Vec3f(pts[2].x - pts[0].x, pts[1].x - pts[0].x, pts[0].x - P.x), Vec3f(pts[2].y - pts[0].y, pts[1].y - pts[0].y, pts[0].y - P.y));
    // TODO:I don't understand this
    /* `pts` and `P` has integer value as coordinates
    so `abs(u[2])` < 1 means `u[2]` is 0, that means
    triangle is degenerate, in this case return something with negative coordinates */
    if (std::abs(u.z) < 1)
        return Vec3f(-1, 1, 1);
    return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}
Vec3f Barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P)
{
    Vec3f s[2];
    for (int i = 2; i--;)
    {
        s[i].x = C.raw[i] - A.raw[i];
        s[i].y = B.raw[i] - A.raw[i];
        s[i].z = A.raw[i] - P.raw[i];
    }
    Vec3f u = cross(s[0], s[1]);
    if (abs(u.z) > 1e-2)
        return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
    return Vec3f(-1, 1, 1);
}
void Triangle(Vec2i *pts, TGAImage &image, TGAColor color)
{
    Vec2i bboxmin(image.get_width() - 1, image.get_height() - 1);
    Vec2i bboxmax(0, 0);
    Vec2i clamp(image.get_width() - 1, image.get_height() - 1);
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            bboxmin.raw[j] = max(0, min(bboxmin.raw[j], pts[i].raw[j]));
            bboxmax.raw[j] = max(clamp.raw[j], max(bboxmax.raw[j], pts[i].raw[j]));
        }
    }
    Vec2i P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
    {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
        {
            Vec3f bc_screen = Barycentric(pts, P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
                continue;
            image.set(P.x, P.y, color);
        }
    }
}
void triangle(Vec3f *pts, float *zBuffer, TGAImage &image, TGAColor color)
{
    Vec2f bboxmin(numeric_limits<float>::max(), numeric_limits<float>::max());
    Vec2f bboxmax(-numeric_limits<float>::max(), -numeric_limits<float>::max());
    Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            bboxmin.raw[j] = max(0.f, min(bboxmin.raw[j], pts[i].raw[j]));
            bboxmax.raw[j] = max(clamp.raw[j], max(bboxmax.raw[j], pts[i].raw[j]));
        }
    }
    Vec3f P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
    {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
        {
            Vec3f bc_screen = Barycentric(pts[0], pts[1], pts[2], P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
                continue;
            P.z = 0;
            for (int i = 0; i < 3; i++)
                P.z += pts[i].raw[2] * bc_screen.raw[i];
            if (zBuffer[int(P.x + P.y * width)] < P.z)
            {
                zBuffer[int(P.x + P.y * width)] = P.z;
                image.set(P.x, P.y, color);
            }
        }
    }
}
void triangle(Vec3f *pts, float *zBuffer, Vec2i uv0, Vec2i uv1, Vec2i uv2, TGAImage &image)
{
    Vec2f bboxmin(numeric_limits<float>::max(), numeric_limits<float>::max());
    Vec2f bboxmax(-numeric_limits<float>::max(), -numeric_limits<float>::max());
    Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            bboxmin.raw[j] = max(0.f, min(bboxmin.raw[j], pts[i].raw[j]));
            bboxmax.raw[j] = max(clamp.raw[j], max(bboxmax.raw[j], pts[i].raw[j]));
        }
    }
    Vec3f P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
    {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
        {
            Vec3f bc_screen = Barycentric(pts[0], pts[1], pts[2], P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
                continue;
            P.z = 0;
            for (int i = 0; i < 3; i++)
                P.z += pts[i].raw[2] * bc_screen.raw[i];
            if (zBuffer[int(P.x + P.y * width)] < P.z)
            {
                TGAColor color = model->diffuse(uv0 * bc_screen[0] + uv1 * bc_screen[1] + uv2 * bc_screen[2]);

                zBuffer[int(P.x + P.y * width)] = P.z;
                image.set(P.x, P.y, color);
            }
        }
    }
}
Vec3f World2Screen(Vec3f v)
{
    return Vec3f(int((v.x + 1.) * width / 2. + .5), int((v.y + 1.) * height / 2. + .5), v.z);
}
int main()
{
    srand((unsigned)time(NULL));
    model = new Model("obj/african_head.obj");
    
    TGAImage image(width, height, TGAImage::RGB);
    TGAImage texture;
    texture.read_tga_file("obj/african_head_diffuse.tga");
    // Vec2i t0[3] = {Vec2i(110, 70), Vec2i(150, 160), Vec2i(170, 80)};
    // Vec2i t1[3] = {Vec2i(180, 50), Vec2i(150, 1), Vec2i(70, 180)};
    // Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};
    // Triangle(t0, image, red);

    float *zBuffer = new float[width * height];
    for (int i = width * height; i--; zBuffer[i] = numeric_limits<float>::max());

    Matrix Projection = Matrix::identity(4);
    Matrix ViewPort = viewport(5., 5., width - 20, height - 20);
    Projection[3][2] = -1.f / camera.z;
    for (int i = 0; i < width * height; i++)
        zBuffer[i] = INT_MIN;
    for (int i = 0; i < model->nfaces(); i++)
    {
        vector<int> face = model->face(i);
        Vec3f screen_coords[3];
        Vec3f world_coords[3];
        for (int j = 0; j < 3; j++)
        {
            Vec3f v = model->vert(face[j]);
            screen_coords[j] = (Vec3i)Matrix2Vector(ViewPort * Projection * Vector2Matrix(v));
            world_coords[j] = v;
        }
        Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
        n.normalize();
        float intensity = n * light_dir;
        if (intensity > 0)
        {
            Vec2i uv[3];
            for (int k = 0; k < 3; k++)
            {
                uv[k] = model->uv(i, k);
            }
            triangle(screen_coords, zBuffer, uv[0], uv[1], uv[2], image);
        }
        // Triangle(screen_coords, image, TGAColor(rand() % 255, rand() % 255, rand() % 255, 255));
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}