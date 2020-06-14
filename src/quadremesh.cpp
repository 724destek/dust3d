#include <QDebug>
#include <stdio.h>
#include <stdlib.h>
#include <igl/avg_edge_length.h>
#include <igl/barycenter.h>
#include <igl/comb_cross_field.h>
#include <igl/comb_frame_field.h>
#include <igl/compute_frame_field_bisectors.h>
#include <igl/cross_field_mismatch.h>
#include <igl/cut_mesh_from_singularities.h>
#include <igl/find_cross_field_singularities.h>
#include <igl/local_basis.h>
#include <igl/rotate_vectors.h>
#include <igl/copyleft/comiso/miq.h>
#include <igl/copyleft/comiso/nrosy.h>
#include <qex.h>
#include "quadremesh.h"

bool quadRemesh(const std::vector<QVector3D> &inputVertices,
    std::vector<std::vector<size_t>> &inputTriangles,
    std::vector<std::vector<size_t>> &inputQuads,
    std::vector<QVector3D> *outputVertices,
    std::vector<std::vector<size_t>> *outputQuads)
{
    qDebug() << "quadRemesh begin...";
    
    qex_TriMesh triMesh = {0};
    qex_QuadMesh quadMesh = {0};
    
    triMesh.vertex_count = inputVertices.size();
    triMesh.tri_count = inputTriangles.size() + inputQuads.size() * 2;
    
    triMesh.vertices = (qex_Point3*)malloc(sizeof(qex_Point3) * triMesh.vertex_count);
    triMesh.tris = (qex_Tri*)malloc(sizeof(qex_Tri) * triMesh.tri_count);
    triMesh.uvTris = (qex_UVTri*)malloc(sizeof(qex_UVTri) * triMesh.tri_count);
    
    for (unsigned int i = 0; i < triMesh.vertex_count; ++i) {
        const auto &src = inputVertices[i];
        triMesh.vertices[i] = qex_Point3 {{(double)src.x(), (double)src.y(), (double)src.z()}};
    }
    size_t triangleNum = 0;
    for (const auto &it: inputTriangles) {
        triMesh.tris[triangleNum++] = qex_Tri {{(qex_Index)it[0], (qex_Index)it[1], (qex_Index)it[2]}};
    }
    for (const auto &it: inputQuads) {
        triMesh.tris[triangleNum++] = qex_Tri {{(qex_Index)it[0], (qex_Index)it[1], (qex_Index)it[2]}};
        triMesh.tris[triangleNum++] = qex_Tri {{(qex_Index)it[2], (qex_Index)it[3], (qex_Index)it[0]}};
    }
    
    // https://github.com/libigl/libigl/blob/master/tutorial/505_MIQ/main.cpp
    
    qDebug() << "quadRemesh UV...";
    
    double gradient_size = 50;
    double iter = 0;
    double stiffness = 5.0;
    bool direct_round = 0;
    
    Eigen::MatrixXd V(triMesh.vertex_count, 3);
    Eigen::MatrixXi F(triMesh.tri_count, 3);
    
    for (decltype(inputVertices.size()) i = 0; i < inputVertices.size(); i++) {
        const auto &vertex = inputVertices[i];
        V.row(i) << vertex.x(), vertex.y(), vertex.z();
    }
    
    size_t rowNum = 0;
    for (const auto &it: inputTriangles) {
        F.row(rowNum++) << it[0], it[1], it[2];
    }
    for (const auto &it: inputQuads) {
        F.row(rowNum++) << it[0], it[1], it[2];
        F.row(rowNum++) << it[2], it[3], it[0];
    }
    
    bool extend_arrows = false;

    // Cross field
    Eigen::MatrixXd X1,X2;

    // Bisector field
    Eigen::MatrixXd BIS1, BIS2;

    // Combed bisector
    Eigen::MatrixXd BIS1_combed, BIS2_combed;

    // Per-corner, integer mismatches
    Eigen::Matrix<int, Eigen::Dynamic, 3> MMatch;

    // Field singularities
    Eigen::Matrix<int, Eigen::Dynamic, 1> isSingularity, singularityIndex;

    // Per corner seams
    Eigen::Matrix<int, Eigen::Dynamic, 3> Seams;

    // Combed field
    Eigen::MatrixXd X1_combed, X2_combed;

    // Global parametrization
    Eigen::MatrixXd UV;
    Eigen::MatrixXi FUV;
    
    Eigen::VectorXi b(1);
    b << 0;
    Eigen::MatrixXd bc(1, 3);
    bc << 1, 0, 0;
    
    Eigen::VectorXd S;
    igl::copyleft::comiso::nrosy(V, F, b, bc, Eigen::VectorXi(), Eigen::VectorXd(), Eigen::MatrixXd(), 4, 0.5, X1, S);

    // Find the orthogonal vector
    qDebug() << "Find the orthogonal vector...";
    Eigen::MatrixXd B1, B2, B3;
    igl::local_basis(V, F, B1, B2, B3);
    X2 = igl::rotate_vectors(X1, Eigen::VectorXd::Constant(1, igl::PI / 2), B1, B2);

    // Always work on the bisectors, it is more general
    qDebug() << "Always work on the bisectors, it is more general...";
    igl::compute_frame_field_bisectors(V, F, X1, X2, BIS1, BIS2);

    // Comb the field, implicitly defining the seams
    qDebug() << "Comb the field, implicitly defining the seams...";
    igl::comb_cross_field(V, F, BIS1, BIS2, BIS1_combed, BIS2_combed);

    // Find the integer mismatches
    qDebug() << "Find the integer mismatches...";
    igl::cross_field_mismatch(V, F, BIS1_combed, BIS2_combed, true, MMatch);

    // Find the singularities
    qDebug() << "Find the singularities...";
    igl::find_cross_field_singularities(V, F, MMatch, isSingularity, singularityIndex);

    // Cut the mesh, duplicating all vertices on the seams
    qDebug() << "Cut the mesh, duplicating all vertices on the seams...";
    igl::cut_mesh_from_singularities(V, F, MMatch, Seams);

    // Comb the frame-field accordingly
    qDebug() << "Comb the frame-field accordingly...";
    igl::comb_frame_field(V, F, X1, X2, BIS1_combed, BIS2_combed, X1_combed, X2_combed);

    // Global parametrization
    qDebug() << "Global parametrization...";
    igl::copyleft::comiso::miq(V,
        F,
        X1_combed,
        X2_combed,
        MMatch,
        isSingularity,
        Seams,
        UV,
        FUV,
        gradient_size,
        stiffness,
        direct_round,
        iter,
        5,
        true);
    qDebug() << "Get parametrization result...";
    
    qDebug() << "Get parametrization result UV.rows():" << UV.rows();
    qDebug() << "Get parametrization result FUV.rows():" << FUV.rows();
    
    triMesh.uvTris = (qex_UVTri*)malloc(sizeof(qex_UVTri) * FUV.rows());
    for (unsigned int i = 0; i < FUV.rows(); ++i) {
        qDebug() << "FUV.row(" << i << ")";
        const auto &triangleVertexIndices = FUV.row(i);
        const auto &v0 = UV.row(triangleVertexIndices[0]);
        const auto &v1 = UV.row(triangleVertexIndices[1]);
        const auto &v2 = UV.row(triangleVertexIndices[2]);
        triMesh.uvTris[i] = qex_UVTri {{
            qex_Point2 {{v0[0], v0[1]}}, 
            qex_Point2 {{v1[0], v1[1]}}, 
            qex_Point2 {{v2[0], v2[1]}}
        }};
    }
    
    qDebug() << "quadRemesh qex_extractQuadMesh...";
    
    qex_extractQuadMesh(&triMesh, nullptr, &quadMesh);
    
    qDebug() << "quadRemesh get result...";
    
    qDebug() << "quadRemesh get result quadMesh.vertex_count:" << quadMesh.vertex_count;
    qDebug() << "quadRemesh get result quadMesh.quad_count:" << quadMesh.quad_count;
    
    outputVertices->resize(quadMesh.vertex_count);
    for (unsigned int i = 0; i < quadMesh.vertex_count; ++i) {
        qDebug() << "quadMesh.vertices:" << i;
        const auto &src = quadMesh.vertices[i];
        (*outputVertices)[i] = QVector3D(src.x[0], src.x[1], src.x[2]);
    }
    outputQuads->resize(quadMesh.quad_count);
    for (unsigned int i = 0; i < quadMesh.quad_count; ++i) {
        qDebug() << "quadMesh.quads:" << i;
        const auto &src = quadMesh.quads[i];
        (*outputQuads)[i] = std::vector<size_t> {src.indices[0], src.indices[1], src.indices[2], src.indices[3]};
    }
    
    free(triMesh.vertices);
    free(triMesh.tris);
    free(triMesh.uvTris);

    free(quadMesh.vertices);
    free(quadMesh.quads);
    
    qDebug() << "quadRemesh end";
    
    return true;
}

extern "C" {
    // http://www.netlib.org/clapack/
    
    typedef long int integer;
    typedef unsigned long int uinteger;
    typedef char* address;
    typedef short int shortint;
    typedef float real;
    typedef double doublereal;

    int daxpy_(integer* n,
               doublereal* da,
               doublereal* dx,
               integer* incx,
               doublereal* dy,
               integer* incy) {
      /* System generated locals */
      integer i__1;

      /* Local variables */
      static thread_local integer i, m, ix, iy, mp1;

    /*     constant times a vector plus a vector.
           uses unrolled loops for increments equal to one.
           jack dongarra, linpack, 3/11/78.
           modified 12/3/93, array(1) declarations changed to array(*)



       Parameter adjustments
           Function Body */
    #define DY(I) dy[(I)-1]
    #define DX(I) dx[(I)-1]

      if (*n <= 0) {
        return 0;
      }
      if (*da == 0.) {
        return 0;
      }
      if (*incx == 1 && *incy == 1) {
        goto L20;
      }

      /*        code for unequal increments or equal increments
                  not equal to 1 */

      ix = 1;
      iy = 1;
      if (*incx < 0) {
        ix = (-(*n) + 1) * *incx + 1;
      }
      if (*incy < 0) {
        iy = (-(*n) + 1) * *incy + 1;
      }
      i__1 = *n;
      for (i = 1; i <= *n; ++i) {
        DY(iy) += *da * DX(ix);
        ix += *incx;
        iy += *incy;
        /* L10: */
      }
      return 0;

    /*        code for both increments equal to 1


              clean-up loop */

    L20:
      m = *n % 4;
      if (m == 0) {
        goto L40;
      }
      i__1 = m;
      for (i = 1; i <= m; ++i) {
        DY(i) += *da * DX(i);
        /* L30: */
      }
      if (*n < 4) {
        return 0;
      }
    L40:
      mp1 = m + 1;
      i__1 = *n;
      for (i = mp1; i <= *n; i += 4) {
        DY(i) += *da * DX(i);
        DY(i + 1) += *da * DX(i + 1);
        DY(i + 2) += *da * DX(i + 2);
        DY(i + 3) += *da * DX(i + 3);
        /* L50: */
      }
      return 0;
    } /* daxpy_ */

    doublereal ddot_(integer* n,
                     doublereal* dx,
                     integer* incx,
                     doublereal* dy,
                     integer* incy) {
      /* System generated locals */
      integer i__1;
      doublereal ret_val;

      /* Local variables */
      static thread_local integer i, m;
      static thread_local doublereal dtemp;
      static thread_local integer ix, iy, mp1;

    /*     forms the dot product of two vectors.
           uses unrolled loops for increments equal to one.
           jack dongarra, linpack, 3/11/78.
           modified 12/3/93, array(1) declarations changed to array(*)



       Parameter adjustments
           Function Body */
    #define DY(I) dy[(I)-1]
    #define DX(I) dx[(I)-1]

      ret_val = 0.;
      dtemp = 0.;
      if (*n <= 0) {
        return ret_val;
      }
      if (*incx == 1 && *incy == 1) {
        goto L20;
      }

      /*        code for unequal increments or equal increments
                  not equal to 1 */

      ix = 1;
      iy = 1;
      if (*incx < 0) {
        ix = (-(*n) + 1) * *incx + 1;
      }
      if (*incy < 0) {
        iy = (-(*n) + 1) * *incy + 1;
      }
      i__1 = *n;
      for (i = 1; i <= *n; ++i) {
        dtemp += DX(ix) * DY(iy);
        ix += *incx;
        iy += *incy;
        /* L10: */
      }
      ret_val = dtemp;
      return ret_val;

    /*        code for both increments equal to 1


              clean-up loop */

    L20:
      m = *n % 5;
      if (m == 0) {
        goto L40;
      }
      i__1 = m;
      for (i = 1; i <= m; ++i) {
        dtemp += DX(i) * DY(i);
        /* L30: */
      }
      if (*n < 5) {
        goto L60;
      }
    L40:
      mp1 = m + 1;
      i__1 = *n;
      for (i = mp1; i <= *n; i += 5) {
        dtemp = dtemp + DX(i) * DY(i) + DX(i + 1) * DY(i + 1) +
                DX(i + 2) * DY(i + 2) + DX(i + 3) * DY(i + 3) +
                DX(i + 4) * DY(i + 4);
        /* L50: */
      }
    L60:
      ret_val = dtemp;
      return ret_val;
    } /* ddot_ */

    doublereal dnrm2_(integer* n, doublereal* x, integer* incx) {
      /* System generated locals */
      integer i__1, i__2;
      doublereal ret_val, d__1;

      /* Builtin functions */
      double sqrt(doublereal);

      /* Local variables */
      static doublereal norm, scale, absxi;
      static integer ix;
      static doublereal ssq;

    /*  DNRM2 returns the euclidean norm of a vector via the function
        name, so that

           DNRM2 := sqrt( x'*x )



        -- This version written on 25-October-1982.
           Modified on 14-October-1993 to inline the call to DLASSQ.
           Sven Hammarling, Nag Ltd.



       Parameter adjustments
           Function Body */
    #define X(I) x[(I)-1]

      if (*n < 1 || *incx < 1) {
        norm = 0.;
      } else if (*n == 1) {
        norm = abs(X(1));
      } else {
        scale = 0.;
        ssq = 1.;
        /*        The following loop is equivalent to this call to the LAPACK

                  auxiliary routine:
                  CALL DLASSQ( N, X, INCX, SCALE, SSQ ) */

        i__1 = (*n - 1) * *incx + 1;
        i__2 = *incx;
        for (ix = 1;
             *incx < 0 ? ix >= (*n - 1) * *incx + 1 : ix <= (*n - 1) * *incx + 1;
             ix += *incx) {
          if (X(ix) != 0.) {
            absxi = (d__1 = X(ix), abs(d__1));
            if (scale < absxi) {
              /* Computing 2nd power */
              d__1 = scale / absxi;
              ssq = ssq * (d__1 * d__1) + 1.;
              scale = absxi;
            } else {
              /* Computing 2nd power */
              d__1 = absxi / scale;
              ssq += d__1 * d__1;
            }
          }
          /* L10: */
        }
        norm = scale * sqrt(ssq);
      }

      ret_val = norm;
      return ret_val;

      /*     End of DNRM2. */

    } /* dnrm2_ */
}