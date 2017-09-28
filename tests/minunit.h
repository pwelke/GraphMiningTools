/*
 * minunit.h
 *
 *  Created on: Sep 26, 2017
 *      Author: pascal
 */

#ifndef TESTS_MINUNIT_H_
#define TESTS_MINUNIT_H_

/* file: minunit.h */
#define mu_assert(message, test) do { if (!(test)) return message; } while (0)
#define mu_run_test(test) do { char *message = test; tests_run++; if (message) return message; } while (0)
extern int tests_run;

#endif /* TESTS_MINUNIT_H_ */
