/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file testBAD.cpp
 * @date September 18, 2014
 * @author Frank Dellaert
 * @author Paul Furgale
 * @brief unit tests for Block Automatic Differentiation
 */

#include <gtsam_unstable/base/Expression.h>
#include <CppUnitLite/TestHarness.h>

using namespace std;
using namespace gtsam;

/* ************************************************************************* */

Point3 transformTo(const Pose3& x, const Point3& p,
    boost::optional<Matrix&> Dpose, boost::optional<Matrix&> Dpoint) {
  return x.transform_to(p, Dpose, Dpoint);
}

Point2 project(const Point3& p, boost::optional<Matrix&> Dpoint) {
  return PinholeCamera<Cal3_S2>::project_to_camera(p, Dpoint);
}

template<class CAL>
Point2 uncalibrate(const CAL& K, const Point2& p, boost::optional<Matrix&> Dcal,
    boost::optional<Matrix&> Dp) {
  return K.uncalibrate(p, Dcal, Dp);
}

/* ************************************************************************* */

TEST(BAD, test) {

  // Create some values
  Values values;
  values.insert(1, Pose3());
  values.insert(2, Point3(0, 0, 1));
  values.insert(3, Cal3_S2());

  // Create old-style factor to create expected value and derivatives
  Point2 measured(-17, 30);
  SharedNoiseModel model = noiseModel::Unit::Create(2);
  GeneralSFMFactor2<Cal3_S2> old(measured, model, 1, 2, 3);
  double expected_error = old.error(values);
  GaussianFactor::shared_ptr expected = old.linearize(values);

  // Test Constant expression
  Expression<int> c(0);

  // Create leaves
  Expression<Pose3> x(1);
  Expression<Point3> p(2);
  Expression<Cal3_S2> K(3);

  // Create expression tree
  Expression<Point3> p_cam(transformTo, x, p);
  Expression<Point2> projection(project, p_cam);
  Expression<Point2> uv_hat(uncalibrate<Cal3_S2>, K, projection);

  // Check keys
  std::set<Key> expectedKeys;
  expectedKeys.insert(1);
  expectedKeys.insert(2);
  expectedKeys.insert(3);
  EXPECT(expectedKeys == uv_hat.keys());

  // Create factor
  BADFactor<Point2> f(measured, uv_hat);

  // Check value
  EXPECT_DOUBLES_EQUAL(expected_error, f.error(values), 1e-9);

  // Check dimension
  EXPECT_LONGS_EQUAL(0, f.dim());

  // Check linearization
  boost::shared_ptr<GaussianFactor> gf = f.linearize(values);
  EXPECT( assert_equal(*expected, *gf, 1e-9));
}

/* ************************************************************************* */

TEST(BAD, compose) {

  // Create expression
  Expression<Rot3> R1(1), R2(2);
  Expression<Rot3> R3 = R1 * R2;

  // Create factor
  BADFactor<Rot3> f(Rot3(), R3);

  // Create some values
  Values values;
  values.insert(1, Rot3());
  values.insert(2, Rot3());

  // Check linearization
  JacobianFactor expected(1, eye(3), 2, eye(3), zero(3));
  boost::shared_ptr<GaussianFactor> gf = f.linearize(values);
  boost::shared_ptr<JacobianFactor> jf = //
      boost::dynamic_pointer_cast<JacobianFactor>(gf);
  EXPECT( assert_equal(expected, *jf,1e-9));
}

/* ************************************************************************* */
// Test compose with arguments referring to the same rotation
TEST(BAD, compose2) {

  // Create expression
  Expression<Rot3> R1(1), R2(1);
  Expression<Rot3> R3 = R1 * R2;

  // Create factor
  BADFactor<Rot3> f(Rot3(), R3);

  // Create some values
  Values values;
  values.insert(1, Rot3());

  // Check linearization
  JacobianFactor expected(1, 2*eye(3), zero(3));
  boost::shared_ptr<GaussianFactor> gf = f.linearize(values);
  boost::shared_ptr<JacobianFactor> jf = //
      boost::dynamic_pointer_cast<JacobianFactor>(gf);
  EXPECT( assert_equal(expected, *jf,1e-9));
}

/* ************************************************************************* */
int main() {
  TestResult tr;
  return TestRegistry::runAllTests(tr);
}
/* ************************************************************************* */
