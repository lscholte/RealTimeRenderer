#ifndef CATMULLROMSPLINE_HPP
#define CATMULLROMSPLINE_HPP

#include <glm/glm.hpp>


class CatmullRomSpline
{
    public:
        CatmullRomSpline();
        CatmullRomSpline(glm::vec3 controlPoints[], int numPoints);
        ~CatmullRomSpline();

        glm::vec3 * getSamplePoints();
        glm::vec3 getNextSamplePoint();

    private:
        glm::vec3 *mSamplePoints;
        int mCurrentSamplePointNum;
        int mNumSamplePoints;
};

double tj(double ti, glm::vec3 Pi, glm::vec3 Pj);

#endif // CATMULLROMSPLINE_HPP
