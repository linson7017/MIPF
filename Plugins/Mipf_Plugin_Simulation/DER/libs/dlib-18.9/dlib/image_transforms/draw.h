// Copyright (C) 2008  Davis E. King (davis@dlib.net)
// License: Boost Software License   See LICENSE.txt for the full license.
#ifndef DLIB_DRAW_IMAGe_
#define DLIB_DRAW_IMAGe_

#include "draw_abstract.h"
#include "../algs.h"
#include "../pixel.h"
#include "../matrix.h"
#include <cmath>

namespace dlib
{

// ----------------------------------------------------------------------------------------

    template <
        typename image_type,
        typename pixel_type
        >
    void draw_line (
        long x1,
        long y1,
        long x2,
        long y2,
        image_type& c,
        const pixel_type& val
    ) 
    {
        if (x1 == x2)
        {
            // make sure y1 comes before y2
            if (y1 > y2)
                swap(y1,y2);

            if (x1 < 0 || x1 >= c.nc())
                return;


            // this is a vertical line
            for (long y = y1; y <= y2; ++y)
            {
                if (y < 0 || y >= c.nr())
                    continue;

                assign_pixel(c[y][x1], val);
            }
        }
        else if (y1 == y2)
        {

            // make sure x1 comes before x2
            if (x1 > x2)
                swap(x1,x2);

            if (y1 < 0 || y1 >= c.nr())
                return;

            // this is a horizontal line
            for (long x = x1; x <= x2; ++x)
            {
                if (x < 0 || x >= c.nc())
                    continue;

                assign_pixel(c[y1][x] , val);
            }
        }
        else
        {
            const long rise = (((long)y2) - ((long)y1));
            const long run = (((long)x2) - ((long)x1));
            if (std::abs(rise) < std::abs(run))
            {
                const double slope = ((double)rise)/run;


                double first, last;


                if (x1 > x2)                
                {
                    first = x2;
                    last = x1;
                }
                else
                {
                    first = x1;
                    last = x2;
                }                             

                long y;
                long x;
                const double x1f = x1;
                const double y1f = y1;
                for (double i = first; i <= last; ++i)
                {   
                    y = static_cast<long>(slope*(i-x1f) + y1f);
                    x = static_cast<long>(i);


                    if (y < 0 || y >= c.nr())
                        continue;

                    if (x < 0 || x >= c.nc())
                        continue;


                    assign_pixel(c[y][x] , val);
                }         
            }
            else
            {
                const double slope = ((double)run)/rise;


                double first, last;


                if (y1 > y2)                
                {
                    first = y2;
                    last = y1;
                }
                else
                {
                    first = y1;
                    last = y2;
                }                             


                long x;
                long y;
                const double x1f = x1;
                const double y1f = y1;
                for (double i = first; i <= last; ++i)
                {   
                    x = static_cast<long>(slope*(i-y1f) + x1f);
                    y = static_cast<long>(i);


                    if (x < 0 || x >= c.nc())
                        continue;

                    if (y < 0 || y >= c.nr())
                        continue;

                    assign_pixel(c[y][x] , val);
                } 
            }
        }

    }

// ----------------------------------------------------------------------------------------

    template <
        typename image_type,
        typename pixel_type
        >
    void draw_line (
        image_type& c,
        const point& p1,
        const point& p2,
        const pixel_type& val
    ) 
    {
        draw_line(p1.x(),p1.y(),p2.x(),p2.y(),c,val);
    }

// ----------------------------------------------------------------------------------------

    template <
        typename image_type,
        typename pixel_type
        >
    void draw_rectangle (
        image_type& c,
        const rectangle& rect,
        const pixel_type& val
    ) 
    {
        draw_line(c, rect.tl_corner(), rect.tr_corner(), val);
        draw_line(c, rect.bl_corner(), rect.br_corner(), val);
        draw_line(c, rect.tl_corner(), rect.bl_corner(), val);
        draw_line(c, rect.tr_corner(), rect.br_corner(), val);
    }

// ----------------------------------------------------------------------------------------

    template <
        typename image_type,
        typename pixel_type
        >
    void draw_rectangle (
        image_type& c,
        const rectangle& rect,
        const pixel_type& val,
        unsigned int thickness
    ) 
    {
        for (int i = 0; i < thickness; ++i)
        {
            if ((i%2)==0)
                draw_rectangle(c,shrink_rect(rect,(i+1)/2),val);
            else
                draw_rectangle(c,grow_rect(rect,(i+1)/2),val);
        }
    }

// ----------------------------------------------------------------------------------------

    template <
        typename image_type,
        typename pixel_type
        >
    void fill_rect (
        image_type& img,
        const rectangle& rect,
        const pixel_type& pixel
    )
    {
        rectangle area = rect.intersect(get_rect(img));

        for (long r = area.top(); r <= area.bottom(); ++r)
        {
            for (long c = area.left(); c <= area.right(); ++c)
            {
                assign_pixel(img[r][c], pixel);
            }
        }
    }

// ----------------------------------------------------------------------------------------

    template <
        typename image_array_type
        >
    matrix<typename image_array_type::value_type::type> tile_images (
        const image_array_type& images
    )
    {
        typedef typename image_array_type::value_type::type T;

        if (images.size() == 0)
            return matrix<T>();

        const unsigned long size_nc = square_root(images.size());
        const unsigned long size_nr = (size_nc*(size_nc-1)>=images.size())? size_nc-1 : size_nc;
        // Figure out the size we have to use for each chip in the big main image.  We will
        // use the largest dimensions seen across all the chips.
        long nr = 0;
        long nc = 0;
        for (unsigned long i = 0; i < images.size(); ++i)
        {
            nr = std::max(images[i].nr(), nr);
            nc = std::max(images[i].nc(), nc);
        }

        matrix<T> temp(size_nr*nr, size_nc*nc);
        T background_color;
        assign_pixel(background_color, 0);
        temp = background_color;
        unsigned long idx = 0;
        for (unsigned long r = 0; r < size_nr; ++r)
        {
            for (unsigned long c = 0; c < size_nc; ++c)
            {
                if (idx < images.size())
                {
                    set_subm(temp, r*nr, c*nc, nr, nc) = mat(images[idx]);
                }
                ++idx;
            }
        }
        return temp;
    }

// ----------------------------------------------------------------------------------------

}

#endif // DLIB_DRAW_IMAGe_




