#pragma once

#include "bspline.h"
#include "string_utils.h"
#include "curve_group.h"

namespace MH
{

static std::string serialize(CurveGroup &bspline_group)
{
    std::string res = "";
    res += Format("%d\n", bspline_group.get_child_count());
    for(int i = 0; i < bspline_group.get_child_count(); i++)
    {
        auto curve = bspline_group.get_child(i);
        if(curve->is_special_color)
        {
            res += Format("%d Green\n", curve->get_degree());
        }
        else
        {
            res += Format("%d\n", curve->get_degree());
        }
        auto& control_points = curve->get_control_points();
        res += Format("%d\n", control_points.size());
        
        for(int j = 0; j < control_points.size(); j++)
        {
            if(curve->get_dimension() == 3)
            {
                res += Format("%f  %f  %f\n", control_points[j].x, control_points[j].y, control_points[j].z);
            }
            else
            {
                res += Format("%f  %f\n", control_points[j].x, control_points[j].y);
            }
        }
        
        res += Format("%d\n", 1);
        
        auto& knot_vector = curve->get_knot_vector();
        for(int j = 0; j < knot_vector.size(); j++)
        {
            res += Format("%f ", knot_vector[j]);
        }
        
        res += "\n";
    }

    return res;
}

static std::vector<std::shared_ptr<BSpline>> deserialize(const std::string &path)
{
    auto content = ReadFile(path);
    if (content.size() == 0)
    {
        content = ReadFile("assets/" + path);
    }
    
    std::vector<std::shared_ptr<BSpline>> result;

    std::istringstream f(content);
    std::string currentLine;

    std::shared_ptr<BSpline> current_bspline;

    int remainPoints = 0;
    int type = 0;
    // 0 curve count
    // 1 point count announce
    // 2 point announce

    while (std::getline(f, currentLine))
    {
        spdlog::debug("current line is {}", currentLine);
        trim(currentLine);
        
        if (currentLine.rfind("#", 0) == 0)
        {
            spdlog::debug("Comments Line");
        }
        else if (currentLine.size() == 0)
        {
            spdlog::debug("Empty Line");
        }
        else if (currentLine[0] == '\r')
        {
            spdlog::debug("Empty Line");
        }
        else
        {
            std::vector<std::string> tokens;
            std::string res = "";

            for (size_t i = 0; i < currentLine.size(); i++)
            {
                if (currentLine[i] != ' ')
                {
                    res += currentLine[i];
                }
                else
                {
                    if (res.size() != 0)
                    {
                        tokens.push_back(res);
                    }
                    res = "";
                }
            }

            if (res.size() != 0)
            {
                tokens.push_back(res);
            }

            spdlog::debug("Token Count: {}", tokens.size());

            if (type == 0)
            {
                // curve count announce
                type = 1;
            }
            else if (type == 1)
            {
                // curve degree announcement
                auto degree = std::stoi(tokens[0]);
                current_bspline = std::make_shared<BSpline>();
                
                current_bspline->set_degree(degree);
                
                if(tokens.size() == 2)
                {
                    current_bspline->is_special_color = true;
                }

                result.push_back(current_bspline);
                type = 5;
            }
            else if (type == 5)
            {
                remainPoints = std::stoi(tokens[0]);
                type = 2;
            }
            else if (type == 2)
            {
                assert(tokens.size() == 2 || tokens.size() == 3);
                
                if(tokens.size() == 2)
                {
                    current_bspline->set_dimension(2);
                    glm::vec3 point = glm::vec3(std::stof(tokens[0]), std::stof(tokens[1]), 0.0f);
                    current_bspline->add_control_point(point);
                }
                if(tokens.size() == 3)
                {
                    current_bspline->set_dimension(3);
                    glm::vec3 point = glm::vec3(std::stof(tokens[0]), std::stof(tokens[1]), std::stof(tokens[2]));
                    current_bspline->add_control_point(point);
                }

                remainPoints -= 1;
                
                if (remainPoints == 0)
                {
                    // read knot vector definition
                    type = 3;
                }
            }
            else if (type == 3)
            {
                // know vector provided annouce
                auto hasKnotVector = std::stoi(tokens[0]);
                if(hasKnotVector == 1)
                {
                    // read knot vector
                    type = 4;
                }
                else
                {
                    // next curve
                    type = 1;
                }
            }
            else if (type == 4)
            {
                for(int i = 0; i < tokens.size(); i++)
                {
                    if(tokens[i].size() == 1)
                    {
                        if(tokens[i][0] == '\r')
                        {
                            break;
                        }
                    }
                    
                    auto knotValue = std::stof(tokens[i]);
                    current_bspline->add_knot_vector(knotValue);
                }
                
                // next curve
                type = 1;
            }
        }
    }

    return result;
}

} // namespace MH
