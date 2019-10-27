#pragma once

#include "bspline.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace MH
{
    class CurveGroup
    {
    public:
        
        std::shared_ptr<BSpline> add_empty_bspline()
        {
            auto new_bspline = std::make_shared<BSpline>();
            bsplines.push_back(new_bspline);
            return new_bspline;
        }
        
        std::shared_ptr<BSpline> add_bspline(glm::vec3 origin = glm::vec3(0.0f, 0.0f, 0.0f))
        {
            auto new_bspline = std::make_shared<BSpline>();
            new_bspline->add_control_point(origin);
            bsplines.push_back(new_bspline);
            
            return new_bspline;
        }
        
        std::shared_ptr<BSpline> add_bspline(std::vector<glm::vec3>& control_points)
        {
            auto new_bspline = std::make_shared<BSpline>();
            new_bspline->add_control_points(control_points);
            bsplines.push_back(new_bspline);
            return new_bspline;
        }
        
        void remove_bspline(size_t index)
        {
            bsplines.erase(bsplines.begin() + index);
        }
        
        int get_child_count()
        {
            return bsplines.size();
        }
        
        std::shared_ptr<BSpline> get_child(int index)
        {   
            return bsplines[index];
        }
        
        void add_child(std::shared_ptr<BSpline> bspline)
        {
            bsplines.push_back(bspline);
        }
        
        void clear()
        {
            bsplines.clear();
        }
        
    private:
        std::vector<std::shared_ptr<BSpline>> bsplines;
    };
} // namespace MH
