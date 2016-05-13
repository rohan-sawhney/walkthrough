#include "BoundingBox.h"

BoundingBox::BoundingBox():
min(Eigen::Vector3f::Zero()),
max(Eigen::Vector3f::Zero()),
extent(Eigen::Vector3f::Zero())
{
    
}

BoundingBox::BoundingBox(const Eigen::Vector3f& min0, const Eigen::Vector3f& max0):
min(min0),
max(max0)
{
    extent = max - min;
}

BoundingBox::BoundingBox(const Eigen::Vector3f& p):
min(p),
max(p)
{
    extent = max - min;
}

void BoundingBox::expandToInclude(const Eigen::Vector3f& p)
{
    if (min.isZero() && max.isZero()) {
        min = max = p;
        return;
    }
    
    if (min.x() > p.x()) min.x() = p.x();
    if (min.y() > p.y()) min.y() = p.y();
    if (min.z() > p.z()) min.z() = p.z();
    
    if (max.x() < p.x()) max.x() = p.x();
    if (max.y() < p.y()) max.y() = p.y();
    if (max.z() < p.z()) max.z() = p.z();
    
    extent = max - min;
}

void BoundingBox::expandToInclude(const BoundingBox& b)
{
    if (b.min.isZero() && b.max.isZero()) {
        min = b.min;
        max = b.max;
        return;
    }
    
    if (min.x() > b.min.x()) min.x() = b.min.x();
    if (min.y() > b.min.y()) min.y() = b.min.y();
    if (min.z() > b.min.z()) min.z() = b.min.z();
    
    if (max.x() < b.max.x()) max.x() = b.max.x();
    if (max.y() < b.max.y()) max.y() = b.max.y();
    if (max.z() < b.max.z()) max.z() = b.max.z();
    
    extent = max - min;
}

int BoundingBox::maxDimension() const
{
    int result = 0;
    if (extent.y() > extent.x()) result = 1;
    if (extent.z() > extent.y() && extent.z() > extent.x()) result = 2;
    
    return result;
}

bool BoundingBox::intersect(const Eigen::Vector3f& o, const Eigen::Vector3f& d,
                            float& dist) const
{
    const float& ox(o.x());
    const float& dx(d.x());
    float tMin, tMax;
    if (dx >= 0) {
        tMin = (min.x() - ox) / dx;
        tMax = (max.x() - ox) / dx;
        
    } else {
        tMin = (max.x() - ox) / dx;
        tMax = (min.x() - ox) / dx;
    }
    
    const float& oy(o.y());
    const float& dy(d.y());
    float tyMin, tyMax;
    if (dy >= 0) {
        tyMin = (min.y() - oy) / dy;
        tyMax = (max.y() - oy) / dy;
        
    } else {
        tyMin = (max.y() - oy) / dy;
        tyMax = (min.y() - oy) / dy;
    }
    
    if (tMin > tyMax || tyMin > tMax) {
        dist = INFINITY;
        return false;
    }
    
    if (tyMin > tMin) tMin = tyMin;
    if (tyMax < tMax) tMax = tyMax;
    
    const float& oz(o.z());
    const float& dz(d.z());
    float tzMin, tzMax;
    if (dz >= 0) {
        tzMin = (min.z() - oz) / dz;
        tzMax = (max.z() - oz) / dz;
        
    } else {
        tzMin = (max.z() - oz) / dz;
        tzMax = (min.z() - oz) / dz;
    }
    
    if (tMin > tzMax || tzMin > tMax) {
        dist = INFINITY;
        return false;
    }
    
    if (tzMin > tMin) tMin = tzMin;
    if (tzMax < tMax) tMax = tzMax;
    
    dist = tMin;
    return true;
}

bool BoundingBox::intersect(const Eigen::Vector3f& p, float& dist) const
{
    if (min.x() <= p.x() && p.x() <= max.x() &&
        min.y() <= p.y() && p.y() <= max.y() &&
        min.z() <= p.z() && p.z() <= max.z()) {
        
        dist = (p - (min + max)*0.5).squaredNorm();
        return true;
    }
    
    dist = INFINITY;
    return false;
}

bool BoundingBox::intersect(const BoundingBox& boundingBox, float& dist) const
{
    Eigen::Vector3f bMin = boundingBox.min;
    Eigen::Vector3f bMax = boundingBox.max;
    
    if (((min.x() <= bMin.x() && bMin.x() <= max.x()) || (bMin.x() <= min.x() && min.x() <= bMax.x())) &&
        ((min.y() <= bMin.y() && bMin.y() <= max.y()) || (bMin.y() <= min.y() && min.y() <= bMax.y())) &&
        ((min.z() <= bMin.z() && bMin.z() <= max.z()) || (bMin.z() <= min.z() && min.z() <= bMax.z()))) {
        
        dist = ((bMin + bMax - min - max)*0.5).squaredNorm();
        return true;
    }
    
    dist = INFINITY;
    return false;
}

Eigen::Vector3f BoundingBox::center() const
{
    return (max + min) / 2.0;
}
