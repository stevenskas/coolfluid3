// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_SchemeCSysLF_hpp
#define CF_Solver_SchemeCSysLF_hpp

#include "RDM/SchemeBase.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

template < typename SF, typename QD, typename PHYS >
class RDM_API SchemeCSysLF : public SchemeBase<SF,QD,PHYS> {

public: // typedefs

  /// base class type
  typedef SchemeBase<SF,QD,PHYS> B;

  /// pointers
  typedef boost::shared_ptr< SchemeCSysLF > Ptr;
  typedef boost::shared_ptr< SchemeCSysLF const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  SchemeCSysLF ( const std::string& name );

  /// Virtual destructor
  virtual ~SchemeCSysLF() {};

  /// Get the class name
  static std::string type_name () { return "SchemeCSysLF<" + SF::type_name() + ">"; }
	
  /// execute the action
  virtual void execute ();

private: // data

  // not needed for LF

  /// right eigen vector matrix
  typename B::PhysicsMT Rv;
  /// left eigen vector matrix
  typename B::PhysicsMT Lv;
  /// diagonal matrix with eigen values
  typename B::PhysicsVT Dv;
  /// diagonal matrix with positive eigen values
  typename B::PhysicsVT DvPlus [SF::nb_nodes];


};

///////////////////////////////////////////////////////////////////////////////////////

template<typename SF, typename QD, typename PHYS>
SchemeCSysLF<SF,QD,PHYS>::SchemeCSysLF ( const std::string& name ) :
  SchemeBase<SF,QD,PHYS>(name)
{
  for(Uint n = 0; n < SF::nb_nodes; ++n)
    DvPlus[n].setZero();
}

/////////////////////////////////////////////////////////////////////////////////////

template<typename SF,typename QD, typename PHYS>
void SchemeCSysLF<SF, QD,PHYS>::execute()
{
  B::interpolate();

//  typename B::SFMatrixT*    dNdX = this->dNdX;
//  typename B::QSolutionMT*  dUdX = this->dUdX;
//  typename B::PhysicsMT*    dFdU = this->dFdU;

//  typename B::DimVT&        dN     = this->dN;
//  typename B::QCoordMT&     X_q    = this->X_q;
//  typename B::QSolutionMT&  U_q    = this->U_q;
//  typename B::PhysicsVT&    LU     = this->LU;
//  typename B::WeightVT&     wj     = this->wj;
//  typename B::SolutionMT&   Phi_n  = this->Phi_n;

  const Mesh::CTable<Uint>::ConstRow nodes_idx = this->connectivity_table->array()[B::idx()];

  // L(N)+ @ each quadrature point

  for(Uint q=0; q < QD::nb_points; ++q)
  {
    for(Uint n=0; n < SF::nb_nodes; ++n)
    {
      B::dN[XX] = B::dNdX[XX](q,n);
      B::dN[YY] = B::dNdX[YY](q,n);

      /// @todo calling jacobian_eigen_structure() is suboptimal
      ///       since we only need the max eigen value,
      ///       so we need a max_eigen_value() function on the physics

      PHYS::jacobian_eigen_structure(B::X_q.row(q),
                                     B::U_q.row(q),
                                     B::dN,
                                     Rv,
                                     Lv,
                                     Dv );

      // diagonal matrix of positive eigen values

      DvPlus[n] = Dv.unaryExpr(std::ptr_fun(plus));

      // Ki_n[n] = Rv * DvPlus[n].asDiagonal() * Lv;
    }

    // compute L(u)

    PHYS::Lu(B::X_q.row(q),
             B::U_q.row(q),
             B::dUdX[XX].row(q).transpose(),
             B::dUdX[YY].row(q).transpose(),
             B::dFdU,
             B::LU );

    // compute the phi_i LF intergral

    Real alpha = 0;
    for(Uint n = 0; n < SF::nb_nodes; ++n)
      alpha = std::max( alpha, DvPlus[n].array().abs().maxCoeff() );

    const Real invdofs = 1./SF::nb_nodes;

    for(Uint i = 0; i < SF::nb_nodes; ++i)
    {
      B::Phi_n.row(i) += invdofs * B::LU * B::wj[q];          // central part
      for(Uint j=0; j < SF::nb_nodes; ++j)    // plus dissipation
      {
        if (i == j) continue;
        B::Phi_n.row(i) += invdofs * alpha * (B::U_n.row(i).transpose() - B::U_n.row(j).transpose()) * B::wj[q]; // dissipation
      }

      // compute the wave_speed for scaling the update

      (*B::wave_speed)[nodes_idx[i]][0] += alpha * B::wj[q];
    }

  } // loop qd points

  // update the residual
  
  for (Uint n=0; n<SF::nb_nodes; ++n)
    for (Uint v=0; v < PHYS::neqs; ++v)
      (*B::residual)[nodes_idx[n]][v] += B::Phi_n(n,v);

}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_SchemeCSysLF_hpp
