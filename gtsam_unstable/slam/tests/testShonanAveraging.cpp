/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010-2019, Georgia Tech Research Corporation,
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file   testShonanAveraging.cpp
 * @date   March 2019
 * @author Frank Dellaert
 * @brief  Unit tests for Shonan Averaging algorithm
 */

#include <gtsam_unstable/slam/ShonanAveraging.h>

#include <CppUnitLite/TestHarness.h>

#include <iostream>
#include <map>

using namespace std;
using namespace gtsam;

string g2oFile = findExampleDataFile("toyExample.g2o");
// string g2oFile = "/Users/dellaert/git/SE-Sync/data/toy3D.g2o";
static const ShonanAveraging kShonan(g2oFile);

/* ************************************************************************* */
TEST(ShonanAveraging, buildGraphAt) {
  auto graph = kShonan.buildGraphAt(5);
  EXPECT_LONGS_EQUAL(5, kShonan.nrPoses());
  EXPECT_LONGS_EQUAL(6, graph.size());
}

/* ************************************************************************* */
TEST(ShonanAveraging, checkOptimalityAt) {
  auto Q = kShonan.buildQ();
  EXPECT_LONGS_EQUAL(3 * 5, Q.rows());
  EXPECT_LONGS_EQUAL(3 * 5, Q.cols());
  const Values values = kShonan.initializeRandomlyAt(4);
  auto Lambda = kShonan.computeLambda(values, Q);
  EXPECT_LONGS_EQUAL(3 * 5, Lambda.rows());
  EXPECT_LONGS_EQUAL(3 * 5, Lambda.cols());
  EXPECT_LONGS_EQUAL(45, Lambda.nonZeros());
}

/* ************************************************************************* */
TEST(ShonanAveraging, tryOptimizingAt3) {
  const Values initial = kShonan.initializeRandomlyAt(3);
  EXPECT(!kShonan.checkOptimalityAt(3, initial));
  const Values result = kShonan.tryOptimizingAt(3, initial);
  EXPECT(kShonan.checkOptimalityAt(3, result));
  EXPECT_DOUBLES_EQUAL(0, kShonan.costAt(3, result), 1e-4);
  const Values SO3Values = kShonan.projectFrom(3, result);
  EXPECT_DOUBLES_EQUAL(0, kShonan.cost(SO3Values), 1e-4);
}

/* ************************************************************************* */
TEST(ShonanAveraging, tryOptimizingAt4) {
  const Values result = kShonan.tryOptimizingAt(4);
  EXPECT(kShonan.checkOptimalityAt(4, result));
  EXPECT_DOUBLES_EQUAL(0, kShonan.costAt(4, result), 1e-3);
  const Values SO3Values = kShonan.projectFrom(4, result);
  EXPECT_DOUBLES_EQUAL(0, kShonan.cost(SO3Values), 1e-4);
}

/* ************************************************************************* */
TEST(ShonanAveraging, tryOptimizingAt5) {
  const Values result = kShonan.tryOptimizingAt(5);
  EXPECT_DOUBLES_EQUAL(0, kShonan.costAt(5, result), 1e-3);
}

/* ************************************************************************* */
TEST(ShonanAveraging, run) { kShonan.run(5); }

/* ************************************************************************* */
int main() {
  TestResult tr;
  return TestRegistry::runAllTests(tr);
}
/* ************************************************************************* */