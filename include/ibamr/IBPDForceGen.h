// Filename: IBPDForceGen.h
// Created on 08 Apr 2016 by Amneet Bhalla
//
// Copyright (c) 2002-2014, Amneet Bhalla and Boyce Griffith.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of The University of North Carolina nor the names of its
//      contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef included_IBPDForceGen
#define included_IBPDForceGen

/////////////////////////////// INCLUDES /////////////////////////////////////
#include <vector>

#include "Eigen/Dense"
#include "ibamr/IBLagrangianForceStrategy.h"
#include "ibtk/LData.h"
#include "ibtk/LDataManager.h"
#include "tbox/DescribedClass.h"

/////////////////////////////// CLASS DEFINITION /////////////////////////////

namespace IBAMR
{
/*!
* \brief Class IBPDForceGen is a concrete IBLagrangianForceStrategy that
* is intended to be used in conjunction with curvilinear mesh data generated by
* class IBStandardInitializer.
*
* Class IBPDForceGen computes peridynamic force between bonds, which eventually
* gets applied at the IB points.
*/

class IBPDForceGen : public IBAMR::IBLagrangianForceStrategy
{
public:
    /*!
     * \brief Default constructor.
     */
    IBPDForceGen(SAMRAI::tbox::Pointer<SAMRAI::tbox::Database> input_db);

    /*!
     * \brief Destructor.
     */
    ~IBPDForceGen();

    /*!
     * \brief Function pointer to compute state based bond forces.
     */
    typedef Eigen::Vector4d (*BondForceDamageFcnPtr)(
        const double horizon,
        const double delta,
        const double R,
        double* parameters,
        const Eigen::Map<const IBTK::Vector>& X0_mastr,
        const Eigen::Map<const IBTK::Vector>& X0_slave,
        const Eigen::Map<const IBTK::Vector>& X_mastr,
        const Eigen::Map<const IBTK::Vector>& X_slave,
        const Eigen::Map<const Eigen::Matrix<double, NDIM, NDIM, Eigen::RowMajor> >& FF_mastr,
        const Eigen::Map<const Eigen::Matrix<double, NDIM, NDIM, Eigen::RowMajor> >& FF_slave,
        const Eigen::Map<const Eigen::Matrix<double, NDIM, NDIM, Eigen::RowMajor> >& B_mastr,
        const Eigen::Map<const Eigen::Matrix<double, NDIM, NDIM, Eigen::RowMajor> >& B_slave,
        Eigen::Map<IBTK::Vector>& F_mastr,
        Eigen::Map<IBTK::Vector>& F_slave,
        const int lag_mastr_node_idx,
        const int lag_slave_node_idx);

    /*!
     * \brief Function pointer to compute influence of a bond based upon its distance.
     */
    typedef double (*BondInfluenceFcnPtr)(double R0, double delta);

    /*!
     * \brief Function pointer to compute influence of a bond based upon its distance.
     */
    typedef double (*BondVolFracFcnPtr)(double R0, double horizon, double delta);

    /*!
     * \brief Function pointer to define PK1 stress tensor.
     */
    typedef void (*BondPK1FcnPtr)(Eigen::Matrix<double, NDIM, NDIM, Eigen::RowMajor>& PK1,
                                  const Eigen::Map<const Eigen::Matrix<double, NDIM, NDIM, Eigen::RowMajor> >& FF,
                                  const Eigen::Map<const IBTK::Vector>& X0,
                                  int lag_idx);

    /*!
     * \brief Register a bond force and damage, influence, volume fraction, PK1 stress
     * specification function with the force generator.
     *
     * These functions are employed to compute the force generated by a
     * particular bond for the specified displacement, PK1 stress tensor, rest
     * length, and Lagrangian index.
     *
     * \note By default, functions default_PK1_fcn(), default_force_damage_fcn(), default_inf_fcn(),
     * and default_vol_frac_fcn() are associated with \a force_fcn_idx 0.
     */
    void registerBondForceSpecificationFunction(int force_fcn_index,
                                                const BondPK1FcnPtr bond_PK1_fcn_ptr,
                                                const BondForceDamageFcnPtr bond_force_damage_fcn_ptr = NULL,
                                                const BondInfluenceFcnPtr bond_inf_fcn_ptr = NULL,
                                                const BondVolFracFcnPtr bond_vol_frac_fcn_ptr = NULL);

    /*!
     * \brief Setup the data needed to compute the forces on the specified level
     * of the patch hierarchy.
     */
    void initializeLevelData(SAMRAI::tbox::Pointer<SAMRAI::hier::PatchHierarchy<NDIM> > hierarchy,
                             int level_number,
                             double init_data_time,
                             bool initial_time,
                             IBTK::LDataManager* l_data_manager);
    /*!
     * \brief Compute the force generated by the Lagrangian structure on the
     * specified level of the patch hierarchy and the damage occured during bond
     * breaking.
     *
     * \note Nodal forces computed by this method are \em added to the force
     * vector.
     */
    void computeLagrangianForceAndDamage(SAMRAI::tbox::Pointer<IBTK::LData> F_data,
                                         SAMRAI::tbox::Pointer<IBTK::LData> D_data,
                                         SAMRAI::tbox::Pointer<IBTK::LData> X_data,
                                         SAMRAI::tbox::Pointer<IBTK::LData> U_data,
                                         SAMRAI::tbox::Pointer<SAMRAI::hier::PatchHierarchy<NDIM> > hierarchy,
                                         int level_number,
                                         double data_time,
                                         IBTK::LDataManager* l_data_manager);

private:
    /*!
     * \brief Copy constructor.
     *
     * \note This constructor is not implemented and should not be used.
     *
     * \param from The value to copy to this object.
     */
    IBPDForceGen(const IBPDForceGen& from);

    /*!
     * \brief Assignment operator.
     *
     * \note This operator is not implemented and should not be used.
     *
     * \param that The value to assign to this object.
     *
     * \return A reference to this object.
     */
    IBPDForceGen& operator=(const IBPDForceGen& that);

    /*!
     * \name Horizon.
     */
    double d_horizon, d_ds;

    /*!
     * \name Data maintained separately for each level of the patch hierarchy.
     */
    //\{
    struct BondData
    {
        std::vector<int> lag_mastr_node_idxs, lag_slave_node_idxs;
        std::vector<int> petsc_mastr_node_idxs, petsc_slave_node_idxs;
        std::vector<int> petsc_global_mastr_node_idxs, petsc_global_slave_node_idxs;
        std::vector<BondForceDamageFcnPtr> force_dmg_fcns;
        std::vector<BondPK1FcnPtr> force_PK1_fcns;
        std::vector<BondInfluenceFcnPtr> force_inf_fcns;
        std::vector<BondVolFracFcnPtr> force_vol_frac_fcns;
        std::vector<double*> parameters;
    };
    std::vector<BondData> d_bond_data;
    std::vector<SAMRAI::tbox::Pointer<IBTK::LData> > d_X_ghost_data, d_X0_ghost_data, d_X_mean_ghost_data,
        d_B_ghost_data, d_FF_ghost_data, d_F_ghost_data, d_N_ghost_data, d_dmg_ghost_data, d_dX_data;
    std::vector<bool> d_is_initialized;
    //\}

    /*!
     * PD tensor routines.
     */
    //\{

    void computeMeanPosition(SAMRAI::tbox::Pointer<IBTK::LData> X_mean_data,
                             SAMRAI::tbox::Pointer<IBTK::LData> N_data,
                             SAMRAI::tbox::Pointer<IBTK::LData> X_data,
                             SAMRAI::tbox::Pointer<SAMRAI::hier::PatchHierarchy<NDIM> > hierarchy,
                             int level_number,
                             double data_time,
                             IBTK::LDataManager* l_data_manager);

    void computeShapeTensor(SAMRAI::tbox::Pointer<IBTK::LData> B_data,
                            SAMRAI::tbox::Pointer<IBTK::LData> X0_data,
                            SAMRAI::tbox::Pointer<SAMRAI::hier::PatchHierarchy<NDIM> > hierarchy,
                            int level_number,
                            double data_time,
                            IBTK::LDataManager* l_data_manager);

    void computeDeformationGradientTensor(SAMRAI::tbox::Pointer<IBTK::LData> FF_data,
                                          SAMRAI::tbox::Pointer<IBTK::LData> X_data,
                                          SAMRAI::tbox::Pointer<IBTK::LData> X0_data,
                                          SAMRAI::tbox::Pointer<SAMRAI::hier::PatchHierarchy<NDIM> > hierarchy,
                                          int level_number,
                                          double data_time,
                                          IBTK::LDataManager* l_data_manager);
    //\}
    /*!
     * Bond force routines and functions.
     */
    //\{
    void initializeBondLevelData(std::set<int>& nonlocal_petsc_idx_set,
                                 SAMRAI::tbox::Pointer<SAMRAI::hier::PatchHierarchy<NDIM> > hierarchy,
                                 int level_number,
                                 double init_data_time,
                                 bool initial_time,
                                 IBTK::LDataManager* l_data_manager);

    void computeLagrangianBondForceAndDamage(SAMRAI::tbox::Pointer<IBTK::LData> F_data,
                                             SAMRAI::tbox::Pointer<IBTK::LData> D_data,
                                             SAMRAI::tbox::Pointer<IBTK::LData> X_data,
                                             SAMRAI::tbox::Pointer<IBTK::LData> X0_data,
                                             SAMRAI::tbox::Pointer<IBTK::LData> FF_data,
                                             SAMRAI::tbox::Pointer<IBTK::LData> B_data,
                                             SAMRAI::tbox::Pointer<SAMRAI::hier::PatchHierarchy<NDIM> > hierarchy,
                                             int level_number,
                                             double data_time,
                                             IBTK::LDataManager* l_data_manager);

    std::map<int, BondForceDamageFcnPtr> d_bond_force_damage_fcn_map;
    std::map<int, BondInfluenceFcnPtr> d_bond_inf_fcn_map;
    std::map<int, BondVolFracFcnPtr> d_bond_vol_frac_fcn_map;
    std::map<int, BondPK1FcnPtr> d_bond_PK1_fcn_map;
    //\}
};
} // namespace IBAMR

//////////////////////////////////////////////////////////////////////////////

#endif // #ifndef included_IBPDForceGen