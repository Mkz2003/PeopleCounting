#include "MyByteTrack.h"

#include <iostream>
#include <new>
#include <algorithm>
#include <cmath>
#include <vector>

#include "Eigen/Dense"
#include "ByteTracker.h"
#include "array.h"
#include "dcmi_ov5640.h"
#include "stm32ipl_imlib.h"
#include "stm32ipl_mem_alloc.h"

#ifdef __cplusplus

void* operator new(std::size_t size) {
    return (xalloc(size));
}

void* operator new[](std::size_t size) {
    return (xalloc(size));
}

void operator delete(void* ptr) {
    return (xfree(ptr));
}

void operator delete[](void* ptr) {
    return (xfree(ptr));
}

// 计算两个矩形框的 IoU（交并比）
float compute_iou(const Object& a, const Object& b) {
    // 转换为边界坐标（左上角 + 右下角）
    float ax1 = a.rect.x;
    float ay1 = a.rect.y;
    float ax2 = (a.rect.x + a.rect.width);
    float ay2 = (a.rect.y + a.rect.height);

    float bx1 = b.rect.x;
    float by1 = b.rect.y;
    float bx2 = (b.rect.x + b.rect.width);
    float by2 = (b.rect.y + b.rect.height);

    // 计算交集区域
    float inter_x1 = std::max(ax1, bx1);
    float inter_y1 = std::max(ay1, by1);
    float inter_x2 = std::min(ax2, bx2);
    float inter_y2 = std::min(ay2, by2);

    float inter_area = std::max(0.0f, inter_x2 - inter_x1) * std::max(0.0f, inter_y2 - inter_y1);
    if (inter_area <= 0.0f) return 0.0f;

    // 计算并集区域
    float a_area = a.rect.width * a.rect.height;
    float b_area = b.rect.width * b.rect.height;
    float union_area = a_area + b_area - inter_area;

    return inter_area / union_area;
}

// NMS 主函数
// objects: 输入检测结果
// iou_threshold: IoU 阈值，高于此值的重叠框将被抑制
// per_class: 是否按类别分别进行 NMS（通常设为 true）
// 返回值：经过 NMS 过滤后的结果
std::vector<Object> nms(const std::vector<Object>& objects, float iou_threshold, bool per_class = true) {
    std::vector<Object> result;
    if (objects.empty()) return result;

    // 如果按类别处理，则先按 label 分组，分别执行 NMS
    if (per_class) {
        // 按 label 分组，每个 label 单独处理
        std::map<int, std::vector<Object>> class_objects;
        for (const auto& obj : objects) {
            class_objects[obj.label].push_back(obj);
        }

        for (auto& pair : class_objects) {
            auto class_result = nms(pair.second, iou_threshold, false); // 递归调用，但关闭 per_class
            result.insert(result.end(), class_result.begin(), class_result.end());
        }
        return result;
    }

    // 全局 NMS（单个类别内或整体）
    // 1. 按概率降序排序
    std::vector<Object> sorted = objects;
    std::sort(sorted.begin(), sorted.end(),
              [](const Object& a, const Object& b) { return a.prob > b.prob; });

    std::vector<bool> suppressed(sorted.size(), false); // 标记是否被抑制

    for (size_t i = 0; i < sorted.size(); ++i) {
        if (suppressed[i]) continue;

        // 保留当前框
        result.push_back(sorted[i]);

        // 抑制与当前框 IoU 过大的后续框
        for (size_t j = i + 1; j < sorted.size(); ++j) {
            if (suppressed[j]) continue;
            float iou = compute_iou(sorted[i], sorted[j]);
            if (iou > iou_threshold) {
                suppressed[j] = true;
            }
        }
    }

    return result;
}


#endif  // __cplusplus

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

BYTETracker tracker(1, 120);

void get(array_t* inArray, array_t* outArray) {
    std::vector<Object> raw_objects;
    raw_objects.reserve(array_length(inArray));

    for(int i = 0; i < array_length(inArray); i++) {
        faceObject_t* f = static_cast<faceObject_t*>(array_at(inArray, i));
        raw_objects.push_back({{f->rect.x, f->rect.y, f->rect.width, f->rect.height}, f->label, f->prob});
    }

    std::vector<Object> objects = nms(raw_objects, 0.5f, true);
    // std::vector<Object> objects = raw_objects;
    vector<STrack> output_stracks = tracker.update(objects);
    
    array_clear(outArray);
    for (int i = 0; i < output_stracks.size(); i++)
    {
        vector<float> tlwh = output_stracks[i].tlwh;


        bool vertical = tlwh[2] / tlwh[3] > 1.6f;
        if((tlwh[2] * tlwh[3] > 20.0f && !vertical) || 1)
        {
            int id = output_stracks[i].track_id;
            uint32_t color = tracker.get_color(id);

            faceIDObject_t* faceID = (faceIDObject_t*)xalloc(sizeof(faceIDObject_t));

            faceID->r.x = (tlwh[0]);
            faceID->r.y = (tlwh[1]);
            faceID->r.width = (tlwh[2]);
            faceID->r.height = (tlwh[3]);
            faceID->id = id;
            faceID->color = color;

            array_push_back(outArray, faceID);
        }
    }
}

#ifdef __cplusplus
}
#endif  // __cplusplus

