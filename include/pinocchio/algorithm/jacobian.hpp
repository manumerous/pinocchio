//
// Copyright (c) 2015-2020 CNRS INRIA
//

#ifndef __pinocchio_algorithm_jacobian_hpp__
#define __pinocchio_algorithm_jacobian_hpp__

#include "pinocchio/multibody/model.hpp"
#include "pinocchio/multibody/data.hpp"

namespace pinocchio
{
  ///
  /// \brief Computes the full model Jacobian, i.e. the stack of all motion subspace expressed in
  /// the world frame.
  ///        The result is accessible through data.J. This function computes also the
  ///        forwardKinematics of the model.
  ///
  /// \note This Jacobian does not correspond to any specific joint frame Jacobian. From this
  /// Jacobian, it is then possible to easily extract the Jacobian of a specific joint frame. \sa
  /// pinocchio::getJointJacobian for doing this specific extraction.
  ///
  /// \tparam JointCollection Collection of Joint types.
  /// \tparam ConfigVectorType Type of the joint configuration vector.
  ///
  /// \param[in] model The model structure of the rigid body system.
  /// \param[in] data The data structure of the rigid body system.
  /// \param[in] q The joint configuration vector (dim model.nq).
  ///
  /// \return The full model Jacobian (matrix 6 x model.nv).
  ///
  template<
    typename Scalar,
    int Options,
    template<typename, int> class JointCollectionTpl,
    typename ConfigVectorType>
  const typename DataTpl<Scalar, Options, JointCollectionTpl>::Matrix6x & computeJointJacobians(
    const ModelTpl<Scalar, Options, JointCollectionTpl> & model,
    DataTpl<Scalar, Options, JointCollectionTpl> & data,
    const Eigen::MatrixBase<ConfigVectorType> & q);

  ///
  /// \brief Computes the full model Jacobian, i.e. the stack of all motion subspace expressed in
  /// the world frame.
  ///        The result is accessible through data.J. This function assumes that
  ///        pinocchio::forwardKinematics has been called before.
  ///
  /// \note This Jacobian does not correspond to any specific joint frame Jacobian. From this
  /// Jacobian, it is then possible to easily extract the Jacobian of a specific joint frame. \sa
  /// pinocchio::getJointJacobian for doing this specific extraction.
  ///
  /// \tparam JointCollection Collection of Joint types.
  ///
  /// \param[in] model The model structure of the rigid body system.
  /// \param[in] data The data structure of the rigid body system.
  ///
  /// \return The full model Jacobian (matrix 6 x model.nv).
  ///
  template<typename Scalar, int Options, template<typename, int> class JointCollectionTpl>
  const typename DataTpl<Scalar, Options, JointCollectionTpl>::Matrix6x & computeJointJacobians(
    const ModelTpl<Scalar, Options, JointCollectionTpl> & model,
    DataTpl<Scalar, Options, JointCollectionTpl> & data);

  /// \brief Computes the Jacobian of a specific joint frame expressed in one of the
  /// pinocchio::ReferenceFrame options.
  ///
  /// \details For the LOCAL reference frame, the Jacobian \f${}^j J_{0j}\f$ from the joint frame
  /// \f$j\f$ to the world frame \f$0\f$ is such that \f${}^j v_{0j} = {}^j J_{0j} \dot{q}\f$, where
  /// \f${}^j v_{0j}\f$ is the velocity of the origin of the moving joint frame relative to the
  /// fixed world frame, projected into the basis of the joint frame. LOCAL_WORLD_ALIGNED is the
  /// same velocity but projected into the world frame basis.
  ///
  /// For the WORLD reference frame, the Jacobian \f${}^0 J_{0j}\f$ from the joint frame \f$j\f$ to
  /// the world frame \f$0\f$ is such that \f${}^0 v_{0j} = {}^0 J_{0j} \dot{q}\f$, where \f${}^0
  /// v_{0j}\f$ is the spatial velocity of the joint frame. The linear component of this spatial
  /// velocity is the velocity of a (possibly imaginary) point attached to the moving joint frame j
  /// which is traveling through the origin of the world frame at that instant. The angular
  /// component is the instantaneous angular velocity of the joint frame as viewed in the world
  /// frame.
  ///
  /// When serialized to a 6D vector, the order of coordinates is: three linear followed by three
  /// angular.
  ///
  /// For further details regarding the different velocities or the Jacobian see Chapters 2 and 3
  /// respectively in [A Mathematical Introduction to Robotic
  /// Manipulation](https://www.cse.lehigh.edu/~trink/Courses/RoboticsII/reading/murray-li-sastry-94-complete.pdf)
  /// by Murray, Li and Sastry.
  ///
  /// \note This jacobian is extracted from data.J. You have to run pinocchio::computeJointJacobians
  /// before calling it.
  ///
  /// \tparam JointCollection Collection of Joint types.
  /// \tparam Matrix6xLike Type of the matrix containing the joint Jacobian.
  ///
  /// \param[in] model The model structure of the rigid body system.
  /// \param[in] data The data structure of the rigid body system.
  /// \param[in] joint_id The id of the joint.
  /// \param[in] reference_frame Reference frame in which the result is expressed.
  /// \param[out] J A reference on the Jacobian matrix where the results will be stored in (dim 6 x
  /// model.nv). You must fill J with zero elements, e.g. J.fill(0.).
  ///
  template<
    typename Scalar,
    int Options,
    template<typename, int> class JointCollectionTpl,
    typename Matrix6Like>
  void getJointJacobian(
    const ModelTpl<Scalar, Options, JointCollectionTpl> & model,
    const DataTpl<Scalar, Options, JointCollectionTpl> & data,
    const JointIndex joint_id,
    const ReferenceFrame reference_frame,
    const Eigen::MatrixBase<Matrix6Like> & J);
  ///
  /// \brief Computes the Jacobian of a specific joint frame expressed either in the world (rf =
  /// WORLD) frame, in the local world aligned (rf = LOCAL_WORLD_ALIGNED) frame or in the local
  /// frame (rf = LOCAL) of the joint. \note This jacobian is extracted from data.J. You have to run
  /// pinocchio::computeJointJacobians before calling it.
  ///
  /// \tparam JointCollection Collection of Joint types.
  ///
  /// \param[in] model The model structure of the rigid body system.
  /// \param[in] data The data structure of the rigid body system.
  /// \param[in] joint_id The index of the joint.
  /// \param[in] reference_frame Reference frame in which the result is expressed.
  ///
  template<typename Scalar, int Options, template<typename, int> class JointCollectionTpl>
  Eigen::Matrix<Scalar, 6, Eigen::Dynamic, Options> getJointJacobian(
    const ModelTpl<Scalar, Options, JointCollectionTpl> & model,
    const DataTpl<Scalar, Options, JointCollectionTpl> & data,
    const JointIndex joint_id,
    const ReferenceFrame reference_frame)
  {
    typedef Eigen::Matrix<Scalar, 6, Eigen::Dynamic, Options> ReturnType;
    ReturnType res(ReturnType::Zero(6, model.nv));

    getJointJacobian(model, data, joint_id, reference_frame, res);
    return res;
  }

  ///
  /// \brief Computes the Jacobian of a specific joint frame expressed in the local frame of the
  /// joint and store the result in the input argument J.
  ///
  /// \tparam JointCollection Collection of Joint types.
  /// \tparam ConfigVectorType Type of the joint configuration vector.
  /// \tparam Matrix6xLike Type of the matrix containing the joint Jacobian.
  ///
  /// \param[in] model The model structure of the rigid body system.
  /// \param[in] data The data structure of the rigid body system.
  /// \param[in] q The joint configuration vector (dim model.nq).
  /// \param[in] joint_id The id of the joint refering to model.joints[jointId].
  /// \param[out] J A reference on the Jacobian matrix where the results will be stored in (dim 6 x
  /// model.nv). You must fill J with zero elements, e.g. J.setZero().
  ///
  /// \return The Jacobian of the specific joint frame expressed in the local frame of the joint
  /// (matrix 6 x model.nv).
  ///
  /// \remarks The result of this function is equivalent to call first
  /// computeJointJacobians(model,data,q) and then call
  /// getJointJacobian(model,data,jointId,LOCAL,J),
  ///         but forwardKinematics is not fully computed.
  ///         It is worth to call jacobian if you only need a single Jacobian for a specific joint.
  ///         Otherwise, for several Jacobians, it is better to call
  ///         computeJointJacobians(model,data,q) followed by
  ///         getJointJacobian(model,data,jointId,LOCAL,J) for each Jacobian.
  ///
  template<
    typename Scalar,
    int Options,
    template<typename, int> class JointCollectionTpl,
    typename ConfigVectorType,
    typename Matrix6Like>
  void computeJointJacobian(
    const ModelTpl<Scalar, Options, JointCollectionTpl> & model,
    DataTpl<Scalar, Options, JointCollectionTpl> & data,
    const Eigen::MatrixBase<ConfigVectorType> & q,
    const JointIndex joint_id,
    const Eigen::MatrixBase<Matrix6Like> & J);

  ///
  /// \brief Computes the full model Jacobian variations with respect to time. It corresponds to
  /// dJ/dt which depends both on q and v.
  ///        The result is accessible through data.dJ.
  ///
  /// \tparam JointCollection Collection of Joint types.
  /// \tparam ConfigVectorType Type of the joint configuration vector.
  /// \tparam TangentVectorType Type of the joint velocity vector.
  ///
  /// \param[in] model The model structure of the rigid body system.
  /// \param[in] data The data structure of the rigid body system.
  /// \param[in] q The joint configuration vector (dim model.nq).
  /// \param[in] v The joint velocity vector (dim model.nv).
  ///
  /// \return The full model Jacobian (matrix 6 x model.nv).
  ///
  template<
    typename Scalar,
    int Options,
    template<typename, int> class JointCollectionTpl,
    typename ConfigVectorType,
    typename TangentVectorType>
  const typename DataTpl<Scalar, Options, JointCollectionTpl>::Matrix6x &
  computeJointJacobiansTimeVariation(
    const ModelTpl<Scalar, Options, JointCollectionTpl> & model,
    DataTpl<Scalar, Options, JointCollectionTpl> & data,
    const Eigen::MatrixBase<ConfigVectorType> & q,
    const Eigen::MatrixBase<TangentVectorType> & v);

  ///
  /// \brief Computes the Jacobian time variation of a specific joint frame expressed either in the
  /// world frame (rf = WORLD), in the local world aligned (rf = LOCAL_WORLD_ALIGNED) frame or in
  /// the local frame (rf = LOCAL) of the joint. \note This jacobian is extracted from data.dJ. You
  /// have to run pinocchio::computeJointJacobiansTimeVariation before calling it.
  ///
  /// \tparam JointCollection Collection of Joint types.
  /// \tparam Matrix6xLike Type of the matrix containing the joint Jacobian.
  ///
  /// \param[in]  model The model structure of the rigid body system.
  /// \param[in]  data The data structure of the rigid body system.
  /// \param[in]  joint_id The id of the joint.
  /// \param[in]  reference_frame Reference frame in which the result is expressed.
  /// \param[out] dJ A reference on the Jacobian matrix where the results will be stored in (dim 6 x
  /// model.nv). You must fill dJ with zero elements, e.g. dJ.fill(0.).
  ///
  template<
    typename Scalar,
    int Options,
    template<typename, int> class JointCollectionTpl,
    typename Matrix6Like>
  void getJointJacobianTimeVariation(
    const ModelTpl<Scalar, Options, JointCollectionTpl> & model,
    const DataTpl<Scalar, Options, JointCollectionTpl> & data,
    const JointIndex joint_id,
    const ReferenceFrame reference_frame,
    const Eigen::MatrixBase<Matrix6Like> & dJ);

} // namespace pinocchio

/* --- Details -------------------------------------------------------------------- */
/* --- Details -------------------------------------------------------------------- */
/* --- Details -------------------------------------------------------------------- */

#include "pinocchio/algorithm/jacobian.hxx"

#if PINOCCHIO_ENABLE_TEMPLATE_INSTANTIATION
  #include "pinocchio/algorithm/jacobian.txx"
#endif // PINOCCHIO_ENABLE_TEMPLATE_INSTANTIATION

#endif // ifndef __pinocchio_algorithm_jacobian_hpp__
