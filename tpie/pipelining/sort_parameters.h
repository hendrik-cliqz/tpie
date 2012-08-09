// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2012, The TPIE development team
// 
// This file is part of TPIE.
// 
// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>

#ifndef __TPIE_PIPELINING_SORT_PARAMETERS_H__
#define __TPIE_PIPELINING_SORT_PARAMETERS_H__

namespace tpie {

namespace pipelining {

struct sort_parameters {
	/** Memory available while forming sorted runs. */
	memory_size_type memoryPhase2;
	/** Memory available while merging runs. */
	memory_size_type memoryPhase3;
	/** Memory available during output phase. */
	memory_size_type memoryPhase4;
	/** Run length, subject to memory restrictions during phase 2.
	 * Although we cannot sort more than 2^32 numbers internally on 32-bit
	 * systems, we are still able to merge sorted streams of run lengths
	 * greater than 2^32. Therefore, a stream_size_type is necessary. */
	stream_size_type runLength;
	/** Maximum item count for internal reporting, subject to memory
	 * restrictions in all phases. Less or equal to runLength.
	 * Same type as runLength. */
	stream_size_type internalReportThreshold;
	/** Fanout of merge tree during phase 3. */
	memory_size_type fanout;
	/** Fanout of merge tree during phase 4. Less or equal to fanout. */
	memory_size_type finalFanout;

	void dump(std::ostream & out) const {
		out << "Merge sort parameters\n"
			<< "Phase 2 memory:              " << memoryPhase2 << '\n'
			<< "Run length:                  " << runLength << '\n'
			<< "Phase 3 memory:              " << memoryPhase3 << '\n'
			<< "Fanout:                      " << fanout << '\n'
			<< "Phase 4 memory:              " << memoryPhase4 << '\n'
			<< "Final merge level fanout:    " << finalFanout << '\n'
			<< "Internal report threshold:   " << internalReportThreshold << '\n';
	}
};

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_SORT_PARAMETERS_H__
