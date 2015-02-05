/*
  -------------------------------------------------------------------

  Copyright (C) 2006-2015, Andrew W. Steiner

  This file is part of O2scl.

  O2scl is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  O2scl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with O2scl. If not, see <http://www.gnu.org/licenses/>.

  -------------------------------------------------------------------
*/
/** \file contour.h
    \brief File defining \ref o2scl::contour
*/
#ifndef O2SCL_CONTOUR_H
#define O2SCL_CONTOUR_H

#include <cmath>

#include <gsl/gsl_math.h>

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>

#include <o2scl/interp.h>
#include <o2scl/uniform_grid.h>

#ifndef DOXYGEN_NO_O2NS
namespace o2scl {
#endif

  /** \brief A contour line 

      The contour lines generated by the \ref o2scl::contour class are given
      as objects of this type.

      \future Make this a subclass of \ref o2scl::contour .
  */
  class contour_line {

  public:

    typedef boost::numeric::ublas::vector<double> ubvector;

    /// The contour level
    double level;
    /// The line x coordinates
    std::vector<double> x;
    /// The line y coordinates
    std::vector<double> y;

    /// Create an empty line
    contour_line() {
    }

    /// Copy constructor
    contour_line(const contour_line &c) {
      level=c.level;
      x=c.x;
      y=c.y;
    }
    
    /// Copy constructor with operator=()
    contour_line &operator=(const contour_line &c) {
      if (this==&c) return *this;
      level=c.level;
      x=c.x;
      y=c.y;
      return *this;
    }

  };

  /** \brief Edges for the contour class
      
      The edge crossings generated by the \ref contour class are given
      as objects of this type.

      The \ref status matrix contains one of four possible values
      - 0 - empty (no edge)
      - 1 - edge which has not yet been assigned to a contour
      - 2 - edge assigned to contour point
      - 3 - edge which has been designated as a contour endpoint

      The matrices returned by \ref contour are not square, their
      size depends on whether or not they contain the "bottom edges"
      or the "right edges". 

      \future Make this a subclass of \ref o2scl::contour .
  */
  class edge_crossings {

  public:

    typedef boost::numeric::ublas::vector<double> ubvector;
    typedef boost::numeric::ublas::vector<int> ubvector_int;
    typedef boost::numeric::ublas::matrix<double> ubmatrix;
    typedef boost::numeric::ublas::matrix<int> ubmatrix_int;

    /// Edge status
    ubmatrix_int status;
    /// Edge values
    ubmatrix values;

    /// Create an empty object
    edge_crossings() {
    }

    /// Copy constructor
    edge_crossings(const edge_crossings &ec) {
      status=ec.status;
      values=ec.values;
    }

    /// Copy constructor with operator=()
    edge_crossings &operator=(const edge_crossings &ec) {
      if (this==&ec) return *this;
      status=ec.status;
      values=ec.values;
      return *this;
    }

  };

  /** \brief Calculate contour lines from a two-dimensional data set

      \b Basic \b Usage
      
      - Specify the data as a two-dimensional square grid of "heights"
      with set_data().
      - Specify the contour levels with set_levels().
      - Compute the contours with calc_contours()

      The contours are generated as a series of x- and y-coordinates,
      defining a line. If the contour is closed, then the first and
      the last set of coordinates will be equal. 

      The convention used by this class is that the first (row) index
      of the matrix enumerates the x coordinate and that the second
      (column) index enumerates the y coordinate. See the discussion
      in the User's guide in the section called \ref rowcol_subsect.

      The data is copied by set_data(), so changing the data will not
      change the contours unless set_data() is called again. The
      functions set_levels() and calc_contours() can be called several
      times for the same data without calling set_data() again.

      Note that in order to simplify the algorithm for computing
      contour lines, the calc_contours() function will adjust the
      user-specified contour levels slightly in order to ensure that
      no contour line passes exactly through any data point on the
      grid. The contours are adjusted by multiplying the original
      contour level by 1 plus a small number (\f$ 10^{-8} \f$ by
      default), which is specified in \ref lev_adjust.

      Linear interpolation is used to decide whether or not a line
      segment from the grid and a contour cross. This choice is
      intentional, since (in addition to making the algorithm much
      simpler) it is the user (and not this contour class) which is
      likely best able to refine the data. In case a simple refinement
      scheme is desired, the method regrid_data() is provided which
      refines the internally stored data for any interpolation type.

      Since linear interpolation is used, the contour calculation
      implicitly assumes that there is not more than one intersection
      of any contour level with any line segment. For contours which
      do not close inside the region of interest, the results will
      always end at either the minimum or maximum values of the x or y
      grid points (no extrapolation is ever done). Note also that the
      points defining the contour are not necessarily equally spaced.
      Two neighboring points will never be farther apart than the
      distance across opposite corners of one cell in the grid.

      \b The \b Algorithm:

      This works by viewing the data as defining a square
      two-dimensional grid. The function calc_contours() exhaustively
      enumerates every line segment in the grid which involves a level
      crossing and then organizes the points defined by the
      intersection of a line segment with a level curve into a full
      contour line.

      \todo Convert distances in find_next_point functions to 
      be scaled by grid spacing. 

      \todo Copy constructor

      \future Rewrite the code which adjusts the contour levels 
      to ensure contours don't go through the data to adjust the
      internal copy of the data instead? This should be more 
      accurate because we're perturbing one point instead of 
      perturbing the entire line.

      \future It would be nice to have a function which creates a set
      of closed regions to fill which represent the data. However,
      this likely requires a completely new algorithm, because it's
      not easy to simply close the contours already generated by the
      calc_contours() function. There are, for example, several cases
      which are difficult to handle, such as filling a region in
      between several closed contours 

      \future Change nx and ny to size_t?

      \comment 

      6/28/12 - Thinking about how the algorithm could work.

      1) Start at the lowest contour level. 

      2) How to create the enclosure in general if it's closed?
      Just make sure that lines betwen closed region and boundary
      don't go through any other contours of the same level.

      3) If one of the two enclosures would enclose a different
      contour line with the same level, and choose the other
      enclosure. If neither enclosure would enclose a contour line
      with the same level, then only one enclosure encloses all other
      contour lines, so that is the one to choose.

      4) Proceed to the next contour line of the same level, and
      then go to the next contour line of the second lowest
      contour level.
     
      \endcomment
  */
  class contour {
    
  public:
    
    typedef boost::numeric::ublas::vector<double> ubvector;
    typedef boost::numeric::ublas::vector<int> ubvector_int;
    typedef boost::numeric::ublas::matrix<double> ubmatrix;

    contour();

    ~contour();

    /// \name Basic usage
    //@{

    /** \brief Set the data 

	The types \c vec_t and \c mat_t can be any types which have \c
	operator[] and \c operator[][] for array and matrix indexing.
	
	Note that this method copies all of the user-specified data to
	local storage so that changes in the data after calling this
	function will not be reflected in the contours that are
	generated.
    */
    template<class vec_t, class mat_t>
      void set_data(size_t sizex, size_t sizey, const vec_t &x_fun, 
		    const vec_t &y_fun, const mat_t &udata) {
      
      if (sizex<2 || sizey<2) { 
	O2SCL_ERR("Not enough data (must be at least 2x2) in set_data().",
		  exc_einval);
      }
      
      nx=sizex;
      ny=sizey;
      xfun.resize(nx);
      yfun.resize(ny);
      data.resize(nx,ny);
      for(int i=0;i<nx;i++) xfun[i]=x_fun[i];
      for(int i=0;i<ny;i++) yfun[i]=y_fun[i];
      for(int i=0;i<nx;i++) {
	for(int j=0;j<ny;j++) {
	  data(i,j)=udata(i,j);
	}
      }

      check_data();
      return;
    }

    /** \brief Set the data 

	The type \c mat_t can be any type which has \c operator[][]
	for matrix indexing.
	
	Note that this method copies all of the user-specified data to
	local storage so that changes in the data after calling this
	function will not be reflected in the contours that are
	generated.
    */
    template<class mat_t>
      void set_data(const uniform_grid<double> &ugx, 
		   const uniform_grid<double> &ugy,
		   const mat_t &udata) {
      
      size_t sizex=ugx.get_npoints();
      size_t sizey=ugy.get_npoints();
      
      if (sizex<2 || sizey<2) { 
	O2SCL_ERR("Not enough data (must be at least 2x2) in set_data().",
		  exc_einval);
      }
      
      nx=sizex;
      ny=sizey;
      xfun.resize(nx);
      yfun.resize(ny);
      data.resize(nx,ny);
      for(int i=0;i<nx;i++) xfun[i]=ugx[i];
      for(int i=0;i<ny;i++) yfun[i]=ugy[i];
      for(int i=0;i<nx;i++) {
	for(int j=0;j<ny;j++) {
	  data(i,j)=udata(i,j);
	}
      }

      check_data();
      return;
    }

    /** \brief Set the contour levels 
	
	This is separate from the function calc_contours() so that
	the user can compute the contours for different data sets using
	the same levels.
    */
    template<class vec_t> void set_levels(size_t nlevels, vec_t &ulevels) {
      nlev=nlevels;
      levels.resize(nlevels);
      for(size_t i=0;i<nlevels;i++) {
	levels[i]=ulevels[i];
      }
      levels_set=true;
      return;
    }

    /** \brief Calculate the contours 
	
	\note There may be zero or more than one contour line for
	each level, so the size of \c clines is not necessarily
	equal to the number of levels specified in \ref set_levels().
    */
    void calc_contours(std::vector<contour_line> &clines);
    //@}

    /// \name Regrid function
    //@{
    /** \brief Regrid the data 
	
	Use interpolation to refine the data set. This can be called
	before calc_contours() in order to attempt make the contour
	levels smoother by providing a smaller grid size. If the
	original number of data points is \f$
	(\mathrm{nx},\mathrm{ny}) \f$, then the new number of data
	points is
	\f[
	(\mathrm{xfact}~(\mathrm{nx}-1)+1,
	\mathrm{yfact}~(\mathrm{ny}-1)+1)
	\f]
	The parameters \c xfact and \c yfact must both be larger than 
	zero and they cannot both be 1.
    */
    void regrid_data(size_t xfact, size_t yfact, 
		    size_t interp_type=o2scl::itp_cspline);
    //@}

    /// \name Obtain internal data
    //@{
    /** \brief Get the data 
	
	This is useful to see how the data has changed after
	a call to regrid_data().

	\future There is probably a better way than returning
	pointers to the internal data.
    */
    void get_data(size_t &sizex, size_t &sizey, ubvector *&x_fun, 
		  ubvector *&y_fun, ubmatrix *&udata) {
      if (nx==0) {
	O2SCL_ERR("Data not set in get_data().",exc_einval);
      }
      sizex=nx;
      sizey=ny;
      x_fun=&xfun;
      y_fun=&yfun;
      udata=&data;
      return;
    }

    /** \brief Return the edges for each contour level

	The size of \c rt_edges and \c bm_edges will both be equal to
	the number of levels set by \ref set_levels().
     */
    void get_edges(std::vector<edge_crossings> &rt_edges,
		  std::vector<edge_crossings> &bm_edges) {
      rt_edges=red;
      bm_edges=bed;
      return;
    }

    /// Print out the edges to cout
    void print_edges(edge_crossings &right,
		     edge_crossings &bottom);
    
    //@}
    
    /** \brief Verbosity parameter (default 0)

	If verbose is greater than 0, then adjustments to the contour
	levels will be output and the contour lines will be output as
	they are built up from the intersections. If verbose is
	greater than 1, then all edges will be output. If verbose is
	greater than 2, a keypress will be required after each contour
	line is constructed.
    */
    int verbose;

    /// (default \f$ 10^{-8} \f$)
    double lev_adjust;
    
    /// \name Edge status
    //@{
    static const int empty=0;
    static const int edge=1;
    static const int contourp=2;
    static const int endpoint=3;
    //@}

#ifndef DOXYGEN_INTERNAL

  protected:

    /// \name Edge direction
    //@{
    static const int dright=0;
    static const int dbottom=1;
    //@}

    /// \name Edge found or not found
    //@{
    static const int efound=1;
    static const int enot_found=0;
    //@}

    /// \name User-specified data
    //@{
    int nx, ny;
    ubvector xfun, yfun;
    ubmatrix data;
    //@}

    /// \name User-specified contour levels
    //@{
    int nlev;
    ubvector levels;
    bool levels_set;
    //@}

    /// Right edge list
    std::vector<edge_crossings> red;
    /// Bottom edge list
    std::vector<edge_crossings> bed;

    /// Find next point starting from a point on a right edge
    int find_next_point_right(int j, int k, int &jnext, int &knext, 
			      int &dir_next, int nsw,
			      edge_crossings &right,
			      edge_crossings &bottom);
    
    /// Find next point starting from a point on a bottom edge
    int find_next_point_bottom(int j, int k, int &jnext, int &knext, 
			       int &dir_next, int nsw,
			       edge_crossings &right,
			       edge_crossings &bottom);

    /// Find all of the intersections of the edges with the contour level
    void find_intersections(size_t ilev, double &level,
			    edge_crossings &right, edge_crossings &bottom);

    /// Interpolate all right edge crossings 
    void right_edges(double level, interp<ubvector> &si,
		    edge_crossings &right);
    
    /// Interpolate all bottom edge crossings
    void bottom_edges(double level, interp<ubvector> &si,
		     edge_crossings &bottom);

    /// Create a contour line from a starting edge
    void process_line(int j, int k, int dir, std::vector<double> &x, 
		      std::vector<double> &y, 
		      bool first, edge_crossings &right,
		      edge_crossings &bottom);

    /// Check to ensure the x- and y-arrays are monotonic
    void check_data();

  private:

    contour(const contour &);
    contour& operator=(const contour&);

#endif

  };
  
#ifndef DOXYGEN_NO_O2NS
}
#endif

#endif


