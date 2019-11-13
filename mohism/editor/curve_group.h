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
        
        void add_child(std::shared_ptr<BSplineSurface> bspline_surface)
        {
            bspline_surfaces.push_back(bspline_surface);
        }
        
        void clear()
        {
            bsplines.clear();
            bspline_surfaces.clear();
        }
        
        glm::vec4 caculate_bounding_box()
        {
            float minX = 9999999.9f;
            float minY = 9999999.9f;
            float maxX = -999999.9f;
            float maxY = -999999.9f;
            
            for(size_t index = 0; index < bsplines.size(); index++)
            {
                auto& controlPoints = bsplines[index]->get_control_points();
                for(size_t point_index = 0; point_index < controlPoints.size(); point_index++)
                {
                    auto point = controlPoints[point_index];
                    if(point.x > maxX)
                    {
                        maxX = point.x;
                    }
                    if(point.x < minX)
                    {
                        minX = point.x;
                    }
                    if(point.y > maxY)
                    {
                        maxY = point.y;
                    }
                    if(point.y < minY)
                    {
                        minY = point.y;
                    }
                }
            }
            
            return glm::vec4(minX, maxX, minY, maxY);
        }
        
    private:
        std::vector<std::shared_ptr<BSpline>> bsplines;
        std::vector<std::shared_ptr<BSplineSurface>> bspline_surfaces;
    };
} // namespace MH
