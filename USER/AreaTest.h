#ifndef __AREATEST_H
#define __AREATEST_H

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

#include <stdint.h>

// 归一化坐标点
typedef struct {
    float x;
    float y;
} Point;

// 多边形：逆时针排列的点序列
typedef struct {
    Point* vertices;  // 点数组指针
    int n;            // 顶点个数
    int id;
    int objectNum;
    void* set;
} Polygon;

uint8_t PointInPolygon(Point p, const Polygon* poly);
uint8_t PolygonInit(Polygon* poly);


#ifdef __cplusplus
}
#endif  // __cplusplus

#endif // __AREATEST_H
