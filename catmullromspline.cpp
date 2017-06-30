#include "catmullromspline.hpp"
#include <algorithm>

CatmullRomSpline::CatmullRomSpline() {

}

CatmullRomSpline::CatmullRomSpline(glm::vec3 controlPoints[], int numPoints)
{
    mCurrentSamplePointNum = 0;
    const int numPointsPerSpline = 100;
    if(numPoints < 4) {
        //TODO: Handle this argument error
    }
    else if(numPoints == 4) {
        mNumSamplePoints = numPointsPerSpline;

        glm::dvec3 p0 = controlPoints[0];
        glm::dvec3 p1 = controlPoints[1];
        glm::dvec3 p2 = controlPoints[2];
        glm::dvec3 p3 = controlPoints[3];

        double t0 = 0;
        double t1 = tj(t0, p0, p1);
        double t2 = tj(t1, p1, p2);
        double t3 = tj(t2, p2, p3);

        mSamplePoints = new glm::vec3[numPointsPerSpline];
        double t = t1;
        for(int i = 0; i < numPointsPerSpline; ++i) {
            glm::dvec3 A1 = (t1-t)/(t1-t0)*p0 + (t-t0)/(t1-t0)*p1;
            glm::dvec3 A2 = (t2-t)/(t2-t1)*p1 + (t-t1)/(t2-t1)*p2;
            glm::dvec3 A3 = (t3-t)/(t3-t2)*p2 + (t-t2)/(t3-t2)*p3;

            glm::dvec3 B1 = (t2-t)/(t2-t0)*A1 + (t-t0)/(t2-t0)*A2;
            glm::dvec3 B2 = (t3-t)/(t3-t1)*A2 + (t-t1)/(t3-t1)*A3;

            glm::dvec3 C = (t2-t)/(t2-t1)*B1 + (t-t1)/(t2-t1)*B2;

            mSamplePoints[i] = (glm::vec3) C;
            t += (t2-t1)/(numPointsPerSpline-1);
        }
    }
    else {
        mNumSamplePoints = 0;
        mSamplePoints = new glm::vec3[(numPoints-3) * numPointsPerSpline];
        for(int i = 0; i < numPoints-3; ++i) {
            glm::vec3 subsetControlPoints[4];
            std::copy(&controlPoints[i], &controlPoints[i+4], subsetControlPoints);
            CatmullRomSpline spline = CatmullRomSpline(subsetControlPoints, 4);
            std::copy(spline.mSamplePoints, &(spline.mSamplePoints[numPointsPerSpline]), &(mSamplePoints[i*numPointsPerSpline]));
            mNumSamplePoints += spline.mNumSamplePoints;
        }
    }
}

glm::vec3 * CatmullRomSpline::getSamplePoints() {
    return mSamplePoints;
}

glm::vec3 CatmullRomSpline::getNextSamplePoint() {
    ++mCurrentSamplePointNum;
    if(mCurrentSamplePointNum == mNumSamplePoints) {
        mCurrentSamplePointNum = 0;
    }
    return mSamplePoints[mCurrentSamplePointNum];
}

CatmullRomSpline::~CatmullRomSpline() {
    if(mSamplePoints) {
        delete[] mSamplePoints;
    }
}

double tj(double ti, glm::vec3 Pi, glm::vec3 Pj) {
    const double alpha = 0.5;

    double result = 0;
    for(int i = 0; i < 3; ++i) {
        double temp = Pj[i] - Pi[i];
        result += temp * temp;
    }
    result = sqrt(result);
    result = pow(result, alpha);
    return result + ti;
}
