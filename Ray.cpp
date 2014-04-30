// *********************************************************
// Ray Class
// Author : Tamy Boubekeur (boubek@gmail.com).
// Copyright (C) 2010 Tamy Boubekeur.
// All rights reserved.
// *********************************************************

#include "Ray.h"
#include "Triangle.h"
#define EPSILON 1e-8

using namespace std;

static const unsigned int NUMDIM = 3, RIGHT = 0, LEFT = 1, MIDDLE = 2;

bool Ray::intersect (const BoundingBox & bbox, Vec3Df & intersectionPoint) const {
    const Vec3Df & minBb = bbox.getMin ();
    const Vec3Df & maxBb = bbox.getMax ();
    bool inside = true;
    unsigned int  quadrant[NUMDIM];
    register unsigned int i;
    unsigned int whichPlane;
    Vec3Df maxT;
    Vec3Df candidatePlane;
    
    for (i=0; i<NUMDIM; i++)
        if (origin[i] < minBb[i]) {
            quadrant[i] = LEFT;
            candidatePlane[i] = minBb[i];
            inside = false;
        } else if (origin[i] > maxBb[i]) {
            quadrant[i] = RIGHT;
            candidatePlane[i] = maxBb[i];
            inside = false;
        } else	{
            quadrant[i] = MIDDLE;
        }

    if (inside)	{
        intersectionPoint = origin;
        return (true);
    }

    for (i = 0; i < NUMDIM; i++)
        if (quadrant[i] != MIDDLE && direction[i] !=0.)
            maxT[i] = (candidatePlane[i]-origin[i]) / direction[i];
        else
            maxT[i] = -1.;

    whichPlane = 0;
    for (i = 1; i < NUMDIM; i++)
        if (maxT[whichPlane] < maxT[i])
            whichPlane = i;

    if (maxT[whichPlane] < 0.) return (false);
    for (i = 0; i < NUMDIM; i++)
        if (whichPlane != i) {
            intersectionPoint[i] = origin[i] + maxT[whichPlane] *direction[i];
            if (intersectionPoint[i] < minBb[i] || intersectionPoint[i] > maxBb[i])
                return (false);
        } else {
            intersectionPoint[i] = candidatePlane[i];
        }
    return (true);			
}

bool Ray::intersectTriangle(const Vec3Df & vertex1, const Vec3Df & vertex2, const Vec3Df & vertex3, Vec3Df & triangleNormal, float & intersectionDistance){


    Vec3Df e0=vertex2-vertex1;
    Vec3Df e1=vertex3-vertex1;

    Vec3Df q = Vec3Df::crossProduct(direction,e1);
    float a = Vec3Df::dotProduct(e0,q);

    if(Vec3Df::dotProduct(triangleNormal,direction)>=0 || a<EPSILON){
        return false;
    }

    else{

            Vec3Df s = (origin - vertex1)/a;
            Vec3Df r = Vec3Df::crossProduct(s,e0);
            float b0 = Vec3Df::dotProduct(s,q);
            float b1 = Vec3Df::dotProduct(r,direction);
            float b2 = 1 - b0 - b1;
            if(b0<0 || b1<0 || b2<0){

                return false;
        }

        else{
            intersectionDistance = Vec3Df::dotProduct(e1,r);
            return true;
        }
    }



}
