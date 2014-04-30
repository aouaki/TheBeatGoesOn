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
    float medianIndix;
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


    inline unsigned getMedianIndix() const {return medianIndix;}
    inline int getAxis() const {return axis;}
    inline KDNode * getLeftChild() const {return leftChild;}
    inline KDNode * getRightChild() const {return rightChild;}
    inline std::vector <unsigned> getTriangles() const {return triangles;}

    inline void setMedianIndix(const float& medianIndix_) {medianIndix= medianIndix_;}
    inline void setAxis(int axis_) {axis = axis_;}
    inline void setLeftChild(KDNode *leftChild_) {leftChild = leftChild_;}
    inline void setRightChild(KDNode *rightChild_) {rightChild = rightChild_;}

    inline bool isNotLeaf() const {
        if (triangles.size() == 0){return true;}
        else {return false;}
    }

    void splitTriangles(std::vector <unsigned> &leftTri, std::vector <unsigned> &rightTri, BoundingBox &leftBox, BoundingBox &rightBox);
    void buildKDTree ();

    inline float findMedianSample(std::vector<unsigned> & triangles, int dim);
};

#endif // KDNODE_H

// Some Emacs-Hints -- please don't remove:
//
//  Local Variables:
//  mode:C++
//  tab-width:4
//  End:
