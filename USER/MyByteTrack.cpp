#include "MyByteTrack.h"

#include <iostream>
#include <new>
#include <vector>

#include "Eigen/Dense"
#include "ByteTracker.h"
#include "array.h"
#include "stm32ipl_imlib.h"
#include "stm32ipl_mem_alloc.h"

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

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

BYTETracker tracker(30, 30);

void get(array_t* inArray, array_t* outArray) {
    std::vector<Object> objects;
    objects.reserve(array_length(inArray));

    for(int i = 0; i < array_length(inArray); i++) {
        rectangle_t* r = static_cast<rectangle_t*>(array_at(inArray, i));
        objects.push_back({{static_cast<float>(r->x), static_cast<float>(r->y), static_cast<float>(r->w),static_cast<float>(r->h)}, 0, 1.0f});
    }

    vector<STrack> output_stracks = tracker.update(objects);
    
    array_clear(outArray);
    for (int i = 0; i < output_stracks.size(); i++)
    {
        vector<float> tlwh = output_stracks[i].tlwh;


        bool vertical = tlwh[2] / tlwh[3] > 1.6f;
        if (tlwh[2] * tlwh[3] > 20 && !vertical)
        {
            int id = output_stracks[i].track_id;
            uint32_t color = tracker.get_color(id);

            faceIDObject_t* faceID = (faceIDObject_t*)xalloc(sizeof(faceIDObject_t));

            faceID->r.x = static_cast<int16_t>(tlwh[0]);
            faceID->r.y = static_cast<int16_t>(tlwh[1]);
            faceID->r.w = static_cast<int16_t>(tlwh[2]);
            faceID->r.h = static_cast<int16_t>(tlwh[3]);
            faceID->id = id;
            faceID->color = color;

            array_push_back(outArray, faceID);
        }
    }
}

#ifdef __cplusplus
}
#endif  // __cplusplus

