/*
 CSL : Cache Simulator Lib

Copyright (C) 2014  Diego Nieto Muñoz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SIM_CACHE_H
#define _SIM_CACHE_H

/** @brief Initializes the simulator
 ∗
 ∗ @param[in] cs Cache size in bytes
 ∗ @param[in] ls Line size in bytes. It must be a power of 2
 ∗ @param[in] assoc Associativity of the cache.
 ∗
 ∗ @pre The number of sets in the cache must be a power of 2,
 ∗ that is, (cs/(ls∗assoc)) must be a power of 2.
 ∗ The line size must also be a power of 2
 */
extern void config_sim(unsigned int cs, unsigned int ls, unsigned int assoc);
/**
 *  @brief Simulates an access to the cache
 ∗
 ∗ @param[in] address Address of the memory position accessed
 */

extern void sim_access(unsigned int address);
/**
 *  @brief Dumps simulation results
 ∗
 ∗ Prints the number of accesses and the number of misses
 */
extern void end_sim();

#endif
