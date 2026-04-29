#include "AreaTest.h"

#include <cmath>
#include <map>

std::map<int, int> basicPolyMap{};

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

// 浮点数比较容差
static const float EPS = 1e-6f;

/**
 * 判断点 p 是否在多边形 poly 内部（含边界）
 * 使用经典的射线交叉算法（Even-Odd Rule）
 * 边界情况处理：认为点在边上也算在内部
 */
int8_t PointInPolygon(Point p, const Polygon* poly)
{
    int count = 0;
    int n = poly->n;
    const Point* v = poly->vertices;

    for(int i = 0; i < n; ++i) {
        const Point* a = &v[i];
        const Point* b = &v[(i + 1) % n];  // 边 a->b

        // 快速排除：点 p 的 y 坐标完全在边的 y 范围之外
        if((a->y > p.y && b->y > p.y) || (a->y < p.y && b->y < p.y)) continue;

        // 计算射线与边交点的 x 坐标
        // 边平行于 x 轴时，只当 p 恰好在边上才可能判断，射线法忽略水平边
        if(fabsf(a->y - b->y) < EPS) {
            // 水平边：检查点是否在水平段上
            if (fabsf(p.y - a->y) < EPS && ((a->x <= p.x && p.x <= b->x) || (b->x <= p.x && p.x <= a->x)))
                return 1;  // 在水平边上
            continue;
        }

        // 交点 x 坐标
        float intersect_x = a->x + (p.y - a->y) * (b->x - a->x) / (b->y - a->y);

        // 情况1：交点恰好在点 p 的正右侧（p 在边上）
        if(fabsf(intersect_x - p.x) < EPS)
            return 1;

        // 情况2：交点在 p 的右侧，且交点的 y 在 (a.y, b.y] 或 [b.y, a.y) 之间
        // 采用半开区间处理避免顶点重复计数：上闭下开（或下闭上开）
        if(intersect_x > p.x) {
            if(a->y < p.y && p.y <= b->y)
            {
                count++;
            }
            else if(b->y < p.y && p.y <= a->y)
            {
                count++;
            }
        }
    }

    // 射线与多边形边界交点个数为奇数 → 内部
    return (count % 2) == 1;
}

int8_t PolygonInit(Polygon* poly, Point* vertices, int n, int id)
{
    if(poly == NULL) return -1;

    // 定义凹多边形顶点（逆时针）

    poly->vertices = vertices;
    poly->n = n;
    poly->id = id;
    poly->objectNum = 0;
    poly->map = new std::map<int, int>{};
    return 1;
}

int8_t PolygonDelete(Polygon* poly)
{
    if(poly == NULL) return -1;

    poly->n = 0;
    delete static_cast<std::map<int, int>*>(poly->map);
    return 1;
}

#ifdef __cplusplus
}
#endif  // __cplusplus
