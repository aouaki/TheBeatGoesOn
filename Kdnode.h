// *********************************************************
// KD-Node Class
// *********************************************************

#ifndef KDNODE_H
#define KDNODE_H

#include <vector>
#include "Vec3D.h"
#include "BoundingBox.h"
#include "Mesh.h"

class Object;

class KDNode
{
protected:
    Object &o;
    std::vector<unsigned> triangles;
    int axis;
    float median;
    KDNode *leftChild, *rightChild;

public:
    static const unsigned MIN_TRIANGLES = 30;
    const BoundingBox bbox;
    KDNode(Object &o);
    KDNode(Object &o, std::vector<unsigned> partition, int &axis, float &q, BoundingBox &box);

    ~KDNode() {
        delete leftChild;
        delete rightChild;
    }


    inline float getMedian() const {return median;}
    inline int getAxis() const {return axis;}
    inline KDNode * getLeftChild() const {return leftChild;}
    inline KDNode * getRightChild() const {return rightChild;}
    inline std::vector <unsigned> getTriangles() const {return triangles;}

    inline void setmedian(const float& median_) {median= median_;}
    inline void setAxis(int axis_) {axis = axis_;}
    inline void setLeftChild(KDNode *leftChild_) {leftChild = leftChild_;}
    inline void setRightChild(KDNode *rightChild_) {rightChild = rightChild_;}

    inline bool isLeaf() const {
        if (triangles.size() == 0){return false;}
        else {return true;}
    }

    void splitTriangles(std::vector <unsigned> &leftTri, std::vector <unsigned> &rightTri, BoundingBox &leftBox, BoundingBox &rightBox);
    void buildKDTree ();

    inline float findMedianSample(std::vector<unsigned> & triangles, int dim);
    bool boxTriangleIntersectionTest(const Vec3Df &A, const Vec3Df &B, const Vec3Df &C, BoundingBox box);
};

#endif // KDNODE_H

// Some Emacs-Hints -- please don't remove:
//
//  Local Variables:
//  mode:C++
//  tab-width:4
//  End:
