//
// Copyright (c) 2022 CNRS INRIA
//

#include "pinocchio/math/fwd.hpp"
#include "pinocchio/multibody/joint/joints.hpp"
#include "pinocchio/algorithm/rnea.hpp"
#include "pinocchio/algorithm/aba.hpp"
#include "pinocchio/algorithm/crba.hpp"
#include "pinocchio/algorithm/jacobian.hpp"
#include "pinocchio/algorithm/compute-all-terms.hpp"

#include <boost/test/unit_test.hpp>
#include <iostream>

using namespace pinocchio;

template<typename D>
void addJointAndBody(Model & model,
                     const JointModelBase<D> & jmodel,
                     const Model::JointIndex parent_id,
                     const SE3 & joint_placement,
                     const std::string & joint_name,
                     const Inertia & Y)
{
  Model::JointIndex idx;
  
  idx = model.addJoint(parent_id,jmodel,joint_placement,joint_name);
  model.appendBodyToJoint(idx,Y);
}

BOOST_AUTO_TEST_SUITE(JointHelical)

BOOST_AUTO_TEST_CASE(vsPXRX)
{
  typedef SE3::Vector3 Vector3;
  typedef SE3::Matrix3 Matrix3;

  Model modelHX, modelPXRX;

  Inertia inertia(1., Vector3(0., 0., 0.0), Matrix3::Identity());
  // Important to have the same mass for both systems, otherwise COM position not the same
  Inertia inertia_zero_mass(0., Vector3(0.0, 0.0, 0.0), Matrix3::Identity());
  const double pitch = 0.4;

  JointModelHX joint_model_HX(pitch);
  addJointAndBody(modelHX,joint_model_HX,0,SE3::Identity(),"helical x",inertia);
  
  JointModelPX joint_model_PX;
  JointModelRX joint_model_RX;
  addJointAndBody(modelPXRX,joint_model_PX,0,SE3::Identity(),"prismatic x",inertia);
  addJointAndBody(modelPXRX,joint_model_RX,1,SE3::Identity(),"revolute x",inertia_zero_mass);

  Data dataHX(modelHX);
  Data dataPXRX(modelPXRX);

  // Set the prismatic joint to corresponding displacement, velocit and acceleration
  Eigen::VectorXd q_hx = Eigen::VectorXd::Ones(modelHX.nq);      // dim 1
  Eigen::VectorXd q_PXRX = Eigen::VectorXd::Ones(modelPXRX.nq);  // dim 2
  q_PXRX(0) = q_hx(0) * pitch;  

  Eigen::VectorXd v_hx = Eigen::VectorXd::Ones(modelHX.nv);
  Eigen::VectorXd v_PXRX = Eigen::VectorXd::Ones(modelPXRX.nv);
  v_PXRX(0) = v_hx(0) * pitch;

  Eigen::VectorXd tauHX = Eigen::VectorXd::Ones(modelHX.nv);
  Eigen::VectorXd tauPXRX = Eigen::VectorXd::Ones(modelPXRX.nv);
  Eigen::VectorXd aHX = Eigen::VectorXd::Ones(modelHX.nv);
  Eigen::VectorXd aPXRX = Eigen::VectorXd::Ones(modelPXRX.nv);
  aPXRX(0) = aHX(0) * pitch * pitch;
  
  forwardKinematics(modelHX, dataHX, q_hx, v_hx);
  forwardKinematics(modelPXRX, dataPXRX, q_PXRX, v_PXRX);

  computeAllTerms(modelHX, dataHX, q_hx, v_hx);
  computeAllTerms(modelPXRX, dataPXRX, q_PXRX, v_PXRX);

  BOOST_CHECK(dataPXRX.oMi[2].isApprox(dataHX.oMi[1]));  // Body absolute placement (wrt world)
  BOOST_CHECK((dataPXRX.liMi[2]*dataPXRX.liMi[1]).isApprox(dataHX.liMi[1]));  // Body relative placement (wrt parent) 
  BOOST_CHECK(dataPXRX.Ycrb[2].matrix().isApprox(dataHX.Ycrb[1].matrix()));  // Inertia of the sub-tree composit rigid body
  BOOST_CHECK((dataPXRX.liMi[2].actInv(dataPXRX.f[1])).toVector().isApprox(dataHX.f[1].toVector()));  // Vector of body forces expressed in the local frame of the joint    
  BOOST_CHECK(dataPXRX.nle.isApprox(dataHX.nle));  // Non Linear Effects (output of nle algorithm)
  BOOST_CHECK(dataPXRX.com[0].isApprox(dataHX.com[0]));  // CoM position of the subtree starting at joint index i.

  // InverseDynamics == rnea
  std::cout << " ------ rnea ------- HX -------" << std::endl;
  tauHX = rnea(modelHX, dataHX, q_hx, v_hx, aHX);
  std::cout << " ------ rnea ------- PXRX -------" << std::endl;
  tauPXRX = rnea(modelPXRX, dataPXRX, q_PXRX, v_PXRX, aPXRX);
  BOOST_CHECK(tauHX.isApprox(Eigen::Matrix<double,1,1>(tauPXRX.dot(Eigen::VectorXd::Ones(2)))));

  std::cout << "tauHX : " << tauHX << std::endl;
  std::cout << "tauPXRX : " << tauPXRX.transpose() << std::endl;


}
  
BOOST_AUTO_TEST_CASE(spatial)
{
  typedef TransformHelicalTpl<double,0,0> TransformX;
  typedef TransformHelicalTpl<double,0,1> TransformY;
  typedef TransformHelicalTpl<double,0,2> TransformZ;

  typedef TransformPrismaticTpl<double,0,0> TransformPX;
  typedef TransformRevoluteTpl<double,0,0> TransformRX;

  
  typedef SE3::Vector3 Vector3;
  
  const double alpha = 0.2, pitch = 0.1;
  double sin_alpha, cos_alpha; SINCOS(alpha,&sin_alpha,&cos_alpha);
  SE3 Mplain, Mrand(SE3::Random());
  
  // TODO Alpha is not necessary, it can be reconstrcuted, but where to put pitch if not here as input ?
  TransformX Mx(sin_alpha,cos_alpha,alpha,pitch);
  Mplain = Mx;
  BOOST_CHECK(Mplain.translation().isApprox(Vector3::UnitX()*alpha*pitch));
  BOOST_CHECK(Mplain.rotation().isApprox(Eigen::AngleAxisd(alpha,Vector3::UnitX()).toRotationMatrix()));
  BOOST_CHECK((Mrand*Mplain).isApprox(Mrand*Mx));

  // TODO: Test against combinatino of revolute and prismatic joint
  // TransformPX MPx(alpha*pitch);
  // TransformRX MRx(sin_alpha,cos_alpha);
  // // auto MPXRX = MPx * MRx;

  // BOOST_CHECK(Mplain.translation().isApprox(MPx.translation()+MRx.translation()));
  // BOOST_CHECK(Mplain.rotation().isApprox(MPx.rotation() * MRx.rotation()));
  // BOOST_CHECK((Mrand*Mplain).isApprox(Mrand*MPx*MRx));
  
  TransformY My(sin_alpha,cos_alpha,alpha,pitch);
  Mplain = My;
  BOOST_CHECK(Mplain.translation().isApprox(Vector3::UnitY()*alpha*pitch));
  BOOST_CHECK(Mplain.rotation().isApprox(Eigen::AngleAxisd(alpha,Vector3::UnitY()).toRotationMatrix()));
  BOOST_CHECK((Mrand*Mplain).isApprox(Mrand*My));
  
  TransformZ Mz(sin_alpha,cos_alpha,alpha,pitch);
  Mplain = Mz;
  BOOST_CHECK(Mplain.translation().isApprox(Vector3::UnitZ()*alpha*pitch));
  BOOST_CHECK(Mplain.rotation().isApprox(Eigen::AngleAxisd(alpha,Vector3::UnitZ()).toRotationMatrix()));
  BOOST_CHECK((Mrand*Mplain).isApprox(Mrand*Mz));
  
  SE3 M(SE3::Random());
  Motion v(Motion::Random());
  
  MotionHelicalTpl<double,0,0> mh_x(2., pitch);
  Motion mh_dense_x(mh_x);
  
  BOOST_CHECK(M.act(mh_x).isApprox(M.act(mh_dense_x)));
  BOOST_CHECK(M.actInv(mh_x).isApprox(M.actInv(mh_dense_x)));
  
  BOOST_CHECK(v.cross(mh_x).isApprox(v.cross(mh_dense_x)));
  
  MotionHelicalTpl<double,0,1> mh_y(2., pitch);
  Motion mh_dense_y(mh_y);
  
  BOOST_CHECK(M.act(mh_y).isApprox(M.act(mh_dense_y)));
  BOOST_CHECK(M.actInv(mh_y).isApprox(M.actInv(mh_dense_y)));
  
  BOOST_CHECK(v.cross(mh_y).isApprox(v.cross(mh_dense_y)));
  
  MotionHelicalTpl<double,0,2> mh_z(2., pitch);
  Motion mh_dense_z(mh_z);
  
  BOOST_CHECK(M.act(mh_z).isApprox(M.act(mh_dense_z)));
  BOOST_CHECK(M.actInv(mh_z).isApprox(M.actInv(mh_dense_z)));
  
  BOOST_CHECK(v.cross(mh_z).isApprox(v.cross(mh_dense_z)));

}

BOOST_AUTO_TEST_SUITE_END()