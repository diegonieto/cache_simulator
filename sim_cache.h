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
