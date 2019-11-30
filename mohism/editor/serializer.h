#pragma once

#include "bspline.h"
#include "string_utils.h"
#include "curve_group.h"
#include "matrix.h"

namespace MH
{

static std::string serialize(CurveGroup &bspline_group)
{
    std::string res = "";
    res += Format("%d", bspline_group.get_child_count());
    auto bounding_box = bspline_group.caculate_bounding_box();
    auto width = bounding_box.y - bounding_box.x;
    auto height = bounding_box.w - bounding_box.z;
    
    // 3 inches width
    auto W = 3.6f;
    auto H = (W * height) / width;
    
    res += Format(" %f %f %f %f\n", W * -0.5f, W * 0.5f, H * -0.5f, H * 0.5f);
    
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
    
static std::vector<std::shared_ptr<BSplineSurface>> deserialize_surface(const std::string &path)
{
    auto content = ReadFile(path);
    if (content.size() == 0)
    {
        content = ReadFile("assets/" + path);
    }
    
    std::vector<std::shared_ptr<BSplineSurface>> result;
    
    std::istringstream f(content);
    std::string currentLine;
    
    std::shared_ptr<BSplineSurface> current_bspline_surface;
    int currentX = 0;
    int currentY = 0;
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
            
            // surface count announce
            if (type == 0)
            {
                type = 1;
            }
            // degree announce
            else if (type == 1)
            {
                auto degree_u = std::stoi(tokens[0]);
                auto degree_v = std::stoi(tokens[1]);
                
                current_bspline_surface = std::make_shared<BSplineSurface>();
                current_bspline_surface->degree_u = degree_u;
                current_bspline_surface->degree_v = degree_v;
                
                result.push_back(current_bspline_surface);
                
                type = 2;
            }
            // knot length announce
            else if (type == 2)
            {
                auto knot_length_u = std::stoi(tokens[0]);
                auto knot_length_v = std::stoi(tokens[1]);
                
                current_bspline_surface->knot_length_u = knot_length_u;
                current_bspline_surface->knot_length_v = knot_length_v;
                
                type = 3;
            }
            // u knot
            else if (type == 3)
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
                    current_bspline_surface->knot_u.push_back(knotValue);
                }
                type = 4;
            }
            // v knot
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
                    current_bspline_surface->knot_v.push_back(knotValue);
                }
                
                currentX = 0;
                currentY = 0;
                type = 5;
            }
            // control points, 0,0 0,1 0,n, 1,0 1,1 1,n m,0 m,1 m,n
            else if (type == 5)
            {
                int n = current_bspline_surface->knot_length_v - current_bspline_surface->degree_v - 1 - 1;
                int m = current_bspline_surface->knot_length_u - current_bspline_surface->degree_u - 1 - 1;
                
                if(tokens.size() >= 4 && tokens[3][0] != '\r')
                {
                    glm::vec4 point = glm::vec4(std::stof(tokens[0]), std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]));
                    current_bspline_surface->control_points.push_back(point);
                }
                else
                {
                    glm::vec4 point = glm::vec4(std::stof(tokens[0]), std::stof(tokens[1]), std::stof(tokens[2]), 1.0f);
                    current_bspline_surface->control_points.push_back(point);
                }
                
               
                currentX++;
                if(currentX > n)
                {
                    currentX = 0;
                    currentY++;
                    if(currentY > m)
                    {
                        current_bspline_surface->compute_derived_date();
                        // next surface
                        type = 1;
                    }
                }
            }
        }
    }
    
    return result;
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
    
static std::vector<std::shared_ptr<BSplineSurface>> deserialize_nodal(const std::string &path)
{
    auto content = ReadFile(path);
    if (content.size() == 0)
    {
        content = ReadFile("assets/" + path);
    }
    
    std::vector<std::shared_ptr<BSplineSurface>> result;
    std::shared_ptr<BSplineSurface> surface = std::make_shared<BSplineSurface>();
    result.push_back(surface);
    
    std::istringstream f(content);
    std::string currentLine;
    
    int type = 1;
    int remain_points = 0;
    int currentPointIndex = 0;
    
    int curveCount;
    int curveDegree;
    int numControlPoints;
    std::vector<float> knots;
    
    std::vector<std::shared_ptr<BSpline>> curves;
    std::shared_ptr<BSpline> curve;
    
    // 1 num, degree, control points annouce
    // 2 knot annouce
    // 3 control points
    
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
            
            // 0 -- curve count 1 -- degree 3 -- control points
            if (type == 1)
            {
                curveCount = std::stoi(tokens[0]);
                curveDegree = std::stoi(tokens[1]);
                numControlPoints = std::stoi(tokens[2]);
                
                remain_points = curveCount * numControlPoints;
                
                type = 2;
            }
            // knot vector
            else if (type == 2)
            {
                int used_knot_num = numControlPoints + curveDegree + 1;
                for(int i = 0; i < used_knot_num; i++)
                {
                    knots.push_back(std::stof(tokens[i]));
                }
                type = 3;
            }
            else if (type == 3)
            {
                if(currentPointIndex == 0)
                {
                    // create a new Curve
                    curve = std::make_shared<BSpline>();
                    curve->add_knot_vector(knots);
                    curve->set_degree(curveDegree);
                }
                
                glm::vec3 point = glm::vec3(
                                            std::stof(tokens[0]),
                                            std::stof(tokens[1]),
                                            std::stof(tokens[2]));
                
                curve->set_dimension(3);
                curve->add_control_point(point);
                
                currentPointIndex++;
                if(currentPointIndex >= numControlPoints)
                {
                    curves.push_back(curve);
                    currentPointIndex = 0;
                }
            }
        }
    }
    
    surface->nodal_curves = curves;
    surface->ForNodal = true;
    
    surface->degree_u = curveDegree;
    surface->degree_v = 2;
    
    surface->knot_u = knots;
    surface->knot_length_u = knots.size();
    
    surface->knot_length_v = surface->degree_v + curveCount + 1;
    // compute knot_v
    std::vector<float> knot_v;
    int used_knot_num = surface->knot_length_v;
    int degree = surface->degree_v;
    
    bool uniform = false;
    
    if(uniform)
    {
        for(int i = 0; i < used_knot_num; i++)
        {
            knot_v.push_back(i);
        }
    }
    else
    {
        for(int i = 0; i < used_knot_num; i++)
        {
            if(i <= degree)
            {
                knot_v.push_back(0.0f);
            }
            else if (i >= (used_knot_num - degree - 1))
            {
                if(i == (used_knot_num - 1))
                {
                    knot_v.push_back(knot_v[used_knot_num - degree - 2] + 1.5f);
                }
                else
                {
                    knot_v.push_back(knot_v[used_knot_num - degree - 2] + 1.0f);
                }
            }
            else
            {
                knot_v.push_back(knot_v[i - 1] + 1.0f);
            }
        }
    }
    
    surface->knot_v = knot_v;
    surface->compute_domain();
    surface->compute_model_spline();
    
    int n = surface->knot_length_v - surface->degree_v - 1 - 1;
    int m = surface->knot_length_u - surface->degree_u - 1 - 1;
    
    // compute control points
    // build H matrix
    double H_X[50][50] = {};
    double H_Y[50][50] = {};
    double H_Z[50][50] = {};
    
    for(int i = 0; i <= n; i++)
    {
        for(int j = 0; j <= m; j++)
        {
            auto theCurve = curves[i];
            H_X[j][i] = theCurve->get_control_points()[j].x;
            H_Y[j][i] = theCurve->get_control_points()[j].y;
            H_Z[j][i] = theCurve->get_control_points()[j].z;
        }
    }
    
    // Build F Matrix
    double F[50][50] = {};
    for(int i = 0; i <= m; i++)
    {
        for(int j = 0; j <= m; j++)
        {
            F[j][i] = surface->get_nodal_blending_u(i, j);
        }
    }
    
    // Build F Inverse
    double FInverse[50][50] = {};
    GetMatrixInverse(F, m + 1, FInverse);
    
    // Build G Transpose
    double GTranspose[50][50] = {};
    for(int i = 0; i <= n; i++)
    {
        for(int j = 0; j <= n; j++)
        {
            GTranspose[i][j] = surface->get_nodal_blending_v(i, j);
        }
    }
    
    // Build G Transpose Inverse
    double GTransposeInverse[50][50] = {};
    GetMatrixInverse(GTranspose, n + 1, GTransposeInverse);
    
    // Calcu FInverse * H
    double FIHX[50][50] = {};
    double FIHY[50][50] = {};
    double FIHZ[50][50] = {};
    MatrixMultiply(FInverse, H_X, FIHX, m + 1, m + 1, n + 1);
    MatrixMultiply(FInverse, H_Y, FIHY, m + 1, m + 1, n + 1);
    MatrixMultiply(FInverse, H_Z, FIHZ, m + 1, m + 1, n + 1);
    
    // calcu FIH * GTranspose Inverse
    double CX[50][50] = {};
    double CY[50][50] = {};
    double CZ[50][50] = {};
    MatrixMultiply(FIHX, GTransposeInverse, CX, m + 1, n + 1, n + 1);
    MatrixMultiply(FIHY, GTransposeInverse, CY, m + 1, n + 1, n + 1);
    MatrixMultiply(FIHZ, GTransposeInverse, CZ, m + 1, n + 1, n + 1);
    
    for(int j = 0; j <= m; j++)
    {
        for(int i = 0; i <= n; i++)
        {
            glm::vec4 point = glm::vec4(CX[j][i], CY[j][i], CZ[j][i], 1.0f);
            surface->control_points.push_back(point);
        }
    }
    
    surface->compute_derived_data_for_nodal();
    
    return result;
}

} // namespace MH
