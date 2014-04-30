#include <math.h>
#include "Kdnode.h"
#include "Object.h"

KDNode::KDNode(Object &o):o(o),
                                 axis(0),
                                 bbox(o.getBoundingBox()) {
    const Mesh & mesh = o.getMesh();
    triangles.resize(mesh.getTriangles().size());
    for(unsigned int i = 0 ; i < mesh.getTriangles().size() ; i++)
    {
        triangles[i] = i;
    }
    buildKDTree();
}

KDNode::KDNode(Object &o, std::vector<unsigned> partition, int &axis, float &q, BoundingBox &box): o(o), triangles(partition), axis(axis), medianIndix(q), bbox(box) {
    buildKDTree();
}

inline float KDNode::findMedianSample(std::vector<unsigned> & triangles, int dim){
    std::vector<float> positions;
//    positions.resize(triangles.size());
    const Mesh & mesh = o.getMesh();
    for (unsigned idT=0; idT<triangles.size(); idT++) {
        unsigned t = triangles[idT];

        for(unsigned i = 0 ; i<3 ; i++) {
            unsigned v = mesh.getTriangles()[t].getVertex(i);
            Vec3Df p = mesh.getVertices()[v].getPos();
            positions.push_back( p[dim] );
            }
        }

    std::vector<float>::iterator first = positions.begin();
    std::vector<float>::iterator last = positions.end();
    std::vector<float>::iterator middle = first + positions.size() / 2;
    std::nth_element(first, middle, last);
    unsigned mil = floor(positions.size()/2);
    return positions[mil];
}

void KDNode::splitTriangles(std::vector <unsigned> &leftTri, std::vector <unsigned> &rightTri, BoundingBox &leftBox, BoundingBox &rightBox){
    const Mesh & mesh = o.getMesh();
    for(unsigned idT=0; idT<triangles.size(); idT++) {
        unsigned t = triangles[idT];
        bool isInLeft = false;
        bool isInRight = false;

        for(unsigned i = 0 ; i<3 ; i++) {
            unsigned v = mesh.getTriangles()[t].getVertex(i);
            const Vec3Df & p = mesh.getVertices()[v].getPos();
            if(leftBox.contains(p))
                isInLeft = true;
            else if(rightBox.contains(p))
                isInRight = true;
        }

        if(isInLeft)
            leftTri.push_back(t);
        if(isInRight)
            rightTri.push_back(t);
    }
}

void KDNode::buildKDTree (){
    if(triangles.size() <= MIN_TRIANGLES) {return;}
    axis = bbox.getMaxAxis();
    medianIndix = findMedianSample(triangles, axis);
    BoundingBox leftBox;
    BoundingBox rightBox;
    bbox.split(medianIndix, axis, leftBox, rightBox);
    std::vector <unsigned> leftTri;
    std::vector <unsigned> rightTri;
    splitTriangles(leftTri, rightTri, leftBox, rightBox);
    leftChild = new KDNode(o, leftTri, axis, medianIndix, leftBox);
    rightChild = new KDNode(o, rightTri, axis, medianIndix, rightBox);
    triangles.clear();
}
