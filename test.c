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

#include "sim_cache.h"
#include <assert.h>
#include <stdio.h>

void test1();
void test2();
void test3();
void test4();
void test5();
void test6();
void test7();
void test8();

int main() {
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();
    test8();
}

/*
 * First iteration  -> 2 miss
 * Second and later -> 1 miss
 *
 * T=10,M=6
 */
void test1() {
    printf("\n*** TEST 1 ***\n");
    const int n = 5;
    int i;
    config_sim(16, 1, 4);

    for(i=0; i<n; i++) {
        sim_access(i);
        sim_access(i+1);
    }

    assert(Access() == 10 && Misses() == 6);

    end_sim();
}

/*
 * Only one miss because is the
 * same address
 *
 * T=10,M=1
 */
void test2() {
    printf("\n*** TEST 2 ***\n");
    const int n = 10;
    int i, number = rand();

    config_sim(4, 1, 4);

    for(i=0; i<n; i++) {
        sim_access(number);
    }

    assert(Access() == 10 && Misses() == 1);

    end_sim();
}

/*
 * There are two misses because the
 * line size is 1 byte
 *
 * T=20,M=2
 */
void test3() {
    printf("\n*** TEST 3 ***\n");
    const int n = 10;
    int i;

    config_sim(2, 1, 2);

    for(i=0; i<n; i++) {
        sim_access(0);
        sim_access(1);
    }

    assert(Access() == 20 && Misses() == 2);

    end_sim();
}

/*
 * Both 0 and 128 address remove to
 * each other.
 * 1 address falls in row 1 and only
 * has one miss.
 *
 * T=30,M=21
 */
void test4() {
    printf("\n*** TEST 4 ***\n");
    const int n = 10;
    int i;

    config_sim(2, 1, 1);

    for(i=0; i<n; i++) {
        sim_access(0);
        sim_access(128);
        sim_access(1);
    }

    assert(Access() == 30 && Misses() == 21);

    end_sim();
}

/*
 * 0, 128 and 256 address remove one
 * of themselves
 * 1 address falls in row 1 and only
 * has one miss.
 * First iter  -     > 4
 * Second and later -> 3
 *
 * T=20,M=16
 */
void test5() {
    printf("\n*** TEST 5 ***\n");
    const int n = 5;
    int i;

    config_sim(4, 1, 2);

    for(i=0; i<n; i++) {
        sim_access(0);
        sim_access(128);
        sim_access(256);
        sim_access(1);
    }

    assert(Access() == 20 && Misses() == 16);

    end_sim();
}

/*
 * 0 and 1 fall in the same line --> 1 miss
 * 2 and 3 fall in the same line --> 1 miss
 * 3                             --> 1 miss
 *
 * T=50,M=3
 */
void test6() {
    printf("\n*** TEST 6 ***\n");
    const int n = 10;
    int i;

    config_sim(8, 2, 1);

    for(i=0; i<n; i++) {
        sim_access(0);
        sim_access(1);
        sim_access(2);
        sim_access(3);
        sim_access(4);
    }

    assert(Access() == 50 && Misses() == 3);

    end_sim();
}

/*
 * 0, 128 and 256 address remove one
 * of themselves
 * 1 address falls in row 1 and only
 * has one miss.
 * First iter  -     > 4
 * Second and later -> 3
 *
 * T=15,M=3
 */
void test7() {
    printf("\n*** TEST 7 ***\n");
    const int n = 5;
    int i;

    config_sim(4, 1, 2);

    for(i=0; i<n; i++) {
        sim_access(0);  // Same set (0)
        sim_access(64); // Same set (0)
        sim_access(1);  // Different set (1)
    }

    assert(Access() == 15 && Misses() == 3);

    end_sim();
}

/*
 * 0 and 1 -> line 0, set 0
 * 4 and 5 -> line 1, set 0
 * 2 and 3 -> line 2, set 1
 * 6 and 7 -> line 3, set 1
 *
 * T=50,M=3
 */
void test8() {
    printf("\n*** TEST 8 ***\n");
    const int n = 10;
    int i;

    config_sim(8, 2, 2);

    for(i=0; i<n; i++) {
        sim_access(0);
        sim_access(1);
        sim_access(2);
        sim_access(3);
        sim_access(4);
        sim_access(5);
        sim_access(6);
        sim_access(7);
    }

    assert(Access() == 80 && Misses() == 4);

    sim_access(8); // Same line, replace line 0 (0,1)
    sim_access(9); // Same line, replace line 0 (0,1)

    sim_access(0); // Same line, replace line 1 (4,5)
    sim_access(1); // Same line, replace line 1 (4,5)

    sim_access(8); // Same line, hit
    sim_access(9); // Same line, hit

    assert(Access() == 86 && Misses() == 6);

    end_sim();
}
