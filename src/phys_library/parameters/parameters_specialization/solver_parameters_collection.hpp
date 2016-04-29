// Copyright (C) 2009-2016 ETH Zurich
// Copyright (C) 2007?-2016 Center for Nanophase Materials Sciences, ORNL
// All rights reserved.
//
// See LICENSE.txt for terms of usage.
// See CITATION.txt for citation guidelines if you use this code for scientific publications.
//
// Author: Giovanni Balduzzi (gbalduzz@itp.phys.ethz.ch)

#ifndef PHYS_LIBRARY_PARAMETERS_PARAMETERS_SPECIALIZATION_SOLVER_PARAMETERS_COLLECTION_HPP
#define PHYS_LIBRARY_PARAMETERS_PARAMETERS_SPECIALIZATION_SOLVER_PARAMETERS_COLLECTION_HPP

// INTERNAL: ct_aux_parameters must be included as they currently provide the read_write interface for the analysis.
#include "phys_library/parameters/parameters_specialization/solver_specializations/MC_solver_ct_aux_parameters.h"
#ifdef USE_SS_CT_HYB
#include "phys_library/parameters/parameters_specialization/solver_specializations/MC_solver_ss_hybridization_parameters.h"
#endif
#include "phys_library/parameters/parameters_specialization/templates/MC_solver_parameters.h"

#endif  // PHYS_LIBRARY_PARAMETERS_PARAMETERS_SPECIALIZATION_SOLVER_PARAMETERS_COLLECTION_HPP
