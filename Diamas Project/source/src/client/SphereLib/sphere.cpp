/* Copyright (C) John W. Ratcliff, 2001.
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright (C) John W. Ratcliff, 2001"
 */

#include "Stdafx.h"
#include "sphere.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>


void Sphere::Set(const Vector3d &center, float radius)
{
#ifdef __STATIC_RANGE__
	assert(center.IsInStaticRange());
#endif
    mCenter = center;
    mRadius = radius;
    mRadius2 = radius * radius;
}

//ray-sphere intersection test from Graphics Gems p.388
// **NOTE** There is a bug in this Graphics Gem.  If the origin
// of the ray is *inside* the sphere being tested, it reports the
// wrong intersection location.  This code has a fix for the bug.
bool Sphere::RayIntersection(const Vector3d &rayOrigin,
                             const Vector3d &dir,
                             Vector3d *intersect)
{
    //notation:
    //point E  = rayOrigin
    //point O  = sphere center

    Vector3d EO = mCenter - rayOrigin;
    Vector3d V = dir;
    float dist2 = EO.x * EO.x + EO.y * EO.y + EO.z * EO.z;
    // Bug Fix For Gem, if origin is *inside* the sphere, invert the
    // direction vector so that we get a valid intersection location.
    if (dist2 < mRadius2)
        V *= -1;

    float v = EO.Dot(V);

    float disc = mRadius2 - (EO.LengthSquared() - v * v);

    if (disc > 0.0f)
    {
        if (intersect)
        {
            float d = sqrtf(disc);

            //float dist2 = rayOrigin.DistanceSq(mCenter);

            *intersect = rayOrigin + V * (v - d);
        }

        return true;
    }
    return false;
}

//
bool Sphere::RayIntersection(const Vector3d &rayOrigin,
                             const Vector3d &V,
                             float distance,
                             Vector3d *intersect)
{
    Vector3d sect;
    bool hit = RayIntersectionInFront(rayOrigin, V, &sect);

    if (hit)
    {
        float d = Vector3d::DistanceSquared(rayOrigin, sect);
        if (d > (distance * distance))
            return false;
        if (intersect)
            *intersect = sect;
        return true;
    }
    return false;
}

bool Sphere::RayIntersectionInFront(const Vector3d &rayOrigin,
                                    const Vector3d &V,
                                    Vector3d *intersect)
{
    Vector3d sect;
    bool hit = RayIntersection(rayOrigin, V, &sect);

    if (hit)
    {
        Vector3d dir = sect - rayOrigin;

        float dot = dir.Dot(V);

        if (dot >= 0) // then it's in front!
        {
            if (intersect)
                *intersect = sect;
            return true;
        }
    }
    return false;
}

void Sphere::Report(void)
{
}

/*
An Efficient Bounding Sphere
by Jack Ritter
from "Graphics Gems", Academic Press, 1990
*/

/* Routine to calculate tight bounding sphere over    */
/* a set of points in 3D */
/* This contains the routine find_bounding_sphere(), */
/* the struct definition, and the globals used for parameters. */
/* The abs() of all coordinates must be < BIGNUMBER */
/* Code written by Jack Ritter and Lyle Rains. */

#define BIGNUMBER 100000000.0  		/* hundred million */

void Sphere::Compute(const SphereInterface &source)
{
    Vector3d xmin, xmax, ymin, ymax, zmin, zmax, dia1, dia2;

    /* FIRST PASS: find 6 minima/maxima points */
    xmin = Vector3d(BIGNUMBER,BIGNUMBER,BIGNUMBER);
    xmax = Vector3d(-BIGNUMBER, -BIGNUMBER, -BIGNUMBER);
    ymin= Vector3d(BIGNUMBER,BIGNUMBER,BIGNUMBER);
    ymax=Vector3d(-BIGNUMBER, -BIGNUMBER, -BIGNUMBER);
    zmin=Vector3d(BIGNUMBER,BIGNUMBER,BIGNUMBER);
    zmax=Vector3d(-BIGNUMBER, -BIGNUMBER, -BIGNUMBER);

    int count = source.GetVertexCount();

    for (int i = 0; i < count; i++)
    {
        Vector3d caller_p;
        source.GetVertex(i, caller_p);

        if (caller_p.x < xmin.x)
            xmin = caller_p; /* New xminimum point */
        if (caller_p.x > xmax.x)
            xmax = caller_p;
        if (caller_p.y < ymin.y)
            ymin = caller_p;
        if (caller_p.y > ymax.y)
            ymax = caller_p;
        if (caller_p.z < zmin.z)
            zmin = caller_p;
        if (caller_p.z > zmax.z)
            zmax = caller_p;
    }

    /* Set xspan = distance between the 2 points xmin & xmax (squared) */
    float dx = xmax.x - xmin.x;
    float dy = xmax.y - xmin.y;
    float dz = xmax.z - xmin.z;
    float xspan = dx * dx + dy * dy + dz * dz;

    /* Same for y & z spans */
    dx = ymax.x - ymin.x;
    dy = ymax.y - ymin.y;
    dz = ymax.z - ymin.z;
    float yspan = dx * dx + dy * dy + dz * dz;

    dx = zmax.x - zmin.x;
    dy = zmax.y - zmin.y;
    dz = zmax.z - zmin.z;
    float zspan = dx * dx + dy * dy + dz * dz;

    /* Set points dia1 & dia2 to the maximally separated pair */
    dia1 = xmin;
    dia2 = xmax; /* assume xspan biggest */
    float maxspan = xspan;

    if (yspan > maxspan)
    {
        maxspan = yspan;
        dia1 = ymin;
        dia2 = ymax;
    }

    if (zspan > maxspan)
    {
        dia1 = zmin;
        dia2 = zmax;
    }

    /* dia1,dia2 is a diameter of initial sphere */
    /* calc initial center */
    mCenter.x=((dia1.x + dia2.x) * 0.5f);
    mCenter.y=((dia1.y + dia2.y) * 0.5f);
    mCenter.z=((dia1.z + dia2.z) * 0.5f);
    /* calculate initial radius**2 and radius */
    dx = dia2.x - mCenter.x; /* x component of radius vector */
    dy = dia2.y - mCenter.y; /* y component of radius vector */
    dz = dia2.z - mCenter.z; /* z component of radius vector */
    mRadius2 = dx * dx + dy * dy + dz * dz;
    mRadius = float(sqrt(mRadius2));

    /* SECOND PASS: increment current sphere */

    for (int j = 0; j < count; j++)
    {
        Vector3d caller_p;
        source.GetVertex(j, caller_p);
        dx = caller_p.x - mCenter.x;
        dy = caller_p.y - mCenter.y;
        dz = caller_p.z - mCenter.z;
        float old_to_p_sq = dx * dx + dy * dy + dz * dz;
        if (old_to_p_sq > mRadius2) /* do r**2 test first */
        {
            /* this point is outside of current sphere */
            float old_to_p = float(sqrt(old_to_p_sq));
            /* calc radius of new sphere */
            mRadius = (mRadius + old_to_p) * 0.5f;
            mRadius2 = mRadius * mRadius; /* for next r**2 compare */
            float old_to_new = old_to_p - mRadius;
            /* calc center of new sphere */
            float recip = 1.0f / old_to_p;

            float cx = (mRadius * mCenter.x + old_to_new * caller_p.x) * recip;
            float cy = (mRadius * mCenter.y + old_to_new * caller_p.y) * recip;
            float cz = (mRadius * mCenter.z + old_to_new * caller_p.z) * recip;

            mCenter.x = cx;
            mCenter.y = cy;
            mCenter.z = cz;
        }
    }
}
