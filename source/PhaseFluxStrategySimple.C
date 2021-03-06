// Copyright (c) 2018, Lawrence Livermore National Security, LLC and
// UT-Battelle, LLC.
// Produced at the Lawrence Livermore National Laboratory and
// the Oak Ridge National Laboratory
// Written by M.R. Dorr, J.-L. Fattebert and M.E. Wickett
// LLNL-CODE-747500
// All rights reserved.
// This file is part of AMPE.
// For details, see https://github.com/LLNL/AMPE
// Please also read AMPE/LICENSE.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// - Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the disclaimer below.
// - Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the disclaimer (as noted below) in the
//   documentation and/or other materials provided with the distribution.
// - Neither the name of the LLNS/LLNL nor the names of its contributors may be
//   used to endorse or promote products derived from this software without
//   specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL LAWRENCE LIVERMORE NATIONAL SECURITY,
// LLC, UT BATTELLE, LLC,
// THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
// IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
#include "PhaseFluxStrategySimple.h"
#include "QuatFort.h"

#include "SAMRAI/geom/CartesianPatchGeometry.h"
#include "SAMRAI/pdat/CellData.h"
#include "SAMRAI/pdat/SideData.h"
#include "SAMRAI/math/PatchSideDataNormOpsReal.h"

void PhaseFluxStrategySimple::computeFluxes(
    const std::shared_ptr<hier::PatchLevel> level, const int phase_id,
    const int quat_id, const int flux_id)
{
   // this strategy is independent of grain orientation
   (void)quat_id;

   //  Compute phase "flux" on patches in level.
   for (hier::PatchLevel::Iterator ip(level->begin()); ip != level->end();
        ++ip) {
      std::shared_ptr<hier::Patch> patch = *ip;

      const std::shared_ptr<geom::CartesianPatchGeometry> patch_geom(
          SAMRAI_SHARED_PTR_CAST<geom::CartesianPatchGeometry,
                                 hier::PatchGeometry>(
              patch->getPatchGeometry()));
      const double* dx = patch_geom->getDx();

      std::shared_ptr<pdat::CellData<double> > phase(
          SAMRAI_SHARED_PTR_CAST<pdat::CellData<double>, hier::PatchData>(
              patch->getPatchData(phase_id)));

      assert(phase->getGhostCellWidth()[0] > 0);

      std::shared_ptr<pdat::SideData<double> > phase_flux(
          SAMRAI_SHARED_PTR_CAST<pdat::SideData<double>, hier::PatchData>(
              patch->getPatchData(flux_id)));

      const hier::Box& pbox = patch->getBox();
      const hier::Index& ifirst = pbox.lower();
      const hier::Index& ilast = pbox.upper();

      GRADIENT_FLUX(ifirst(0), ilast(0), ifirst(1), ilast(1),
#if (NDIM == 3)
                    ifirst(2), ilast(2),
#endif
                    dx, d_epsilon_phase, phase->getPointer(),
                    phase->getGhostCellWidth()[0], phase_flux->getPointer(0),
                    phase_flux->getPointer(1),
#if (NDIM == 3)
                    phase_flux->getPointer(2),
#endif
                    phase_flux->getGhostCellWidth()[0]);

#ifdef DEBUG_CHECK_ASSERTIONS
      SAMRAI::math::PatchSideDataNormOpsReal<double> ops;
      double l2f = ops.L2Norm(phase_flux, pbox);
      assert(l2f == l2f);
#endif
   }
}
